[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)]
    [string]$ManifestPath,
    [Parameter(Mandatory = $true)]
    [string]$OutputPath,
    [string]$CodecPath = '',
    # Keep this order equal to the decoder-visible BlockMode IDs.
    [ValidateSet('auto', 'stored', 'predictive', 'zstd', 'fse', 'lzma',
        'donor-match', 'bwt-zstd', 'bwt-mtf-zstd', 'bwt-rlt-zstd', 'x86-bcj-zstd',
        'shuffle-zstd', 'bitshuffle-zstd', 'delta-zstd', 'fastpfor', 'rans',
        'bcj2-zstd', 'record-transpose-zstd', 'jpegls', 'flac-residual',
        'brotli-text', 'cmix-word-zstd', 'neural-lstm', 'shared-neural-lstm',
        'lstm-compress', 'delta-of-delta-zstd', 'bgpt-shared-prior',
        'jax-compress-portable', 'ppmd7', 'ppmd8', 'zpaq', 'ctw',
        'paq8px-apm', 'paq8px-record-model', 'paq8px-linear-prediction',
        'paq8px-similarity', 'paq8px-similarity-sse', 'paq8px-generic-sse',
        'paq8px-detected-sse', 'wavpack', 'lz4', 'kanzi-ans', 'lmic-arithmetic',
        'delta-binary-packed-zstd')]
    [string]$R2Mode = 'auto',
    [ValidateSet(32)]
    [int]$ScopeKiB = 32,
    [ValidateRange(1, 604800)]
    [int]$TimeoutSeconds = 3600
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'
$scriptRoot = Split-Path -Parent $MyInvocation.MyCommand.Path
if ([string]::IsNullOrWhiteSpace($CodecPath)) {
    $CodecPath = Join-Path $scriptRoot '..\build\Release\hybridzip.exe'
}
$ManifestPath = [IO.Path]::GetFullPath($ManifestPath)
$OutputPath = [IO.Path]::GetFullPath($OutputPath)
$CodecPath = [IO.Path]::GetFullPath($CodecPath)
if (-not (Test-Path -LiteralPath $ManifestPath -PathType Leaf)) {
    throw "Manifest not found: $ManifestPath"
}
if (-not (Test-Path -LiteralPath $CodecPath -PathType Leaf)) {
    throw "Codec not found: $CodecPath"
}
if (Test-Path -LiteralPath $OutputPath) {
    throw "Refusing to overwrite existing output: $OutputPath"
}

function Get-Sha256([string]$Path) {
    (Get-FileHash -LiteralPath $Path -Algorithm SHA256).Hash.ToUpperInvariant()
}
function Write-NoBom([string]$Path, [string]$Text) {
    [IO.File]::WriteAllText($Path, $Text, (New-Object Text.UTF8Encoding($false)))
}
function Quote-Argument([string]$Value) {
    if ($Value -notmatch '[\s"]') { return $Value }
    return '"' + $Value.Replace('"', '\"') + '"'
}
function Invoke-Measured {
    param(
        [string]$Executable,
        [string[]]$Arguments,
        [string]$StdoutPath,
        [string]$StderrPath
    )
    $argumentList = @($Arguments | ForEach-Object { Quote-Argument $_ })
    $watch = [Diagnostics.Stopwatch]::StartNew()
    $process = Start-Process -FilePath $Executable `
        -ArgumentList $argumentList `
        -RedirectStandardOutput $StdoutPath `
        -RedirectStandardError $StderrPath `
        -WindowStyle Hidden -PassThru
    $handle = $process.Handle
    $peakBytes = 0L
    $timedOut = $false
    while (-not $process.HasExited) {
        try {
            $process.Refresh()
            $peakBytes = [Math]::Max($peakBytes, $process.PeakWorkingSet64)
        } catch { }
        if ($watch.Elapsed.TotalSeconds -gt $TimeoutSeconds) {
            $timedOut = $true
            $process.Kill()
            break
        }
        Start-Sleep -Milliseconds 10
    }
    $process.WaitForExit()
    $watch.Stop()
    try {
        $process.Refresh()
        $peakBytes = [Math]::Max($peakBytes, $process.PeakWorkingSet64)
    } catch { }
    if ($timedOut) { throw "Process timed out after $TimeoutSeconds seconds" }
    if ($null -eq $process.ExitCode) {
        throw "Process exit code unavailable (handle $handle)"
    }
    [pscustomobject]@{
        ExitCode = $process.ExitCode
        Seconds = $watch.Elapsed.TotalSeconds
        PeakMiB = [double]$peakBytes / 1MB
    }
}
function Write-Prefix([string]$Source, [string]$Destination, [int]$Bytes) {
    $input = [IO.File]::OpenRead($Source)
    try {
        if ($input.Length -lt $Bytes) { throw "Source shorter than $Bytes bytes: $Source" }
        $buffer = New-Object byte[] $Bytes
        $offset = 0
        while ($offset -lt $Bytes) {
            $read = $input.Read($buffer, $offset, $Bytes - $offset)
            if ($read -eq 0) { throw "Unexpected end of source: $Source" }
            $offset += $read
        }
        [IO.File]::WriteAllBytes($Destination, $buffer)
    } finally { $input.Dispose() }
}
function Get-BlockTypes([string]$StdoutPath) {
    if (-not (Test-Path -LiteralPath $StdoutPath -PathType Leaf)) { return 'UNKNOWN' }
    $text = [IO.File]::ReadAllText($StdoutPath)
    $match = [Regex]::Match($text, 'blocks\(([^)]*)\)')
    if ($match.Success) { return 'blocks(' + $match.Groups[1].Value + ')' }
    return 'UNKNOWN'
}

