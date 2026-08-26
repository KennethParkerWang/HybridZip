[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)]
    [string]$ExperimentPath
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'
$ExperimentPath = [IO.Path]::GetFullPath($ExperimentPath)
$resultsPath = Join-Path $ExperimentPath 'results.csv'
if (-not (Test-Path -LiteralPath $resultsPath -PathType Leaf)) {
    throw "results.csv not found: $resultsPath"
}
$modeNames = @(
    'stored', 'predictive', 'zstd', 'fse', 'lzma', 'donor-match',
    'bwt-zstd', 'bwt-mtf-zstd', 'bwt-rlt-zstd', 'x86-bcj-zstd',
    'shuffle-zstd', 'bitshuffle-zstd', 'delta-zstd', 'delta-of-delta-zstd', 'fastpfor', 'rans',
    'bcj2-zstd', 'record-transpose-zstd', 'jpegls', 'flac-residual',
    'brotli-text', 'cmix-word-zstd', 'neural-lstm', 'shared-neural-lstm',
    'lstm-compress', 'bgpt-shared-prior'
)
function Write-NoBom([string]$Path, [string]$Text) {
    [IO.File]::WriteAllText($Path, $Text, (New-Object Text.UTF8Encoding($false)))
}
$rows = @(Import-Csv $resultsPath)
$selected = @()
foreach ($row in $rows) {
    $logPath = Join-Path $ExperimentPath ("logs\$($row.id).encode.stdout.log")
    $text = [IO.File]::ReadAllText($logPath)
    $candidateMatch = [Regex]::Match($text, 'candidates=(\d+)')
    $selectedMatch = [Regex]::Match($text, 'selected=(\d+)')
    $oracleMatch = [Regex]::Match($text, 'oracle=(\d+)')
    $gapMatch = [Regex]::Match($text, 'oracle_gap=(\d+)')
    $blocksMatch = [Regex]::Match($text, 'blocks\(([^)]*)\)=([0-9/]+)')
    if (-not $blocksMatch.Success) { throw "Missing block counts: $($row.id)" }
    $counts = @($blocksMatch.Groups[2].Value.Split('/') | ForEach-Object { [int]$_ })
    if ($counts.Count -ne $modeNames.Count) { throw "Unexpected block count width: $($row.id)" }
    $winnerIndexes = @()
    for ($i = 0; $i -lt $counts.Count; ++$i) {
        if ($counts[$i] -gt 0) { $winnerIndexes += $i }
    }
    if ($winnerIndexes.Count -ne 1) { throw "Expected one selected mode: $($row.id)" }
    $selected += [pscustomobject][ordered]@{
        id = $row.id
        family = $row.family
        archive_bytes = [int64]$row.archive_bytes
        input_bytes = [int64]$row.input_bytes
        selected_mode = $modeNames[$winnerIndexes[0]]
        candidates = if ($candidateMatch.Success) { [int]$candidateMatch.Groups[1].Value } else { -1 }
        selected_payload_bytes = if ($selectedMatch.Success) { [int64]$selectedMatch.Groups[1].Value } else { -1 }
        oracle_payload_bytes = if ($oracleMatch.Success) { [int64]$oracleMatch.Groups[1].Value } else { -1 }
        oracle_gap_bytes = if ($gapMatch.Success) { [int64]$gapMatch.Groups[1].Value } else { -1 }
    }
}

$familyAggregates = foreach ($family in @($rows | ForEach-Object family | Sort-Object -Unique)) {
    $familyRows = @($rows | Where-Object family -eq $family)
    $inputTotal = ($familyRows | Measure-Object input_bytes -Sum).Sum
    $archiveTotal = ($familyRows | Measure-Object archive_bytes -Sum).Sum
    $familySelected = @($selected | Where-Object family -eq $family)
    $modeCounts = @($familySelected | Group-Object selected_mode | Sort-Object Name |
        ForEach-Object { "$($_.Name)=$($_.Count)" })
    [pscustomobject][ordered]@{
        family = $family
        rows = $familyRows.Count
        input_bytes = $inputTotal
        archive_bytes = $archiveTotal
        ratio = [double]$archiveTotal / [double]$inputTotal
        bpb = [double]$archiveTotal * 8.0 / [double]$inputTotal
        encode_seconds = ($familyRows | Measure-Object encode_seconds -Sum).Sum
        decode_seconds = ($familyRows | Measure-Object decode_seconds -Sum).Sum
        peak_ram_mib = ($familyRows | Measure-Object peak_ram_mib -Maximum).Maximum
        selected_modes = ($modeCounts -join ',')
        max_oracle_gap_bytes = ($familySelected | Measure-Object oracle_gap_bytes -Maximum).Maximum
    }
}

$selectionSummary = foreach ($mode in @($selected | ForEach-Object selected_mode | Sort-Object -Unique)) {
    $modeRows = @($selected | Where-Object selected_mode -eq $mode)
    [pscustomobject][ordered]@{
        selected_mode = $mode
        files = $modeRows.Count
        ids = (($modeRows | ForEach-Object id) -join ',')
        total_archive_bytes = ($modeRows | Measure-Object archive_bytes -Sum).Sum
        mean_archive_bytes = ($modeRows | Measure-Object archive_bytes -Average).Average
    }
}

Write-NoBom (Join-Path $ExperimentPath 'results_with_selection.tsv') `
    (($selected | ConvertTo-Csv -Delimiter "`t" -NoTypeInformation) -join "`r`n")
Write-NoBom (Join-Path $ExperimentPath 'family_aggregate.tsv') `
    (($familyAggregates | ConvertTo-Csv -Delimiter "`t" -NoTypeInformation) -join "`r`n")
Write-NoBom (Join-Path $ExperimentPath 'selection_summary.tsv') `
    (($selectionSummary | ConvertTo-Csv -Delimiter "`t" -NoTypeInformation) -join "`r`n")

Write-Output "analyzed=$ExperimentPath rows=$($rows.Count) selected_modes=$($selectionSummary.Count)"
