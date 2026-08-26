[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)] [string]$AutoPath,
    [Parameter(Mandatory = $true)] [string]$PredictivePath,
    [Parameter(Mandatory = $true)] [string]$CmixPath,
    [Parameter(Mandatory = $true)] [string]$OutputPath
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'
$AutoPath = [IO.Path]::GetFullPath($AutoPath)
$PredictivePath = [IO.Path]::GetFullPath($PredictivePath)
$CmixPath = [IO.Path]::GetFullPath($CmixPath)
$OutputPath = [IO.Path]::GetFullPath($OutputPath)
if (Test-Path -LiteralPath $OutputPath) { throw "Output exists: $OutputPath" }
function Get-Sha256([string]$Path) {
    (Get-FileHash -LiteralPath $Path -Algorithm SHA256).Hash.ToUpperInvariant()
}
function Write-NoBom([string]$Path, [string]$Text) {
    [IO.File]::WriteAllText($Path, $Text, (New-Object Text.UTF8Encoding($false)))
}
function Read-Rows([string]$Root) {
    $path = Join-Path $Root 'results.csv'
    if (-not (Test-Path $path)) { throw "Missing results: $path" }
    @(Import-Csv $path)
}
$packages = [ordered]@{
    auto = $AutoPath
    predictive = $PredictivePath
    cmix_word_zstd = $CmixPath
}
$rowsByMode = @{}
$codecHashes = New-Object 'System.Collections.Generic.HashSet[string]'
foreach ($mode in $packages.Keys) {
    $rows = Read-Rows $packages[$mode]
    if ($rows.Count -ne 8) { throw "$mode has $($rows.Count) rows" }
    $rowsByMode[$mode] = @{}
    foreach ($row in $rows) {
        if ($row.status -ne 'COMPLETE' -or $row.roundtrip -ne 'PASS') {
            throw "$mode/$($row.id) is not PASS"
        }
        if ($rowsByMode[$mode].ContainsKey($row.id)) { throw "Duplicate id: $mode/$($row.id)" }
        [void]$codecHashes.Add([string]$row.codec_sha256)
        $archivePath = Join-Path $packages[$mode] ($row.archive_path -replace '/', '\')
        $decodedPath = Join-Path $packages[$mode] ($row.decoded_path -replace '/', '\')
        if ((Get-Sha256 $archivePath) -ne $row.archive_sha256) { throw "Archive hash mismatch: $mode/$($row.id)" }
        if ((Get-Sha256 $decodedPath) -ne $row.decoded_sha256 -or $row.input_sha256 -ne $row.decoded_sha256) {
            throw "Roundtrip hash mismatch: $mode/$($row.id)"
        }
        $rowsByMode[$mode][$row.id] = $row
    }
}
if ($codecHashes.Count -ne 1) { throw "Expected one codec hash" }
$ids = @($rowsByMode.auto.Keys | Sort-Object)
foreach ($mode in $packages.Keys) {
    foreach ($id in $ids) { if (-not $rowsByMode[$mode].ContainsKey($id)) { throw "Missing $mode/$id" } }
}

$comparison = foreach ($id in $ids) {
    $a = $rowsByMode.auto[$id]
    $p = $rowsByMode.predictive[$id]
    $c = $rowsByMode.cmix_word_zstd[$id]
    $best = [Math]::Min([int64]$a.archive_bytes, [Math]::Min([int64]$p.archive_bytes, [int64]$c.archive_bytes))
    $winners = @()
    if ([int64]$a.archive_bytes -eq $best) { $winners += 'auto' }
    if ([int64]$p.archive_bytes -eq $best) { $winners += 'predictive' }
    if ([int64]$c.archive_bytes -eq $best) { $winners += 'cmix-word-zstd' }
    $selection = (Import-Csv (Join-Path $AutoPath 'results_with_selection.tsv') -Delimiter "`t" |
        Where-Object id -eq $id)[0]
    [pscustomobject][ordered]@{
        id = $id
        family = $a.family
        input_bytes = [int64]$a.input_bytes
        auto_archive_bytes = [int64]$a.archive_bytes
        predictive_archive_bytes = [int64]$p.archive_bytes
        cmix_word_zstd_archive_bytes = [int64]$c.archive_bytes
        auto_selected_mode = $selection.selected_mode
        auto_candidates = [int]$selection.candidates
        best_archive_bytes = $best
        winner_modes = ($winners -join ',')
        auto_gap_to_best = [int64]$a.archive_bytes - $best
        auto_encode_seconds = [double]$a.encode_seconds
        auto_decode_seconds = [double]$a.decode_seconds
        auto_peak_ram_mib = [double]$a.peak_ram_mib
    }
}

$aggregates = foreach ($mode in $packages.Keys) {
    $modeRows = @($rowsByMode[$mode].Values)
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

New-Item -ItemType Directory -Path $OutputPath | Out-Null
Write-NoBom (Join-Path $OutputPath 'comparison.tsv') `
    (($comparison | ConvertTo-Csv -Delimiter "`t" -NoTypeInformation) -join "`r`n")
Write-NoBom (Join-Path $OutputPath 'mode_aggregate.tsv') `
    (($aggregates | ConvertTo-Csv -Delimiter "`t" -NoTypeInformation) -join "`r`n")
$manifestPath = Join-Path (Split-Path $AutoPath -Parent) '..\corpus\r2-family-raw-20260821\manifest.tsv'
$manifestPath = [IO.Path]::GetFullPath($manifestPath)
$manifestHash = if (Test-Path $manifestPath) { Get-Sha256 $manifestPath } else { '' }
$autoWins = @($comparison | Where-Object auto_gap_to_best -eq 0).Count
$readme = @"
# HybridZip R2 Family-Specific Raw Corpus Comparison

This comparison uses eight provenance-tracked donor source prefixes, exactly 32 KiB each, with the current Release binary. It contains Auto, forced predictive, and forced cmix-word-zstd results. No 64/128 KiB case was run.

- rows per mode: 8
- input bytes per mode: 262144
- Auto wins or ties: $autoWins/8
- codec SHA-256: ``$($codecHashes | Select-Object -First 1)``
- source manifest: ``$manifestPath``
- manifest SHA-256: ``$manifestHash``

## Outputs

- `comparison.tsv`: per-source archive sizes, selected Auto mode, and Auto gap to the best of the three modes.
- `mode_aggregate.tsv`: aggregate bytes, timing, memory, and PASS counts.
- `../experiments/hybridzip-r2-family-auto-20260821-d41`: Auto source package with selection/oracle logs.

All 24 rows are COMPLETE/PASS and input/decoded SHA-256 values match. This is a family-specific donor-source probe, not an independent generalization benchmark: the inputs are extracted from the donor warehouse and should be interpreted as engineering evidence only.
"@
Write-NoBom (Join-Path $OutputPath 'README.md') $readme
Write-Output "created=$OutputPath rows=$($comparison.Count) auto_wins_or_ties=$autoWins"
