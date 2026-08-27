[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)]
    [string]$InputPath,
    [string]$CodecPath = '',
    [string]$OutputPath = '',
    [ValidateRange(1, 16384)]
    [int]$SegmentKiB = 32,
    [ValidateRange(0, 9223372036854775807)]
    [Int64]$StartOffset = 0,
    [ValidateRange(0, 2147483647)]
    [int]$MaxSegments = 0,
    [switch]$IncludePartialSegment,
    [switch]$IncludeAuto,
    [ValidateRange(1, 604800)]
    [int]$ProcessTimeoutSeconds = 3600,
    [switch]$AuthorizeRuntimeExperiment,
    [switch]$ListOnly
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

$scriptRoot = Split-Path -Parent $MyInvocation.MyCommand.Path
if ([string]::IsNullOrWhiteSpace($scriptRoot)) {
    throw 'Unable to resolve the segment-oracle script directory'
}
if ([string]::IsNullOrWhiteSpace($CodecPath)) {
    $CodecPath = Join-Path $scriptRoot '..\build\Release\hybridzip.exe'
}
$InputPath = [IO.Path]::GetFullPath($InputPath)
$CodecPath = [IO.Path]::GetFullPath($CodecPath)

# Keep this list in decoder-visible BlockMode order 0..42.
$forcedModes = @(
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
$modes = if ($IncludeAuto) { @('auto') + $forcedModes } else { $forcedModes }
$modeIds = @{}
for ($index = 0; $index -lt $forcedModes.Count; ++$index) {
    $modeIds[$forcedModes[$index]] = $index
}

function Get-Sha256([string]$Path) {
    return (Get-FileHash -LiteralPath $Path -Algorithm SHA256).Hash.ToUpperInvariant()
}

function Get-FileLengthOrZero([string]$Path) {
    if (-not (Test-Path -LiteralPath $Path -PathType Leaf)) {
        return 0L
    }
    return [Int64](Get-Item -LiteralPath $Path).Length
}

function Write-Utf8NoBom([string]$Path, [string]$Content) {
    [IO.File]::WriteAllText($Path, $Content, (New-Object Text.UTF8Encoding($false)))
}

function Write-Csv([string]$Path, [object[]]$Rows) {
    $csv = if ($Rows.Count -eq 0) {
        ''
    } else {
        ([string]::Join("`r`n", @($Rows | ConvertTo-Csv -NoTypeInformation))) + "`r`n"
    }
    Write-Utf8NoBom $Path $csv
}

function Read-Segment([string]$Source, [Int64]$Offset, [int]$Bytes) {
    $stream = [IO.File]::Open($Source, [IO.FileMode]::Open, [IO.FileAccess]::Read,
        [IO.FileShare]::Read)
    try {
        if ($Offset -lt 0 -or $Offset + $Bytes -gt $stream.Length) {
            throw "Segment lies outside source file: offset=$Offset bytes=$Bytes"
        }
        $stream.Position = $Offset
        $buffer = New-Object byte[] $Bytes
        $filled = 0
        while ($filled -lt $Bytes) {
            $read = $stream.Read($buffer, $filled, $Bytes - $filled)
            if ($read -eq 0) {
                throw "Unexpected EOF while reading segment at offset $Offset"
            }
            $filled += $read
        }
        return $buffer
    }
    finally {
        $stream.Dispose()
    }
}

function Quote-RecordedPath([string]$Path) {
    return '"' + $Path + '"'
}

function Format-RecordedCommand(
    [string]$Executable,
    [string[]]$Arguments,
    [string]$Input,
    [string]$Output
) {
    $parts = New-Object System.Collections.Generic.List[string]
    $parts.Add((Quote-RecordedPath $Executable))
    foreach ($argument in $Arguments) {
        $parts.Add($argument)
    }
    $parts.Add((Quote-RecordedPath $Input))
    $parts.Add((Quote-RecordedPath $Output))
    return [string]::Join(' ', $parts)
}

function Invoke-MeasuredCodec(
    [string]$Executable,
    [string[]]$PrefixArguments,
    [string]$Input,
    [string]$Output,
    [string]$LogBase,
    [int]$TimeoutSeconds
) {
    $stdoutPath = $LogBase + '.stdout.log'
    $stderrPath = $LogBase + '.stderr.log'
    $arguments = @($PrefixArguments) + @((Quote-RecordedPath $Input),
        (Quote-RecordedPath $Output))
    $stopwatch = [Diagnostics.Stopwatch]::StartNew()
    $process = Start-Process -FilePath $Executable -ArgumentList $arguments `
        -RedirectStandardOutput $stdoutPath -RedirectStandardError $stderrPath `
        -WindowStyle Hidden -PassThru
    $processHandle = $process.Handle
    $peakBytes = 0L
    $timedOut = $false
    while (-not $process.HasExited) {
        try {
            $process.Refresh()
            $peakBytes = [Math]::Max($peakBytes, $process.PeakWorkingSet64)
        }
        catch {
            # The process can exit between HasExited and Refresh.
        }
        if ($stopwatch.Elapsed.TotalSeconds -ge $TimeoutSeconds) {
            $timedOut = $true
            try {
                if (-not $process.HasExited) {
                    $process.Kill()
                }
            }
            catch {
                if (-not $process.HasExited) {
                    throw "Timed-out codec process could not be terminated (handle $processHandle)"
                }
            }
            if (-not $process.WaitForExit(10000)) {
                throw "Timed-out codec process did not terminate (handle $processHandle)"
            }
            break
        }
        Start-Sleep -Milliseconds 10
    }
    $process.WaitForExit()
    $stopwatch.Stop()
    try {
        $process.Refresh()
        $peakBytes = [Math]::Max($peakBytes, $process.PeakWorkingSet64)
    }
    catch {
    }
    $exitCode = if ($timedOut) { -2 } else { $process.ExitCode }
    return [pscustomobject]@{
        ExitCode = $exitCode
        TimedOut = $timedOut
        Seconds = $stopwatch.Elapsed.TotalSeconds
        PeakMiB = $peakBytes / 1MB
        StdoutPath = $stdoutPath
        StderrPath = $stderrPath
    }
}

function Get-BlockTypes([string]$EncodeStdoutPath) {
    if (-not (Test-Path -LiteralPath $EncodeStdoutPath -PathType Leaf)) {
        return 'UNKNOWN'
    }
    $text = Get-Content -LiteralPath $EncodeStdoutPath -Raw -Encoding UTF8
    $match = [regex]::Match($text, 'blocks\(([^)]+)\)=([0-9]+(?:/[0-9]+)*)')
    if (-not $match.Success) {
        return 'UNKNOWN'
    }
    $names = $match.Groups[1].Value.Split('/')
    $counts = $match.Groups[2].Value.Split('/')
    if ($names.Count -ne $counts.Count) {
        return 'UNKNOWN'
    }
    $selected = New-Object System.Collections.Generic.List[string]
    for ($index = 0; $index -lt $names.Count; ++$index) {
        if ([int]$counts[$index] -gt 0) {
            $selected.Add(('{0}={1}' -f $names[$index], $counts[$index]))
        }
    }
    if ($selected.Count -eq 0) {
        return 'none'
    }
    return [string]::Join(';', $selected)
}

function Assert-R2BlockTypes([string]$RequestedMode,
                             [string]$BlockTypes,
                             [int64]$InputBytes,
                             [string]$Description) {
    if ([string]::IsNullOrWhiteSpace($BlockTypes) -or
        $BlockTypes -eq 'none' -or $BlockTypes -eq 'UNKNOWN') {
        throw "Missing recorded HZ02 block modes for $Description"
    }

    $counts = New-Object 'System.Collections.Generic.Dictionary[string,int64]' `
        ([System.StringComparer]::Ordinal)
    foreach ($part in $BlockTypes.Split(';')) {
        $match = [regex]::Match(
            $part.Trim(), '^(?<name>[^=;]+)=(?<count>[1-9][0-9]*)$')
        if (-not $match.Success) {
            throw "Malformed HZ02 block record for ${Description}: $part"
        }
        $matches = @($forcedModes | Where-Object {
            [string]::Equals($_, $match.Groups['name'].Value,
                [System.StringComparison]::Ordinal)
        })
        if ($matches.Count -ne 1) {
            throw "Unknown HZ02 block mode for ${Description}: $($match.Groups['name'].Value)"
        }
        [int64]$count = 0
        if (-not [int64]::TryParse(
            $match.Groups['count'].Value,
            [Globalization.NumberStyles]::None,
            [Globalization.CultureInfo]::InvariantCulture,
            [ref]$count
        )) {
            throw "Invalid HZ02 block count for ${Description}: $($match.Groups['count'].Value)"
        }
        if ($counts.ContainsKey($matches[0])) {
            throw "Duplicate HZ02 block mode for ${Description}: $($matches[0])"
        }
        $counts.Add($matches[0], $count)
    }

    [int64]$total = 0
    foreach ($count in $counts.Values) {
        if ($count -gt ([int64]::MaxValue - $total)) {
            throw "HZ02 block count overflow for $Description"
        }
        $total += $count
    }
    $expectedBlocks = [int64][Math]::Ceiling([double]$InputBytes / 65536.0)
    if ($total -ne $expectedBlocks) {
        throw "HZ02 block count mismatch for ${Description}: recorded=$total expected=$expectedBlocks"
    }
    if ($RequestedMode -ne 'auto' -and
        ($counts.Count -ne 1 -or -not $counts.ContainsKey($RequestedMode) -or
         $counts[$RequestedMode] -ne $expectedBlocks)) {
        throw "Forced HZ02 mode mismatch for ${Description}: requested=$RequestedMode recorded=$BlockTypes"
    }
}

function Is-BwtRltInapplicable([string]$Mode, [string]$StderrPath) {
    if ($Mode -ne 'bwt-rlt-zstd' -or
        -not (Test-Path -LiteralPath $StderrPath -PathType Leaf)) {
        return $false
    }
    $stderr = Get-Content -LiteralPath $StderrPath -Raw -Encoding UTF8
    return $stderr -match 'Kanzi RLT did not reduce the BWT block'
}

function Write-Metadata([string]$State) {
    $metadata = [ordered]@{
        schema_version = 1
        experiment_id = $script:experimentId
        state = $State
        description = 'Fixed-size intra-file R2 forced-mode oracle experiment.'
        input_path = $script:InputPath
        input_sha256 = $script:sourceSha256
        source_bytes = $script:sourceBytes
        start_offset = $script:StartOffset
        segment_bytes = $script:segmentBytes
        segment_count = $script:segments.Count
        include_partial_segment = [bool]$script:IncludePartialSegment
        include_auto = [bool]$script:IncludeAuto
        forced_modes = $forcedModes
        modes = $modes
        codec_path = $script:CodecPath
        codec_sha256 = $script:codecSha256
        configuration = 'profile_id=2; block_size=65536; zstd_level=19; lzma_level=9; threads=1'
        command_template = 'hybridzip c --profile=r2 --r2-mode=<mode> <input> <archive>; hybridzip d <archive> <output>'
        selection_policy = 'Contiguous source segments; forced modes provide the oracle; optional Auto records route gap.'
        created_at = $script:createdAt
    }
    Write-Utf8NoBom (Join-Path $script:OutputPath 'experiment.json') `
        (($metadata | ConvertTo-Json -Depth 8) + "`r`n")
}

if (-not (Test-Path -LiteralPath $InputPath -PathType Leaf)) {
    throw "Input file not found: $InputPath"
}
$sourceBytes = [Int64](Get-Item -LiteralPath $InputPath).Length
if ($StartOffset -ge $sourceBytes) {
    throw "StartOffset must be inside the source file: $StartOffset >= $sourceBytes"
}
$segmentBytes = [Int64]$SegmentKiB * 1024L
$remainingBytes = $sourceBytes - $StartOffset
$availableSegments = [Int64][Math]::Floor($remainingBytes / $segmentBytes)
if ($IncludePartialSegment -and ($remainingBytes % $segmentBytes) -ne 0) {
    ++$availableSegments
}
if ($MaxSegments -gt 0) {
    $availableSegments = [Math]::Min($availableSegments, [Int64]$MaxSegments)
}
if ($availableSegments -le 0) {
    throw 'No complete segment is selected; use -IncludePartialSegment or lower -SegmentKiB'
}

$segments = New-Object System.Collections.Generic.List[object]
for ($index = 0L; $index -lt $availableSegments; ++$index) {
    $offset = $StartOffset + $index * $segmentBytes
    $length = [Int64][Math]::Min($segmentBytes, $sourceBytes - $offset)
    if ($length -lt $segmentBytes -and -not $IncludePartialSegment) {
        break
    }
    $segments.Add([pscustomobject][ordered]@{
        segment_id = ('segment-{0:D6}' -f ($index + 1))
        segment_index = [int]($index + 1)
        offset = $offset
        bytes = $length
    })
}

if ($ListOnly) {
    [pscustomobject][ordered]@{
        input_path = $InputPath
        source_bytes = $sourceBytes
        start_offset = $StartOffset
        segment_bytes = $segmentBytes
        segment_count = $segments.Count
        forced_modes = $forcedModes.Count
        include_auto = [bool]$IncludeAuto
        planned_codec_invocations = $segments.Count * $modes.Count * 2
        runtime_started = $false
    } | ConvertTo-Json -Compress
    return
}

if (-not $AuthorizeRuntimeExperiment) {
    throw 'Refusing runtime execution. Re-run with -AuthorizeRuntimeExperiment after reviewing the segment plan.'
}
if (-not (Test-Path -LiteralPath $CodecPath -PathType Leaf)) {
    throw "Codec executable not found: $CodecPath"
}
if ([string]::IsNullOrWhiteSpace($OutputPath)) {
    $OutputPath = Join-Path $scriptRoot ('..\results\experiments\hybridzip-r2-segment-oracle-{0}-{1}' -f `
        [DateTime]::UtcNow.ToString('yyyyMMdd-HHmmss'), [Guid]::NewGuid().ToString('N').Substring(0, 8))
}
$OutputPath = [IO.Path]::GetFullPath($OutputPath)
if (Test-Path -LiteralPath $OutputPath) {
    throw "Refusing to overwrite existing output package: $OutputPath"
}

$experimentId = Split-Path -Leaf $OutputPath
if ($experimentId -cnotmatch '^[a-z0-9-]+$') {
    throw ('Output package name must match ^[a-z0-9-]+$: {0}' -f $experimentId)
}
$sourceSha256 = Get-Sha256 $InputPath
$codecSha256 = Get-Sha256 $CodecPath
$createdAt = [DateTimeOffset]::Now.ToString('o')
New-Item -ItemType Directory -Path $OutputPath | Out-Null
foreach ($directory in @('inputs', 'archives', 'decoded', 'logs')) {
    New-Item -ItemType Directory -Path (Join-Path $OutputPath $directory) | Out-Null
}

Write-Metadata 'testing'
$manifestRows = foreach ($segment in $segments) {
    [pscustomobject][ordered]@{
        segment_id = $segment.segment_id
        segment_index = $segment.segment_index
        source_path = $InputPath
        source_sha256 = $sourceSha256
        segment_offset = $segment.offset
        segment_bytes = $segment.bytes
    }
}
Write-Csv (Join-Path $OutputPath 'manifest.csv') @($manifestRows)

$rows = New-Object System.Collections.Generic.List[object]
$failedRows = 0
foreach ($segment in $segments) {
    $inputRelative = 'inputs/{0}.bin' -f $segment.segment_id
    $inputArtifact = Join-Path $OutputPath ($inputRelative -replace '/', '\\')
    [IO.File]::WriteAllBytes($inputArtifact, (Read-Segment $InputPath $segment.offset [int]$segment.bytes))
    $inputSha256 = Get-Sha256 $inputArtifact

    foreach ($mode in $modes) {
        $modeId = if ($mode -eq 'auto') { -1 } else { $modeIds[$mode] }
        $archiveRelative = 'archives/{0}.{1}.hz2' -f $segment.segment_id, $mode
        $decodedRelative = 'decoded/{0}.{1}.decoded' -f $segment.segment_id, $mode
        $logRelative = 'logs/{0}.{1}' -f $segment.segment_id, $mode
        $archiveArtifact = Join-Path $OutputPath ($archiveRelative -replace '/', '\\')
        $decodedArtifact = Join-Path $OutputPath ($decodedRelative -replace '/', '\\')
        $logBase = Join-Path $OutputPath ($logRelative -replace '/', '\\')
        $startedAt = [DateTimeOffset]::Now.ToString('o')
        $encode = $null
        $decode = $null
        $status = 'FAILED'
        $roundtrip = 'NOT_VERIFIED'
        $notes = ''
        try {
            if ((Get-Sha256 $CodecPath) -ne $codecSha256) {
                throw 'Codec SHA-256 changed during the segment-oracle experiment'
            }
            $encode = Invoke-MeasuredCodec $CodecPath @('c', '--profile=r2', "--r2-mode=$mode") `
                $inputArtifact $archiveArtifact ($logBase + '.encode') $ProcessTimeoutSeconds
            if ($encode.TimedOut) {
                throw "Encode exceeded timeout of $ProcessTimeoutSeconds seconds"
            }
            if ($encode.ExitCode -ne 0) {
                if (Is-BwtRltInapplicable $mode $encode.StderrPath) {
                    $status = 'SKIPPED'
                    $notes = 'Kanzi RLT did not reduce this BWT segment; no forced archive was emitted.'
                }
                else {
                    throw "Encode exit code $($encode.ExitCode)"
                }
            }
            if ($status -ne 'SKIPPED') {
                $recordedBlockTypes = Get-BlockTypes $encode.StdoutPath
                Assert-R2BlockTypes $mode $recordedBlockTypes $segment.bytes `
                    "$($segment.segment_id)/$mode"
                if (-not (Test-Path -LiteralPath $archiveArtifact -PathType Leaf)) {
                    throw 'Encoder returned success but did not produce an archive'
                }
                $decode = Invoke-MeasuredCodec $CodecPath @('d') $archiveArtifact $decodedArtifact `
                    ($logBase + '.decode') $ProcessTimeoutSeconds
                if ($decode.TimedOut) {
                    throw "Decode exceeded timeout of $ProcessTimeoutSeconds seconds"
                }
                if ($decode.ExitCode -ne 0) {
                    throw "Decode exit code $($decode.ExitCode)"
                }
                $decodedHash = Get-Sha256 $decodedArtifact
                if ((Get-FileLengthOrZero $decodedArtifact) -ne $segment.bytes -or
                    $decodedHash -ne $inputSha256) {
                    $roundtrip = 'FAIL'
                    throw 'Decoded output is not byte-exact'
                }
                $status = 'COMPLETE'
                $roundtrip = 'PASS'
                $notes = 'Complete archive bytes, timing, peak memory, and SHA-256 byte-exact roundtrip recorded.'
            }
        }
        catch {
            $notes = $_.Exception.Message
            if ($roundtrip -ne 'FAIL') {
                $roundtrip = 'NOT_VERIFIED'
            }
        }

        if ($status -eq 'FAILED') {
            ++$failedRows
        }
        $archiveBytes = Get-FileLengthOrZero $archiveArtifact
        $decodedBytes = Get-FileLengthOrZero $decodedArtifact
        $encodeSeconds = if ($null -ne $encode) { $encode.Seconds } else { 0.0 }
        $decodeSeconds = if ($null -ne $decode) { $decode.Seconds } else { 0.0 }
        $encodePeak = if ($null -ne $encode) { $encode.PeakMiB } else { 0.0 }
        $decodePeak = if ($null -ne $decode) { $decode.PeakMiB } else { 0.0 }
        $rows.Add([pscustomobject][ordered]@{
            experiment_id = $experimentId
            segment_id = $segment.segment_id
            segment_index = $segment.segment_index
            source_path = $InputPath
            source_sha256 = $sourceSha256
            segment_offset = $segment.offset
            input_path = $inputRelative
            input_bytes = $segment.bytes
            input_sha256 = $inputSha256
            mode = $mode
            mode_id = $modeId
            archive_path = $archiveRelative
            archive_bytes = $archiveBytes
            archive_bpb = if ($archiveBytes -gt 0) { $archiveBytes * 8.0 / $segment.bytes } else { 0.0 }
            archive_sha256 = if ($archiveBytes -gt 0) { Get-Sha256 $archiveArtifact } else { '' }
            decoded_path = $decodedRelative
            decoded_bytes = $decodedBytes
            decoded_sha256 = if ($decodedBytes -gt 0) { Get-Sha256 $decodedArtifact } else { '' }
            encode_seconds = $encodeSeconds
            decode_seconds = $decodeSeconds
            encode_peak_ram_mib = $encodePeak
            decode_peak_ram_mib = $decodePeak
            peak_ram_mib = [Math]::Max([double]$encodePeak, [double]$decodePeak)
            codec_sha256 = $codecSha256
            encode_command = Format-RecordedCommand $CodecPath @('c', '--profile=r2', "--r2-mode=$mode") $inputArtifact $archiveArtifact
            decode_command = if ($status -eq 'COMPLETE') { Format-RecordedCommand $CodecPath @('d') $archiveArtifact $decodedArtifact } else { '' }
            encode_exit_code = if ($null -ne $encode) { $encode.ExitCode } else { -1 }
            decode_exit_code = if ($null -ne $decode) { $decode.ExitCode } else { -1 }
            started_at = $startedAt
            status = $status
            roundtrip = $roundtrip
            block_types = if ($null -ne $encode) { Get-BlockTypes $encode.StdoutPath } else { 'UNKNOWN' }
            notes = $notes
        })
        Write-Csv (Join-Path $OutputPath 'results.csv') @($rows.ToArray())
        Write-Host ('[{0}/{1}] {2} {3}: {4}/{5}' -f $segment.segment_index,
            $segments.Count, $segment.segment_id, $mode, $status, $roundtrip)
    }
}

