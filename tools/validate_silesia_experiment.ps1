[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)]
    [string]$PackagePath,
    [string]$CodecPath = (Join-Path $PSScriptRoot '..\build\Release\hybridzip.exe'),
    [string]$ExpectedDatasetPath = 'F:\paq8px\silesia',
    [ValidateSet(
        '', 'profile-v1', 'r2-auto', 'r2-stored', 'r2-predictive',
        'r2-donor-match', 'r2-zstd', 'r2-fse', 'r2-lzma'
    )]
    [string]$ExpectedVariant = ''
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

$expectedFiles = @(
    'dickens', 'mozilla', 'mr', 'nci', 'ooffice', 'osdb',
    'reymont', 'samba', 'sao', 'webster', 'x-ray', 'xml'
)
$expectedScopes = @(32, 64, 128)
$expectedHeader = @(
    'experiment_id', 'variant', 'repeat', 'case_order', 'file', 'scope_kib',
    'input_path', 'input_bytes', 'input_sha256', 'archive_path',
    'archive_bytes', 'archive_sha256', 'decoded_path', 'decoded_bytes',
    'decoded_sha256', 'encode_seconds', 'decode_seconds',
    'encode_peak_ram_mib', 'decode_peak_ram_mib', 'peak_ram_mib',
    'codec_sha256', 'parameters', 'encode_command', 'decode_command',
    'encode_exit_code', 'decode_exit_code', 'started_at', 'status',
    'roundtrip', 'block_types', 'notes'
)
$completeState = -join @([char]0x5B8C, [char]0x6210)

function Assert-True([bool]$Condition, [string]$Message) {
    if (-not $Condition) {
        throw $Message
    }
}

function Get-Sha256([string]$Path) {
    return (Get-FileHash -LiteralPath $Path -Algorithm SHA256).Hash
}

