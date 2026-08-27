[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)]
    [string]$ForcedLedgerPath,
    [string]$E5PackagePath = '',
    [string]$OutputPath = '',
    [switch]$RequireE5Coverage,
    [switch]$ListOnly
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

# This registry is the ratio portfolio only. Mode 43 is Fast-only and must
# never enter an ENC_RATIO_V1 forced oracle or a K=2/K=4/K=8 candidate set.
$ratioModes = @(
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
    'lmic-arithmetic', 'delta-binary-packed-zstd'
)
$modeIdByName = @{}
for ($index = 0; $index -lt $ratioModes.Count; ++$index) {
    $modeIdByName[$ratioModes[$index]] = $index
}

function Require-Property($Object, [string]$Name, [string]$Context) {
    $property = $Object.PSObject.Properties[$Name]
    if ($null -eq $property) {
        throw "$Context is missing required property: $Name"
    }
    return $property.Value
}

function Parse-Int64([object]$Value, [string]$Context) {
    [int64]$parsed = 0
    if (-not [int64]::TryParse(
            [string]$Value,
            [Globalization.NumberStyles]::Integer,
            [Globalization.CultureInfo]::InvariantCulture,
            [ref]$parsed)) {
        throw "$Context is not an integer: $Value"
    }
    return $parsed
}

function Assert-Sha256([string]$Value, [string]$Context) {
    if ($Value -cnotmatch '^[0-9A-F]{64}$') {
        throw "$Context is not an uppercase SHA-256: $Value"
    }
}

function Get-CaseKey([string]$File, [int]$ScopeKiB, [string]$InputSha256) {
    return '{0}|{1}|{2}' -f $File, $ScopeKiB, $InputSha256
}

function Write-Utf8Json([string]$Path, $Value) {
    $encoding = New-Object System.Text.UTF8Encoding($false)
    [IO.File]::WriteAllText(
        $Path, (($Value | ConvertTo-Json -Depth 8) + "`n"), $encoding)
}

function Write-Csv([string]$Path, [object[]]$Rows) {
    $encoding = New-Object System.Text.UTF8Encoding($false)
    $content = if ($Rows.Count -eq 0) {
        ''
    }
    else {
        [string]::Join("`r`n", @($Rows | ConvertTo-Csv -NoTypeInformation)) + "`r`n"
    }
    [IO.File]::WriteAllText($Path, $content, $encoding)
}

function Get-CandidateModeIds([string]$Text, [string]$Context) {
    $ids = New-Object 'System.Collections.Generic.HashSet[int]'
    if ([string]::IsNullOrWhiteSpace($Text) -or $Text -eq 'none') {
        throw "$Context has no materialized candidate modes"
    }
    foreach ($entry in $Text.Split(',')) {
        $match = [regex]::Match($entry, '^(?<id>[0-9]+):(?<count>[1-9][0-9]*)$')
        if (-not $match.Success) {
            throw "$Context has malformed candidate mode entry: $entry"
        }
        $id = [int]$match.Groups['id'].Value
        if ($id -lt 0 -or $id -ge $ratioModes.Count -or -not $ids.Add($id)) {
            throw "$Context has an invalid or duplicate ratio mode ID: $entry"
        }
    }
    return $ids
}

$ForcedLedgerPath = [IO.Path]::GetFullPath($ForcedLedgerPath)
$manifestPath = Join-Path $ForcedLedgerPath 'manifest.tsv'
if (-not (Test-Path -LiteralPath $manifestPath -PathType Leaf)) {
    throw "Forced-ledger manifest is missing: $manifestPath"
}
$manifest = @(Import-Csv -LiteralPath $manifestPath -Delimiter "`t" -Encoding UTF8)
if ($manifest.Count -ne ($ratioModes.Count + 1)) {
    throw "Forced-ledger manifest has $($manifest.Count) rows; expected $($ratioModes.Count + 1)"
}

$autoEntries = @($manifest | Where-Object { [string]$_.mode -ceq 'auto' })
$forcedEntries = @($manifest | Where-Object { [string]$_.mode -cne 'auto' })
if ($autoEntries.Count -ne 1 -or $forcedEntries.Count -ne $ratioModes.Count) {
    throw 'Forced ledger must contain one Auto package and every ratio forced mode'
}

