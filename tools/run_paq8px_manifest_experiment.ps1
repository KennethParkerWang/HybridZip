[CmdletBinding()]
param(
    [string]$ManifestPath = '',
    [string]$PaqPath = 'F:\paq8px\experiment\build\paq8px.exe',
    [string]$OutputRoot = '',
    [string]$ExperimentId = '',
    [string[]]$CaseId = @(),
    [ValidateRange(1, 604800)]
    [int]$ProcessTimeoutSeconds = 600,
    [ValidateRange(1, 16384)]
    [int]$MemoryLimitMiB = 4096,
    [switch]$AuthorizeRuntimeExperiment,
    [switch]$ListOnly
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

$repoRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..'))
if ([string]::IsNullOrWhiteSpace($ManifestPath)) {
    $ManifestPath = Join-Path $repoRoot 'bench\manifests\silesia-leading-32-64-128.tsv'
}
if ([string]::IsNullOrWhiteSpace($OutputRoot)) {
    $OutputRoot = Join-Path $repoRoot 'results\experiments'
}
$ManifestPath = [IO.Path]::GetFullPath($ManifestPath)
$PaqPath = [IO.Path]::GetFullPath($PaqPath)
$OutputRoot = [IO.Path]::GetFullPath($OutputRoot)

$canonicalFiles = @(
    'dickens', 'mozilla', 'mr', 'nci', 'ooffice', 'osdb',
    'reymont', 'samba', 'sao', 'webster', 'x-ray', 'xml'
)
$canonicalScopes = @(32, 64, 128)
$resultColumns = @(
    'experiment_id', 'variant', 'repeat', 'case_order', 'file', 'scope_kib',
    'input_path', 'input_bytes', 'input_sha256', 'archive_path', 'archive_bytes',
    'archive_sha256', 'decoded_path', 'decoded_bytes', 'decoded_sha256',
    'encode_seconds', 'decode_seconds', 'encode_peak_ram_mib',
    'decode_peak_ram_mib', 'peak_ram_mib', 'codec_sha256', 'parameters',
    'encode_command', 'decode_command', 'encode_exit_code', 'decode_exit_code',
    'started_at', 'status', 'roundtrip', 'block_types', 'notes'
)
$zeroHash = '0' * 64

function Get-Sha256OrZero([string]$Path) {
    if (-not (Test-Path -LiteralPath $Path -PathType Leaf)) {
        return $zeroHash
    }
    return (Get-FileHash -LiteralPath $Path -Algorithm SHA256).Hash.ToUpperInvariant()
}

function Get-FileLengthOrZero([string]$Path) {
    if (-not (Test-Path -LiteralPath $Path -PathType Leaf)) {
        return [Int64]0
    }
    return [Int64](Get-Item -LiteralPath $Path).Length
}

function Write-Utf8NoBom([string]$Path, [string]$Content) {
    [IO.File]::WriteAllText($Path, $Content, (New-Object Text.UTF8Encoding($false)))
}

function Write-ResultsCsv([string]$Path, [object[]]$Rows) {
    $csv = if ($Rows.Count -eq 0) {
        ($resultColumns -join ',') + "`r`n"
    }
    else {
        (($Rows | Select-Object $resultColumns | ConvertTo-Csv -NoTypeInformation) -join "`r`n") + "`r`n"
    }
    Write-Utf8NoBom -Path $Path -Content $csv
}

function ConvertTo-ProcessArgumentText([string[]]$Arguments) {
    $parts = foreach ($argument in $Arguments) {
        if ($argument -match '[\s"]') {
            '"' + ($argument -replace '"', '\"') + '"'
        }
        else {
            $argument
        }
    }
    return [string]::Join(' ', $parts)
}

function Format-RecordedCommand([string]$Executable, [string[]]$Arguments) {
    return '"' + $Executable + '" ' + (ConvertTo-ProcessArgumentText -Arguments $Arguments)
}

function Invoke-MeasuredPaq([string]$Executable, [string[]]$Arguments,
                             [string]$LogBase, [int]$TimeoutSeconds,
                             [Int64]$MemoryLimitBytes) {
    $stdoutPath = $LogBase + '.stdout.log'
    $stderrPath = $LogBase + '.stderr.log'
    if ((Test-Path -LiteralPath $stdoutPath) -or (Test-Path -LiteralPath $stderrPath)) {
        throw "Refusing to overwrite PAQ log artifact: $LogBase"
    }

    $startInfo = New-Object Diagnostics.ProcessStartInfo
    $startInfo.FileName = $Executable
    $startInfo.Arguments = ConvertTo-ProcessArgumentText -Arguments $Arguments
    $startInfo.WorkingDirectory = Split-Path -Parent $Executable
    $startInfo.UseShellExecute = $false
    $startInfo.CreateNoWindow = $true
    $startInfo.RedirectStandardOutput = $true
    $startInfo.RedirectStandardError = $true
    $process = New-Object Diagnostics.Process
    $process.StartInfo = $startInfo
    $watch = [Diagnostics.Stopwatch]::StartNew()
    [void]$process.Start()
    $stdoutTask = $process.StandardOutput.ReadToEndAsync()
    $stderrTask = $process.StandardError.ReadToEndAsync()
    [Int64]$peakBytes = 0
    $termination = ''

    while (-not $process.HasExited) {
        try {
            $process.Refresh()
            $peakBytes = [Math]::Max($peakBytes, [Int64]$process.PeakWorkingSet64)
        }
        catch {
            # The process can exit between HasExited and Refresh.
        }
        if ($peakBytes -gt $MemoryLimitBytes) {
            $termination = 'MEMORY_LIMIT'
            $process.Kill()
            break
        }
        if ($watch.Elapsed.TotalSeconds -ge $TimeoutSeconds) {
            $termination = 'TIMEOUT'
            $process.Kill()
            break
        }
        Start-Sleep -Milliseconds 25
    }
    $process.WaitForExit()
    $watch.Stop()
    try {
        $process.Refresh()
        $peakBytes = [Math]::Max($peakBytes, [Int64]$process.PeakWorkingSet64)
    }
    catch {
    }

    Write-Utf8NoBom -Path $stdoutPath -Content $stdoutTask.Result
    Write-Utf8NoBom -Path $stderrPath -Content $stderrTask.Result
    $exitCode = if ([string]::IsNullOrWhiteSpace($termination)) {
        [int]$process.ExitCode
    }
    else {
        -2
    }
    $process.Dispose()
    return [pscustomobject]@{
        exit_code = $exitCode
        termination = $termination
        seconds = $watch.Elapsed.TotalSeconds
        peak_mib = $peakBytes / 1MB
        stdout_path = $stdoutPath
        stderr_path = $stderrPath
    }
}

function Write-LeadingPrefix([string]$SourcePath, [string]$DestinationPath,
                             [Int64]$PrefixBytes) {
    if (Test-Path -LiteralPath $DestinationPath) {
        throw "Refusing to overwrite input artifact: $DestinationPath"
    }
    $input = [IO.File]::Open($SourcePath, [IO.FileMode]::Open, [IO.FileAccess]::Read,
        [IO.FileShare]::Read)
    try {
        if ($input.Length -lt $PrefixBytes) {
            throw "Source is shorter than declared prefix: $SourcePath"
        }
        $buffer = New-Object byte[] $PrefixBytes
        $offset = 0
        while ($offset -lt $PrefixBytes) {
            $read = $input.Read($buffer, $offset, $PrefixBytes - $offset)
            if ($read -eq 0) {
                throw "Unexpected end of source while reading prefix: $SourcePath"
            }
            $offset += $read
        }
        [IO.File]::WriteAllBytes($DestinationPath, $buffer)
    }
    finally {
        $input.Dispose()
    }
}

function Get-RelativeArtifactPath([string]$Category, [int]$ScopeKiB,
                                  [string]$FileName) {
    return ('{0}/{1}KiB/{2}' -f $Category, $ScopeKiB, $FileName)
}

function Get-CurrentEnvironmentDescription {
    $cpu = 'unknown'
    try {
        $value = (Get-CimInstance Win32_Processor | Select-Object -First 1).Name
        if (-not [string]::IsNullOrWhiteSpace($value)) {
            $cpu = $value
        }
    }
    catch {
    }
    return "Windows $([Environment]::OSVersion.Version); CPU=$cpu; sequential execution"
}

function Assert-Manifest([object[]]$Rows) {
    if ($Rows.Count -ne 36) {
        throw "Manifest has $($Rows.Count) rows; expected 36"
    }
    $expectedOrder = 0
    $seen = New-Object 'System.Collections.Generic.HashSet[string]' ([StringComparer]::Ordinal)
    foreach ($file in $canonicalFiles) {
        foreach ($scopeKiB in $canonicalScopes) {
            $expectedOrder++
            $row = $Rows[$expectedOrder - 1]
            $expectedId = '{0}-leading-{1}k' -f $file, $scopeKiB
            if ([string]$row.manifest_version -cne '1' -or
                [int]$row.case_order -ne $expectedOrder -or
                [string]$row.case_id -cne $expectedId -or
                [string]$row.file -cne $file -or
                [int]$row.scope_kib -ne $scopeKiB -or
                [Int64]$row.prefix_bytes -ne ([Int64]$scopeKiB * 1024) -or
                [string]$row.input_policy -cne 'leading-prefix-v1') {
                throw "Manifest row $expectedOrder does not match the canonical Silesia leading-prefix matrix"
            }
            if (-not $seen.Add([string]$row.case_id)) {
                throw "Manifest has a duplicate case_id: $($row.case_id)"
            }
            foreach ($hashField in @('source_sha256', 'prefix_sha256')) {
                if ([string]$row.$hashField -cnotmatch '^[0-9A-Fa-f]{64}$') {
                    throw "Manifest has an invalid ${hashField} for $($row.case_id)"
                }
            }
            if ([string]::IsNullOrWhiteSpace([string]$row.source_path) -or
                [Int64]$row.source_bytes -lt [Int64]$row.prefix_bytes) {
                throw "Manifest has an invalid source identity for $($row.case_id)"
            }
        }
    }
}

if (-not (Test-Path -LiteralPath $ManifestPath -PathType Leaf)) {
    throw "Frozen prefix manifest not found: $ManifestPath"
}
$manifestRows = @(Import-Csv -LiteralPath $ManifestPath -Delimiter "`t")
Assert-Manifest -Rows $manifestRows

$requestedCaseIds = @($CaseId | ForEach-Object { ([string]$_).Split(',') } |
    Where-Object { -not [string]::IsNullOrWhiteSpace([string]$_) })
$selectedRows = @()
if ($requestedCaseIds.Count -eq 0) {
    $selectedRows = @($manifestRows)
}
else {
    $requested = New-Object 'System.Collections.Generic.HashSet[string]' ([StringComparer]::Ordinal)
    foreach ($id in $requestedCaseIds) {
        if (-not $requested.Add($id)) {
            throw "Duplicate -CaseId value: $id"
        }
    }
    $selectedRows = @($manifestRows | Where-Object { $requested.Contains([string]$_.case_id) })
    if ($selectedRows.Count -ne $requested.Count) {
        $known = @($selectedRows | ForEach-Object case_id)
        $missing = @($requested | Where-Object { $known -notcontains $_ })
        throw ('Unknown -CaseId value: ' + [string]::Join(',', $missing))
    }
}

$paqHash = if (Test-Path -LiteralPath $PaqPath -PathType Leaf) {
    Get-Sha256OrZero $PaqPath
}
else {
    ''
}
if ($ListOnly) {
    [pscustomobject][ordered]@{
        manifest_path = $ManifestPath
        manifest_rows = $manifestRows.Count
        selected_case_count = $selectedRows.Count
        selected_case_ids = @($selectedRows | ForEach-Object case_id)
        paq_path = $PaqPath
        paq_sha256 = $paqHash
        paq_arguments_encode = @('-1', '<input>', '<archive>')
        paq_arguments_decode = @('-d', '<archive>', '<decoded>')
        output_root = $OutputRoot
        runtime_started = $false
    } | ConvertTo-Json -Depth 4
    return
}

if (-not $AuthorizeRuntimeExperiment) {
    throw 'Refusing runtime execution. Re-run with -AuthorizeRuntimeExperiment after reviewing the frozen manifest and output path.'
}
if (-not (Test-Path -LiteralPath $PaqPath -PathType Leaf)) {
    throw "PAQ8px executable not found: $PaqPath"
}
if ([string]::IsNullOrWhiteSpace($ExperimentId)) {
    $experimentSuffix = [Guid]::NewGuid().ToString('N').Substring(0, 8)
    $ExperimentId = 'paq8px-v216-level1-silesia-leading-{0}-{1}' -f
        [DateTime]::UtcNow.ToString('yyyyMMdd-HHmmss'), $experimentSuffix
}
if ($ExperimentId -cnotmatch '^[a-z0-9-]+$') {
    throw "ExperimentId must match ^[a-z0-9-]+`$: $ExperimentId"
}
$packagePath = Join-Path $OutputRoot $ExperimentId
if (Test-Path -LiteralPath $packagePath) {
    throw "Refusing to overwrite existing output package: $packagePath"
}

foreach ($row in $selectedRows) {
    $sourcePath = [IO.Path]::GetFullPath([string]$row.source_path)
    if (-not (Test-Path -LiteralPath $sourcePath -PathType Leaf)) {
        throw "Source listed by manifest does not exist: $sourcePath"
    }
    $source = Get-Item -LiteralPath $sourcePath
    if ([Int64]$source.Length -ne [Int64]$row.source_bytes) {
        throw "Source size differs from frozen manifest: $sourcePath"
    }
    if ((Get-Sha256OrZero $sourcePath) -cne [string]$row.source_sha256) {
        throw "Source SHA-256 differs from frozen manifest: $sourcePath"
    }
}

New-Item -ItemType Directory -Path $packagePath | Out-Null
foreach ($category in @('inputs', 'archives', 'decoded', 'logs')) {
    foreach ($scopeKiB in $canonicalScopes) {
        New-Item -ItemType Directory -Path (Join-Path $packagePath ("$category\${scopeKiB}KiB")) | Out-Null
    }
}
Copy-Item -LiteralPath $ManifestPath -Destination (Join-Path $packagePath 'manifest.tsv')

$environment = Get-CurrentEnvironmentDescription
$experiment = [ordered]@{
    schema_version = 1
    experiment_id = $ExperimentId
    name = 'PAQ8px v216 -1 Silesia leading-prefix baseline'
    state = '测试中'
    description = 'PAQ8px v216 level -1 on the frozen Silesia leading-prefix matrix.'
    model_structure = 'PAQ8px context mixing and arithmetic coding with automatic detection.'
    change_from_baseline = 'Creates the same-input PAQ8px reference required for R2 comparison.'
    hypothesis = 'This package establishes a valid same-input reference; it does not by itself prove HybridZip superiority.'
    baseline_description = 'HybridZip R2 current-hash leading-prefix ledger uses the same Silesia files and prefix policy at 32 KiB.'
    research_line = 'HybridZip R2 fair baseline'
    created_at = [DateTimeOffset]::Now.ToString('o')
    dataset_name = 'Silesia'
    dataset_path = 'F:\paq8px\silesia'
    files = $canonicalFiles
    scopes_kib = $canonicalScopes
    variants = @('level-1')
    repeat_count = 1
    codec_name = 'PAQ8px'
    codec_version = 'v216'
    codec_path = $PaqPath
    codec_sha256 = $paqHash
    source_revision = 'local v216 Release snapshot; source Git metadata unavailable'
    command_template = 'paq8px.exe -1 <input> <archive>; paq8px.exe -d <archive> <decoded>'
    configuration = 'level=-1; automatic-detection=on; sequential; prefix-policy=leading-prefix-v1'
    environment = $environment
    notes = 'Complete archive bytes are recorded. Selected cases run now; unselected canonical rows remain PENDING in this partial package.'
}
Write-Utf8NoBom -Path (Join-Path $packagePath 'experiment.json') -Content ($experiment | ConvertTo-Json -Depth 5)

$rowsByCaseId = @{}
foreach ($row in $manifestRows) {
    $scopeKiB = [int]$row.scope_kib
    $inputRelative = Get-RelativeArtifactPath -Category 'inputs' -ScopeKiB $scopeKiB -FileName ($row.file + '.bin')
    $archiveRelative = Get-RelativeArtifactPath -Category 'archives' -ScopeKiB $scopeKiB -FileName ($row.file + '.paq8px216')
    $decodedRelative = Get-RelativeArtifactPath -Category 'decoded' -ScopeKiB $scopeKiB -FileName ($row.file + '.decoded')
    $rowsByCaseId[[string]$row.case_id] = [ordered]@{
        experiment_id = $ExperimentId
        variant = 'level-1'
        repeat = 1
        case_order = [int]$row.case_order
        file = [string]$row.file
        scope_kib = $scopeKiB
        input_path = $inputRelative
        input_bytes = [Int64]$row.prefix_bytes
        input_sha256 = [string]$row.prefix_sha256
        archive_path = $archiveRelative
        archive_bytes = [Int64]0
        archive_sha256 = $zeroHash
        decoded_path = $decodedRelative
        decoded_bytes = [Int64]0
        decoded_sha256 = $zeroHash
        encode_seconds = 0.0
        decode_seconds = 0.0
        encode_peak_ram_mib = 0.0
        decode_peak_ram_mib = 0.0
        peak_ram_mib = 0.0
        codec_sha256 = $paqHash
        parameters = 'level=-1; automatic-detection=on; threads=1; prefix-policy=leading-prefix-v1'
        encode_command = ''
        decode_command = ''
        encode_exit_code = -3
        decode_exit_code = -3
        started_at = ''
        status = 'PENDING'
        roundtrip = 'NOT_VERIFIED'
        block_types = 'N/A'
        notes = 'Not selected for this partial runtime package.'
    }
}

$resultsPath = Join-Path $packagePath 'results.csv'
Write-ResultsCsv -Path $resultsPath -Rows @($manifestRows | ForEach-Object {
    [pscustomobject]$rowsByCaseId[[string]$_.case_id]
})

[Int64]$memoryLimitBytes = [Int64]$MemoryLimitMiB * 1MB
$failures = New-Object System.Collections.Generic.List[string]
$selectedIndex = 0
foreach ($manifestRow in $selectedRows) {
    $selectedIndex++
    $scopeKiB = [int]$manifestRow.scope_kib
    $record = $rowsByCaseId[[string]$manifestRow.case_id]
    $inputPath = Join-Path $packagePath ($record.input_path -replace '/', '\')
    $archivePath = Join-Path $packagePath ($record.archive_path -replace '/', '\')
    $decodedPath = Join-Path $packagePath ($record.decoded_path -replace '/', '\')
    $logBase = Join-Path $packagePath (('logs/{0}KiB/{1}' -f $scopeKiB, $manifestRow.file) -replace '/', '\')
    $record.started_at = [DateTimeOffset]::Now.ToString('o')
    $record.notes = ''
    $record.encode_command = Format-RecordedCommand -Executable $PaqPath -Arguments @('-1', $inputPath, $archivePath)
    $record.decode_command = Format-RecordedCommand -Executable $PaqPath -Arguments @('-d', $archivePath, $decodedPath)

    try {
        Write-Host ('[{0}/{1}] {2}' -f $selectedIndex, $selectedRows.Count, $manifestRow.case_id)
        Write-LeadingPrefix -SourcePath $manifestRow.source_path -DestinationPath $inputPath -PrefixBytes ([Int64]$manifestRow.prefix_bytes)
        if ((Get-Sha256OrZero $inputPath) -cne [string]$manifestRow.prefix_sha256 -or
            (Get-FileLengthOrZero $inputPath) -ne [Int64]$manifestRow.prefix_bytes) {
            throw "Materialized prefix does not match frozen manifest: $($manifestRow.case_id)"
        }
        if ((Get-Sha256OrZero $PaqPath) -cne $paqHash) {
            throw 'PAQ8px executable SHA-256 changed after experiment setup'
        }
        $encode = Invoke-MeasuredPaq -Executable $PaqPath -Arguments @('-1', $inputPath, $archivePath) -LogBase ($logBase + '.encode') -TimeoutSeconds $ProcessTimeoutSeconds -MemoryLimitBytes $memoryLimitBytes
        $record.encode_seconds = $encode.seconds
        $record.encode_peak_ram_mib = $encode.peak_mib
        $record.encode_exit_code = $encode.exit_code
        if ($encode.exit_code -ne 0 -or -not (Test-Path -LiteralPath $archivePath -PathType Leaf)) {
            $record.status = 'FAILED'
            $record.roundtrip = 'NOT_VERIFIED'
            $record.notes = if ([string]::IsNullOrWhiteSpace($encode.termination)) {
                'PAQ8px encode failed; inspect encode logs.'
            } else {
                "PAQ8px encode terminated: $($encode.termination)"
            }
        }
        else {
            $decode = Invoke-MeasuredPaq -Executable $PaqPath -Arguments @('-d', $archivePath, $decodedPath) -LogBase ($logBase + '.decode') -TimeoutSeconds $ProcessTimeoutSeconds -MemoryLimitBytes $memoryLimitBytes
            $record.decode_seconds = $decode.seconds
            $record.decode_peak_ram_mib = $decode.peak_mib
            $record.decode_exit_code = $decode.exit_code
            if ($decode.exit_code -eq 0 -and (Test-Path -LiteralPath $decodedPath -PathType Leaf) -and
                (Get-FileLengthOrZero $decodedPath) -eq [Int64]$manifestRow.prefix_bytes -and
                (Get-Sha256OrZero $decodedPath) -ceq [string]$manifestRow.prefix_sha256) {
                $record.status = 'COMPLETE'
                $record.roundtrip = 'PASS'
                $record.notes = 'Complete PAQ8px archive; byte-exact SHA-256 round-trip verified.'
            }
            else {
                $record.status = 'FAILED'
                $record.roundtrip = 'FAIL'
                $record.notes = if ([string]::IsNullOrWhiteSpace($decode.termination)) {
                    'PAQ8px decode failed or did not reproduce the frozen prefix hash.'
                } else {
                    "PAQ8px decode terminated: $($decode.termination)"
                }
            }
        }
    }
    catch {
        $record.status = 'FAILED'
        $record.roundtrip = 'NOT_VERIFIED'
        $record.notes = $_.Exception.Message
        $failures.Add(('{0}: {1}' -f $manifestRow.case_id, $_.Exception.Message))
    }

    $record.archive_bytes = Get-FileLengthOrZero $archivePath
    $record.archive_sha256 = Get-Sha256OrZero $archivePath
    $record.decoded_bytes = Get-FileLengthOrZero $decodedPath
    $record.decoded_sha256 = Get-Sha256OrZero $decodedPath
    $record.peak_ram_mib = [Math]::Max([double]$record.encode_peak_ram_mib, [double]$record.decode_peak_ram_mib)
    if ($record.status -ne 'COMPLETE') {
        $failures.Add([string]$manifestRow.case_id)
    }
    Write-ResultsCsv -Path $resultsPath -Rows @($manifestRows | ForEach-Object {
        [pscustomobject]$rowsByCaseId[[string]$_.case_id]
    })
}

$experiment.state = if ($failures.Count -ne 0) { '失败' } elseif ($selectedRows.Count -eq $manifestRows.Count) { '完成' } else { '测试中' }
$experiment.notes = if ($failures.Count -ne 0) {
    'One or more selected cases failed. See results.csv and logs; failed evidence was retained.'
} elseif ($selectedRows.Count -eq $manifestRows.Count) {
    'All frozen manifest cases completed with complete archive bytes and byte-exact SHA-256 verification.'
} else {
    'Partial smoke package. Unselected canonical rows remain PENDING and must not be imported as a completed benchmark.'
}
Write-Utf8NoBom -Path (Join-Path $packagePath 'experiment.json') -Content ($experiment | ConvertTo-Json -Depth 5)

if ($failures.Count -ne 0) {
    throw ('PAQ8px manifest experiment retained failed cases at ' + $packagePath + ': ' + [string]::Join('; ', $failures))
}
Write-Output ('complete={0} selected_cases={1} manifest_cases={2}' -f $packagePath, $selectedRows.Count, $manifestRows.Count)
