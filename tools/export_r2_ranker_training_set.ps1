[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)]
    [string]$ForcedOraclePath,
    [string]$DatasetPath = 'F:\paq8px\silesia',
    [string]$FeatureDumpPath = '',
    [string]$OutputPath = '',
    [Parameter(Mandatory = $true)]
    [string[]]$ValidationFiles,
    [switch]$ListOnly
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

function Get-NormalizedDirectoryPath([string]$Path) {
    $fullPath = [IO.Path]::GetFullPath($Path)
    $root = [IO.Path]::GetPathRoot($fullPath)
    if ($fullPath.Length -gt $root.Length) {
        return $fullPath.TrimEnd([char[]]@(
            [IO.Path]::DirectorySeparatorChar,
            [IO.Path]::AltDirectorySeparatorChar
        ))
    }
    return $fullPath
}

function Get-Sha256([string]$Path) {
    return (Get-FileHash -LiteralPath $Path -Algorithm SHA256).Hash.ToUpperInvariant()
}

function Get-PrefixSha256([string]$Path, [int]$Bytes) {
    $stream = [IO.File]::Open($Path, [IO.FileMode]::Open, [IO.FileAccess]::Read,
        [IO.FileShare]::Read)
    $hash = [Security.Cryptography.SHA256]::Create()
    try {
        $buffer = New-Object byte[] 65536
        $remaining = $Bytes
        while ($remaining -gt 0) {
            $requested = [Math]::Min($buffer.Length, $remaining)
            $read = $stream.Read($buffer, 0, $requested)
            if ($read -le 0) {
                throw "Source is shorter than the requested prefix: $Path"
            }
            [void]$hash.TransformBlock($buffer, 0, $read, $buffer, 0)
            $remaining -= $read
        }
        [void]$hash.TransformFinalBlock([byte[]]::new(0), 0, 0)
        return [Convert]::ToHexString($hash.Hash)
    }
    finally {
        $hash.Dispose()
        $stream.Dispose()
    }
}

function Require-Property($Object, [string]$Name, [string]$Context) {
    $property = $Object.PSObject.Properties[$Name]
    if ($null -eq $property) {
        throw "$Context is missing required property: $Name"
    }
    return $property.Value
}

function Parse-Int([object]$Value, [string]$Context) {
    [int]$parsed = 0
    if (-not [int]::TryParse([string]$Value,
            [Globalization.NumberStyles]::Integer,
            [Globalization.CultureInfo]::InvariantCulture,
            [ref]$parsed)) {
        throw "$Context is not an integer: $Value"
    }
    return $parsed
}

function Write-Utf8Json([string]$Path, $Value) {
    $encoding = New-Object System.Text.UTF8Encoding($false)
    [IO.File]::WriteAllText($Path, (($Value | ConvertTo-Json -Depth 8) + "`n"),
        $encoding)
}

function Write-Csv([string]$Path, [object[]]$Rows) {
    $encoding = New-Object System.Text.UTF8Encoding($false)
    $content = if ($Rows.Count -eq 0) { '' } else {
        [string]::Join("`r`n", @($Rows | ConvertTo-Csv -NoTypeInformation)) + "`r`n"
    }
    [IO.File]::WriteAllText($Path, $content, $encoding)
}

$scriptRoot = Split-Path -Parent $MyInvocation.MyCommand.Path
if ([string]::IsNullOrWhiteSpace($scriptRoot)) {
    throw 'Unable to resolve the script directory'
}
if ([string]::IsNullOrWhiteSpace($FeatureDumpPath)) {
    $FeatureDumpPath = Join-Path $scriptRoot '..\build\Release\hz_r2_feature_dump.exe'
}

$ForcedOraclePath = Get-NormalizedDirectoryPath $ForcedOraclePath
$DatasetPath = Get-NormalizedDirectoryPath $DatasetPath
$FeatureDumpPath = [IO.Path]::GetFullPath($FeatureDumpPath)
$summaryPath = Join-Path $ForcedOraclePath 'summary.json'
$rowsPath = Join-Path $ForcedOraclePath 'forced_oracle_rows.csv'
if (-not (Test-Path -LiteralPath $summaryPath -PathType Leaf) -or
    -not (Test-Path -LiteralPath $rowsPath -PathType Leaf)) {
    throw "Forced-oracle evidence is incomplete: $ForcedOraclePath"
}
if (-not (Test-Path -LiteralPath $DatasetPath -PathType Container)) {
    throw "Dataset directory not found: $DatasetPath"
}
$forcedSummary = Get-Content -LiteralPath $summaryPath -Raw -Encoding UTF8 |
    ConvertFrom-Json