$entryByMode = @{}
$blockSizeKiB = -1
$expectedRows = -1
$ledgerCodecSha256 = ''
foreach ($entry in $manifest) {
    $mode = [string](Require-Property $entry 'mode' 'Manifest row')
    if ($entryByMode.ContainsKey($mode)) {
        throw "Forced-ledger manifest has duplicate mode: $mode"
    }
    $entryByMode[$mode] = $entry
    if ([string](Require-Property $entry 'status' "Manifest mode $mode") -cne 'COMPLETE') {
        throw "Forced-ledger manifest mode is not COMPLETE: $mode"
    }
    $entryBlockSize = Parse-Int64 (Require-Property $entry 'block_size_kib' "Manifest mode $mode") "Manifest block_size_kib for $mode"
    if ($entryBlockSize -notin @(32, 64, 128)) {
        throw "Manifest mode $mode has unsupported block_size_kib: $entryBlockSize"
    }
    if ($blockSizeKiB -lt 0) {
        $blockSizeKiB = [int]$entryBlockSize
    }
    elseif ($blockSizeKiB -ne $entryBlockSize) {
        throw 'Forced ledger mixes internal block sizes'
    }
    $entryExpectedRows = Parse-Int64 (Require-Property $entry 'expected_rows' "Manifest mode $mode") "Manifest expected_rows for $mode"
    if ($expectedRows -lt 0) {
        $expectedRows = $entryExpectedRows
    }
    elseif ($expectedRows -ne $entryExpectedRows) {
        throw 'Forced ledger mixes expected row counts'
    }
}
foreach ($mode in $ratioModes) {
    if (-not $entryByMode.ContainsKey($mode)) {
        throw "Forced ledger is missing ratio mode: $mode"
    }
    if ((Parse-Int64 $entryByMode[$mode].mode_index "Manifest mode_index for $mode") -ne $modeIdByName[$mode]) {
        throw "Forced ledger mode index disagrees with the frozen ratio registry: $mode"
    }
}

