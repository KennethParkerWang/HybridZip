[CmdletBinding()]
param(
    [string]$D20Path = '',
    [string]$D22Path = '',
    [string]$OutputPath = ''
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

if ([string]::IsNullOrWhiteSpace($D20Path)) {
    $D20Path = Join-Path $PSScriptRoot '..\results\experiments\hybridzip-r2-donor-match-silesia-32k-20260821-d20'
}
if ([string]::IsNullOrWhiteSpace($D22Path)) {
    $D22Path = Join-Path $PSScriptRoot '..\results\experiments\hybridzip-r2-donor-match-silesia-32k-20260821-d22'
}
if ([string]::IsNullOrWhiteSpace($OutputPath)) {
    $OutputPath = Join-Path $PSScriptRoot '..\results\analysis\r2-silesia-32k-currenthash-donor-match-20260821-d23'
}

$D20Path = [IO.Path]::GetFullPath($D20Path)
$D22Path = [IO.Path]::GetFullPath($D22Path)
$OutputPath = [IO.Path]::GetFullPath($OutputPath)
if (Test-Path -LiteralPath $OutputPath) {
    throw "Output already exists: $OutputPath"
}

$rows = @(
    (Import-Csv (Join-Path $D20Path 'results.csv'))
    (Import-Csv (Join-Path $D22Path 'results.csv'))
)
$order = @('dickens', 'mozilla', 'mr', 'nci', 'ooffice', 'osdb',
    'reymont', 'samba', 'sao', 'webster', 'x-ray', 'xml')
$rows = @($rows | Sort-Object @{ Expression = { [Array]::IndexOf($order, $_.file) } })
if ($rows.Count -ne 12) {
    throw "Expected 12 source rows, found $($rows.Count)"
}
$codecHashes = @($rows | ForEach-Object codec_sha256 | Sort-Object -Unique)
if ($codecHashes.Count -ne 1) {
    throw 'Source packages use different codec hashes'
}

$checks = @()
foreach ($row in $rows) {
    if ($row.status -ne 'COMPLETE' -or $row.roundtrip -ne 'PASS') {
        throw "Row is not COMPLETE/PASS: $($row.file)"
    }
    $root = if ($row.experiment_id -eq 'hybridzip-r2-donor-match-silesia-32k-20260821-d20') {
        $D20Path
    } else {
        $D22Path
    }
    $inputPath = Join-Path $root ($row.input_path -replace '/', '\')
    $archivePath = Join-Path $root ($row.archive_path -replace '/', '\')
    $decodedPath = Join-Path $root ($row.decoded_path -replace '/', '\')
    foreach ($path in @($inputPath, $archivePath, $decodedPath)) {
        if (-not (Test-Path -LiteralPath $path -PathType Leaf)) {
            throw "Missing artifact: $path"
        }
    }
    $inputHash = (Get-FileHash $inputPath -Algorithm SHA256).Hash.ToUpperInvariant()
    $archiveHash = (Get-FileHash $archivePath -Algorithm SHA256).Hash.ToUpperInvariant()
    $decodedHash = (Get-FileHash $decodedPath -Algorithm SHA256).Hash.ToUpperInvariant()
    if ($inputHash -ne $row.input_sha256 -or
        $archiveHash -ne $row.archive_sha256 -or
        $decodedHash -ne $row.decoded_sha256 -or
        $inputHash -ne $decodedHash) {
        throw "SHA-256 mismatch: $($row.file)"
    }
    if ((Get-Item $inputPath).Length -ne 32768 -or
        (Get-Item $decodedPath).Length -ne 32768 -or
        (Get-Item $archivePath).Length -ne [int64]$row.archive_bytes) {
        throw "Byte-length mismatch: $($row.file)"
    }
    $checks += $row.file
}

$modeRows = foreach ($row in $rows) {
    [pscustomobject][ordered]@{
        mode = 'donor-match'
        variant = $row.variant
        file = $row.file
        scope_kib = [int]$row.scope_kib
        input_bytes = [int64]$row.input_bytes
        archive_bytes = [int64]$row.archive_bytes
        ratio = [double]$row.archive_bytes / [double]$row.input_bytes
        bpb = [double]$row.archive_bytes * 8.0 / [double]$row.input_bytes
        encode_seconds = [double]$row.encode_seconds
        decode_seconds = [double]$row.decode_seconds
        peak_ram_mib = [double]$row.peak_ram_mib
        roundtrip = $row.roundtrip
        block_types = $row.block_types
        codec_sha256 = $row.codec_sha256
        package = $row.experiment_id
    }
}

$inputTotal = ($rows | Measure-Object -Property input_bytes -Sum).Sum
$archiveTotal = ($rows | Measure-Object -Property archive_bytes -Sum).Sum
$aggregate = [pscustomobject][ordered]@{
    mode = 'donor-match'
    rows = $rows.Count
    input_bytes = $inputTotal
    archive_bytes = $archiveTotal
    ratio = [double]$archiveTotal / [double]$inputTotal
    bpb = [double]$archiveTotal * 8.0 / [double]$inputTotal
    encode_seconds = ($rows | Measure-Object -Property encode_seconds -Sum).Sum
    decode_seconds = ($rows | Measure-Object -Property decode_seconds -Sum).Sum
    peak_ram_mib = ($rows | Measure-Object -Property peak_ram_mib -Maximum).Maximum
    pass_rows = @($rows | Where-Object {
        $_.status -eq 'COMPLETE' -and $_.roundtrip -eq 'PASS'
    }).Count
    codec_sha256 = $codecHashes[0]
}

$winnerRows = foreach ($row in $rows) {
    [pscustomobject][ordered]@{
        file = $row.file
        scope_kib = [int]$row.scope_kib
        winner_modes = 'donor-match'
        winner_archive_bytes = [int64]$row.archive_bytes
        auto_archive_bytes = ''
        auto_gap_bytes = ''
    }
}

New-Item -ItemType Directory -Path $OutputPath | Out-Null
function Write-NoBom([string]$Path, [string]$Text) {
    [IO.File]::WriteAllText($Path, $Text, (New-Object Text.UTF8Encoding($false)))
}
Write-NoBom (Join-Path $OutputPath 'mode_rows.tsv') `
    (($modeRows | ConvertTo-Csv -Delimiter "`t" -NoTypeInformation) -join "`r`n")
Write-NoBom (Join-Path $OutputPath 'portfolio_aggregate.tsv') `
    (($aggregate | ConvertTo-Csv -Delimiter "`t" -NoTypeInformation) -join "`r`n")
Write-NoBom (Join-Path $OutputPath 'per_file_winners.tsv') `
    (($winnerRows | ConvertTo-Csv -Delimiter "`t" -NoTypeInformation) -join "`r`n")

$sha20 = (Get-FileHash (Join-Path $D20Path 'results.csv') -Algorithm SHA256).Hash.ToUpperInvariant()
$sha22 = (Get-FileHash (Join-Path $D22Path 'results.csv') -Algorithm SHA256).Hash.ToUpperInvariant()
$codecHash = $codecHashes[0]
$readme = @"
# HybridZip R2 Current-Hash Donor-Match Ledger

This derived ledger combines two explicitly scoped 32 KiB donor-match packages into one 12-file result set. No 64/128 KiB case is included.

- d20 source: `hybridzip-r2-donor-match-silesia-32k-20260821-d20` (6 rows)
- d22 source: `hybridzip-r2-donor-match-silesia-32k-20260821-d22` (6 rows)
- codec SHA-256: $codecHash
- input: 12 Silesia files x 32 KiB = $inputTotal bytes
- archive bytes: $archiveTotal
- byte-exact rows: $($checks.Count)/12

## Verification

- d22 official validator: 6/6 PASS with explicit 32 KiB and six-file scope.
- d20 remains marked interrupted/testing because it was originally stopped after six cases; its six rows were independently revalidated here by checking all input, archive, and decoded SHA-256 values, lengths, COMPLETE, and PASS.
- Source ``results.csv`` SHA-256:
  - d20: $sha20
  - d22: $sha22

## Files

- `mode_rows.tsv`: 12 normalized per-file rows.
- `portfolio_aggregate.tsv`: aggregate bytes, time, memory, and PASS count.
- `per_file_winners.tsv`: donor-match row for each file; Auto comparison is intentionally blank because this ledger is single-mode.

This is complete donor-match evidence for the current Release hash at 32 KiB only. It is not the complete 24-mode current-hash R2 portfolio.
"@
Write-NoBom (Join-Path $OutputPath 'README.md') $readme

Write-Output "created=$OutputPath"
Write-Output "rows=$($rows.Count) byte_exact=$($checks.Count) archive_bytes=$archiveTotal"
