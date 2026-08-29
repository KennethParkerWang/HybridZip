[CmdletBinding()]
param(
    [string]$CodecPath = '',
    [string]$DatasetPath = 'F:\paq8px\silesia',
    [string]$OutputRoot = '',
    [string]$ExperimentId = '',
    [ValidateSet('v1', 'r2')]
    [string]$Profile = 'v1',
    # Keep this order equal to the decoder-visible BlockMode IDs.
    [ValidateSet(
        'auto', 'auto-k2', 'auto-k4', 'auto-k8', 'fast', 'fast-ext', 'stored', 'predictive', 'zstd', 'fse', 'lzma', 'donor-match',
        'bwt-zstd', 'bwt-mtf-zstd', 'bwt-rlt-zstd', 'x86-bcj-zstd',
        'shuffle-zstd', 'bitshuffle-zstd', 'delta-zstd', 'fastpfor', 'rans',
        'bcj2-zstd', 'record-transpose-zstd', 'jpegls', 'flac-residual',
        'brotli-text', 'cmix-word-zstd', 'neural-lstm', 'shared-neural-lstm',
        'lstm-compress', 'delta-of-delta-zstd', 'bgpt-shared-prior',
        'jax-compress-portable', 'ppmd7', 'ppmd8', 'zpaq', 'ctw',
        'paq8px-apm', 'paq8px-record-model', 'paq8px-linear-prediction',
        'paq8px-similarity', 'paq8px-similarity-sse', 'paq8px-generic-sse',
        'paq8px-detected-sse', 'wavpack', 'lz4', 'kanzi-ans',
        'lmic-arithmetic', 'delta-binary-packed-zstd', 'fast-ext'
    )]
    [string]$R2Mode = 'auto',
    [switch]$Resume,
    [ValidateRange(1, 604800)]
    [int]$ProcessTimeoutSeconds = 3600,
    [ValidateSet(32, 64, 128)]
    [int[]]$ScopesKiB = @(32, 64, 128),
    [ValidateSet(32, 64, 128)]
    [int]$BlockSizeKiB = 64,
    [ValidateRange(1, 256)]
    [int]$ThreadCount = 1,
    [string[]]$SilesiaFiles = @(),
    [switch]$ListOnly,
    [switch]$AllowAllFiles
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

 $scriptRoot = Split-Path -Parent $MyInvocation.MyCommand.Path
if ([string]::IsNullOrWhiteSpace($scriptRoot)) {
    throw 'Unable to resolve the experiment script directory'
}
if ([string]::IsNullOrWhiteSpace($OutputRoot)) {
    $OutputRoot = Join-Path $scriptRoot '..\results\experiments'
}
if ([string]::IsNullOrWhiteSpace($CodecPath)) {
    $CodecPath = Join-Path $scriptRoot '..\build\Release\hybridzip.exe'
}

# Windows PowerShell 5 can inherit both `Path` and `PATH` from mixed tooling.
# Start-Process materializes its environment through a case-insensitive map
# and fails before launching the codec when both aliases exist. Keep the
# canonical spelling for this runner process only.
$pathEnvironmentNames = @(
    [System.Environment]::GetEnvironmentVariables().Keys |
        Where-Object {
            [string]::Equals(
                [string]$_, 'PATH', [System.StringComparison]::OrdinalIgnoreCase)
        }
)
if ($pathEnvironmentNames.Count -gt 1) {
    foreach ($pathEnvironmentName in $pathEnvironmentNames) {
        if ([string]$pathEnvironmentName -cne 'Path') {
            Remove-Item -LiteralPath ("Env:{0}" -f $pathEnvironmentName) `
                -ErrorAction SilentlyContinue
        }
    }
}

$allFiles = @(
    'dickens', 'mozilla', 'mr', 'nci', 'ooffice', 'osdb',
    'reymont', 'samba', 'sao', 'webster', 'x-ray', 'xml'
)
# Keep this order equal to decoder-visible BlockMode IDs 0..43.
$r2BlockModes = @(
    'stored', 'predictive', 'zstd', 'fse', 'lzma', 'donor-match',
    'bwt-zstd', 'bwt-mtf-zstd', 'bwt-rlt-zstd', 'x86-bcj-zstd',
    'shuffle-zstd', 'bitshuffle-zstd', 'delta-zstd', 'fastpfor', 'rans',
    'bcj2-zstd', 'record-transpose-zstd', 'jpegls', 'flac-residual',
    'brotli-text', 'cmix-word-zstd', 'neural-lstm', 'shared-neural-lstm',
    'lstm-compress', 'delta-of-delta-zstd', 'bgpt-shared-prior',
    'jax-compress-portable', 'ppmd7', 'ppmd8', 'zpaq', 'ctw',
    'paq8px-apm', 'paq8px-record-model', 'paq8px-linear-prediction',
    'paq8px-similarity', 'paq8px-similarity-sse', 'paq8px-generic-sse',
    'paq8px-detected-sse', 'wavpack', 'lz4', 'kanzi-ans',
    'lmic-arithmetic', 'delta-binary-packed-zstd', 'fast-ext'
)
if ($SilesiaFiles.Count -eq 0) {
    $files = @($allFiles)
}
else {
    $requestedFiles = @($SilesiaFiles | ForEach-Object {
        ([string]$_).Split(',')
    } | Where-Object { -not [string]::IsNullOrWhiteSpace([string]$_) })
    $files = @()
    foreach ($requestedFile in $requestedFiles) {
        $canonical = @($allFiles | Where-Object {
            [string]::Equals(
                $_,
                [string]$requestedFile,
                [System.StringComparison]::OrdinalIgnoreCase)
        })
        if ($canonical.Count -ne 1) {
            throw "Unknown Silesia file in -SilesiaFiles: $requestedFile"
        }
        if ($files -contains $canonical[0]) {
            throw "Duplicate Silesia file in -SilesiaFiles: $requestedFile"
        }
        $files += $canonical[0]
    }
}
$scopesKiB = @($ScopesKiB | Sort-Object -Unique)
if ($files.Count -eq 0) {
    throw 'At least one Silesia file must be selected'
}
if ($Profile -eq 'v1' -and $ThreadCount -ne 1) {
    throw 'ThreadCount is only supported by the R2 Fast policy'
}
if ($Profile -eq 'r2' -and $R2Mode -ne 'fast' -and $ThreadCount -ne 1) {
    throw 'ThreadCount greater than one is only supported by R2 Fast'
}
if ($SilesiaFiles.Count -eq 0 -and -not $AllowAllFiles -and -not $ListOnly) {
    throw 'Refusing an implicit all-file experiment; specify -SilesiaFiles or -AllowAllFiles'
}
if ($ListOnly) {
    [pscustomobject]@{
        files = [string]::Join(',', $files)
        scopes_kib = [string]::Join(',', $scopesKiB)
        block_size_kib = $BlockSizeKiB
        thread_count = $ThreadCount
        profile = $Profile
        r2_mode = $R2Mode
    } | ConvertTo-Json -Compress
    exit 0
}
$zeroHash = '0' * 64
$stateTesting = -join @([char]0x6D4B, [char]0x8BD5, [char]0x4E2D)
$stateComplete = -join @([char]0x5B8C, [char]0x6210)
$stateFailed = -join @([char]0x5931, [char]0x8D25)
if ([string]::IsNullOrWhiteSpace($ExperimentId)) {
    $profileToken = if ($Profile -eq 'v1') { 'profile-v1' } else { "r2-$R2Mode" }
    $ExperimentId = "hybridzip-$profileToken-silesia-prefix-$([DateTime]::UtcNow.ToString('yyyyMMdd-HHmmss-fff'))-$([Guid]::NewGuid().ToString('N').Substring(0, 8))"
}
$resultColumns = @(
    'experiment_id', 'variant', 'repeat', 'case_order', 'file', 'scope_kib',
    'input_path', 'input_bytes', 'input_sha256', 'archive_path', 'archive_bytes',
    'archive_sha256', 'decoded_path', 'decoded_bytes', 'decoded_sha256',
    'encode_seconds', 'decode_seconds', 'encode_peak_ram_mib',
    'decode_peak_ram_mib', 'peak_ram_mib', 'codec_sha256', 'parameters',
    'encode_command', 'decode_command', 'encode_exit_code', 'decode_exit_code',
    'started_at', 'status', 'roundtrip', 'block_types', 'notes'
)

function Get-Sha256([string]$Path) {
    if (-not (Test-Path -LiteralPath $Path -PathType Leaf)) {
        return $zeroHash
    }
    return (Get-FileHash -LiteralPath $Path -Algorithm SHA256).Hash.ToUpperInvariant()
}

function Get-FileLengthOrZero([string]$Path) {
    if (-not (Test-Path -LiteralPath $Path -PathType Leaf)) {
        return 0L
    }
    return (Get-Item -LiteralPath $Path).Length
}

function Get-NormalizedDirectoryPath([string]$Path) {
    $fullPath = [System.IO.Path]::GetFullPath($Path)
    $pathRoot = [System.IO.Path]::GetPathRoot($fullPath)
    if ($fullPath.Length -gt $pathRoot.Length) {
        return $fullPath.TrimEnd([char[]]@(
            [System.IO.Path]::DirectorySeparatorChar,
            [System.IO.Path]::AltDirectorySeparatorChar
        ))
    }
    return $fullPath
}

function Test-PathIsInside([string]$Root, [string]$Candidate) {
    $normalizedRoot = Get-NormalizedDirectoryPath $Root
    $normalizedCandidate = [System.IO.Path]::GetFullPath($Candidate)
    $rootPrefix = $normalizedRoot
    if (-not $rootPrefix.EndsWith([System.IO.Path]::DirectorySeparatorChar.ToString())) {
        $rootPrefix += [System.IO.Path]::DirectorySeparatorChar
    }
    return $normalizedCandidate.StartsWith(
        $rootPrefix,
        [System.StringComparison]::OrdinalIgnoreCase
    )
}

function Test-SamePath([string]$Left, [string]$Right) {
    return [string]::Equals(
        (Get-NormalizedDirectoryPath $Left),
        (Get-NormalizedDirectoryPath $Right),
        [System.StringComparison]::OrdinalIgnoreCase
    )
}

function Test-IntegerEquals($Value, [long]$Expected) {
    $parsed = 0L
    $valid = [long]::TryParse(
        [string]$Value,
        [System.Globalization.NumberStyles]::Integer,
        [System.Globalization.CultureInfo]::InvariantCulture,
        [ref]$parsed
    )
    return $valid -and $parsed -eq $Expected
}

function Assert-NotReparsePoint([string]$Path, [string]$Description) {
    try {
        $attributes = [System.IO.File]::GetAttributes($Path)
    }
    catch [System.IO.FileNotFoundException] {
        return
    }
    catch [System.IO.DirectoryNotFoundException] {
        return
    }
    if (($attributes -band [System.IO.FileAttributes]::ReparsePoint) -ne 0) {
        throw "$Description must not be a reparse point: $Path"
    }
}

function New-FatalExperimentException([string]$Message) {
    $exception = New-Object System.InvalidOperationException -ArgumentList $Message
    $exception.Data['HybridZipExperimentFatal'] = $true
    return $exception
}

function Test-IsFatalExperimentException([System.Exception]$Exception) {
    $current = $Exception
    while ($null -ne $current) {
        if ($current.Data.Contains('HybridZipExperimentFatal') -and
            $current.Data['HybridZipExperimentFatal'] -eq $true) {
            return $true
        }
        $current = $current.InnerException
    }
    return $false
}

function Write-AtomicUtf8NoBom([string]$Path, [string]$Content) {
    $fullPath = [System.IO.Path]::GetFullPath($Path)
    $directory = [System.IO.Path]::GetDirectoryName($fullPath)
    if (-not (Test-Path -LiteralPath $directory -PathType Container)) {
        throw "Atomic write target directory does not exist: $directory"
    }
    Assert-NotReparsePoint $fullPath 'Atomic write target'

    $tempPath = Join-Path $directory (
        '.{0}.{1}.tmp' -f [System.IO.Path]::GetFileName($fullPath),
        [Guid]::NewGuid().ToString('N')
    )
    $backupPath = Join-Path $directory (
        '.{0}.{1}.bak' -f [System.IO.Path]::GetFileName($fullPath),
        [Guid]::NewGuid().ToString('N')
    )
    $encoding = New-Object System.Text.UTF8Encoding($false)
    try {
        [System.IO.File]::WriteAllText($tempPath, $Content, $encoding)
        if ([System.IO.File]::Exists($fullPath)) {
            [System.IO.File]::Replace($tempPath, $fullPath, $backupPath)
            [System.IO.File]::Delete($backupPath)
        }
        else {
            [System.IO.File]::Move($tempPath, $fullPath)
        }
    }
    finally {
        if ([System.IO.File]::Exists($tempPath)) {
            [System.IO.File]::Delete($tempPath)
        }
        if ([System.IO.File]::Exists($backupPath)) {
            [System.IO.File]::Delete($backupPath)
        }
    }
}

function Write-Prefix([string]$Source, [string]$Destination, [int]$Bytes) {
    $input = [System.IO.File]::OpenRead($Source)
    try {
        if ($input.Length -lt $Bytes) {
            throw "Source is shorter than requested prefix: $Source"
        }
        $buffer = New-Object byte[] $Bytes
        $offset = 0
        while ($offset -lt $Bytes) {
            $read = $input.Read($buffer, $offset, $Bytes - $offset)
            if ($read -eq 0) {
                throw "Unexpected end of source: $Source"
            }
            $offset += $read
        }
        [System.IO.File]::WriteAllBytes($Destination, $buffer)
    }
    finally {
        $input.Dispose()
    }
}

function Get-PrefixSha256([string]$Source, [int]$Bytes) {
    $input = [System.IO.File]::OpenRead($Source)
    try {
        if ($input.Length -lt $Bytes) {
            throw "Source is shorter than requested prefix: $Source"
        }
        $buffer = New-Object byte[] $Bytes
        $offset = 0
        while ($offset -lt $Bytes) {
            $read = $input.Read($buffer, $offset, $Bytes - $offset)
            if ($read -eq 0) {
                throw "Unexpected end of source: $Source"
            }
            $offset += $read
        }
    }
    finally {
        $input.Dispose()
    }

    $sha256 = [System.Security.Cryptography.SHA256]::Create()
    try {
        $hashBytes = $sha256.ComputeHash($buffer)
    }
    finally {
        $sha256.Dispose()
    }
    return [BitConverter]::ToString($hashBytes).Replace('-', '')
}

function Quote-RecordedPath([string]$Path) {
    return '"' + $Path + '"'
}

function Format-RecordedCommand(
    [string]$Executable,
    [string[]]$Arguments,
    [string]$InputPath,
    [string]$OutputPath
) {
    $parts = New-Object System.Collections.Generic.List[string]
    $parts.Add((Quote-RecordedPath $Executable))
    foreach ($argument in $Arguments) {
        $parts.Add($argument)
    }
    $parts.Add((Quote-RecordedPath $InputPath))
    $parts.Add((Quote-RecordedPath $OutputPath))
    return [string]::Join(' ', $parts)
}

function Assert-CodecUnchanged([string]$Checkpoint) {
    $currentHash = Get-Sha256 $script:CodecPath
    if (-not [string]::Equals(
        $currentHash,
        $script:codecSha256,
        [System.StringComparison]::OrdinalIgnoreCase
    )) {
        throw (New-FatalExperimentException `
            "Codec SHA-256 changed at $Checkpoint. Expected $($script:codecSha256), found $currentHash")
    }
}

function Get-CurrentEnvironmentDescription {
    $cpu = 'unknown'
    try {
        $detectedCpu = (Get-CimInstance Win32_Processor | Select-Object -First 1).Name
        if (-not [string]::IsNullOrWhiteSpace($detectedCpu)) {
            $cpu = $detectedCpu
        }
    }
    catch {
    }
    return "Windows $([Environment]::OSVersion.Version); CPU=$cpu; sequential execution"
}

function Get-SourceRevision {
    $repositoryRoot = if ([string]::IsNullOrWhiteSpace($PSScriptRoot)) {
        [Environment]::CurrentDirectory
    }
    else {
        [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..'))
    }
    try {
        $revision = ([string]::Join('', @(& git -C $repositoryRoot rev-parse HEAD 2>$null))).Trim()
        if ([string]::IsNullOrWhiteSpace($revision)) {
            return 'unknown'
        }
        $dirty = -not [string]::IsNullOrWhiteSpace(
            ([string]::Join('', @(& git -C $repositoryRoot status --porcelain 2>$null))).Trim())
        if ($dirty) {
            return "$revision-dirty"
        }
        return $revision
    }
    catch {
        return 'unknown'
    }
}

function Invoke-MeasuredCodec {
    param(
        [string]$Executable,
        [string[]]$PrefixArguments,
        [string]$InputPath,
        [string]$OutputPath,
        [string]$LogBase,
        [int]$TimeoutSeconds
    )

    $stdoutPath = $LogBase + '.stdout.log'
    $stderrPath = $LogBase + '.stderr.log'
    $arguments = @($PrefixArguments) +
        @(('"' + $InputPath + '"'), ('"' + $OutputPath + '"'))
    $stopwatch = [System.Diagnostics.Stopwatch]::StartNew()
    $process = Start-Process -FilePath $Executable `
        -ArgumentList $arguments `
        -RedirectStandardOutput $stdoutPath `
        -RedirectStandardError $stderrPath `
        -WindowStyle Hidden `
        -PassThru
    $processHandle = $process.Handle

    $peakBytes = 0L
    $timedOut = $false
    while (-not $process.HasExited) {
        try {
            $process.Refresh()
            $peakBytes = [Math]::Max($peakBytes, $process.PeakWorkingSet64)
        }
        catch {
            # The process can disappear between HasExited and Refresh.
        }

        if ($stopwatch.Elapsed.TotalSeconds -ge $TimeoutSeconds) {
            $timedOut = $true
            try {
                if (-not $process.HasExited) {
                    $process.Kill()
                }
            }
            catch {
                if (-not $process.HasExited) {
                    throw (New-FatalExperimentException `
                        "Process timed out and could not be terminated (handle $processHandle): $($_.Exception.Message)")
                }
            }
            if (-not $process.WaitForExit(10000)) {
                throw (New-FatalExperimentException `
                    "Process timed out and did not terminate within 10 seconds (handle $processHandle)")
            }
            break
        }
        Start-Sleep -Milliseconds 10
    }
    $process.WaitForExit()
    $stopwatch.Stop()
    try {
        $process.Refresh()
        $peakBytes = [Math]::Max($peakBytes, $process.PeakWorkingSet64)
    }
    catch {
    }

    $exitCode = -2
    if (-not $timedOut) {
        if ($null -eq $process.ExitCode) {
            throw "Process exit code was unavailable (handle $processHandle)"
        }
        $exitCode = $process.ExitCode
    }

    [pscustomobject]@{
        ExitCode = $exitCode
        TimedOut = $timedOut
        Seconds = $stopwatch.Elapsed.TotalSeconds
        PeakMiB = $peakBytes / 1MB
        StdoutPath = $stdoutPath
        StderrPath = $stderrPath
    }
}

function Get-RecordedBlockTypes([string]$EncodeStdoutPath) {
    if ($script:Profile -ne 'r2') {
        return 'N/A'
    }
    if (-not (Test-Path -LiteralPath $EncodeStdoutPath -PathType Leaf)) {
        return 'UNKNOWN'
    }
    $stdout = Get-Content -LiteralPath $EncodeStdoutPath -Raw -Encoding UTF8
    if ([string]::IsNullOrWhiteSpace($stdout)) {
        return 'UNKNOWN'
    }
    $pattern = 'blocks\(([^)]+)\)=([0-9]+(?:/[0-9]+)*)'
    $match = [regex]::Match($stdout, $pattern)
    if (-not $match.Success) {
        return 'UNKNOWN'
    }
    $names = $match.Groups[1].Value.Split('/')
    $counts = $match.Groups[2].Value.Split('/')
    if ($names.Count -ne $counts.Count) {
        return 'UNKNOWN'
    }
    $parts = New-Object System.Collections.Generic.List[string]
    for ($index = 0; $index -lt $names.Count; ++$index) {
        if ([int]$counts[$index] -gt 0) {
            $parts.Add(('{0}={1}' -f $names[$index], $counts[$index]))
        }
    }
    if ($parts.Count -eq 0) {
        return 'none'
    }
    return [string]::Join(';', $parts)
}

function Assert-R2BlockTypes([string]$RequestedMode,
                             [string]$BlockTypes,
                             [int64]$InputBytes,
                             [int64]$BlockSizeBytes,
                             [string]$Description) {
    if ([string]::IsNullOrWhiteSpace($BlockTypes) -or
        $BlockTypes -eq 'none' -or $BlockTypes -eq 'UNKNOWN') {
        throw "Missing recorded HZ02 block modes for $Description"
    }

    $counts = New-Object 'System.Collections.Generic.Dictionary[string,int64]' `
        ([System.StringComparer]::Ordinal)
    foreach ($part in $BlockTypes.Split(';')) {
        $match = [regex]::Match(
            $part.Trim(), '^(?<name>[^=;]+)=(?<count>[1-9][0-9]*)$')
        if (-not $match.Success) {
            throw "Malformed HZ02 block record for ${Description}: $part"
        }
        $matches = @($r2BlockModes | Where-Object {
            [string]::Equals($_, $match.Groups['name'].Value,
                [System.StringComparison]::Ordinal)
        })
        if ($matches.Count -ne 1) {
            throw "Unknown HZ02 block mode for ${Description}: $($match.Groups['name'].Value)"
        }
        [int64]$count = 0
        if (-not [int64]::TryParse(
            $match.Groups['count'].Value,
            [Globalization.NumberStyles]::None,
            [Globalization.CultureInfo]::InvariantCulture,
            [ref]$count
        )) {
            throw "Invalid HZ02 block count for ${Description}: $($match.Groups['count'].Value)"
        }
        if ($counts.ContainsKey($matches[0])) {
            throw "Duplicate HZ02 block mode for ${Description}: $($matches[0])"
        }
        $counts.Add($matches[0], $count)
    }

    [int64]$total = 0
    foreach ($count in $counts.Values) {
        if ($count -gt ([int64]::MaxValue - $total)) {
            throw "HZ02 block count overflow for $Description"
        }
        $total += $count
    }
    $expectedBlocks = [int64][Math]::Ceiling(
        [double]$InputBytes / [double]$BlockSizeBytes)
    if ($total -ne $expectedBlocks) {
        throw "HZ02 block count mismatch for ${Description}: recorded=$total expected=$expectedBlocks"
    }
    $shortlistModes = @('auto', 'auto-k2', 'auto-k4', 'auto-k8')
    if ($RequestedMode -eq 'fast') {
        # Fast K=4 is a policy, not a single archive mode.  It may select the
        # stored fallback, the append-only zstd extension, or LZ4 per block.
        $fastModes = @('stored', 'fast-ext', 'lz4')
        $invalid = @($counts.Keys | Where-Object { $_ -notin $fastModes })
        $totalFastBlocks = [int64]0
        foreach ($mode in $fastModes) {
            if ($counts.ContainsKey($mode)) {
                $totalFastBlocks += $counts[$mode]
            }
        }
        if ($invalid.Count -ne 0 -or $totalFastBlocks -ne $expectedBlocks) {
            throw "Fast HZ02 block policy mismatch for ${Description}: requested=$RequestedMode recorded=$BlockTypes"
        }
    }
    elseif ($RequestedMode -notin $shortlistModes -and
            ($counts.Count -ne 1 -or -not $counts.ContainsKey($RequestedMode) -or
             $counts[$RequestedMode] -ne $expectedBlocks)) {
        throw "Forced HZ02 mode mismatch for ${Description}: requested=$RequestedMode recorded=$BlockTypes"
    }
}

function Write-ExperimentJson([string]$State) {
    $metadata = [ordered]@{
        schema_version = 1
        experiment_id = $script:ExperimentId
        name = $script:experimentName
        state = $State
        description = $script:experimentDescription
        model_structure = $script:modelStructure
        change_from_baseline = $script:changeFromBaseline
        hypothesis = $script:hypothesis
        baseline_description = 'Product comparison uses available gzip, zstd, brotli, xz, and 7-Zip binaries on identical bytes.'
        created_at = $script:createdAt
        dataset_name = 'Silesia'
        dataset_path = $script:DatasetPath
        files = $files
        scopes_kib = $scopesKiB
        variants = @($script:variant)
        repeat_count = 1
        codec_name = 'HybridZip'
        codec_version = $script:codecVersion
        codec_path = $script:CodecPath
        codec_sha256 = $script:codecSha256
        source_revision = $script:sourceRevision
        command_template = $script:commandTemplate
        configuration = $script:configuration
        environment = $script:environmentDescription
        notes = $script:metadataNotes
    }
    $json = ($metadata | ConvertTo-Json -Depth 8) + "`r`n"
    Write-AtomicUtf8NoBom `
        -Path (Join-Path $script:packagePath 'experiment.json') `
        -Content $json
}

function Get-OrderedRows {
    $orderedRows = New-Object System.Collections.Generic.List[object]
    foreach ($case in $script:cases) {
        if ($script:rowMap.ContainsKey($case.Key)) {
            $orderedRows.Add($script:rowMap[$case.Key])
        }
    }
    return $orderedRows.ToArray()
}

function Write-ResultsCsv {
    $orderedRows = @(Get-OrderedRows)
    if ($orderedRows.Count -eq 0) {
        return
    }
    $csvLines = @($orderedRows | ConvertTo-Csv -NoTypeInformation)
    $csv = [string]::Join("`r`n", $csvLines) + "`r`n"
    Write-AtomicUtf8NoBom `
        -Path (Join-Path $script:packagePath 'results.csv') `
        -Content $csv
}

function Get-RequiredMetadataValue($Metadata, [string]$Name) {
    $property = $Metadata.PSObject.Properties[$Name]
    if ($null -eq $property) {
        throw "Resume metadata is missing required property: $Name"
    }
    return $property.Value
}

function Read-AndValidateResumeMetadata([string]$Path) {
    if (-not (Test-Path -LiteralPath $Path -PathType Leaf)) {
        throw "Resume metadata not found: $Path"
    }
    try {
        $metadata = Get-Content -LiteralPath $Path -Raw -Encoding UTF8 | ConvertFrom-Json
    }
    catch {
        throw "Resume metadata is not valid JSON: $Path ($($_.Exception.Message))"
    }

    $metadataExperimentId = [string](Get-RequiredMetadataValue $metadata 'experiment_id')
    if ($metadataExperimentId -cne $script:ExperimentId) {
        throw "Resume experiment ID mismatch. Expected $($script:ExperimentId), found $metadataExperimentId"
    }

    $metadataDatasetPath = [string](Get-RequiredMetadataValue $metadata 'dataset_path')
    if (-not (Test-SamePath $metadataDatasetPath $script:DatasetPath)) {
        throw "Resume dataset path mismatch. Expected $($script:DatasetPath), found $metadataDatasetPath"
    }

    $metadataFiles = @((Get-RequiredMetadataValue $metadata 'files') | ForEach-Object { [string]$_ })
    if (($metadataFiles -join '|') -ne ($files -join '|')) {
        throw "Resume file selection mismatch. Expected $($files -join ', '), found $($metadataFiles -join ', ')"
    }
    $metadataScopes = @((Get-RequiredMetadataValue $metadata 'scopes_kib') | ForEach-Object { [int]$_ })
    if (($metadataScopes -join '|') -ne ($scopesKiB -join '|')) {
        throw "Resume scope selection mismatch. Expected $($scopesKiB -join ', '), found $($metadataScopes -join ', ')"
    }

    $metadataCodecHash = [string](Get-RequiredMetadataValue $metadata 'codec_sha256')
    if (-not [string]::Equals(
        $metadataCodecHash,
        $script:codecSha256,
        [System.StringComparison]::OrdinalIgnoreCase
    )) {
        throw "Resume codec SHA-256 mismatch. Expected $($script:codecSha256), found $metadataCodecHash"
    }

    $metadataConfiguration = [string](Get-RequiredMetadataValue $metadata 'configuration')
    if ($metadataConfiguration -cne $script:configuration) {
        throw "Resume configuration mismatch. Expected $($script:configuration), found $metadataConfiguration"
    }
    $metadataEnvironment = [string](Get-RequiredMetadataValue $metadata 'environment')
    if ($metadataEnvironment -cne $script:environmentDescription) {
        throw "Resume environment mismatch. Expected $($script:environmentDescription), found $metadataEnvironment"
    }
    $metadataSourceRevision = [string](Get-RequiredMetadataValue $metadata 'source_revision')
    if ($metadataSourceRevision -cne $script:sourceRevision) {
        throw "Resume source revision mismatch. Expected $($script:sourceRevision), found $metadataSourceRevision"
    }

    $metadataCreatedAt = [string](Get-RequiredMetadataValue $metadata 'created_at')
    if ([string]::IsNullOrWhiteSpace($metadataCreatedAt)) {
        throw 'Resume metadata created_at is empty'
    }
    return [pscustomobject]@{
        Metadata = $metadata
        CreatedAt = $metadataCreatedAt
    }
}

function Read-AndValidateResumeRows([string]$Path) {
    if (-not (Test-Path -LiteralPath $Path -PathType Leaf)) {
        return
    }
    if ((Get-Item -LiteralPath $Path).Length -eq 0) {
        return
    }

    try {
        $loadedRows = @(Import-Csv -LiteralPath $Path -Encoding UTF8)
    }
    catch {
        throw "Resume results are not valid CSV: $Path ($($_.Exception.Message))"
    }

    foreach ($row in $loadedRows) {
        foreach ($column in $resultColumns) {
            if ($null -eq $row.PSObject.Properties[$column]) {
                throw "Resume results are missing required column: $column"
            }
        }
        if ([string]$row.experiment_id -cne $script:ExperimentId) {
            throw "Resume row experiment ID mismatch: $($row.experiment_id)"
        }
        if ([string]$row.variant -cne $script:variant -or
            -not (Test-IntegerEquals $row.repeat 1)) {
            throw "Resume row has an unsupported variant/repeat: $($row.variant)/$($row.repeat)"
        }

        $scope = 0L
        if (-not [long]::TryParse(
            [string]$row.scope_kib,
            [System.Globalization.NumberStyles]::Integer,
            [System.Globalization.CultureInfo]::InvariantCulture,
            [ref]$scope
        )) {
            throw "Resume row has invalid scope_kib: $($row.scope_kib)"
        }
        $key = '{0}|{1}' -f [string]$row.file, $scope
        if (-not $script:caseByKey.ContainsKey($key)) {
            throw "Resume row is not one of the $($cases.Count) expected cases: $key"
        }
        $expectedCase = $script:caseByKey[$key]
        if (-not (Test-IntegerEquals $row.case_order $expectedCase.Order)) {
            throw "Resume row case_order mismatch for $key. Expected $($expectedCase.Order), found $($row.case_order)"
        }
        if (-not [string]::Equals(
            [string]$row.codec_sha256,
            $script:codecSha256,
            [System.StringComparison]::OrdinalIgnoreCase
        )) {
            throw "Resume row codec SHA-256 mismatch for $key"
        }
        if ($script:rowMap.ContainsKey($key)) {
            throw "Duplicate resume result key: $key"
        }
        $script:rowMap[$key] = $row
    }
}

function Get-CompletedArtifactValidationError($Row, $Case) {
    if ([string]$Row.status -cne 'COMPLETE' -or [string]$Row.roundtrip -cne 'PASS') {
        return 'stored row is not COMPLETE/PASS'
    }
    if ([string]$Row.input_path -cne $Case.InputRelative -or
        [string]$Row.archive_path -cne $Case.ArchiveRelative -or
        [string]$Row.decoded_path -cne $Case.DecodedRelative) {
        return 'stored artifact paths do not match the canonical case paths'
    }
    if (-not (Test-IntegerEquals $Row.input_bytes $Case.InputBytes)) {
        return 'stored input size does not match the canonical case size'
    }
    if (-not (Test-IntegerEquals $Row.encode_exit_code 0) -or
        -not (Test-IntegerEquals $Row.decode_exit_code 0)) {
        return 'stored codec exit code is not zero'
    }

    foreach ($path in @($Case.InputPath, $Case.ArchivePath, $Case.DecodedPath)) {
        if (-not (Test-Path -LiteralPath $path -PathType Leaf)) {
            return "artifact is missing: $path"
        }
    }

    $actualInputBytes = (Get-Item -LiteralPath $Case.InputPath).Length
    $actualArchiveBytes = (Get-Item -LiteralPath $Case.ArchivePath).Length
    $actualDecodedBytes = (Get-Item -LiteralPath $Case.DecodedPath).Length
    if ($actualInputBytes -ne $Case.InputBytes -or
        $actualDecodedBytes -ne $Case.InputBytes -or
        -not (Test-IntegerEquals $Row.input_bytes $actualInputBytes) -or
        -not (Test-IntegerEquals $Row.archive_bytes $actualArchiveBytes) -or
        -not (Test-IntegerEquals $Row.decoded_bytes $actualDecodedBytes)) {
        return 'artifact size does not match the stored result'
    }

    $actualInputHash = Get-Sha256 $Case.InputPath
    $actualArchiveHash = Get-Sha256 $Case.ArchivePath
    $actualDecodedHash = Get-Sha256 $Case.DecodedPath
    $currentSourcePrefixHash = Get-PrefixSha256 `
        (Join-Path $script:DatasetPath $Case.File) $Case.InputBytes
    if (-not [string]::Equals(
        [string]$Row.input_sha256,
        $actualInputHash,
        [System.StringComparison]::OrdinalIgnoreCase
    ) -or -not [string]::Equals(
        [string]$Row.archive_sha256,
        $actualArchiveHash,
        [System.StringComparison]::OrdinalIgnoreCase
    ) -or -not [string]::Equals(
        [string]$Row.decoded_sha256,
        $actualDecodedHash,
        [System.StringComparison]::OrdinalIgnoreCase
    )) {
        return 'one or more artifact SHA-256 values do not match the stored result'
    }
    if (-not [string]::Equals(
        $actualInputHash,
        $actualDecodedHash,
        [System.StringComparison]::OrdinalIgnoreCase
    )) {
        return 'decoded artifact is not byte-exact'
    }
    if (-not [string]::Equals(
        $actualInputHash,
        $currentSourcePrefixHash,
        [System.StringComparison]::OrdinalIgnoreCase
    )) {
        return 'stored input does not match the current dataset source prefix'
    }
    return $null
}

if ($Profile -eq 'v1' -and $R2Mode -ne 'auto') {
    throw 'R2Mode is only valid when Profile is r2'
}
if ([string]::IsNullOrWhiteSpace($ExperimentId) -or
    $ExperimentId -cnotmatch '^[a-z0-9-]+$') {
    throw "ExperimentId must match ^[a-z0-9-]+`$: $ExperimentId"
}

$CodecPath = [System.IO.Path]::GetFullPath($CodecPath)
$DatasetPath = Get-NormalizedDirectoryPath $DatasetPath
$OutputRoot = Get-NormalizedDirectoryPath $OutputRoot
$packagePath = [System.IO.Path]::GetFullPath((Join-Path $OutputRoot $ExperimentId))
if (-not (Test-PathIsInside $OutputRoot $packagePath)) {
    throw "Experiment package escapes OutputRoot: $packagePath"
}

if (-not (Test-Path -LiteralPath $CodecPath -PathType Leaf)) {
    throw "Codec executable not found: $CodecPath"
}
if (-not (Test-Path -LiteralPath $DatasetPath -PathType Container)) {
    throw "Dataset directory not found: $DatasetPath"
}
foreach ($file in $files) {
    $source = Join-Path $DatasetPath $file
    if (-not (Test-Path -LiteralPath $source -PathType Leaf)) {
        throw "Missing Silesia file: $source"
    }
    if ((Get-Item -LiteralPath $source).Length -lt 128KB) {
        throw "Silesia file is shorter than 128 KiB: $source"
    }
}

$codecSha256 = Get-Sha256 $CodecPath
if ($Profile -eq 'v1') {
    $variant = 'profile-v1'
    $archiveExtension = '.hz'
    $encodeArguments = @('c')
    $decodeArguments = @('d')
    $configuration = "profile_id=1; experts=NGram,PPMD,Match,OnlineLSTM; mixer_eta=0.5; cdf_bits=24; coder_state_bits=32; threads=1; process_timeout_seconds=$ProcessTimeoutSeconds"
    $rowParameters = "profile_id=1;threads=1;process_timeout_seconds=$ProcessTimeoutSeconds"
    $experimentName = 'HybridZip PROFILE_V1 Silesia prefix experiment'
    $experimentDescription = 'HybridZip first-generation four-expert lossless compressor on Silesia 32/64/128 KiB prefixes.'
    $modelStructure = 'NGram + PPMD + Match + Online LSTM -> AdaptiveLinearMixer -> CDF24 -> Nayuki ArithmeticCoder32.'
    $changeFromBaseline = 'New PROFILE_V1 byte-native hybrid compressor; no baseline component is substituted at run time.'
    $hypothesis = 'HybridZip should improve over fast LZ baselines on structured prefixes, at substantially higher encode/decode cost.'
    $codecVersion = '1.0.0-profile-v1'
    $commandTemplate = '"<codec-path>" c "<input-path>" "<archive-path>"; "<codec-path>" d "<archive-path>" "<output-path>"'
    $metadataNotes = 'archive_bytes includes the complete 40-byte HZ01 header and arithmetic payload.'
}
else {
    $variant = "r2-$R2Mode"
    $archiveExtension = '.hz2'
    $blockSizeBytes = [int64]$BlockSizeKiB * 1024L
    $zstdLevel = if ($R2Mode -eq 'fast' -or $R2Mode -eq 'fast-ext') { 3 } else { 19 }
    $encodeArguments = @(
        'c', '--profile=r2', "--r2-mode=$R2Mode",
        "--block-size=$blockSizeBytes"
    )
    if ($R2Mode -eq 'fast' -or $R2Mode -eq 'fast-ext') {
        $encodeArguments += '--zstd-level=3'
    }
    if ($R2Mode -eq 'fast') {
        $encodeArguments += "--threads=$ThreadCount"
    }
    $decodeArguments = @('d')
    $configuration = "profile_id=2;mode=$R2Mode;block_size=$blockSizeBytes;zstd_level=$zstdLevel;lzma_level=9;threads=$ThreadCount;process_timeout_seconds=$ProcessTimeoutSeconds"
    $rowParameters = $configuration
    $experimentName = "HybridZip R2 $R2Mode Silesia prefix experiment"
    $experimentDescription = "HybridZip HZ02 block portfolio mode $R2Mode on Silesia 32/64/128 KiB prefixes."
    $modelStructure = 'HZ02 donor-driven block portfolio with representation, specialist, neural, LZ, and multi-coder candidates.'
    $changeFromBaseline = "Uses HZ02 mode $R2Mode instead of the fixed HZ01 PROFILE_V1 full-file path."
    $hypothesis = 'The block portfolio should retain byte-exact reconstruction and choose only payloads smaller than stored in auto mode.'
    $codecVersion = "1.0.0-r2-$R2Mode"
    $commandTemplate = '"<codec-path>" c --profile=r2 --r2-mode=<mode> "<input-path>" "<archive-path>"; "<codec-path>" d "<archive-path>" "<output-path>"'
    $metadataNotes = 'archive_bytes includes the complete HZ02 archive header, every block header, CRC32 metadata, backend envelope, and payload.'
}
$environmentDescription = Get-CurrentEnvironmentDescription
$sourceRevision = Get-SourceRevision
$cases = New-Object System.Collections.Generic.List[object]
$caseByKey = @{}
$caseOrder = 0
foreach ($file in $files) {
    foreach ($scope in $scopesKiB) {
        ++$caseOrder
        $scopeName = "${scope}KiB"
        $inputRelative = "inputs/$scopeName/$file.bin"
        $archiveRelative = "archives/$scopeName/$file$archiveExtension"
        $decodedRelative = "decoded/$scopeName/$file.decoded"
        $case = [pscustomobject]@{
            Key = "$file|$scope"
            Order = $caseOrder
            File = $file
            Scope = $scope
            ScopeName = $scopeName
            InputBytes = $scope * 1024
            InputRelative = $inputRelative
            ArchiveRelative = $archiveRelative
            DecodedRelative = $decodedRelative
            InputPath = Join-Path $packagePath ($inputRelative -replace '/', '\')
            ArchivePath = Join-Path $packagePath ($archiveRelative -replace '/', '\')
            DecodedPath = Join-Path $packagePath ($decodedRelative -replace '/', '\')
            LogBase = Join-Path $packagePath "logs\$scopeName\$file"
        }
        $cases.Add($case)
        $caseByKey[$case.Key] = $case
    }
}
$rowMap = @{}

if ($Resume) {
    if (-not (Test-Path -LiteralPath $packagePath -PathType Container)) {
        throw "Resume package does not exist: $packagePath"
    }
    Assert-NotReparsePoint $packagePath 'Resume package'
    Assert-NotReparsePoint `
        (Join-Path $packagePath 'experiment.json') 'Resume metadata'
    Assert-NotReparsePoint `
        (Join-Path $packagePath 'results.csv') 'Resume results'
    $resumeMetadata = Read-AndValidateResumeMetadata `
        (Join-Path $packagePath 'experiment.json')
    $createdAt = $resumeMetadata.CreatedAt
    Read-AndValidateResumeRows (Join-Path $packagePath 'results.csv')
}
else {
    if (Test-Path -LiteralPath $packagePath) {
        throw "Refusing to overwrite experiment package: $packagePath"
    }
    New-Item -ItemType Directory -Path $packagePath | Out-Null
    $createdAt = [DateTimeOffset]::Now.ToString('o')
}

foreach ($scope in $scopesKiB) {
    $scopeName = "${scope}KiB"
    foreach ($kind in @('inputs', 'archives', 'decoded', 'logs')) {
        $kindPath = Join-Path $packagePath $kind
        $scopePath = Join-Path $kindPath $scopeName
        Assert-NotReparsePoint $kindPath 'Experiment artifact directory'
        Assert-NotReparsePoint $scopePath 'Experiment scope directory'
        New-Item -ItemType Directory `
            -Path $scopePath `
            -Force | Out-Null
    }
}

Assert-CodecUnchanged 'experiment initialization'
Write-ExperimentJson -State $stateTesting

foreach ($case in $cases) {
    Assert-CodecUnchanged "before case $($case.Order)/$($cases.Count) ($($case.Key))"
    foreach ($casePath in @(
        $case.InputPath,
        $case.ArchivePath,
        $case.DecodedPath,
        ($case.LogBase + '.encode.stdout.log'),
        ($case.LogBase + '.encode.stderr.log'),
        ($case.LogBase + '.decode.stdout.log'),
        ($case.LogBase + '.decode.stderr.log')
    )) {
        Assert-NotReparsePoint $casePath 'Experiment case artifact'
    }

    if ($rowMap.ContainsKey($case.Key)) {
        $validationError = $null
        try {
            $validationError = Get-CompletedArtifactValidationError `
                $rowMap[$case.Key] $case
        }
        catch {
            $validationError = "artifact revalidation error: $($_.Exception.Message)"
        }
        if ($null -eq $validationError) {
            Write-Host ("[{0}/{1}] {2} {3} KiB: COMPLETE/PASS (resume artifacts revalidated)" -f `
                $case.Order, $cases.Count, $case.File, $case.Scope)
            continue
        }
        Write-Host ("[{0}/{1}] {2} {3} KiB: rerunning ({4})" -f `
            $case.Order, $cases.Count, $case.File, $case.Scope, $validationError)
    }

    $startedAt = [DateTimeOffset]::Now.ToString('o')
    $inputHash = $zeroHash
    $encode = $null
    $decode = $null
    $status = 'FAILED'
    $roundtrip = 'NOT_VERIFIED'
    $notes = ''
    try {
        # The codec commits archives through `<archive>.tmp`. A prior
        # interrupted encode may leave that temporary path behind even when
        # the final archive is absent; remove only this case's known artifact.
        foreach ($stalePath in @(
            $case.ArchivePath,
            ($case.ArchivePath + '.tmp'),
            $case.DecodedPath
        )) {
            if (Test-Path -LiteralPath $stalePath -PathType Leaf) {
                [System.IO.File]::Delete($stalePath)
            }
        }
        Write-Prefix -Source (Join-Path $DatasetPath $case.File) `
            -Destination $case.InputPath -Bytes $case.InputBytes
        $inputHash = Get-Sha256 $case.InputPath

        Assert-CodecUnchanged "before encode for case $($case.Order)/$($cases.Count) ($($case.Key))"
        $encode = Invoke-MeasuredCodec -Executable $CodecPath `
            -PrefixArguments $encodeArguments `
            -InputPath $case.InputPath -OutputPath $case.ArchivePath `
            -LogBase ($case.LogBase + '.encode') `
            -TimeoutSeconds $ProcessTimeoutSeconds
        if ($encode.TimedOut) {
            throw "Encode exceeded timeout of $ProcessTimeoutSeconds seconds"
        }
        if ($encode.ExitCode -ne 0) {
            throw "Encode exit code $($encode.ExitCode)"
        }
        if ($Profile -eq 'r2') {
            $recordedBlockTypes = Get-RecordedBlockTypes $encode.StdoutPath
            Assert-R2BlockTypes $R2Mode $recordedBlockTypes $case.InputBytes `
                $blockSizeBytes `
                "case $($case.Order)/$($cases.Count) ($($case.Key))"
        }

        Assert-CodecUnchanged "before decode for case $($case.Order)/$($cases.Count) ($($case.Key))"
        $decode = Invoke-MeasuredCodec -Executable $CodecPath `
            -PrefixArguments $decodeArguments `
            -InputPath $case.ArchivePath -OutputPath $case.DecodedPath `
            -LogBase ($case.LogBase + '.decode') `
            -TimeoutSeconds $ProcessTimeoutSeconds
        if ($decode.TimedOut) {
            throw "Decode exceeded timeout of $ProcessTimeoutSeconds seconds"
        }
        if ($decode.ExitCode -ne 0) {
            throw "Decode exit code $($decode.ExitCode)"
        }
        Assert-CodecUnchanged "before result commit for case $($case.Order)/$($cases.Count) ($($case.Key))"

        $decodedHash = Get-Sha256 $case.DecodedPath
        if ((Get-FileLengthOrZero $case.DecodedPath) -ne $case.InputBytes -or
            $decodedHash -ne $inputHash) {
            $roundtrip = 'FAIL'
            throw 'Decoded output is not byte-exact'
        }
        $status = 'COMPLETE'
        $roundtrip = 'PASS'
        $notes = "Complete $($Profile.ToUpperInvariant()) archive size; SHA-256 byte-exact roundtrip verified."
    }
    catch {
        if (Test-IsFatalExperimentException $_.Exception) {
            throw
        }
        $notes = $_.Exception.Message
        if ($roundtrip -ne 'FAIL') {
            $roundtrip = 'NOT_VERIFIED'
        }
    }

    $archiveBytes = Get-FileLengthOrZero $case.ArchivePath
    $decodedBytes = Get-FileLengthOrZero $case.DecodedPath
    $encodeSeconds = if ($null -ne $encode) { $encode.Seconds } else { 0 }
    $decodeSeconds = if ($null -ne $decode) { $decode.Seconds } else { 0 }
    $encodePeak = if ($null -ne $encode) { $encode.PeakMiB } else { 0 }
    $decodePeak = if ($null -ne $decode) { $decode.PeakMiB } else { 0 }

    $rowMap[$case.Key] = [pscustomobject][ordered]@{
        experiment_id = $ExperimentId
        variant = $variant
        repeat = 1
        case_order = $case.Order
        file = $case.File
        scope_kib = $case.Scope
        input_path = $case.InputRelative
        input_bytes = $case.InputBytes
        input_sha256 = $inputHash
        archive_path = $case.ArchiveRelative
        archive_bytes = $archiveBytes
        archive_sha256 = Get-Sha256 $case.ArchivePath
        decoded_path = $case.DecodedRelative
        decoded_bytes = $decodedBytes
        decoded_sha256 = Get-Sha256 $case.DecodedPath
        encode_seconds = $encodeSeconds
        decode_seconds = $decodeSeconds
        encode_peak_ram_mib = $encodePeak
        decode_peak_ram_mib = $decodePeak
        # Force the floating-point overload; PowerShell can otherwise select
        # the integer overload when one sampled value is an exact integer.
        peak_ram_mib = [Math]::Max([double]$encodePeak, [double]$decodePeak)
        codec_sha256 = $codecSha256
        parameters = $rowParameters
        encode_command = Format-RecordedCommand `
            -Executable $CodecPath -Arguments $encodeArguments `
            -InputPath $case.InputPath -OutputPath $case.ArchivePath
        decode_command = Format-RecordedCommand `
            -Executable $CodecPath -Arguments $decodeArguments `
            -InputPath $case.ArchivePath -OutputPath $case.DecodedPath
        encode_exit_code = if ($null -ne $encode) { $encode.ExitCode } else { -1 }
        decode_exit_code = if ($null -ne $decode) { $decode.ExitCode } else { -1 }
        started_at = $startedAt
        status = $status
        roundtrip = $roundtrip
        block_types = if ($null -ne $encode) {
            Get-RecordedBlockTypes $encode.StdoutPath
        } else {
            if ($Profile -eq 'r2') { 'UNKNOWN' } else { 'N/A' }
        }
        notes = $notes
    }

    Write-ResultsCsv
    Write-Host ("[{0}/{1}] {2} {3} KiB: {4}/{5}" -f `
        $case.Order, $cases.Count, $case.File, $case.Scope, $status, $roundtrip)
}

$orderedRows = @(Get-OrderedRows)
if ($orderedRows.Count -ne $cases.Count) {
    throw "Result row count is $($orderedRows.Count), expected $($cases.Count)"
}
$duplicates = $orderedRows |
    Group-Object file, scope_kib, variant, repeat |
    Where-Object Count -ne 1
if ($duplicates) {
    throw 'Duplicate or missing experiment result key detected'
}

Assert-CodecUnchanged 'experiment finalization'
$failed = @($orderedRows | Where-Object {
    $_.status -ne 'COMPLETE' -or $_.roundtrip -ne 'PASS'
})
Write-ResultsCsv
Write-ExperimentJson -State $(if ($failed.Count -eq 0) {
    $stateComplete
} else {
    $stateFailed
})

if ($failed.Count -ne 0) {
    throw "$($failed.Count) experiment cases failed; evidence retained at $packagePath"
}

Write-Host "Experiment complete: $packagePath"