$blockBytes = [int64]$blockSizeKiB * 1024L
$caseByKey = New-Object 'System.Collections.Generic.Dictionary[string,object]' `
    ([System.StringComparer]::Ordinal)
$forcedArchiveRows = New-Object System.Collections.Generic.List[object]

foreach ($mode in $ratioModes) {
    $entry = $entryByMode[$mode]
    $packagePath = [IO.Path]::GetFullPath([string](Require-Property $entry 'package_path' "Manifest mode $mode"))
    $resultsPath = Join-Path $packagePath 'results.csv'
    if (-not (Test-Path -LiteralPath $resultsPath -PathType Leaf)) {
        throw "Forced-mode results are missing for ${mode}: $resultsPath"
    }
    $rows = @(Import-Csv -LiteralPath $resultsPath -Encoding UTF8)
    if ($rows.Count -ne $expectedRows) {
        throw "Forced-mode $mode has $($rows.Count) rows; expected $expectedRows"
    }
    $modeKeys = New-Object 'System.Collections.Generic.HashSet[string]' `
        ([System.StringComparer]::Ordinal)
    foreach ($row in $rows) {
        $context = "Forced mode $mode row"
        if ([string](Require-Property $row 'status' $context) -cne 'COMPLETE' -or
            [string](Require-Property $row 'roundtrip' $context) -cne 'PASS') {
            throw "$context is not COMPLETE/PASS"
        }
        if ([string](Require-Property $row 'variant' $context) -cne "r2-$mode") {
            throw "$context has wrong variant: $($row.variant)"
        }
        $scopeKiB = [int](Parse-Int64 (Require-Property $row 'scope_kib' $context) "$context scope_kib")
        $inputBytes = Parse-Int64 (Require-Property $row 'input_bytes' $context) "$context input_bytes"
        if ($scopeKiB -ne $blockSizeKiB -or $inputBytes -ne $blockBytes) {
            throw "$context is not one complete internal block (scope=$scopeKiB, bytes=$inputBytes, block=$blockSizeKiB KiB)"
        }
        $inputSha256 = [string](Require-Property $row 'input_sha256' $context)
        Assert-Sha256 $inputSha256 "$context input_sha256"
        $codecSha256 = [string](Require-Property $row 'codec_sha256' $context)
        Assert-Sha256 $codecSha256 "$context codec_sha256"
        if ([string]::IsNullOrWhiteSpace($ledgerCodecSha256)) {
            $ledgerCodecSha256 = $codecSha256
        }
        elseif ($ledgerCodecSha256 -cne $codecSha256) {
            throw 'Forced ledger mixes codec executable SHA-256 values'
        }
        $archiveBytes = Parse-Int64 (Require-Property $row 'archive_bytes' $context) "$context archive_bytes"
        if ($archiveBytes -le 0) {
            throw "$context has a nonpositive complete archive size"
        }
        if ([string](Require-Property $row 'block_types' $context) -cne "$mode=1") {
            throw "$context lacks exact one-block forced-mode attribution: $($row.block_types)"
        }
        $file = [string](Require-Property $row 'file' $context)
        $key = Get-CaseKey $file $scopeKiB $inputSha256
        if (-not $modeKeys.Add($key)) {
            throw "$context duplicates input case: $key"
        }
        if ($caseByKey.ContainsKey($key)) {
            $expected = $caseByKey[$key]
            if ($expected.input_bytes -ne $inputBytes) {
                throw "Forced ledger has mismatched input bytes for $key"
            }
        }
        else {
            $caseByKey.Add($key, [pscustomobject][ordered]@{
                file = $file
                scope_kib = $scopeKiB
                input_bytes = $inputBytes
                input_sha256 = $inputSha256
            })
        }
        $forcedArchiveRows.Add([pscustomobject][ordered]@{
            file = $file
            scope_kib = $scopeKiB
            input_bytes = $inputBytes
            input_sha256 = $inputSha256
            mode = $mode
            mode_id = $modeIdByName[$mode]
            complete_archive_bytes = $archiveBytes
            archive_sha256 = [string](Require-Property $row 'archive_sha256' $context)
            decoded_sha256 = [string](Require-Property $row 'decoded_sha256' $context)
            codec_sha256 = $codecSha256
            source_package = $packagePath
        })
    }
    if ($modeKeys.Count -ne $expectedRows) {
        throw "Forced-mode $mode does not cover every expected input case"
    }
}

if ($caseByKey.Count -ne $expectedRows) {
    throw "Forced ledger has $($caseByKey.Count) distinct inputs; expected $expectedRows"
}

# Validate the retained full-Auto package too. It is not used for the forced
# oracle bytes, but it proves the ledger was planned as a complete comparison.
$autoEntry = $entryByMode['auto']
$autoResultsPath = Join-Path ([IO.Path]::GetFullPath([string]$autoEntry.package_path)) 'results.csv'
if (-not (Test-Path -LiteralPath $autoResultsPath -PathType Leaf)) {
    throw "Auto reference results are missing: $autoResultsPath"
}
$autoRows = @(Import-Csv -LiteralPath $autoResultsPath -Encoding UTF8)
if ($autoRows.Count -ne $expectedRows) {
    throw "Auto reference has $($autoRows.Count) rows; expected $expectedRows"
}
foreach ($row in $autoRows) {
    $context = 'Auto reference row'
    if ([string]$row.status -cne 'COMPLETE' -or [string]$row.roundtrip -cne 'PASS' -or
        [string]$row.variant -cne 'r2-auto' -or [string]$row.codec_sha256 -cne $ledgerCodecSha256) {
        throw "$context is not compatible with the forced ledger"
    }
    $scopeKiB = [int](Parse-Int64 $row.scope_kib "$context scope_kib")
    $inputSha256 = [string]$row.input_sha256
    $key = Get-CaseKey ([string]$row.file) $scopeKiB $inputSha256
    if (-not $caseByKey.ContainsKey($key)) {
        throw "$context input does not match the forced ledger: $key"
    }
}

