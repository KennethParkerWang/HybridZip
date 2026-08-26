[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)]
    [string]$ManifestPath,
    [string]$OutputPath = '',
    [string]$ExpectedCodecSha256 = ''
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

$ManifestPath = [IO.Path]::GetFullPath($ManifestPath)
if (-not (Test-Path -LiteralPath $ManifestPath -PathType Leaf)) {
    throw "Manifest not found: $ManifestPath"
}
if ([string]::IsNullOrWhiteSpace($OutputPath)) {
    $OutputPath = Join-Path (Split-Path -Parent $ManifestPath) 'derived'
}
$OutputPath = [IO.Path]::GetFullPath($OutputPath)
if (Test-Path -LiteralPath $OutputPath) {
    throw "Output already exists: $OutputPath"
}

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

function Write-NoBom([string]$Path, [string]$Text) {
    [IO.File]::WriteAllText($Path, $Text, (New-Object Text.UTF8Encoding($false)))
}

function Get-Sha256([string]$Path) {
    (Get-FileHash -LiteralPath $Path -Algorithm SHA256).Hash.ToUpperInvariant()
}

function Get-SelectedModes([string]$Mode, [string]$BlockTypes) {
    if ($Mode -ne 'auto') {
        return @($Mode)
    }
    if ([string]::IsNullOrWhiteSpace($BlockTypes) -or $BlockTypes -eq 'none') {
        return @()
    }
    $selected = New-Object System.Collections.Generic.List[string]
    foreach ($part in $BlockTypes.Split(';')) {
        $match = [regex]::Match($part.Trim(), '^(?<name>[^=]+)=(?<count>[0-9]+)$')
        if ($match.Success -and [int]$match.Groups['count'].Value -gt 0) {
            $selected.Add($match.Groups['name'].Value)
        }
    }
    return $selected.ToArray()
}

function Get-RequiredProperty($Object, [string]$Name) {
    $property = $Object.PSObject.Properties[$Name]
    if ($null -eq $property) {
        throw "Missing required property '$Name'"
    }
    return [string]$property.Value
}

function Get-ExpectedKey([string]$File, [int]$Scope) {
    return "$File|$Scope"
}

function Assert-EqualHash([string]$Expected, [string]$Actual, [string]$Description) {
    if (-not [string]::Equals($Expected, $Actual, [StringComparison]::OrdinalIgnoreCase)) {
        throw "SHA-256 mismatch for ${Description}: expected $Expected, found $Actual"
    }
}

function Get-RequiredNonNegativeDouble($Object, [string]$Name, [string]$Description) {
    $raw = [string](Get-RequiredProperty $Object $Name)
    if ([string]::IsNullOrWhiteSpace($raw)) {
        throw "Missing numeric value for ${Description}"
    }
    [double]$value = 0.0
    $parsed = [double]::TryParse(
        $raw.Trim(),
        [Globalization.NumberStyles]::Float,
        [Globalization.CultureInfo]::InvariantCulture,
        [ref]$value
    )
    if (-not $parsed -or [double]::IsNaN($value) -or
        [double]::IsInfinity($value) -or $value -lt 0.0) {
        throw "Invalid non-negative finite number for ${Description}: $raw"
    }
    return $value
}

function Get-RequiredInteger($Object, [string]$Name, [string]$Description) {
    $raw = [string](Get-RequiredProperty $Object $Name)
    if ([string]::IsNullOrWhiteSpace($raw)) {
        throw "Missing integer value for ${Description}"
    }
    [int]$value = 0
    $parsed = [int]::TryParse(
        $raw.Trim(),
        [Globalization.NumberStyles]::Integer,
        [Globalization.CultureInfo]::InvariantCulture,
        [ref]$value
    )
    if (-not $parsed) {
        throw "Invalid integer for ${Description}: $raw"
    }
    return $value
}

function Get-RequiredSha256($Object, [string]$Name, [string]$Description) {
    $value = ([string](Get-RequiredProperty $Object $Name)).Trim().ToUpperInvariant()
    if ($value -notmatch '^[0-9A-F]{64}$') {
        throw "Invalid SHA-256 for ${Description}: $value"
    }
    return $value
}