$manifestRows = @(Import-Csv -LiteralPath $ManifestPath -Delimiter "`t")
if ($manifestRows.Count -eq 0) { throw 'Manifest has no rows' }
$ids = @($manifestRows | ForEach-Object id)
if (@($ids | Sort-Object -Unique).Count -ne $ids.Count) {
    throw 'Manifest contains duplicate ids'
}
$scopeBytes = $ScopeKiB * 1024
foreach ($entry in $manifestRows) {
    foreach ($property in @('id', 'family', 'source_path', 'source_sha256', 'selection', 'license', 'provenance')) {
        if ([string]::IsNullOrWhiteSpace([string]$entry.$property)) {
            throw "Manifest field is empty: $property/$($entry.id)"
        }
    }
    if (-not (Test-Path -LiteralPath $entry.source_path -PathType Leaf)) {
        throw "Manifest source missing: $($entry.source_path)"
    }
    if (-not (Test-Path -LiteralPath $entry.provenance -PathType Leaf)) {
        throw "Manifest provenance missing: $($entry.provenance)"
    }
    $item = Get-Item -LiteralPath $entry.source_path
    if ($item.Length -lt $scopeBytes) { throw "Manifest source is shorter than scope: $($entry.id)" }
    if ([int64]$entry.source_bytes -ne $item.Length) { throw "Manifest source length changed: $($entry.id)" }
    if ((Get-Sha256 $entry.source_path) -ne $entry.source_sha256.ToUpperInvariant()) {
        throw "Manifest source hash changed: $($entry.id)"
    }
}

$codecHash = Get-Sha256 $CodecPath
$manifestHash = Get-Sha256 $ManifestPath
$experimentId = (Split-Path -Leaf $OutputPath)
New-Item -ItemType Directory -Path $OutputPath | Out-Null
foreach ($kind in @('inputs', 'archives', 'decoded', 'logs')) {
    New-Item -ItemType Directory -Path (Join-Path $OutputPath $kind) | Out-Null
}
$metadata = [ordered]@{
    schema_version = 1
    experiment_id = $experimentId
    state = 'testing'
    dataset_name = 'KU family-specific raw corpus'
    manifest_path = $ManifestPath
    manifest_sha256 = $manifestHash
    files = @($ids)
    scope_kib = $ScopeKiB
    mode = $R2Mode
    codec_path = $CodecPath
    codec_sha256 = $codecHash
    selection_policy = 'first 32768 bytes from each provenance-tracked donor source'
    notes = '32 KiB only; source files remain in KU and are not modified.'
}
Write-NoBom (Join-Path $OutputPath 'experiment.json') ($metadata | ConvertTo-Json -Depth 5)