$oracleRows = New-Object System.Collections.Generic.List[object]
$oracleByKey = New-Object 'System.Collections.Generic.Dictionary[string,object]' `
    ([System.StringComparer]::Ordinal)
foreach ($key in ($caseByKey.Keys | Sort-Object)) {
    $case = $caseByKey[$key]
    $candidates = @($forcedArchiveRows | Where-Object {
        $_.file -ceq $case.file -and $_.scope_kib -eq $case.scope_kib -and
        $_.input_sha256 -ceq $case.input_sha256
    })
    if ($candidates.Count -ne $ratioModes.Count) {
        throw "Forced oracle case has $($candidates.Count) modes; expected $($ratioModes.Count): $key"
    }
    $minimum = @($candidates.complete_archive_bytes | Measure-Object -Minimum).Minimum
    $winners = @($candidates | Where-Object { $_.complete_archive_bytes -eq $minimum } |
        Sort-Object mode_id)
    $winnerNames = @($winners | ForEach-Object { $_.mode })
    $oracle = [pscustomobject][ordered]@{
        file = $case.file
        scope_kib = $case.scope_kib
        input_bytes = $case.input_bytes
        input_sha256 = $case.input_sha256
        forced_mode_count = $ratioModes.Count
        oracle_complete_archive_bytes = [int64]$minimum
        tied_winner_count = $winnerNames.Count
        tied_winner_modes = [string]::Join(',', $winnerNames)
        codec_sha256 = $ledgerCodecSha256
    }
    $oracleRows.Add($oracle)
    $oracleByKey.Add($key, $oracle)
}

$recallRows = New-Object System.Collections.Generic.List[object]
$rankerIdentity = $null
$uncoveredE5Rows = 0
if (-not [string]::IsNullOrWhiteSpace($E5PackagePath)) {
    $E5PackagePath = [IO.Path]::GetFullPath($E5PackagePath)
    $e5RowsPath = Join-Path $E5PackagePath 'matrix_rows.csv'
    if (-not (Test-Path -LiteralPath $e5RowsPath -PathType Leaf)) {
        throw "E5 matrix rows are missing: $e5RowsPath"
    }
    $matrixRows = @(Import-Csv -LiteralPath $e5RowsPath -Encoding UTF8)
    foreach ($row in ($matrixRows | Where-Object { $_.policy -in @('auto-k2', 'auto-k4', 'auto-k8') })) {
        $context = "E5 $($row.policy) row"
        if ([string]$row.status -cne 'COMPLETE' -or [string]$row.roundtrip -cne 'PASS') {
            throw "$context is not COMPLETE/PASS"
        }
        if ([string]$row.codec_sha256 -cne $ledgerCodecSha256) {
            throw "$context executable SHA-256 does not match the forced ledger"
        }
        $scopeKiB = [int](Parse-Int64 $row.scope_kib "$context scope_kib")
        $internalBlockKiB = [int](Parse-Int64 $row.block_size_kib "$context block_size_kib")
        $key = Get-CaseKey ([string]$row.file) $scopeKiB ([string]$row.input_sha256)
        if ($internalBlockKiB -ne $blockSizeKiB -or -not $oracleByKey.ContainsKey($key)) {
            ++$uncoveredE5Rows
            continue
        }
        $version = [string](Require-Property $row 'ranker_version' $context)
        $crc32 = [string](Require-Property $row 'ranker_crc32' $context)
        $sha256 = [string](Require-Property $row 'ranker_sha256' $context)
        if ($version -cnotmatch '^[0-9A-F]{8}$' -or $crc32 -cnotmatch '^[0-9A-F]{8}$') {
            throw "$context ranker version or CRC32 is malformed"
        }
        Assert-Sha256 $sha256 "$context ranker_sha256"
        $identity = "$version|$crc32|$sha256"
        if ($null -eq $rankerIdentity) {
            $rankerIdentity = $identity
        }
        elseif ($rankerIdentity -cne $identity) {
            throw 'E5 package contains multiple fixed-point ranker identities'
        }
        $candidateIds = Get-CandidateModeIds ([string]$row.candidate_modes) $context
        $requiredCount = switch ([string]$row.policy) {
            'auto-k2' { 2 }
            'auto-k4' { 4 }
            'auto-k8' { 8 }
        }
        if ($candidateIds.Count -ne $requiredCount) {
            throw "$context has $($candidateIds.Count) candidates; expected $requiredCount"
        }
        $oracle = $oracleByKey[$key]
        $winnerIds = @($oracle.tied_winner_modes.Split(',') | ForEach-Object { $modeIdByName[$_] })
        $covered = $false
        foreach ($winnerId in $winnerIds) {
            if ($candidateIds.Contains($winnerId)) {
                $covered = $true
                break
            }
        }
        $recallRows.Add([pscustomobject][ordered]@{
            policy = [string]$row.policy
            block_size_kib = $internalBlockKiB
            file = $oracle.file
            scope_kib = $oracle.scope_kib
            input_sha256 = $oracle.input_sha256
            candidate_modes = [string]$row.candidate_modes
            oracle_complete_archive_bytes = $oracle.oracle_complete_archive_bytes
            tied_winner_modes = $oracle.tied_winner_modes
            contains_tied_oracle_winner = $covered
            tie_aware_winner_recall_available = $true
            shortlist_complete_archive_bytes = (Parse-Int64 $row.archive_bytes "$context archive_bytes")
            regret_vs_forced_oracle_bytes = (Parse-Int64 $row.archive_bytes "$context archive_bytes") - $oracle.oracle_complete_archive_bytes
            ranker_version = $version
            ranker_crc32 = $crc32
            ranker_sha256 = $sha256
        })
    }
    if ($RequireE5Coverage -and $recallRows.Count -ne ($oracleRows.Count * 3)) {
        throw "E5 coverage is incomplete: $($recallRows.Count) matching rows; expected $($oracleRows.Count * 3)"
    }
}

$recallSummary = @($recallRows | Group-Object policy | ForEach-Object {
    $rows = @($_.Group)
    [pscustomobject][ordered]@{
        policy = $_.Name
        evaluated_blocks = $rows.Count
        tied_winner_hits = @($rows | Where-Object contains_tied_oracle_winner).Count
        tie_aware_winner_recall = if ($rows.Count -eq 0) { 0.0 } else {
            @($rows | Where-Object contains_tied_oracle_winner).Count / $rows.Count
        }
        aggregate_regret_vs_forced_oracle_bytes = @($rows | ForEach-Object {
            [int64]$_.regret_vs_forced_oracle_bytes
        } | Measure-Object -Sum).Sum
    }
})

$summary = [ordered]@{
    status = 'COMPLETE'
    evidence_boundary = 'Each forced input is exactly one internal HZ02 block. Oracle bytes include the complete archive. Tied winners are all ratio modes with the minimum complete archive bytes.'
    forced_ledger_path = $ForcedLedgerPath
    forced_ledger_id = [string]$manifest[0].ledger_id
    ratio_mode_count = $ratioModes.Count
    block_size_kib = $blockSizeKiB
    input_cases = $oracleRows.Count
    codec_sha256 = $ledgerCodecSha256
    e5_package_path = $E5PackagePath
    e5_matching_rows = $recallRows.Count
    e5_uncovered_rows = $uncoveredE5Rows
    ranker_identity = $rankerIdentity
    tie_aware_recall_available = $recallRows.Count -gt 0
}

if ($ListOnly) {
    $summary.runtime_started = $false
    $summary | ConvertTo-Json -Depth 8
    return
}
if ([string]::IsNullOrWhiteSpace($OutputPath)) {
    throw 'OutputPath is required unless -ListOnly is used'
}
$OutputPath = [IO.Path]::GetFullPath($OutputPath)
if (Test-Path -LiteralPath $OutputPath) {
    throw "Refusing to overwrite forced-oracle output package: $OutputPath"
}
New-Item -ItemType Directory -Path $OutputPath | Out-Null
Write-Csv (Join-Path $OutputPath 'forced_archive_rows.csv') $forcedArchiveRows.ToArray()
Write-Csv (Join-Path $OutputPath 'forced_oracle_rows.csv') $oracleRows.ToArray()
Write-Csv (Join-Path $OutputPath 'tie_aware_recall_rows.csv') $recallRows.ToArray()
Write-Csv (Join-Path $OutputPath 'tie_aware_recall_summary.csv') $recallSummary
Write-Utf8Json (Join-Path $OutputPath 'summary.json') $summary
Write-Host "Forced oracle derivation complete: $OutputPath"
