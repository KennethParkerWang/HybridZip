[CmdletBinding()]
param(
    [ValidateSet('K0')]
    [string]$Stage = 'K0',
    [string]$OutputRoot = '',
    [string[]]$Candidate = @(),
    [ValidateRange(1, 604800)]
    [int]$ProcessTimeoutSeconds = 600,
    [ValidateRange(64, 16384)]
    [int]$MemoryLimitMiB = 4096,
    [switch]$AuthorizeRuntimeExperiment,
    [switch]$ListOnly
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

$repoRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..'))
if ([string]::IsNullOrWhiteSpace($OutputRoot)) {
    $OutputRoot = Join-Path $repoRoot 'results\experiments'
}
$OutputRoot = [IO.Path]::GetFullPath($OutputRoot)
$experimentId = 'hybridzip-external-killtest-k0-20260829'
$packagePath = Join-Path $OutputRoot $experimentId
$smokeInput = Join-Path $packagePath 'smoke-input-1k.bin'
$resultPath = Join-Path $packagePath 'results.csv'
$zeroHash = '0' * 64
$gccBin = Split-Path -Parent (Get-Command g++.exe).Source
$runtimeBins = @($gccBin)

$candidates = [ordered]@{
    'kanzi-l7' = [ordered]@{
        kind = 'kanzi'
        executable = 'E:\MIXER\KU\external-core-killtest-20260829\kanzi-66a80678-build-cpu\kanzi_static.exe'
        archive_extension = '.knz'
        codec_sha256 = '4F515070CBA8FA9A992C2EDB6EC5A00009199BBB932A320F528848F38E7B38D8'
        parameters = 'Kanzi 2.5.3; level=7; block=1k; jobs=1; CPU-only'
    }
    'kanzi-l8' = [ordered]@{
        kind = 'kanzi'
        executable = 'E:\MIXER\KU\external-core-killtest-20260829\kanzi-66a80678-build-cpu\kanzi_static.exe'
        archive_extension = '.knz'
        codec_sha256 = '4F515070CBA8FA9A992C2EDB6EC5A00009199BBB932A320F528848F38E7B38D8'
        parameters = 'Kanzi 2.5.3; level=8; block=1k; jobs=1; CPU-only'
    }
    'kanzi-l9' = [ordered]@{
        kind = 'kanzi'
        executable = 'E:\MIXER\KU\external-core-killtest-20260829\kanzi-66a80678-build-cpu\kanzi_static.exe'
        archive_extension = '.knz'
        codec_sha256 = '4F515070CBA8FA9A992C2EDB6EC5A00009199BBB932A320F528848F38E7B38D8'
        parameters = 'Kanzi 2.5.3; level=9; block=1k; jobs=1; CPU-only'
    }
    'libbsc-e2' = [ordered]@{
        kind = 'bsc'
        executable = 'E:\MIXER\KU\external-core-killtest-20260829\libbsc-baffa62c\libbsc-baffa62c70b6ebbecc9af14ce550e965ea247680\build-cpu\bsc.exe'
        archive_extension = '.bsc'
        codec_sha256 = '4775DE34927D70780497CF6ADEDF6BDD9620701A2A2CB1F10EB5D1CE011CCCCA'
        parameters = 'libbsc 3.3.12; entropy=e2; block=1MiB minimum; threads=1; CPU-only'
    }
    'paq8px-l1' = [ordered]@{
        kind = 'paq'
        executable = 'F:\paq8px\experiment\build\paq8px.exe'
        archive_extension = '.paq8px216'
        codec_sha256 = 'F79343702F596A4FA6C7CC3E25F2FA9C05199EAF11F06655B065F687E5F42533'
        parameters = 'PAQ8px v216; level=-1; automatic detection'
    }
    'paq8px-l2' = [ordered]@{
        kind = 'paq'
        executable = 'F:\paq8px\experiment\build\paq8px.exe'
        archive_extension = '.paq8px216'
        codec_sha256 = 'F79343702F596A4FA6C7CC3E25F2FA9C05199EAF11F06655B065F687E5F42533'
        parameters = 'PAQ8px v216; level=-2; automatic detection'
    }
    'paq8px-l3' = [ordered]@{
        kind = 'paq'
        executable = 'F:\paq8px\experiment\build\paq8px.exe'
        archive_extension = '.paq8px216'
        codec_sha256 = 'F79343702F596A4FA6C7CC3E25F2FA9C05199EAF11F06655B065F687E5F42533'
        parameters = 'PAQ8px v216; level=-3; automatic detection'
    }
    'paq8px-l4' = [ordered]@{
        kind = 'paq'
        executable = 'F:\paq8px\experiment\build\paq8px.exe'
        archive_extension = '.paq8px216'
        codec_sha256 = 'F79343702F596A4FA6C7CC3E25F2FA9C05199EAF11F06655B065F687E5F42533'
        parameters = 'PAQ8px v216; level=-4; automatic detection'
    }
    'xz-9e' = [ordered]@{
        kind = 'xz'
        executable = 'D:\anaconda\Library\bin\xz.exe'
        archive_extension = '.xz'
        codec_sha256 = ''
        parameters = 'XZ Utils 5.6.4; level=-9e; threads=1'
    }
    'hybridzip-auto' = [ordered]@{
        kind = 'hybridzip'
        executable = Join-Path $repoRoot 'build\Release\hybridzip.exe'
        archive_extension = '.hz'
        codec_sha256 = '74FF260A939B01673667723D8351AAEDB679339610009ECB23C70E373B862D9F'
        parameters = 'HybridZip current Release; profile=r2; mode=auto'
    }
    'hybridzip-fast' = [ordered]@{
        kind = 'hybridzip'
        executable = Join-Path $repoRoot 'build\Release\hybridzip.exe'
        archive_extension = '.hz'
        codec_sha256 = '74FF260A939B01673667723D8351AAEDB679339610009ECB23C70E373B862D9F'
        parameters = 'HybridZip current Release; profile=r2; mode=fast; threads=1'
    }
}

$resultColumns = @(
    'experiment_id', 'stage', 'candidate', 'input_path', 'input_bytes',
    'input_sha256', 'archive_path', 'archive_bytes', 'archive_sha256',
    'decoded_path', 'decoded_bytes', 'decoded_sha256', 'encode_seconds',
    'decode_seconds', 'encode_peak_ram_mib', 'decode_peak_ram_mib',
    'peak_ram_mib', 'codec_sha256', 'parameters', 'encode_command',
    'decode_command', 'encode_exit_code', 'decode_exit_code', 'status',
    'roundtrip', 'notes'
)

function Get-Sha256([string]$Path) {
    if (-not (Test-Path -LiteralPath $Path -PathType Leaf)) { return $zeroHash }
    return (Get-FileHash -LiteralPath $Path -Algorithm SHA256).Hash.ToUpperInvariant()
}

function Write-Utf8NoBom([string]$Path, [string]$Content) {
    [IO.File]::WriteAllText($Path, $Content, (New-Object Text.UTF8Encoding($false)))
}

function Format-Args([string[]]$Arguments) {
    return [string]::Join(' ', @($Arguments | ForEach-Object {
        $value = [string]$_
        if ($value -match '[\s"]') { '"' + $value.Replace('"', '\"') + '"' }
        else { $value }
    }))
}

function Invoke-Measured([string]$Executable, [string[]]$Arguments,
                         [string]$WorkingDirectory, [string]$LogBase) {
    $stdoutPath = $LogBase + '.stdout.log'
    $stderrPath = $LogBase + '.stderr.log'
    if ((Test-Path -LiteralPath $stdoutPath) -or (Test-Path -LiteralPath $stderrPath)) {
        throw "Refusing to overwrite log artifacts: $LogBase"
    }
    $startInfo = New-Object Diagnostics.ProcessStartInfo
    $startInfo.FileName = $Executable
    $startInfo.Arguments = Format-Args $Arguments
    $startInfo.WorkingDirectory = $WorkingDirectory
    $startInfo.UseShellExecute = $false
    $startInfo.CreateNoWindow = $true
    $startInfo.RedirectStandardOutput = $true
    $startInfo.RedirectStandardError = $true
    $startInfo.EnvironmentVariables['PATH'] = ([string]::Join(';', $runtimeBins + @($env:PATH)))
    $process = New-Object Diagnostics.Process
    $process.StartInfo = $startInfo
    $watch = [Diagnostics.Stopwatch]::StartNew()
    [void]$process.Start()
    $stdoutTask = $process.StandardOutput.ReadToEndAsync()
    $stderrTask = $process.StandardError.ReadToEndAsync()
    [Int64]$peakBytes = 0
    $termination = ''
    while (-not $process.HasExited) {
        try { $process.Refresh(); $peakBytes = [Math]::Max($peakBytes, [Int64]$process.PeakWorkingSet64) } catch { }
        if ($peakBytes -gt ([Int64]$MemoryLimitMiB * 1MB)) {
            $termination = 'MEMORY_LIMIT'; $process.Kill(); break
        }
        if ($watch.Elapsed.TotalSeconds -ge $ProcessTimeoutSeconds) {
            $termination = 'TIMEOUT'; $process.Kill(); break
        }
        Start-Sleep -Milliseconds 25
    }
    $process.WaitForExit(); $watch.Stop()
    try { $process.Refresh(); $peakBytes = [Math]::Max($peakBytes, [Int64]$process.PeakWorkingSet64) } catch { }
    Write-Utf8NoBom $stdoutPath $stdoutTask.Result
    Write-Utf8NoBom $stderrPath $stderrTask.Result
    $exitCode = if ([string]::IsNullOrWhiteSpace($termination)) { [int]$process.ExitCode } else { -2 }
    $process.Dispose()
    return [pscustomobject]@{
        exit_code = $exitCode
        termination = $termination
        seconds = $watch.Elapsed.TotalSeconds
        peak_mib = [double]$peakBytes / 1MB
    }
}

function Write-SmokeInput([string]$Path) {
    if (Test-Path -LiteralPath $Path) { throw "Refusing to overwrite smoke input: $Path" }
    $bytes = New-Object byte[] 1024
    for ($i = 0; $i -lt $bytes.Length; $i++) { $bytes[$i] = [byte](($i * 73 + 19) -band 255) }
    [IO.File]::WriteAllBytes($Path, $bytes)
}

function Invoke-Candidate([string]$Name, [hashtable]$Spec) {
    $candidateDir = Join-Path $packagePath $Name
    New-Item -ItemType Directory -Path (Join-Path $candidateDir 'logs') | Out-Null
    $inputPath = Join-Path $candidateDir 'input-1k.bin'
    $archivePath = Join-Path $candidateDir ('archive' + $Spec.archive_extension)
    $decodedPath = Join-Path $candidateDir 'decoded-1k.bin'
    Copy-Item -LiteralPath $smokeInput -Destination $inputPath
    $inputHash = Get-Sha256 $inputPath
    $kind = [string]$Spec.kind
    $encodeArgs = @()
    $decodeArgs = @()
    $decodeStage = ''
    if ($kind -eq 'kanzi') {
        $level = if ($Name -eq 'kanzi-l7') { '7' } elseif ($Name -eq 'kanzi-l8') { '8' } else { '9' }
        $encodeArgs = @('-c', "-i=$inputPath", "-o=$archivePath", '-f', '-j=1', '-b=1k', "-l=$level")
        $decodeArgs = @('-d', "-i=$archivePath", "-o=$decodedPath", '-f', '-j=1')
    }
    elseif ($kind -eq 'bsc') {
        $encodeArgs = @('e', $inputPath, $archivePath, '-b1', '-e2', '-T1')
        $decodeArgs = @('d', $archivePath, $decodedPath, '-T1')
    }
    elseif ($kind -eq 'paq') {
        $level = $Name.Substring($Name.Length - 2, 2).TrimStart('l')
        $encodeArgs = @('-' + $level, $inputPath, $archivePath)
        $decodeArgs = @('-d', $archivePath, $decodedPath)
    }
    elseif ($kind -eq 'xz') {
        $stageInput = Join-Path $candidateDir 'xz-input.raw'
        Copy-Item -LiteralPath $inputPath -Destination $stageInput
        $encodeArgs = @('-9e', '-T1', '-f', $stageInput)
        $decodeStage = [IO.Path]::ChangeExtension($stageInput, '.raw')
        $decodeArgs = @('-d', '-f', ($stageInput + '.xz'))
    }
    elseif ($kind -eq 'hybridzip') {
        $mode = if ($Name -eq 'hybridzip-auto') { 'auto' } else { 'fast' }
        $encodeArgs = @('c', '--profile=r2', "--r2-mode=$mode", $inputPath, $archivePath)
        if ($mode -eq 'fast') { $encodeArgs += '--threads=1' }
        $decodeArgs = @('d', $archivePath, $decodedPath)
    }
    else { throw "Unknown candidate kind: $kind" }
    $encodeCommand = '"' + $Spec.executable + '" ' + (Format-Args $encodeArgs)
    $decodeCommand = '"' + $Spec.executable + '" ' + (Format-Args $decodeArgs)
    $row = [ordered]@{
        experiment_id = $experimentId; stage = $Stage; candidate = $Name
        input_path = "${Name}/input-1k.bin"; input_bytes = 1024; input_sha256 = $inputHash
        archive_path = "${Name}/archive$($Spec.archive_extension)"; archive_bytes = 0; archive_sha256 = $zeroHash
        decoded_path = "${Name}/decoded-1k.bin"; decoded_bytes = 0; decoded_sha256 = $zeroHash
        encode_seconds = 0.0; decode_seconds = 0.0; encode_peak_ram_mib = 0.0; decode_peak_ram_mib = 0.0
        peak_ram_mib = 0.0; codec_sha256 = [string]$Spec.codec_sha256; parameters = [string]$Spec.parameters
        encode_command = $encodeCommand; decode_command = $decodeCommand; encode_exit_code = -3; decode_exit_code = -3
        status = 'FAILED'; roundtrip = 'NOT_VERIFIED'; notes = ''
    }
    try {
        if (-not (Test-Path -LiteralPath $Spec.executable -PathType Leaf)) { throw "Executable missing: $($Spec.executable)" }
        if ([string]$Spec.codec_sha256 -ne '' -and (Get-Sha256 $Spec.executable) -cne [string]$Spec.codec_sha256) { throw 'Executable SHA-256 differs from pinned identity' }
        $encode = Invoke-Measured $Spec.executable $encodeArgs $candidateDir (Join-Path $candidateDir 'logs/encode')
        $row.encode_seconds = $encode.seconds; $row.encode_peak_ram_mib = $encode.peak_mib; $row.encode_exit_code = $encode.exit_code
        if ($kind -eq 'xz') {
            $xzArchive = $stageInput + '.xz'
            if (Test-Path -LiteralPath $xzArchive) { Move-Item -LiteralPath $xzArchive -Destination $archivePath }
        }
        if ($encode.exit_code -ne 0 -or -not (Test-Path -LiteralPath $archivePath -PathType Leaf)) { throw "Encode failed ($($encode.exit_code), $($encode.termination))" }
        $decode = Invoke-Measured $Spec.executable $decodeArgs $candidateDir (Join-Path $candidateDir 'logs/decode')
        $row.decode_seconds = $decode.seconds; $row.decode_peak_ram_mib = $decode.peak_mib; $row.decode_exit_code = $decode.exit_code
        if ($kind -eq 'xz' -and (Test-Path -LiteralPath $decodeStage)) { Move-Item -LiteralPath $decodeStage -Destination $decodedPath }
        if ($decode.exit_code -ne 0 -or -not (Test-Path -LiteralPath $decodedPath -PathType Leaf)) { throw "Decode failed ($($decode.exit_code), $($decode.termination))" }
        $row.archive_bytes = (Get-Item -LiteralPath $archivePath).Length; $row.archive_sha256 = Get-Sha256 $archivePath
        $row.decoded_bytes = (Get-Item -LiteralPath $decodedPath).Length; $row.decoded_sha256 = Get-Sha256 $decodedPath
        if ($row.decoded_bytes -ne 1024 -or $row.decoded_sha256 -cne $inputHash) { throw 'Decoded bytes are not byte-exact' }
        $row.status = 'COMPLETE'; $row.roundtrip = 'PASS'; $row.notes = 'K0 1 KiB byte-exact smoke passed.'
    }
    catch { $row.notes = $_.Exception.Message }
    $row.peak_ram_mib = [Math]::Max([double]$row.encode_peak_ram_mib, [double]$row.decode_peak_ram_mib)
    return [pscustomobject]$row
}

if ($Candidate.Count -eq 0) { $selectedNames = @($candidates.Keys) }
else {
    $selectedNames = @($Candidate | ForEach-Object { ([string]$_).Split(',') } | Where-Object { $_ })
    foreach ($name in $selectedNames) { if (-not $candidates.Contains($name)) { throw "Unknown candidate: $name" } }
}

if ($ListOnly -or -not $AuthorizeRuntimeExperiment) {
    [pscustomobject][ordered]@{
        stage = $Stage; runtime_started = $false; output_package = $packagePath
        candidates = @($selectedNames); executable_identities = @($selectedNames | ForEach-Object {
            [pscustomobject]@{ name = $_; path = $candidates[$_].executable; sha256 = $candidates[$_].codec_sha256; parameters = $candidates[$_].parameters }
        })
    } | ConvertTo-Json -Depth 6
    if (-not $AuthorizeRuntimeExperiment) { return }
}

if (Test-Path -LiteralPath $packagePath) { throw "Refusing to overwrite existing output package: $packagePath" }
New-Item -ItemType Directory -Path $packagePath | Out-Null
Write-SmokeInput $smokeInput
$metadata = [ordered]@{
    schema_version = 1; experiment_id = $experimentId; stage = $Stage; state = 'testing'
    created_at = [DateTimeOffset]::Now.ToString('o'); input_bytes = 1024; input_sha256 = Get-Sha256 $smokeInput
    candidates = $selectedNames; runtime_path = [string]::Join(';', $runtimeBins)
    notes = 'External candidates only; no HybridZip source or archive format changes.'
}
Write-Utf8NoBom (Join-Path $packagePath 'experiment.json') ($metadata | ConvertTo-Json -Depth 6)
$rows = New-Object System.Collections.Generic.List[object]
foreach ($name in $selectedNames) {
    Write-Host ("[{0}/{1}] {2}" -f ($rows.Count + 1), $selectedNames.Count, $name)
    $rows.Add((Invoke-Candidate $name $candidates[$name]))
    Write-Utf8NoBom $resultPath ((@($rows) | Select-Object $resultColumns | ConvertTo-Csv -NoTypeInformation) -join "`r`n")
}
$failed = @($rows | Where-Object { $_.status -ne 'COMPLETE' -or $_.roundtrip -ne 'PASS' })
$metadata.state = if ($failed.Count -eq 0) { 'complete' } else { 'failed' }
$metadata.completed_at = [DateTimeOffset]::Now.ToString('o'); $metadata.failed_candidates = @($failed | ForEach-Object candidate)
Write-Utf8NoBom (Join-Path $packagePath 'experiment.json') ($metadata | ConvertTo-Json -Depth 6)
if ($failed.Count -ne 0) { throw "$($failed.Count) external K0 candidates failed; inspect results.csv and logs." }
Write-Host "External K0 smoke complete: $packagePath"