$rows = @()
$caseOrder = 0
foreach ($entry in $manifestRows) {
    ++$caseOrder
    $inputPath = Join-Path $OutputPath ("inputs\$($entry.id).bin")
    $archivePath = Join-Path $OutputPath ("archives\$($entry.id).hz2")
    $decodedPath = Join-Path $OutputPath ("decoded\$($entry.id).decoded")
    $logBase = Join-Path $OutputPath ("logs\$($entry.id)")
    $status = 'FAILED'
    $roundtrip = 'NOT_VERIFIED'
    $notes = ''
    $encode = $null
    $decode = $null
    $inputHash = ''
    try {
        Write-Prefix $entry.source_path $inputPath $scopeBytes
        $inputHash = Get-Sha256 $inputPath
        $encodeArgs = @('c', '--profile=r2', "--r2-mode=$R2Mode", $inputPath, $archivePath)
        $encode = Invoke-Measured $CodecPath $encodeArgs "$logBase.encode.stdout.log" "$logBase.encode.stderr.log"
        if ($encode.ExitCode -ne 0) { throw "encode exit $($encode.ExitCode)" }
        $decodeArgs = @('d', $archivePath, $decodedPath)
        $decode = Invoke-Measured $CodecPath $decodeArgs "$logBase.decode.stdout.log" "$logBase.decode.stderr.log"
        if ($decode.ExitCode -ne 0) { throw "decode exit $($decode.ExitCode)" }
        $decodedHash = Get-Sha256 $decodedPath
        if ((Get-Item $decodedPath).Length -ne $scopeBytes -or $decodedHash -ne $inputHash) {
            throw 'decoded output is not byte-exact'
        }
        $status = 'COMPLETE'
        $roundtrip = 'PASS'
        $notes = 'Input, archive, and decoded artifact SHA-256 recorded; byte-exact roundtrip verified.'
    } catch {
        $notes = $_.Exception.Message
    }
    $archiveBytes = if (Test-Path $archivePath) { (Get-Item $archivePath).Length } else { 0 }
    $decodedBytes = if (Test-Path $decodedPath) { (Get-Item $decodedPath).Length } else { 0 }
    $row = [pscustomobject][ordered]@{
        experiment_id = $experimentId
        variant = "r2-$R2Mode"
        case_order = $caseOrder
        id = $entry.id
        family = $entry.family
        scope_kib = $ScopeKiB
        source_path = $entry.source_path
        source_sha256 = $entry.source_sha256
        input_path = "inputs/$($entry.id).bin"
        input_bytes = $scopeBytes
        input_sha256 = $inputHash
        archive_path = "archives/$($entry.id).hz2"
        archive_bytes = $archiveBytes
        archive_sha256 = if (Test-Path $archivePath) { Get-Sha256 $archivePath } else { '' }
        decoded_path = "decoded/$($entry.id).decoded"
        decoded_bytes = $decodedBytes
        decoded_sha256 = if (Test-Path $decodedPath) { Get-Sha256 $decodedPath } else { '' }
        encode_seconds = if ($null -ne $encode) { $encode.Seconds } else { 0 }
        decode_seconds = if ($null -ne $decode) { $decode.Seconds } else { 0 }
        peak_ram_mib = if ($null -ne $encode -and $null -ne $decode) {
            [Math]::Max([double]$encode.PeakMiB, [double]$decode.PeakMiB)
        } else { 0 }
        codec_sha256 = $codecHash
        encode_exit_code = if ($null -ne $encode) { $encode.ExitCode } else { -1 }
        decode_exit_code = if ($null -ne $decode) { $decode.ExitCode } else { -1 }
        status = $status
        roundtrip = $roundtrip
        block_types = Get-BlockTypes "$logBase.encode.stdout.log"
        notes = $notes
    }
    $rows += $row
    Write-NoBom (Join-Path $OutputPath 'results.csv') (($rows | ConvertTo-Csv -NoTypeInformation) -join "`r`n")
    Write-Host ("[{0}/{1}] {2} {3}: {4}/{5}" -f $caseOrder, $manifestRows.Count, $entry.id, $entry.family, $status, $roundtrip)
}

$failed = @($rows | Where-Object { $_.status -ne 'COMPLETE' -or $_.roundtrip -ne 'PASS' })
$metadata.state = if ($failed.Count -eq 0) { 'complete' } else { 'failed' }
Write-NoBom (Join-Path $OutputPath 'experiment.json') ($metadata | ConvertTo-Json -Depth 5)
if ($failed.Count -ne 0) { throw "$($failed.Count) family experiment cases failed" }
Write-Host "Experiment complete: $OutputPath"
