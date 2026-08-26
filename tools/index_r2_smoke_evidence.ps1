[CmdletBinding()]
param(
    [string]$SmokeRoot = '',
    [string]$OutputPath = '',
    [string]$CodecSha256 = ''
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

if ([string]::IsNullOrWhiteSpace($SmokeRoot)) {
    $SmokeRoot = Join-Path $PSScriptRoot '..\results\smoke'
}
if ([string]::IsNullOrWhiteSpace($OutputPath)) {
    $OutputPath = Join-Path $PSScriptRoot '..\results\analysis\r2-smoke-evidence-index'
}
$SmokeRoot = [IO.Path]::GetFullPath($SmokeRoot)
$OutputPath = [IO.Path]::GetFullPath($OutputPath)
if (Test-Path -LiteralPath $OutputPath) {
    throw "Output already exists: $OutputPath"
}

$records = @()
$modeNames = @(
    'Stored', 'PredictiveV1', 'Zstd', 'Fse', 'Lzma', 'DonorMatchPredictive',
    'BwtZstd', 'BwtMtfZstd', 'BwtRltZstd', 'X86BcjZstd', 'ShuffleZstd',
    'BitshuffleZstd', 'DeltaZstd', 'FastPfor', 'Rans', 'Bcj2Zstd',
    'RecordTransposeZstd', 'JpegLs', 'FlacResidual', 'BrotliText',
    'CmixWordDictionaryZstd', 'NeuralLstm', 'SharedNeuralLstm',
    'LstmCompress', 'DeltaOfDeltaZstd', 'BgptSharedPrior',
    'JaxCompressPortable', 'Ppmd7', 'Ppmd8', 'Zpaq', 'Ctw',
    'Paq8pxApmPredictive', 'Paq8pxRecordModel', 'Paq8pxLinearPrediction',
    'Paq8pxSimilarity', 'Paq8pxSimilaritySse', 'Paq8pxGenericSse',
    'Paq8pxDetectedSse', 'Wavpack', 'Lz4', 'KanziAns', 'LmicArithmetic',
    'DeltaBinaryPackedZstd'
)
function Convert-ModeValue($Value) {
    if ($Value -is [int] -or $Value -is [long] -or $Value -is [double]) {
        return [int]$Value
    }
    $text = [string]$Value
    $numeric = 0
    if ([int]::TryParse($text, [ref]$numeric)) { return $numeric }
    $index = [Array]::IndexOf($modeNames, $text)
    if ($index -ge 0) { return $index }
    return $null
}
function Get-JsonProperty($Object, [string]$Name) {
    $property = $Object.PSObject.Properties[$Name]
    if ($null -eq $property) { return $null }
    return $property.Value
}
foreach ($path in Get-ChildItem -LiteralPath $SmokeRoot -Recurse -Filter 'verification.json' -File) {
    try {
        $parsed = Get-Content -LiteralPath $path.FullName -Raw | ConvertFrom-Json
    } catch {
        continue
    }
    $items = if ($parsed -is [System.Array]) { @($parsed) } else { @($parsed) }
    foreach ($item in $items) {
        $mode = $null
        $modeValue = Get-JsonProperty $item 'mode'
        if ($null -ne $modeValue) { $mode = Convert-ModeValue $modeValue }
        if ($null -eq $mode -or $mode -lt 0 -or $mode -gt 42) { continue }
        $inputValue = Get-JsonProperty $item 'input_bytes'
        $decodedValue = Get-JsonProperty $item 'decoded_bytes'
        $exactValue = Get-JsonProperty $item 'byte_exact'
        $roundtripValue = Get-JsonProperty $item 'roundtrip'
        $inputBytes = if ($null -ne $inputValue) { [int64]$inputValue } else { 0 }
        $decodedBytes = if ($null -ne $decodedValue) { [int64]$decodedValue } else { 0 }
        $exact = if ($null -ne $exactValue) { [bool]$exactValue } `
            elseif ($null -ne $roundtripValue) { [string]$roundtripValue -eq 'PASS' } `
            else { $false }
        $hashValue = Get-JsonProperty $item 'codec_sha256'
        $archiveValue = Get-JsonProperty $item 'archive_bytes'
        $bpbValue = Get-JsonProperty $item 'bpb'
        $inputHashValue = Get-JsonProperty $item 'input_sha256'
        $decodedHashValue = Get-JsonProperty $item 'decoded_sha256'
        $hash = if ($null -ne $hashValue) { [string]$hashValue } else { '' }
        if ($inputBytes -ne 1024 -or $decodedBytes -ne 1024 -or -not $exact) { continue }
        if (-not [string]::IsNullOrWhiteSpace($CodecSha256) -and
            $hash.ToUpperInvariant() -ne $CodecSha256.ToUpperInvariant()) { continue }
        $records += [pscustomobject][ordered]@{
            mode = $mode
            mode_name = $modeNames[$mode]
            input_bytes = $inputBytes
            archive_bytes = if ($null -ne $archiveValue) { [int64]$archiveValue } else { 0 }
            bpb = if ($null -ne $bpbValue) { [double]$bpbValue } else { $null }
            codec_sha256 = $hash.ToUpperInvariant()
            input_sha256 = if ($null -ne $inputHashValue) { [string]$inputHashValue } else { '' }
            decoded_sha256 = if ($null -ne $decodedHashValue) { [string]$decodedHashValue } else { '' }
            evidence_path = $path.FullName
            evidence_mtime = $path.LastWriteTimeUtc.ToString('o')
        }
    }
}

$latest = @($records | Sort-Object mode, evidence_mtime -Descending |
    Group-Object mode | ForEach-Object { $_.Group | Select-Object -First 1 })
$expected = 0..42
$covered = @($latest | ForEach-Object mode)
$missing = @($expected | Where-Object { $_ -notin $covered })
$registry = foreach ($mode in $expected) {
    $record = @($latest | Where-Object mode -eq $mode | Select-Object -First 1)
    if ($record.Count -eq 1) {
        [pscustomobject][ordered]@{
            mode = $mode
            mode_name = $modeNames[$mode]
            status = 'PASS'
            input_bytes = $record[0].input_bytes
            archive_bytes = $record[0].archive_bytes
            bpb = $record[0].bpb
            codec_sha256 = $record[0].codec_sha256
            evidence_path = $record[0].evidence_path
        }
    } else {
        [pscustomobject][ordered]@{
            mode = $mode
            mode_name = $modeNames[$mode]
            status = 'MISSING_CURRENT_HASH_EVIDENCE'
            input_bytes = ''
            archive_bytes = ''
            bpb = ''
            codec_sha256 = ''
            evidence_path = ''
        }
    }
}

New-Item -ItemType Directory -Path $OutputPath | Out-Null
function Write-NoBom([string]$Path, [string]$Text) {
    [IO.File]::WriteAllText($Path, $Text, (New-Object Text.UTF8Encoding($false)))
}
Write-NoBom (Join-Path $OutputPath 'latest_by_mode.tsv') `
    (($latest | Sort-Object mode | ConvertTo-Csv -Delimiter "`t" -NoTypeInformation) -join "`r`n")
Write-NoBom (Join-Path $OutputPath 'missing_modes.tsv') `
    (($missing | ForEach-Object { [pscustomobject]@{ mode = $_ } } |
        ConvertTo-Csv -Delimiter "`t" -NoTypeInformation) -join "`r`n")
Write-NoBom (Join-Path $OutputPath 'mode_registry.tsv') `
    (($registry | ConvertTo-Csv -Delimiter "`t" -NoTypeInformation) -join "`r`n")

$hashFilterLine = if ([string]::IsNullOrWhiteSpace($CodecSha256)) {
    '- codec hash filter: none'
} else {
    '- codec hash filter: `' + $CodecSha256.ToUpperInvariant() + '`'
}
$readme = @"
# R2 Smoke Evidence Index

This index scans existing verification.json files only. It accepts one-byte-
exact 1 KiB record per HZ02 mode and does not execute the codec.

- candidate modes: 0..42 (43 total)
- qualifying records found: $($records.Count)
- unique modes covered: $($latest.Count)
- missing modes: $(if ($missing.Count -eq 0) { 'none' } else { $missing -join ', ' })
$hashFilterLine

`latest_by_mode.tsv` keeps the newest qualifying record per mode. Historical
records and rebuild duplicates are intentionally reduced rather than treated
as independent final evidence. `mode_registry.tsv` is a fixed 43-row view
that includes every mode name and explicitly marks missing current-hash
evidence; it does not infer success from router-only records.
"@
Write-NoBom (Join-Path $OutputPath 'README.md') $readme

Write-Output "created=$OutputPath"
Write-Output "qualifying_records=$($records.Count) unique_modes=$($latest.Count) missing=$($missing -join ',')"
