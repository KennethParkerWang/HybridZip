[CmdletBinding()]
param(
    [string]$ExperimentsRoot = '',
    [string]$OutputPath = ''
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

if ([string]::IsNullOrWhiteSpace($ExperimentsRoot)) {
    $ExperimentsRoot = Join-Path $PSScriptRoot '..\results\experiments'
}
if ([string]::IsNullOrWhiteSpace($OutputPath)) {
    $OutputPath = Join-Path $PSScriptRoot '..\results\analysis\r2-silesia-32k-currenthash-20260821-d40'
}
$ExperimentsRoot = [IO.Path]::GetFullPath($ExperimentsRoot)
$OutputPath = [IO.Path]::GetFullPath($OutputPath)
if (Test-Path -LiteralPath $OutputPath) {
    throw "Output already exists: $OutputPath"
}

$records = @(
    [pscustomobject]@{ mode = 'auto'; package = 'hybridzip-r2-auto-silesia-32k-20260821-d9' }
    [pscustomobject]@{ mode = 'predictive'; package = 'hybridzip-r2-predictive-silesia-32k-20260821-d11' }
    [pscustomobject]@{ mode = 'lstm-compress'; package = 'hybridzip-r2-lstm-compress-silesia-32k-20260821-d10' }
    [pscustomobject]@{ mode = 'stored'; package = 'hybridzip-r2-stored-silesia-32k-20260821-d13' }
    [pscustomobject]@{ mode = 'zstd'; package = 'hybridzip-r2-zstd-silesia-32k-20260821-d14' }
    [pscustomobject]@{ mode = 'lzma'; package = 'hybridzip-r2-lzma-silesia-32k-20260821-d15' }
    [pscustomobject]@{ mode = 'fse'; package = 'hybridzip-r2-fse-silesia-32k-20260821-d17' }
    [pscustomobject]@{ mode = 'rans'; package = 'hybridzip-r2-rans-silesia-32k-20260821-d18' }
    [pscustomobject]@{ mode = 'donor-match'; package = 'hybridzip-r2-donor-match-silesia-32k-20260821-d20' }
    [pscustomobject]@{ mode = 'donor-match'; package = 'hybridzip-r2-donor-match-silesia-32k-20260821-d22' }
    [pscustomobject]@{ mode = 'bwt-zstd'; package = 'hybridzip-r2-bwt-zstd-silesia-32k-20260821-d24' }
    [pscustomobject]@{ mode = 'bwt-mtf-zstd'; package = 'hybridzip-r2-bwt-mtf-zstd-silesia-32k-20260821-d25' }
    [pscustomobject]@{ mode = 'bwt-rlt-zstd'; package = 'hybridzip-r2-bwt-rlt-zstd-silesia-32k-20260821-d26' }
    [pscustomobject]@{ mode = 'x86-bcj-zstd'; package = 'hybridzip-r2-x86-bcj-zstd-silesia-32k-20260821-d27' }
    [pscustomobject]@{ mode = 'shuffle-zstd'; package = 'hybridzip-r2-shuffle-zstd-silesia-32k-20260821-d28' }
    [pscustomobject]@{ mode = 'bitshuffle-zstd'; package = 'hybridzip-r2-bitshuffle-zstd-silesia-32k-20260821-d29' }
    [pscustomobject]@{ mode = 'delta-zstd'; package = 'hybridzip-r2-delta-zstd-silesia-32k-20260821-d30' }
    [pscustomobject]@{ mode = 'fastpfor'; package = 'hybridzip-r2-fastpfor-silesia-32k-20260821-d31' }
    [pscustomobject]@{ mode = 'bcj2-zstd'; package = 'hybridzip-r2-bcj2-zstd-silesia-32k-20260821-d32' }
    [pscustomobject]@{ mode = 'record-transpose-zstd'; package = 'hybridzip-r2-record-transpose-zstd-silesia-32k-20260821-d33' }
    [pscustomobject]@{ mode = 'jpegls'; package = 'hybridzip-r2-jpegls-silesia-32k-20260821-d34' }
    [pscustomobject]@{ mode = 'flac-residual'; package = 'hybridzip-r2-flac-residual-silesia-32k-20260821-d35' }
    [pscustomobject]@{ mode = 'brotli-text'; package = 'hybridzip-r2-brotli-text-silesia-32k-20260821-d36' }
    [pscustomobject]@{ mode = 'cmix-word-zstd'; package = 'hybridzip-r2-cmix-word-zstd-silesia-32k-20260821-d37' }
    [pscustomobject]@{ mode = 'neural-lstm'; package = 'hybridzip-r2-neural-lstm-silesia-32k-20260821-d38' }
    [pscustomobject]@{ mode = 'shared-neural-lstm'; package = 'hybridzip-r2-shared-neural-lstm-silesia-32k-20260821-d39' }
)
$files = @('dickens', 'mozilla', 'mr', 'nci', 'ooffice', 'osdb',
    'reymont', 'samba', 'sao', 'webster', 'x-ray', 'xml')
$modeOrder = @($records | ForEach-Object mode | Select-Object -Unique)
$allRows = @()
$sourceManifest = @()
$codecHashes = New-Object 'System.Collections.Generic.HashSet[string]'

foreach ($record in $records) {
    $packagePath = Join-Path $ExperimentsRoot $record.package
    $csvPath = Join-Path $packagePath 'results.csv'
    if (-not (Test-Path -LiteralPath $csvPath -PathType Leaf)) {
        throw "Missing source results: $csvPath"
    }
    $sourceRows = @(Import-Csv $csvPath)
    $sourceManifest += [pscustomobject][ordered]@{
        mode = $record.mode
        package = $record.package
        rows = $sourceRows.Count
        results_csv_sha256 = (Get-FileHash $csvPath -Algorithm SHA256).Hash.ToUpperInvariant()
    }
    foreach ($row in $sourceRows) {
        if ($row.status -ne 'COMPLETE' -or $row.roundtrip -ne 'PASS') {
            throw "Source row is not COMPLETE/PASS: $($record.package)/$($row.file)"
        }
        [void]$codecHashes.Add([string]$row.codec_sha256)
        $inputPath = Join-Path $packagePath ($row.input_path -replace '/', '\')
        $archivePath = Join-Path $packagePath ($row.archive_path -replace '/', '\')
        $decodedPath = Join-Path $packagePath ($row.decoded_path -replace '/', '\')
        foreach ($path in @($inputPath, $archivePath, $decodedPath)) {
            if (-not (Test-Path -LiteralPath $path -PathType Leaf)) {
                throw "Missing source artifact: $path"
            }
        }
        $inputHash = (Get-FileHash $inputPath -Algorithm SHA256).Hash.ToUpperInvariant()
        $archiveHash = (Get-FileHash $archivePath -Algorithm SHA256).Hash.ToUpperInvariant()
        $decodedHash = (Get-FileHash $decodedPath -Algorithm SHA256).Hash.ToUpperInvariant()
        if ($inputHash -ne $row.input_sha256 -or
            $archiveHash -ne $row.archive_sha256 -or
            $decodedHash -ne $row.decoded_sha256 -or
            $inputHash -ne $decodedHash) {
            throw "SHA-256 mismatch: $($record.package)/$($row.file)"
        }
        if ([int]$row.scope_kib -ne 32 -or
            [int64]$row.input_bytes -ne 32768 -or
            (Get-Item $inputPath).Length -ne 32768 -or
            (Get-Item $decodedPath).Length -ne 32768 -or
            (Get-Item $archivePath).Length -ne [int64]$row.archive_bytes) {
            throw "Scope or length mismatch: $($record.package)/$($row.file)"
        }
        $allRows += [pscustomobject]@{
            mode = $record.mode
            row = $row
        }
    }
}

$normalized = @()
foreach ($entry in $allRows) {
    $row = $entry.row
    $normalized += [pscustomobject][ordered]@{
        mode = $entry.mode
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

$normalized = @($normalized | Sort-Object `
    @{ Expression = { [Array]::IndexOf($modeOrder, $_.mode) } }, `
    @{ Expression = { [Array]::IndexOf($files, $_.file) } })
$expectedRows = $modeOrder.Count * $files.Count
if ($normalized.Count -ne $expectedRows) {
    throw "Expected $expectedRows normalized rows, found $($normalized.Count)"
}
if ($codecHashes.Count -ne 1) {
    throw "Expected one codec hash, found $($codecHashes.Count)"
}
foreach ($mode in $modeOrder) {
    $modeRows = @($normalized | Where-Object mode -eq $mode)
    if ($modeRows.Count -ne 12) {
        throw "Mode $mode has $($modeRows.Count) rows, expected 12"
    }
    $duplicateFiles = @($modeRows | Group-Object file | Where-Object Count -ne 1)
    if ($duplicateFiles.Count -ne 0) {
        throw "Mode $mode has duplicate or missing files"
    }
}

$aggregates = foreach ($mode in $modeOrder) {
    $modeRows = @($normalized | Where-Object mode -eq $mode)
    $inputTotal = ($modeRows | Measure-Object input_bytes -Sum).Sum
    $archiveTotal = ($modeRows | Measure-Object archive_bytes -Sum).Sum
    [pscustomobject][ordered]@{
        mode = $mode
        rows = $modeRows.Count
        input_bytes = $inputTotal
        archive_bytes = $archiveTotal
        ratio = [double]$archiveTotal / [double]$inputTotal
        bpb = [double]$archiveTotal * 8.0 / [double]$inputTotal
        encode_seconds = ($modeRows | Measure-Object encode_seconds -Sum).Sum
        decode_seconds = ($modeRows | Measure-Object decode_seconds -Sum).Sum
        peak_ram_mib = ($modeRows | Measure-Object peak_ram_mib -Maximum).Maximum
        pass_rows = @($modeRows | Where-Object roundtrip -eq 'PASS').Count
        codec_sha256 = $modeRows[0].codec_sha256
    }
}

$winnerRows = foreach ($file in $files) {
    $fileRows = @($normalized | Where-Object file -eq $file)
    $minimum = ($fileRows | Measure-Object archive_bytes -Minimum).Minimum
    $winnerModes = @($fileRows | Where-Object { [int64]$_.archive_bytes -eq [int64]$minimum } |
        ForEach-Object mode)
    $autoBytes = [int64](@($fileRows | Where-Object mode -eq 'auto')[0].archive_bytes)
    [pscustomobject][ordered]@{
        file = $file
        scope_kib = 32
        winner_modes = ($winnerModes -join ',')
        winner_archive_bytes = [int64]$minimum
        auto_archive_bytes = $autoBytes
        auto_gap_bytes = $autoBytes - [int64]$minimum
    }
}

New-Item -ItemType Directory -Path $OutputPath | Out-Null
function Write-NoBom([string]$Path, [string]$Text) {
    [IO.File]::WriteAllText($Path, $Text, (New-Object Text.UTF8Encoding($false)))
}
Write-NoBom (Join-Path $OutputPath 'mode_rows.tsv') `
    (($normalized | ConvertTo-Csv -Delimiter "`t" -NoTypeInformation) -join "`r`n")
Write-NoBom (Join-Path $OutputPath 'portfolio_aggregate.tsv') `
    (($aggregates | ConvertTo-Csv -Delimiter "`t" -NoTypeInformation) -join "`r`n")
Write-NoBom (Join-Path $OutputPath 'per_file_winners.tsv') `
    (($winnerRows | ConvertTo-Csv -Delimiter "`t" -NoTypeInformation) -join "`r`n")
Write-NoBom (Join-Path $OutputPath 'source_manifest.tsv') `
    (($sourceManifest | ConvertTo-Csv -Delimiter "`t" -NoTypeInformation) -join "`r`n")

$codecHash = @($codecHashes)[0]
$codecCode = '`' + $codecHash + '`'
$sourceLines = ($sourceManifest | ForEach-Object {
    "- $($_.mode): ``$($_.package)`` ($($_.rows) rows, CSV SHA-256 ``$($_.results_csv_sha256)``)"
}) -join "`r`n"
$readme = @"
# HybridZip R2 Current-Hash Complete Portfolio Ledger

This ledger combines the current Release binary's Auto path and all 24 forced R2 modes on the 12 Silesia files at exactly 32 KiB per file. It contains 25 modes and 300 independently byte-exact rows. No 64 KiB or 128 KiB case is included.

- input bytes: 393216
- codec SHA-256: $codecCode
- normalized rows: $($normalized.Count)
- PASS rows: $(@($normalized | Where-Object roundtrip -eq 'PASS').Count)

## Source packages

$sourceLines

The donor-match mode combines its six-row d20 package and six-row d22 package. d20 remains marked testing because it was intentionally stopped; its six rows are revalidated here at artifact level. d22 passed its explicit six-file validator. Historical d6 packages are not used in this ledger.

## Files

- `mode_rows.tsv`: 300 normalized per-file rows.
- `portfolio_aggregate.tsv`: one aggregate row per mode.
- `per_file_winners.tsv`: minimum archive bytes and Auto gap for each file.
- `source_manifest.tsv`: source package row counts and CSV hashes.

This ledger is the current-hash 32 KiB portfolio evidence. It does not by itself claim broader corpus generalization, repeated-run statistics, or 64/128 KiB performance.
"@
Write-NoBom (Join-Path $OutputPath 'README.md') $readme

Write-Output "created=$OutputPath"
Write-Output "modes=$($modeOrder.Count) rows=$($normalized.Count) pass=$(@($normalized | Where-Object roundtrip -eq 'PASS').Count) codec=$codecHash"