$forcedBlockSize = Parse-Int (Require-Property $forcedSummary 'block_size_kib' `
    'Forced-oracle summary') 'Forced-oracle block_size_kib'
if ([string](Require-Property $forcedSummary 'status' 'Forced-oracle summary') -cne 'COMPLETE' -or
    $forcedBlockSize -ne 32) {
    throw 'Ranker training requires a COMPLETE 32 KiB forced-oracle package'
}

$oracleRows = @(Import-Csv -LiteralPath $rowsPath -Encoding UTF8)
if ($oracleRows.Count -eq 0) {
    throw 'Forced-oracle package has no label rows'
}
$caseByFile = @{}
foreach ($row in $oracleRows) {
    $file = [string](Require-Property $row 'file' 'Forced-oracle row')
    $scopeKiB = Parse-Int (Require-Property $row 'scope_kib' "Forced-oracle row $file") "Forced-oracle scope_kib for $file"
    if ($scopeKiB -ne 32) {
        throw "Forced-oracle row is not a 32 KiB block: $file"
    }
    $inputSha256 = [string](Require-Property $row 'input_sha256' "Forced-oracle row $file")
    if ($inputSha256 -cnotmatch '^[0-9A-F]{64}$') {
        throw "Forced-oracle input SHA-256 is malformed: $file"
    }
    $winners = [string](Require-Property $row 'tied_winner_modes' "Forced-oracle row $file")
    if ([string]::IsNullOrWhiteSpace($winners)) {
        throw "Forced-oracle row has no tied winners: $file"
    }
    if ($caseByFile.ContainsKey($file)) {
        throw "Forced-oracle package has duplicate file label: $file"
    }
    $caseByFile[$file] = $row
}

$allFiles = @($caseByFile.Keys | Sort-Object)
$validationSet = New-Object 'System.Collections.Generic.HashSet[string]' `
    ([System.StringComparer]::OrdinalIgnoreCase)
foreach ($entry in $ValidationFiles) {
    foreach ($requested in ([string]$entry).Split(',') | Where-Object {
        -not [string]::IsNullOrWhiteSpace([string]$_)
    }) {
        $canonical = @($allFiles | Where-Object {
            [string]::Equals($_, [string]$requested,
                [System.StringComparison]::OrdinalIgnoreCase)
        })
        if ($canonical.Count -ne 1 -or -not $validationSet.Add($canonical[0])) {
            throw "ValidationFiles has an invalid or duplicate oracle file: $requested"
        }
    }
}
if ($validationSet.Count -eq 0 -or $validationSet.Count -ge $allFiles.Count) {
    throw 'ValidationFiles must select at least one but not all oracle files'
}

$cases = New-Object System.Collections.Generic.List[object]
foreach ($file in $allFiles) {
    $sourcePath = Join-Path $DatasetPath $file
    if (-not (Test-Path -LiteralPath $sourcePath -PathType Leaf) -or
        (Get-Item -LiteralPath $sourcePath).Length -lt 32KB) {
        throw "Missing or too-short source for forced-oracle file: $sourcePath"
    }
    $row = $caseByFile[$file]
    $inputSha256 = Get-PrefixSha256 $sourcePath 32KB
    if ($inputSha256 -cne [string]$row.input_sha256) {
        throw "Dataset prefix SHA-256 does not match forced-oracle evidence: $file"
    }
    $cases.Add([pscustomobject]@{
        File = $file
        SourcePath = $sourcePath
        InputSha256 = $inputSha256
        OracleBytes = (Require-Property $row 'oracle_complete_archive_bytes' "Forced-oracle row $file")
        TiedWinnerModes = [string]$row.tied_winner_modes
        Split = if ($validationSet.Contains($file)) { 'validation' } else { 'training' }
    })
}

$plan = [ordered]@{
    status = 'PREVIEW'
    runtime_started = $false
    codec_invocations = 0
    feature_dump_invocations = $cases.Count
    feature_dump_started = $false
    forced_oracle_path = $ForcedOraclePath
    forced_oracle_summary_sha256 = Get-Sha256 $summaryPath
    dataset_path = $DatasetPath
    scope_kib = 32
    input_cases = $cases.Count
    training_files = @($cases | Where-Object Split -eq 'training' | ForEach-Object File)
    validation_files = @($cases | Where-Object Split -eq 'validation' | ForEach-Object File)
    no_leakage_partition = 'file-level'
    feature_dump_path = $FeatureDumpPath
}
if ($ListOnly) {
    $plan | ConvertTo-Json -Depth 8
    return
}

if ([string]::IsNullOrWhiteSpace($OutputPath)) {
    throw 'OutputPath is required unless -ListOnly is used'
}
$OutputPath = [IO.Path]::GetFullPath($OutputPath)
if (Test-Path -LiteralPath $OutputPath) {
    throw "Refusing to overwrite ranker training-data package: $OutputPath"
}
if (-not (Test-Path -LiteralPath $FeatureDumpPath -PathType Leaf)) {
    throw "Feature-dump executable not found: $FeatureDumpPath"
}

$examples = New-Object System.Collections.Generic.List[object]
$rankerIdentity = ''
foreach ($case in $cases) {
    $feature = & $FeatureDumpPath $case.SourcePath '--offset=0' '--length=32768' |
        ConvertFrom-Json
    if (-not $?) {
        throw "Feature dump failed: $($case.File)"
    }
    if ([string]$feature.schema -cne 'r2-block-features-v1' -or
        (Parse-Int $feature.input_bytes "Feature dump input_bytes for $($case.File)") -ne 32768 -or
        @($feature.feature_values).Count -ne 28 -or
        @($feature.auto_k8_mode_ids).Count -ne 8) {
        throw "Feature dump contract is invalid: $($case.File)"
    }
    $version = ('{0:X8}' -f [uint32]$feature.ranker_model.version)
    $crc32 = ('{0:X8}' -f [uint32]$feature.ranker_model.crc32)
    $sha256 = [string]$feature.ranker_model.sha256
    if ($sha256 -cnotmatch '^[0-9A-F]{64}$') {
        throw "Feature dump ranker SHA-256 is malformed: $($case.File)"
    }
    $identity = "$version|$crc32|$sha256"
    if ([string]::IsNullOrWhiteSpace($rankerIdentity)) {
        $rankerIdentity = $identity
    }
    elseif ($rankerIdentity -cne $identity) {
        throw 'Feature dump returned multiple ranker identities'
    }
    $record = [ordered]@{
        schema = 'r2-ranker-example-v1'
        split = $case.Split
        file = $case.File
        scope_kib = 32
        source_offset_bytes = 0
        input_bytes = 32768
        input_sha256 = $case.InputSha256
        oracle_complete_archive_bytes = [int64]$case.OracleBytes
        tied_winner_modes = $case.TiedWinnerModes
        block_class = [string]$feature.block_class
        auto_k8_mode_ids = (@($feature.auto_k8_mode_ids) -join ',')
        ranker_version = $version
        ranker_crc32 = $crc32
        ranker_sha256 = $sha256
    }
    for ($index = 0; $index -lt 28; ++$index) {
        $record[('f{0:D2}' -f $index)] = [int]$feature.feature_values[$index]
    }
    $examples.Add([pscustomobject]$record)
}

$plan.status = 'COMPLETE'
$plan.feature_dump_started = $true
$plan.feature_dump_sha256 = Get-Sha256 $FeatureDumpPath
$plan.ranker_identity = $rankerIdentity
$plan.training_rows = @($examples | Where-Object split -eq 'training').Count
$plan.validation_rows = @($examples | Where-Object split -eq 'validation').Count

New-Item -ItemType Directory -Path $OutputPath | Out-Null
Write-Csv (Join-Path $OutputPath 'ranker_examples.csv') $examples.ToArray()
Write-Utf8Json (Join-Path $OutputPath 'split.json') ([ordered]@{
    no_leakage_partition = 'file-level'
    training_files = $plan.training_files
    validation_files = $plan.validation_files
    source_forced_oracle_path = $ForcedOraclePath
    source_forced_oracle_summary_sha256 = $plan.forced_oracle_summary_sha256
})
Write-Utf8Json (Join-Path $OutputPath 'summary.json') $plan
Write-Host "Ranker training-data export complete: $OutputPath"