$oracleRows = New-Object System.Collections.Generic.List[object]
foreach ($segment in $segments) {
    $segmentRows = @($rows | Where-Object { $_.segment_id -eq $segment.segment_id })
    $forcedRows = @($segmentRows | Where-Object { $_.mode -ne 'auto' })
    $passing = @($forcedRows | Where-Object { $_.status -eq 'COMPLETE' -and $_.roundtrip -eq 'PASS' })
    $skipped = @($forcedRows | Where-Object { $_.status -eq 'SKIPPED' })
    $failed = @($forcedRows | Where-Object { $_.status -eq 'FAILED' })
    $winner = @($passing | Sort-Object @{ Expression = { [Int64]$_.archive_bytes }; Ascending = $true },
        @{ Expression = { [int]$_.mode_id }; Ascending = $true } | Select-Object -First 1)
    $auto = @($segmentRows | Where-Object { $_.mode -eq 'auto' } | Select-Object -First 1)
    $oracleStatus = if ($failed.Count -gt 0) {
        'PARTIAL'
    } elseif ($skipped.Count -gt 0) {
        'COMPLETE_WITH_INAPPLICABLE'
    } else {
        'COMPLETE'
    }
    $oracleRows.Add([pscustomobject][ordered]@{
        experiment_id = $experimentId
        segment_id = $segment.segment_id
        segment_index = $segment.segment_index
        segment_offset = $segment.offset
        input_bytes = $segment.bytes
        forced_modes_expected = $forcedModes.Count
        forced_modes_passing = $passing.Count
        forced_modes_inapplicable = $skipped.Count
        forced_modes_failed = $failed.Count
        oracle_status = $oracleStatus
        oracle_mode = if ($winner.Count -eq 1) { $winner[0].mode } else { '' }
        oracle_mode_id = if ($winner.Count -eq 1) { $winner[0].mode_id } else { -1 }
        oracle_archive_bytes = if ($winner.Count -eq 1) { $winner[0].archive_bytes } else { 0 }
        oracle_bpb = if ($winner.Count -eq 1) { $winner[0].archive_bpb } else { 0.0 }
        auto_status = if ($auto.Count -eq 1) { $auto[0].status } else { 'NOT_RUN' }
        auto_archive_bytes = if ($auto.Count -eq 1) { $auto[0].archive_bytes } else { 0 }
        auto_bpb = if ($auto.Count -eq 1) { $auto[0].archive_bpb } else { 0.0 }
        auto_oracle_gap_bytes = if ($auto.Count -eq 1 -and $winner.Count -eq 1 -and
            $auto[0].status -eq 'COMPLETE') {
            [Math]::Max(0, [Int64]$auto[0].archive_bytes - [Int64]$winner[0].archive_bytes)
        } else { 0 }
    })
}
Write-Csv (Join-Path $OutputPath 'oracle.csv') @($oracleRows.ToArray())
Write-Metadata $(if ($failedRows -eq 0) { 'complete' } else { 'failed' })

if ($failedRows -ne 0) {
    throw "$failedRows segment-mode cases failed; evidence was retained at $OutputPath"
}
Write-Output "complete=$OutputPath segments=$($segments.Count) forced_modes=$($forcedModes.Count) include_auto=$([bool]$IncludeAuto)"
