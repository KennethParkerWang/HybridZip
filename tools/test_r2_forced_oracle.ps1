[CmdletBinding()]
param()

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

$scriptRoot = Split-Path -Parent $MyInvocation.MyCommand.Path
$deriveScript = Join-Path $scriptRoot 'derive_r2_forced_oracle.ps1'
if (-not (Test-Path -LiteralPath $deriveScript -PathType Leaf)) {
    throw "Forced-oracle derivation script is missing: $deriveScript"
}

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

function Write-Csv([string]$Path, [object[]]$Rows, [string]$Delimiter = ',') {
    $encoding = New-Object System.Text.UTF8Encoding($false)
    $content = [string]::Join(
        "`r`n", @($Rows | ConvertTo-Csv -NoTypeInformation -Delimiter $Delimiter)) + "`r`n"
    [IO.File]::WriteAllText($Path, $content, $encoding)
}

$testRoot = Join-Path ([IO.Path]::GetTempPath()) (
    'hybridzip-forced-oracle-test-' + [Guid]::NewGuid().ToString('N'))
New-Item -ItemType Directory -Path $testRoot | Out-Null
try {
    $ledgerPath = Join-Path $testRoot 'ledger'
    $e5Path = Join-Path $testRoot 'e5'
    $outputPath = Join-Path $testRoot 'derived'
    New-Item -ItemType Directory -Path $ledgerPath | Out-Null
    New-Item -ItemType Directory -Path $e5Path | Out-Null

    $codecHash = 'A' * 64
    $inputHash = 'B' * 64
    $archiveHash = 'C' * 64
    $manifestRows = New-Object System.Collections.Generic.List[object]
    foreach ($mode in (@('auto') + $ratioModes)) {
        $packagePath = Join-Path $ledgerPath $mode
        New-Item -ItemType Directory -Path $packagePath | Out-Null
        $modeIndex = if ($mode -eq 'auto') {
            -1
        }
        else {
            [Array]::IndexOf($ratioModes, $mode)
        }
        $archiveBytes = if ($mode -eq 'auto') {
            490
        }
        elseif ($mode -in @('zstd', 'fse')) {
            500
        }
        else {
            700
        }
        Write-Csv (Join-Path $packagePath 'results.csv') @(
            [pscustomobject][ordered]@{
                experiment_id = 'synthetic'
                variant = 'r2-' + $mode
                file = 'dickens'
                scope_kib = 32
                input_bytes = 32768
                input_sha256 = $inputHash
                archive_bytes = $archiveBytes
                archive_sha256 = $archiveHash
                decoded_sha256 = $inputHash
                codec_sha256 = $codecHash
                status = 'COMPLETE'
                roundtrip = 'PASS'
                block_types = $mode + '=1'
            })
        $manifestRows.Add([pscustomobject][ordered]@{
            ledger_id = 'synthetic-ledger'
            mode = $mode
            mode_index = $modeIndex
            package_path = $packagePath
            expected_rows = 1
            block_size_kib = 32
            status = 'COMPLETE'
        })
    }
    Write-Csv (Join-Path $ledgerPath 'manifest.tsv') $manifestRows.ToArray() "`t"

    $e5Rows = @(
        [pscustomobject][ordered]@{
            policy = 'auto-k2'; block_size_kib = 32; file = 'dickens';
            scope_kib = 32; input_sha256 = $inputHash;
            candidate_modes = '0:1,36:1'; archive_bytes = 530;
            codec_sha256 = $codecHash; status = 'COMPLETE'; roundtrip = 'PASS';
            ranker_version = '00010000'; ranker_crc32 = '1025B343';
            ranker_sha256 = 'D' * 64
        },
        [pscustomobject][ordered]@{
            policy = 'auto-k4'; block_size_kib = 32; file = 'dickens';
            scope_kib = 32; input_sha256 = $inputHash;
            candidate_modes = '0:1,2:1,36:1,37:1'; archive_bytes = 520;
            codec_sha256 = $codecHash; status = 'COMPLETE'; roundtrip = 'PASS';
            ranker_version = '00010000'; ranker_crc32 = '1025B343';
            ranker_sha256 = 'D' * 64
        },
        [pscustomobject][ordered]@{
            policy = 'auto-k8'; block_size_kib = 32; file = 'dickens';
            scope_kib = 32; input_sha256 = $inputHash;
            candidate_modes = '0:1,2:1,3:1,4:1,27:1,28:1,36:1,37:1';
            archive_bytes = 510; codec_sha256 = $codecHash;
            status = 'COMPLETE'; roundtrip = 'PASS';
            ranker_version = '00010000'; ranker_crc32 = '1025B343';
            ranker_sha256 = 'D' * 64
        }
    )
    Write-Csv (Join-Path $e5Path 'matrix_rows.csv') $e5Rows

    & $deriveScript -ForcedLedgerPath $ledgerPath -E5PackagePath $e5Path `
        -OutputPath $outputPath -RequireE5Coverage
    if (-not $?) {
        throw 'Forced-oracle derivation failed'
    }
    $summary = Get-Content -LiteralPath (Join-Path $outputPath 'summary.json') `
        -Raw -Encoding UTF8 | ConvertFrom-Json
    if ($summary.input_cases -ne 1 -or $summary.e5_matching_rows -ne 3 -or
        -not $summary.tie_aware_recall_available) {
        throw 'Synthetic forced-oracle summary is incorrect'
    }
    $oracleRows = @(Import-Csv -LiteralPath (Join-Path $outputPath 'forced_oracle_rows.csv') -Encoding UTF8)
    if ($oracleRows.Count -ne 1 -or $oracleRows[0].tied_winner_modes -cne 'zstd,fse' -or
        [int]$oracleRows[0].oracle_complete_archive_bytes -ne 500) {
        throw 'Synthetic tied forced oracle is incorrect'
    }
    $recallRows = @(Import-Csv -LiteralPath (Join-Path $outputPath 'tie_aware_recall_rows.csv') `
        -Encoding UTF8 | Sort-Object policy)
    if ($recallRows.Count -ne 3 -or
        [bool]::Parse($recallRows[0].contains_tied_oracle_winner) -or
        -not [bool]::Parse($recallRows[1].contains_tied_oracle_winner) -or
        -not [bool]::Parse($recallRows[2].contains_tied_oracle_winner)) {
        throw 'Synthetic tie-aware recall is incorrect'
    }
    $preview = & $deriveScript -ForcedLedgerPath $ledgerPath -E5PackagePath $e5Path `
        -RequireE5Coverage -ListOnly | ConvertFrom-Json
    if ($preview.runtime_started -ne $false -or $preview.e5_matching_rows -ne 3) {
        throw 'Synthetic forced-oracle ListOnly preview is incorrect'
    }
    Write-Host 'Synthetic forced-oracle derivation: PASS'
}
finally {
    if (Test-Path -LiteralPath $testRoot) {
        Remove-Item -LiteralPath $testRoot -Recurse -Force
    }
}