function Test-SamePath([string]$Left, [string]$Right) {
    return [string]::Equals(
        [System.IO.Path]::GetFullPath($Left).TrimEnd('\'),
        [System.IO.Path]::GetFullPath($Right).TrimEnd('\'),
        [System.StringComparison]::OrdinalIgnoreCase)
}

function Resolve-PackageFile([string]$Root, [string]$RelativePath) {
    Assert-True (-not [System.IO.Path]::IsPathRooted($RelativePath)) `
        "Artifact path must be relative: $RelativePath"
    $normalized = $RelativePath.Replace('/', '\')
    $resolved = [System.IO.Path]::GetFullPath((Join-Path $Root $normalized))
    $prefix = $Root.TrimEnd('\') + '\'
    Assert-True ($resolved.StartsWith(
        $prefix, [System.StringComparison]::OrdinalIgnoreCase)) `
        "Artifact path escapes package: $RelativePath"
    return $resolved
}

$PackagePath = [System.IO.Path]::GetFullPath($PackagePath)
$CodecPath = [System.IO.Path]::GetFullPath($CodecPath)
$ExpectedDatasetPath = [System.IO.Path]::GetFullPath($ExpectedDatasetPath)
$metadataPath = Join-Path $PackagePath 'experiment.json'
$resultsPath = Join-Path $PackagePath 'results.csv'

Assert-True (Test-Path -LiteralPath $PackagePath -PathType Container) `
    "Package directory not found: $PackagePath"
Assert-True (Test-Path -LiteralPath $CodecPath -PathType Leaf) `
    "Codec not found: $CodecPath"
Assert-True (Test-Path -LiteralPath $metadataPath -PathType Leaf) `
    "experiment.json not found: $metadataPath"
Assert-True (Test-Path -LiteralPath $resultsPath -PathType Leaf) `
    "results.csv not found: $resultsPath"

$utf8 = New-Object System.Text.UTF8Encoding($false, $true)
$metadataText = [System.IO.File]::ReadAllText($metadataPath, $utf8)
$metadata = $metadataText | ConvertFrom-Json
$rows = @(Import-Csv -LiteralPath $resultsPath)
$supportedVariants = @(
    'profile-v1', 'r2-auto', 'r2-stored', 'r2-predictive',
    'r2-donor-match', 'r2-zstd', 'r2-fse', 'r2-lzma'
)
$metadataVariants = @($metadata.variants)
Assert-True ($metadataVariants.Count -eq 1) `
    'metadata must contain exactly one variant'
$packageVariant = [string]$metadataVariants[0]
Assert-True ($supportedVariants -contains $packageVariant) `
    "metadata contains unsupported variant: $packageVariant"
if (-not [string]::IsNullOrWhiteSpace($ExpectedVariant)) {
    Assert-True ($packageVariant -eq $ExpectedVariant) `
        "metadata variant does not match expected variant: $ExpectedVariant"
}

Assert-True ([int]$metadata.schema_version -eq 1) 'schema_version must be 1'
Assert-True ($metadata.experiment_id -match '^[a-z0-9-]+$') `
    'experiment_id contains unsupported characters'
Assert-True ([string]$metadata.state -eq $completeState) `
    'experiment state is not complete'
Assert-True ([string]$metadata.dataset_name -eq 'Silesia') `
    'dataset_name must be Silesia'
Assert-True (Test-SamePath $metadata.dataset_path $ExpectedDatasetPath) `
    "dataset_path does not match canonical path: $ExpectedDatasetPath"
Assert-True (@($metadata.files).Count -eq $expectedFiles.Count) `
    'metadata file count mismatch'
Assert-True ((@($metadata.files) -join '|') -eq ($expectedFiles -join '|')) `
    'metadata file order or names mismatch'
Assert-True ((@($metadata.scopes_kib) -join '|') -eq ($expectedScopes -join '|')) `
    'metadata scopes must be 32,64,128'
Assert-True ([int]$metadata.repeat_count -eq 1) 'repeat_count must be 1'
Assert-True (Test-SamePath $metadata.codec_path $CodecPath) `
    'metadata codec_path does not match the validated executable'

$codecHash = Get-Sha256 $CodecPath
Assert-True ([string]$metadata.codec_sha256 -eq $codecHash) `
    'metadata codec_sha256 does not match the executable'
Assert-True ($rows.Count -eq 36) "results.csv has $($rows.Count) rows, expected 36"
Assert-True (($rows[0].PSObject.Properties.Name -join '|') -eq
    ($expectedHeader -join '|')) 'results.csv header mismatch'

$duplicates = @($rows | Group-Object file, scope_kib, variant, repeat |
    Where-Object Count -ne 1)
Assert-True ($duplicates.Count -eq 0) 'duplicate result key found'

$seen = New-Object 'System.Collections.Generic.HashSet[string]'
$inputBytesTotal = 0L
$archiveBytesTotal = 0L
$encodeSecondsTotal = 0.0
$decodeSecondsTotal = 0.0
$peakMiB = 0.0
for ($index = 0; $index -lt $rows.Count; ++$index) {
    $row = $rows[$index]
    $position = $index + 1
    Assert-True ([string]$row.experiment_id -eq [string]$metadata.experiment_id) `
        "row $position experiment_id mismatch"
    Assert-True ([int]$row.case_order -eq $position) `
        "row $position case_order mismatch"
    Assert-True ($expectedFiles -contains [string]$row.file) `
        "row $position has unknown file"
    Assert-True ($expectedScopes -contains [int]$row.scope_kib) `
        "row $position has unknown scope"
    Assert-True ([string]$row.variant -eq $packageVariant) `
        "row $position variant mismatch"
    Assert-True ([int]$row.repeat -eq 1) "row $position repeat mismatch"
    Assert-True ([string]$row.status -eq 'COMPLETE') `
        "row $position status is not COMPLETE"
    Assert-True ([string]$row.roundtrip -eq 'PASS') `
        "row $position roundtrip is not PASS"
    Assert-True ([int]$row.encode_exit_code -eq 0) `
        "row $position encode exit code is not zero"
    Assert-True ([int]$row.decode_exit_code -eq 0) `
        "row $position decode exit code is not zero"
    Assert-True ([string]$row.codec_sha256 -eq $codecHash) `
        "row $position codec hash mismatch"

    $key = "$($row.file)|$($row.scope_kib)|$($row.variant)|$($row.repeat)"
    Assert-True ($seen.Add($key)) "row $position duplicates key $key"
    $expectedBytes = [int64]$row.scope_kib * 1024L
    Assert-True ([int64]$row.input_bytes -eq $expectedBytes) `
        "row $position input_bytes does not match scope"

    $inputPath = Resolve-PackageFile $PackagePath $row.input_path
    $archivePath = Resolve-PackageFile $PackagePath $row.archive_path
    $decodedPath = Resolve-PackageFile $PackagePath $row.decoded_path
    foreach ($path in @($inputPath, $archivePath, $decodedPath)) {
        Assert-True (Test-Path -LiteralPath $path -PathType Leaf) `
            "row $position artifact missing: $path"
    }
    Assert-True ((Get-Item -LiteralPath $inputPath).Length -eq [int64]$row.input_bytes) `
        "row $position input length mismatch"
    Assert-True ((Get-Item -LiteralPath $archivePath).Length -eq [int64]$row.archive_bytes) `
        "row $position archive length mismatch"
    Assert-True ((Get-Item -LiteralPath $decodedPath).Length -eq [int64]$row.decoded_bytes) `
        "row $position decoded length mismatch"
    Assert-True ([int64]$row.decoded_bytes -eq [int64]$row.input_bytes) `
        "row $position decoded byte count differs from input"

    $inputHash = Get-Sha256 $inputPath
    $archiveHash = Get-Sha256 $archivePath
    $decodedHash = Get-Sha256 $decodedPath
    Assert-True ([string]$row.input_sha256 -eq $inputHash) `
        "row $position input SHA-256 mismatch"
    Assert-True ([string]$row.archive_sha256 -eq $archiveHash) `
        "row $position archive SHA-256 mismatch"
    Assert-True ([string]$row.decoded_sha256 -eq $decodedHash) `
        "row $position decoded SHA-256 mismatch"
    Assert-True ($inputHash -eq $decodedHash) `
        "row $position is not byte-exact"

    $timestamp = [DateTimeOffset]::MinValue
    Assert-True ([DateTimeOffset]::TryParse(
        [string]$row.started_at, [ref]$timestamp)) `
        "row $position started_at is invalid"
    Assert-True ([double]$row.encode_seconds -ge 0) `
        "row $position encode time is negative"
    Assert-True ([double]$row.decode_seconds -ge 0) `
        "row $position decode time is negative"
    $expectedPeak = [Math]::Max(
        [double]$row.encode_peak_ram_mib, [double]$row.decode_peak_ram_mib)
    Assert-True ([Math]::Abs([double]$row.peak_ram_mib - $expectedPeak) -lt 1e-9) `
        "row $position peak RAM is not max(encode, decode)"

    $inputBytesTotal += [int64]$row.input_bytes
    $archiveBytesTotal += [int64]$row.archive_bytes
    $encodeSecondsTotal += [double]$row.encode_seconds
    $decodeSecondsTotal += [double]$row.decode_seconds
    $peakMiB = [Math]::Max($peakMiB, [double]$row.peak_ram_mib)
}

$expectedKeys = foreach ($file in $expectedFiles) {
    foreach ($scope in $expectedScopes) {
        "$file|$scope|$packageVariant|1"
    }
}
foreach ($key in $expectedKeys) {
    Assert-True ($seen.Contains($key)) "missing expected key $key"
}

[pscustomobject]@{
    package_path = $PackagePath
    experiment_id = [string]$metadata.experiment_id
    variant = $packageVariant
    rows = $rows.Count
    input_bytes = $inputBytesTotal
    archive_bytes = $archiveBytesTotal
    ratio = $archiveBytesTotal / $inputBytesTotal
    bpb = ($archiveBytesTotal * 8.0) / $inputBytesTotal
    encode_seconds = $encodeSecondsTotal
    decode_seconds = $decodeSecondsTotal
    peak_ram_mib = $peakMiB
    codec_sha256 = $codecHash
    experiment_json_sha256 = Get-Sha256 $metadataPath
    results_csv_sha256 = Get-Sha256 $resultsPath
    status = 'PASS'
} | Format-List
