[CmdletBinding()]
param(
    [string]$FeatureDumpPath = ''
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

function Write-Utf8Json([string]$Path, $Value) {
    $encoding = New-Object System.Text.UTF8Encoding($false)
    [IO.File]::WriteAllText($Path, (($Value | ConvertTo-Json -Depth 8) + "`n"),
        $encoding)
}

function Write-Csv([string]$Path, [object[]]$Rows) {
    $encoding = New-Object System.Text.UTF8Encoding($false)
    $content = [string]::Join("`r`n", @($Rows | ConvertTo-Csv -NoTypeInformation)) + "`r`n"
    [IO.File]::WriteAllText($Path, $content, $encoding)
}

$scriptRoot = Split-Path -Parent $MyInvocation.MyCommand.Path
$exportScript = Join-Path $scriptRoot 'export_r2_ranker_training_set.ps1'
if (-not (Test-Path -LiteralPath $exportScript -PathType Leaf)) {
    throw "Ranker training-data exporter is missing: $exportScript"
}
if ([string]::IsNullOrWhiteSpace($FeatureDumpPath)) {
    $FeatureDumpPath = Join-Path $scriptRoot '..\build\Release\hz_r2_feature_dump.exe'
}
$FeatureDumpPath = [IO.Path]::GetFullPath($FeatureDumpPath)
if (-not (Test-Path -LiteralPath $FeatureDumpPath -PathType Leaf)) {
    throw "Feature-dump executable is missing: $FeatureDumpPath"
}

$testRoot = Join-Path ([IO.Path]::GetTempPath()) (
    'hybridzip-ranker-training-test-' + [Guid]::NewGuid().ToString('N'))
try {
    $datasetPath = Join-Path $testRoot 'dataset'
    $oraclePath = Join-Path $testRoot 'forced-oracle'
    $outputPath = Join-Path $testRoot 'training-data'
    New-Item -ItemType Directory -Path $datasetPath, $oraclePath | Out-Null

    $rows = New-Object System.Collections.Generic.List[object]
    foreach ($name in @('alpha', 'beta')) {
        $bytes = [byte[]]::new(32768)
        for ($index = 0; $index -lt $bytes.Length; ++$index) {
            $bytes[$index] = if ($name -eq 'alpha') {
                [byte](65 + ($index % 23))
            }
            else {
                [byte](($index * 17 + 29) % 256)
            }
        }
        $sourcePath = Join-Path $datasetPath $name
        [IO.File]::WriteAllBytes($sourcePath, $bytes)
        $rows.Add([pscustomobject][ordered]@{
            file = $name
            scope_kib = 32
            input_sha256 = (Get-FileHash -LiteralPath $sourcePath -Algorithm SHA256).Hash.ToUpperInvariant()
            oracle_complete_archive_bytes = if ($name -eq 'alpha') { 321 } else { 654 }
            tied_winner_modes = if ($name -eq 'alpha') { 'zstd,fse' } else { 'paq8px-generic-sse' }
        })
    }
    Write-Csv (Join-Path $oraclePath 'forced_oracle_rows.csv') $rows.ToArray()
    Write-Utf8Json (Join-Path $oraclePath 'summary.json') ([ordered]@{
        status = 'COMPLETE'
        block_size_kib = 32
        input_cases = 2
    })

    $preview = & $exportScript -ForcedOraclePath $oraclePath -DatasetPath $datasetPath `
        -FeatureDumpPath $FeatureDumpPath -ValidationFiles beta -ListOnly | ConvertFrom-Json
    if ($preview.runtime_started -ne $false -or $preview.codec_invocations -ne 0 -or
        $preview.feature_dump_invocations -ne 2 -or $preview.no_leakage_partition -cne 'file-level') {
        throw 'Ranker training-data ListOnly preview is incorrect'
    }

    & $exportScript -ForcedOraclePath $oraclePath -DatasetPath $datasetPath `
        -FeatureDumpPath $FeatureDumpPath -ValidationFiles beta -OutputPath $outputPath
    if (-not $?) {
        throw 'Ranker training-data export failed'
    }
    $summary = Get-Content -LiteralPath (Join-Path $outputPath 'summary.json') `
        -Raw -Encoding UTF8 | ConvertFrom-Json
    $examples = @(Import-Csv -LiteralPath (Join-Path $outputPath 'ranker_examples.csv') -Encoding UTF8)
    if ($summary.status -cne 'COMPLETE' -or $summary.runtime_started -ne $false -or
        $summary.codec_invocations -ne 0 -or -not $summary.feature_dump_started -or
        $summary.training_rows -ne 1 -or $summary.validation_rows -ne 1 -or
        $examples.Count -ne 2 -or @($examples[0].PSObject.Properties.Name |
            Where-Object { $_ -match '^f[0-2][0-9]$' }).Count -ne 28) {
        throw 'Ranker training-data export summary is incorrect'
    }
    $trainFiles = @($examples | Where-Object split -eq 'training' | ForEach-Object file)
    $validationFiles = @($examples | Where-Object split -eq 'validation' | ForEach-Object file)
    if ($trainFiles.Count -ne 1 -or $validationFiles.Count -ne 1 -or
        $trainFiles[0] -eq $validationFiles[0] -or $validationFiles[0] -cne 'beta' -or
        @($examples | Where-Object { $_.ranker_sha256 -cne
            '4B1AC26C40AD4DA50312FD3B694D7E636FB768C2336FE773BC82D36424C27A4B' }).Count -ne 0) {
        throw 'Ranker training-data export leaks files or has wrong model identity'
    }
    Write-Host 'Ranker training-data exporter: PASS'
}
finally {
    if (Test-Path -LiteralPath $testRoot) {
        Remove-Item -LiteralPath $testRoot -Recurse -Force
    }
}