$manifestRows = @(Import-Csv -LiteralPath $ManifestPath -Delimiter "`t")
if ($manifestRows.Count -ne $modes.Count) {
    throw "Manifest has $($manifestRows.Count) rows; expected $($modes.Count)"
}
$manifestModes = @($manifestRows | ForEach-Object { [string]$_.mode })
if (($manifestModes -join '|') -ne ($modes -join '|')) {
    throw 'Manifest mode order does not match the fixed decoder-visible R2 registry'
}

$ledgerId = Get-RequiredProperty $manifestRows[0] 'ledger_id'
$files = @((Get-RequiredProperty $manifestRows[0] 'files').Split(',') |
    Where-Object { -not [string]::IsNullOrWhiteSpace($_) })
$scopes = @((Get-RequiredProperty $manifestRows[0] 'scopes_kib').Split(',') |
    ForEach-Object { [int]$_.Trim() } | Sort-Object -Unique)
if ($files.Count -eq 0 -or $scopes.Count -eq 0) {
    throw 'Manifest has no files or scopes'
}
$expectedRows = $files.Count * $scopes.Count

$codecHashes = New-Object 'System.Collections.Generic.HashSet[string]'
$normalized = New-Object System.Collections.Generic.List[object]
$modeMaps = @{}
foreach ($mode in $modes) {
    $modeMaps[$mode] = @{}
}

