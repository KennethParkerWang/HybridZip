[CmdletBinding()]
param(
    [string]$CodecPath = '',
    [string]$DatasetPath = 'F:\paq8px\silesia',
    [string]$OutputRoot = '',
    [string]$LedgerId = '',
    [ValidateSet(32, 64, 128)]
    [int[]]$ScopesKiB = @(32),
    [string[]]$SilesiaFiles = @(),
    [ValidateRange(1, 604800)]
    [int]$ProcessTimeoutSeconds = 3600,
    [switch]$Resume,
    [switch]$ContinueOnError,
    [switch]$AuthorizeRuntimeExperiment,
    [switch]$ListOnly
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

$scriptRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..'))
$runnerPath = Join-Path $scriptRoot 'tools\run_silesia_experiment.ps1'
if ([string]::IsNullOrWhiteSpace($CodecPath)) {
    $CodecPath = Join-Path $scriptRoot 'build\Release\hybridzip.exe'
}
if ([string]::IsNullOrWhiteSpace($OutputRoot)) {
    $OutputRoot = Join-Path $scriptRoot 'results\experiments'
}
$CodecPath = [IO.Path]::GetFullPath($CodecPath)
$DatasetPath = [IO.Path]::GetFullPath($DatasetPath)
$OutputRoot = [IO.Path]::GetFullPath($OutputRoot)

$allFiles = @(
    'dickens', 'mozilla', 'mr', 'nci', 'ooffice', 'osdb',
    'reymont', 'samba', 'sao', 'webster', 'x-ray', 'xml'
)
if ($SilesiaFiles.Count -eq 0) {
    $SilesiaFiles = @($allFiles)
}

# The order is the decoder-visible R2 mode order: stored is mode 0.
$modes = @(
    'auto', 'stored', 'predictive', 'zstd', 'fse', 'lzma', 'donor-match',
    'bwt-zstd', 'bwt-mtf-zstd', 'bwt-rlt-zstd', 'x86-bcj-zstd',
    'shuffle-zstd', 'bitshuffle-zstd', 'delta-zstd', 'delta-of-delta-zstd',
    'fastpfor', 'rans', 'bcj2-zstd', 'record-transpose-zstd', 'jpegls',
    'flac-residual', 'brotli-text', 'cmix-word-zstd', 'neural-lstm',
    'shared-neural-lstm', 'lstm-compress', 'bgpt-shared-prior',
    'jax-compress-portable', 'ppmd7', 'ppmd8', 'zpaq', 'ctw',
    'paq8px-apm', 'paq8px-record-model', 'paq8px-linear-prediction',
    'paq8px-similarity', 'paq8px-similarity-sse', 'paq8px-generic-sse',
    'paq8px-detected-sse', 'wavpack', 'lz4', 'kanzi-ans', 'lmic-arithmetic',
    'delta-binary-packed-zstd'
)
$forcedModes = @($modes | Where-Object { $_ -ne 'auto' })

if ($ListOnly) {
    [pscustomobject][ordered]@{
        modes = $modes.Count
        forced_modes = $forcedModes.Count
        files = [string]::Join(',', $SilesiaFiles)
        scopes_kib = [string]::Join(',', (@($ScopesKiB | Sort-Object -Unique)))
        codec_path = $CodecPath
        output_root = $OutputRoot
        runtime_started = $false
    } | ConvertTo-Json -Compress
    return
}

if (-not $AuthorizeRuntimeExperiment) {
    throw 'Refusing runtime execution. Re-run with -AuthorizeRuntimeExperiment after reviewing the manifest.'
}
if (-not (Test-Path -LiteralPath $runnerPath -PathType Leaf)) {
    throw "Silesia runner not found: $runnerPath"
}
if (-not (Test-Path -LiteralPath $CodecPath -PathType Leaf)) {
    throw "Codec executable not found: $CodecPath"
}
if (-not (Test-Path -LiteralPath $DatasetPath -PathType Container)) {
    throw "Dataset directory not found: $DatasetPath"
}

foreach ($file in $SilesiaFiles) {
    if ($allFiles -notcontains $file) {
        throw "Unknown Silesia file: $file"
    }
}
$ScopesKiB = @($ScopesKiB | Sort-Object -Unique)
if ($ScopesKiB.Count -eq 0) {
    throw 'At least one scope must be selected'
}

if ([string]::IsNullOrWhiteSpace($LedgerId)) {
    if ($Resume) {
        throw '-LedgerId is required with -Resume'
    }
    $LedgerId = 'hybridzip-r2-complete-{0}-{1}' -f `
        [DateTime]::UtcNow.ToString('yyyyMMdd-HHmmss'),
        [Guid]::NewGuid().ToString('N').Substring(0, 8)
}
if ($LedgerId -cnotmatch '^[a-z0-9-]+$') {
    throw "LedgerId must match ^[a-z0-9-]+`$: $LedgerId"
}

$analysisRoot = Join-Path $scriptRoot 'results\analysis\r2-complete-ledger'
$ledgerPath = Join-Path $analysisRoot $LedgerId
$manifestPath = Join-Path $ledgerPath 'manifest.tsv'
if (-not (Test-Path -LiteralPath $ledgerPath -PathType Container)) {
    New-Item -ItemType Directory -Path $ledgerPath -Force | Out-Null
}

function Write-NoBom([string]$Path, [string]$Text) {
    [IO.File]::WriteAllText($Path, $Text, (New-Object Text.UTF8Encoding($false)))
}

function Write-Manifest([object[]]$Rows) {
    $csv = (($Rows | ConvertTo-Csv -Delimiter "`t" -NoTypeInformation) -join "`r`n") + "`r`n"
    Write-NoBom $manifestPath $csv
}

$expectedRows = $SilesiaFiles.Count * $ScopesKiB.Count
$manifestRows = @()
if (Test-Path -LiteralPath $manifestPath -PathType Leaf) {
    if (-not $Resume) {
        throw "Manifest already exists; use -Resume to continue: $manifestPath"
    }
    $manifestRows = @(Import-Csv -LiteralPath $manifestPath -Delimiter "`t")
    if ($manifestRows.Count -ne $modes.Count) {
        throw "Manifest has $($manifestRows.Count) rows; expected $($modes.Count)"
    }
    $manifestModes = @($manifestRows | ForEach-Object mode)
    if (($manifestModes -join '|') -ne ($modes -join '|')) {
        throw 'Existing manifest mode order does not match the fixed R2 registry'
    }
    foreach ($row in $manifestRows) {
        if ([string]$row.ledger_id -cne $LedgerId) {
            throw "Manifest ledger_id mismatch: $($row.ledger_id)"
        }
        if ([int]$row.expected_rows -ne $expectedRows) {
            throw "Manifest scope/file mismatch for mode $($row.mode)"
        }
    }
}
else {
    foreach ($mode in $modes) {
        $packageName = "$LedgerId-$mode"
        $manifestRows += [pscustomobject][ordered]@{
            ledger_id = $LedgerId
            mode = $mode
            mode_index = if ($mode -eq 'auto') { -1 } else { [Array]::IndexOf($modes, $mode) - 1 }
            package_name = $packageName
            package_path = [IO.Path]::GetFullPath((Join-Path $OutputRoot $packageName))
            files = [string]::Join(',', $SilesiaFiles)
            scopes_kib = [string]::Join(',', $ScopesKiB)
            expected_rows = $expectedRows
            status = 'PENDING'
            started_at = ''
            completed_at = ''
            error = ''
        }
    }
    Write-Manifest $manifestRows
}

$failures = New-Object System.Collections.Generic.List[string]
for ($index = 0; $index -lt $manifestRows.Count; ++$index) {
    $row = $manifestRows[$index]
    $expectedPackagePath = [IO.Path]::GetFullPath((Join-Path $OutputRoot ([string]$row.package_name)))
    $packagePath = [IO.Path]::GetFullPath([string]$row.package_path)
    if (-not [string]::Equals(
        $packagePath.TrimEnd('\'),
        $expectedPackagePath.TrimEnd('\'),
        [StringComparison]::OrdinalIgnoreCase
    )) {
        throw "Manifest package path does not match OutputRoot/package_name for mode $($row.mode)"
    }
    if (Test-Path -LiteralPath $packagePath) {
        if (-not $Resume) {
            throw "Refusing to overwrite existing package: $packagePath"
        }
    }

    $row.status = 'TESTING'
    $row.started_at = [DateTimeOffset]::Now.ToString('o')
    $row.completed_at = ''
    $row.error = ''
    Write-Manifest $manifestRows
    Write-Host ("[{0}/{1}] {2}: starting ({3})" -f ($index + 1), $manifestRows.Count, $row.mode, $row.package_name)

    try {
        $invoke = @{
            CodecPath = $CodecPath
            DatasetPath = $DatasetPath
            OutputRoot = $OutputRoot
            ExperimentId = [string]$row.package_name
            Profile = 'r2'
            R2Mode = [string]$row.mode
            ScopesKiB = $ScopesKiB
            SilesiaFiles = $SilesiaFiles
            ProcessTimeoutSeconds = $ProcessTimeoutSeconds
        }
        if ($Resume -and (Test-Path -LiteralPath $packagePath -PathType Container)) {
            $invoke.Resume = $true
        }
        & $runnerPath @invoke
        $row.status = 'COMPLETE'
        $row.completed_at = [DateTimeOffset]::Now.ToString('o')
        Write-Host ("[{0}/{1}] {2}: complete" -f ($index + 1), $manifestRows.Count, $row.mode)
    }
    catch {
        $row.status = 'FAILED'
        $row.completed_at = [DateTimeOffset]::Now.ToString('o')
        $row.error = $_.Exception.Message
        $failures.Add(("{0}: {1}" -f $row.mode, $row.error))
        Write-Warning ("[{0}/{1}] {2}: failed: {3}" -f ($index + 1), $manifestRows.Count, $row.mode, $row.error)
        Write-Manifest $manifestRows
        if (-not $ContinueOnError) {
            throw
        }
    }
    Write-Manifest $manifestRows
}

if ($failures.Count -ne 0) {
    throw ("Complete ledger runner finished with failures: " + [string]::Join('; ', $failures))
}
Write-Output ("complete=$ledgerPath modes=$($modes.Count) forced=$($forcedModes.Count) rows_per_mode=$expectedRows")