foreach ($manifestRow in $manifestRows) {
    $mode = [string]$manifestRow.mode
    if ([string]$manifestRow.ledger_id -cne $ledgerId) {
        throw "Manifest ledger_id mismatch for mode $mode"
    }
    $packagePath = [IO.Path]::GetFullPath([string]$manifestRow.package_path)
    $metadataPath = Join-Path $packagePath 'experiment.json'
    $resultsPath = Join-Path $packagePath 'results.csv'
    if (-not (Test-Path -LiteralPath $metadataPath -PathType Leaf) -or
        -not (Test-Path -LiteralPath $resultsPath -PathType Leaf)) {
        throw "Incomplete package for mode ${mode}: $packagePath"
    }
    try {
        $metadata = Get-Content -LiteralPath $metadataPath -Raw -Encoding UTF8 | ConvertFrom-Json
    }
    catch {
        throw "Invalid experiment metadata for mode ${mode}: $($_.Exception.Message)"
    }
    $rows = @(Import-Csv -LiteralPath $resultsPath -Encoding UTF8)
    if ($rows.Count -ne $expectedRows) {
        throw "Mode $mode has $($rows.Count) rows; expected $expectedRows"
    }
    $expectedVariant = "r2-$mode"
    $modeMap = $modeMaps[$mode]
    foreach ($row in $rows) {
        if ([string]$row.variant -cne $expectedVariant -or
            [string]$row.status -cne 'COMPLETE' -or
            [string]$row.roundtrip -cne 'PASS') {
            throw "Mode ${mode} contains a non-COMPLETE/PASS row: $($row.file)/$($row.scope_kib)"
        }
        $file = [string]$row.file
        $scope = [int]$row.scope_kib
        if ($files -notcontains $file -or $scopes -notcontains $scope) {
            throw "Unexpected case in mode ${mode}: $file/$scope"
        }
        $key = Get-ExpectedKey $file $scope
        if ($modeMap.ContainsKey($key)) {
            throw "Duplicate case in mode ${mode}: $key"
        }
        $inputPath = Join-Path $packagePath ([string]$row.input_path -replace '/', '\')
        $archivePath = Join-Path $packagePath ([string]$row.archive_path -replace '/', '\')
        $decodedPath = Join-Path $packagePath ([string]$row.decoded_path -replace '/', '\')
        foreach ($artifact in @($inputPath, $archivePath, $decodedPath)) {
            if (-not (Test-Path -LiteralPath $artifact -PathType Leaf)) {
                throw "Missing artifact for ${mode}/${key}: $artifact"
            }
        }
        $actualInputBytes = (Get-Item -LiteralPath $inputPath).Length
        $actualArchiveBytes = (Get-Item -LiteralPath $archivePath).Length
        $actualDecodedBytes = (Get-Item -LiteralPath $decodedPath).Length
        $expectedInputBytes = [int64]$scope * 1024
        if ($actualInputBytes -ne $expectedInputBytes -or
            $actualDecodedBytes -ne $expectedInputBytes -or
            [int64]$row.input_bytes -ne $actualInputBytes -or
            [int64]$row.archive_bytes -ne $actualArchiveBytes -or
            [int64]$row.decoded_bytes -ne $actualDecodedBytes) {
            throw "Length mismatch for $mode/$key"
        }
        $encodeSeconds = Get-RequiredNonNegativeDouble $row 'encode_seconds' "$mode/$key encode_seconds"
        $decodeSeconds = Get-RequiredNonNegativeDouble $row 'decode_seconds' "$mode/$key decode_seconds"
        $encodePeak = Get-RequiredNonNegativeDouble $row 'encode_peak_ram_mib' "$mode/$key encode_peak_ram_mib"
        $decodePeak = Get-RequiredNonNegativeDouble $row 'decode_peak_ram_mib' "$mode/$key decode_peak_ram_mib"
        $peakRam = Get-RequiredNonNegativeDouble $row 'peak_ram_mib' "$mode/$key peak_ram_mib"
        $encodeExitCode = Get-RequiredInteger $row 'encode_exit_code' "$mode/$key encode_exit_code"
        $decodeExitCode = Get-RequiredInteger $row 'decode_exit_code' "$mode/$key decode_exit_code"
        if ($encodeExitCode -ne 0 -or $decodeExitCode -ne 0) {
            throw "Non-zero codec exit code for ${mode}/${key}: encode=$encodeExitCode decode=$decodeExitCode"
        }
        $expectedPeakRam = [Math]::Max($encodePeak, $decodePeak)
        if ([Math]::Abs($peakRam - $expectedPeakRam) -gt 0.000001) {
            throw "Peak memory mismatch for ${mode}/${key}: row=$peakRam expected=$expectedPeakRam"
        }
        $rowCodecHash = Get-RequiredSha256 $row 'codec_sha256' "$mode/$key codec_sha256"

        $inputHash = Get-Sha256 $inputPath
        $archiveHash = Get-Sha256 $archivePath
        $decodedHash = Get-Sha256 $decodedPath
        Assert-EqualHash ([string]$row.input_sha256) $inputHash "$mode/$key input"
        Assert-EqualHash ([string]$row.archive_sha256) $archiveHash "$mode/$key archive"
        Assert-EqualHash ([string]$row.decoded_sha256) $decodedHash "$mode/$key decoded"
        Assert-EqualHash $inputHash $decodedHash "$mode/$key roundtrip"
        [void]$codecHashes.Add($rowCodecHash)
        $selectedModes = @(Get-SelectedModes $mode ([string]$row.block_types))
        $normalizedRow = [pscustomobject][ordered]@{
            ledger_id = $ledgerId
            mode = $mode
            mode_index = if ($mode -eq 'auto') { -1 } else { [Array]::IndexOf($modes, $mode) - 1 }
            file = $file
            scope_kib = $scope
            input_bytes = [int64]$row.input_bytes
            archive_bytes = [int64]$row.archive_bytes
            decoded_bytes = [int64]$row.decoded_bytes
            ratio = [double]$row.archive_bytes / [double]$row.input_bytes
            bpb = [double]$row.archive_bytes * 8.0 / [double]$row.input_bytes
            encode_seconds = $encodeSeconds
            decode_seconds = $decodeSeconds
            encode_peak_ram_mib = $encodePeak
            decode_peak_ram_mib = $decodePeak
            peak_ram_mib = $peakRam
            input_sha256 = $inputHash
            archive_sha256 = $archiveHash
            decoded_sha256 = $decodedHash
            codec_sha256 = $rowCodecHash
            selected_modes = [string]::Join(';', $selectedModes)
            block_types = [string]$row.block_types
            package_name = [IO.Path]::GetFileName($packagePath)
        }
        $modeMap[$key] = $normalizedRow
        $normalized.Add($normalizedRow)
    }
    if ($metadata.PSObject.Properties['codec_sha256']) {
        foreach ($row in $rows) {
            Assert-EqualHash ([string]$metadata.codec_sha256) ([string]$row.codec_sha256) "$mode metadata/row"
        }
    }
}

$codecHash = @($codecHashes)[0]
if ($codecHashes.Count -ne 1) {
    throw "Expected one codec SHA-256 across all packages; found $($codecHashes.Count)"
}
if (-not [string]::IsNullOrWhiteSpace($ExpectedCodecSha256)) {
    Assert-EqualHash $ExpectedCodecSha256 $codecHash 'expected codec'
}

# Verify the Cartesian case matrix before comparing archive bytes.
foreach ($mode in $modes) {
    foreach ($file in $files) {
        foreach ($scope in $scopes) {
            $key = Get-ExpectedKey $file $scope
            if (-not $modeMaps[$mode].ContainsKey($key)) {
                throw "Missing case $mode/$key"
            }
        }
    }
}

$caseSummaries = New-Object System.Collections.Generic.List[object]
foreach ($file in $files) {
    foreach ($scope in $scopes) {
        $key = Get-ExpectedKey $file $scope
        $minimum = [int64]::MaxValue
        $winnerModes = New-Object System.Collections.Generic.List[string]
        foreach ($mode in $forcedModes) {
            $archiveBytes = [int64]$modeMaps[$mode][$key].archive_bytes
            if ($archiveBytes -lt $minimum) {
                $minimum = $archiveBytes
                $winnerModes.Clear()
                $winnerModes.Add($mode)
            }
            elseif ($archiveBytes -eq $minimum) {
                $winnerModes.Add($mode)
            }
        }
        $auto = $modeMaps['auto'][$key]
        $autoSelected = [string]$auto.selected_modes
        $caseSummaries.Add([pscustomobject][ordered]@{
            ledger_id = $ledgerId
            file = $file
            scope_kib = $scope
            input_bytes = [int64]$auto.input_bytes
            input_sha256 = [string]$auto.input_sha256
            auto_archive_bytes = [int64]$auto.archive_bytes
            auto_bpb = [double]$auto.archive_bytes * 8.0 / [double]$auto.input_bytes
            auto_selected_modes = $autoSelected
            oracle_archive_bytes = $minimum
            oracle_bpb = [double]$minimum * 8.0 / [double]$auto.input_bytes
            oracle_winner_modes = [string]::Join(';', $winnerModes.ToArray())
            auto_gap_bytes = [int64]$auto.archive_bytes - $minimum
            auto_gap_bpb = ([double]$auto.archive_bytes - [double]$minimum) * 8.0 / [double]$auto.input_bytes
        })
    }
}

$autoSelectionCounts = @{}
foreach ($case in $caseSummaries) {
    foreach ($selected in ([string]$case.auto_selected_modes).Split(';')) {
        if ([string]::IsNullOrWhiteSpace($selected)) { continue }
        if (-not $autoSelectionCounts.ContainsKey($selected)) { $autoSelectionCounts[$selected] = 0 }
        $autoSelectionCounts[$selected]++
    }
}

$modeAggregates = New-Object System.Collections.Generic.List[object]
foreach ($mode in $modes) {
    $modeRows = @($normalized | Where-Object { $_.mode -eq $mode })
    $inputTotal = ($modeRows | Measure-Object input_bytes -Sum).Sum
    $archiveTotal = ($modeRows | Measure-Object archive_bytes -Sum).Sum
    $oracleWins = if ($mode -eq 'auto') { 0 } else {
        @($caseSummaries | Where-Object {
            @([string]$_.oracle_winner_modes -split ';') -contains $mode
        }).Count
    }
    $autoSelectedRows = if ($autoSelectionCounts.ContainsKey($mode)) { $autoSelectionCounts[$mode] } else { 0 }
    $recommendation = if ($mode -eq 'auto') { 'routing-observation' }
        elseif ($oracleWins -gt 0) { 'retain-candidate' }
        else { 'candidate-not-oracle-winner' }
    $modeAggregates.Add([pscustomobject][ordered]@{
        ledger_id = $ledgerId
        mode = $mode
        mode_index = if ($mode -eq 'auto') { -1 } else { [Array]::IndexOf($modes, $mode) - 1 }
        rows = $modeRows.Count
        input_bytes = [int64]$inputTotal
        archive_bytes = [int64]$archiveTotal
        ratio = [double]$archiveTotal / [double]$inputTotal
        bpb = [double]$archiveTotal * 8.0 / [double]$inputTotal
        encode_seconds = ($modeRows | Measure-Object encode_seconds -Sum).Sum
        decode_seconds = ($modeRows | Measure-Object decode_seconds -Sum).Sum
        peak_ram_mib = ($modeRows | Measure-Object peak_ram_mib -Maximum).Maximum
        pass_rows = @($modeRows | Where-Object { $_.mode -eq $mode }).Count
        oracle_win_rows = $oracleWins
        auto_selected_rows = $autoSelectedRows
        recommendation = $recommendation
        codec_sha256 = $codecHash
    })
}

New-Item -ItemType Directory -Path $OutputPath | Out-Null
$normalizedArray = @($normalized | Sort-Object mode_index, file, scope_kib)
$caseArray = @($caseSummaries)
$aggregateArray = @($modeAggregates)
$manifestArray = @($manifestRows)
Write-NoBom (Join-Path $OutputPath 'mode_rows.tsv') `
    ((($normalizedArray | ConvertTo-Csv -Delimiter "`t" -NoTypeInformation) -join "`r`n") + "`r`n")
Write-NoBom (Join-Path $OutputPath 'per_case_oracle.tsv') `
    ((($caseArray | ConvertTo-Csv -Delimiter "`t" -NoTypeInformation) -join "`r`n") + "`r`n")
Write-NoBom (Join-Path $OutputPath 'mode_aggregate.tsv') `
    ((($aggregateArray | ConvertTo-Csv -Delimiter "`t" -NoTypeInformation) -join "`r`n") + "`r`n")
Write-NoBom (Join-Path $OutputPath 'auto_selection.tsv') `
    ((($caseArray | Select-Object ledger_id, file, scope_kib, auto_selected_modes, auto_archive_bytes, oracle_archive_bytes, auto_gap_bytes |
        ConvertTo-Csv -Delimiter "`t" -NoTypeInformation) -join "`r`n") + "`r`n")
Write-NoBom (Join-Path $OutputPath 'package_manifest.tsv') `
    ((($manifestArray | ConvertTo-Csv -Delimiter "`t" -NoTypeInformation) -join "`r`n") + "`r`n")

$autoRows = @($caseArray)
$autoGapTotal = ($autoRows | Measure-Object auto_gap_bytes -Sum).Sum
$autoGapPositive = @($autoRows | Where-Object { [int64]$_.auto_gap_bytes -gt 0 }).Count
$oracleWinsTotal = ($modeAggregates | Measure-Object oracle_win_rows -Sum).Sum
$sourceManifestHash = Get-Sha256 $ManifestPath
$readme = @"
# HybridZip R2 Complete Current-Hash Ledger

This derived ledger validates Auto plus all 43 forced HZ02 modes on the exact
file/scope matrix declared by `manifest.tsv`. Every archive byte count comes
from the complete `.hz2` file, including the HZ02 header, block headers, CRC32
metadata, backend envelope, and payload. Rows are accepted only when input,
archive, and decoded artifact SHA-256 values match and the decoded bytes equal
the input bytes.

- ledger id: `$ledgerId`
- modes: $($modes.Count) total (Auto + $($forcedModes.Count) forced)
- rows per mode: $expectedRows
- total validated rows: $($normalizedArray.Count)
- codec SHA-256: `$codecHash`
- source manifest SHA-256: `$sourceManifestHash`
- Auto gap-positive cases: $autoGapPositive/$($autoRows.Count)
- total Auto gap bytes: $autoGapTotal
- total forced-mode oracle winner rows (ties counted): $oracleWinsTotal

## Outputs

- `mode_rows.tsv`: normalized archive bytes, timing, memory, and SHA-256 data.
- `per_case_oracle.tsv`: Auto archive bytes versus the minimum complete archive
  bytes among all 43 forced modes for each file and scope.
- `mode_aggregate.tsv`: weighted archive totals, encode/decode time, peak
  memory, Auto selections, oracle wins, and evidence-based recommendation.
- `auto_selection.tsv`: compact Auto/oracle view for review.
- `package_manifest.tsv`: exact package inputs used for this derivation.

`candidate-not-oracle-winner` is a measured retention signal, not permission
to delete donor source. Candidate removal from the product requires a separate
review of corpus coverage, license constraints, and future inputs.
"@
Write-NoBom (Join-Path $OutputPath 'README.md') $readme

Write-Output "created=$OutputPath modes=$($modes.Count) forced=$($forcedModes.Count) rows=$($normalizedArray.Count) codec=$codecHash"
