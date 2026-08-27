[CmdletBinding()]
param(
    [ValidateSet('e5-router', 'e6-fast')]
    [string]$Stage = 'e5-router',
    [string]$CodecPath = '',
    [string]$DatasetPath = 'F:\paq8px\silesia',
    [string]$OutputRoot = '',
    [string]$ExperimentId = '',
    [ValidateSet(32, 64, 128)]
    [int[]]$ScopesKiB = @(32, 64, 128),
    [ValidateSet(32, 64, 128)]
    [int[]]$BlockSizesKiB = @(32, 64, 128),
    [ValidateRange(1, 256)]
    [int]$FastThreadCount = 1,
    [string[]]$SilesiaFiles = @(),
    [switch]$AllowAllFiles,
    [ValidateRange(1, 604800)]
    [int]$ProcessTimeoutSeconds = 3600,
    [switch]$ListOnly = $true,
    [switch]$AuthorizeRuntimeExperiment,
    [switch]$Resume
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

$scriptRoot = Split-Path -Parent $MyInvocation.MyCommand.Path
if ([string]::IsNullOrWhiteSpace($scriptRoot)) {
    throw 'Unable to resolve the matrix runner directory'
}
$childRunner = Join-Path $scriptRoot 'run_silesia_experiment.ps1'
if (-not (Test-Path -LiteralPath $childRunner -PathType Leaf)) {
    throw "Required child runner is missing: $childRunner"
}
if ([string]::IsNullOrWhiteSpace($CodecPath)) {
    $CodecPath = Join-Path $scriptRoot '..\build\Release\hybridzip.exe'
}
if ([string]::IsNullOrWhiteSpace($OutputRoot)) {
    $OutputRoot = Join-Path $scriptRoot '..\results\experiments'
}

$allFiles = @(
    'dickens', 'mozilla', 'mr', 'nci', 'ooffice', 'osdb',
    'reymont', 'samba', 'sao', 'webster', 'x-ray', 'xml'
)
$r2BlockModes = @(
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
    'lmic-arithmetic', 'delta-binary-packed-zstd', 'fast-ext'
)

function Get-NormalizedDirectoryPath([string]$Path) {
    $fullPath = [System.IO.Path]::GetFullPath($Path)
    $root = [System.IO.Path]::GetPathRoot($fullPath)
    if ($fullPath.Length -gt $root.Length) {
        return $fullPath.TrimEnd([char[]]@(
            [System.IO.Path]::DirectorySeparatorChar,
            [System.IO.Path]::AltDirectorySeparatorChar
        ))
    }
    return $fullPath
}

function Get-Sha256([string]$Path) {
    return (Get-FileHash -LiteralPath $Path -Algorithm SHA256).Hash.ToUpperInvariant()
}

function Write-Utf8Json([string]$Path, $Value) {
    $encoding = New-Object System.Text.UTF8Encoding($false)
    [System.IO.File]::WriteAllText(
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
    [System.IO.File]::WriteAllText($Path, $content, $encoding)
}

function Get-R2Telemetry([string]$Path) {
    if (-not (Test-Path -LiteralPath $Path -PathType Leaf)) {
        throw "Encode telemetry is missing: $Path"
    }
    $text = Get-Content -LiteralPath $Path -Raw -Encoding UTF8
    $candidate = [regex]::Match($text, 'candidates=(?<value>[0-9]+)')
    $workers = [regex]::Match($text, 'workers=(?<value>[1-9][0-9]*)')
    $fullOracle = [regex]::Match($text, 'full_oracle=(?<value>[01])')
    $rankerVersion = [regex]::Match($text, 'ranker_version=0x(?<value>[0-9A-F]{8})')
    $rankerCrc32 = [regex]::Match($text, 'ranker_crc32=0x(?<value>[0-9A-F]{8})')
    $rankerSha256 = [regex]::Match($text, 'ranker_sha256=(?<value>[0-9A-F]{64})')
    $candidateModes = [regex]::Match($text, 'candidate_modes=(?<value>[^\s]+)')
    if (-not $candidate.Success -or -not $workers.Success -or
        -not $fullOracle.Success -or
        -not $rankerVersion.Success -or -not $rankerCrc32.Success -or
        -not $rankerSha256.Success -or
        -not $candidateModes.Success) {
        throw "Malformed R2 telemetry: $Path"
    }
    [pscustomobject]@{
        Candidates = [int]$candidate.Groups['value'].Value
        Workers = [int]$workers.Groups['value'].Value
        FullOracle = [int]$fullOracle.Groups['value'].Value
        RankerVersion = $rankerVersion.Groups['value'].Value
        RankerCrc32 = $rankerCrc32.Groups['value'].Value
        RankerSha256 = $rankerSha256.Groups['value'].Value
        CandidateModes = $candidateModes.Groups['value'].Value
    }
}

function Get-ModeIds([string]$CandidateModes) {
    $ids = New-Object 'System.Collections.Generic.HashSet[int]'
    if ($CandidateModes -eq 'none') {
        return $ids
    }
    foreach ($entry in $CandidateModes.Split(',')) {
        $match = [regex]::Match($entry, '^(?<id>[0-9]+):(?<count>[1-9][0-9]*)$')
        if (-not $match.Success) {
            throw "Malformed candidate mode entry: $entry"
        }
        $id = [int]$match.Groups['id'].Value
        if ($id -lt 0 -or $id -ge $r2BlockModes.Count -or -not $ids.Add($id)) {
            throw "Invalid or duplicate candidate mode ID: $entry"
        }
    }
    return $ids
}

function Get-BlockModeNames([string]$BlockTypes) {
    $names = New-Object 'System.Collections.Generic.HashSet[string]' `
        ([System.StringComparer]::Ordinal)
    foreach ($entry in $BlockTypes.Split(';')) {
        $match = [regex]::Match($entry, '^(?<name>[^=;]+)=(?<count>[1-9][0-9]*)$')
        if (-not $match.Success -or -not $names.Add($match.Groups['name'].Value)) {
            throw "Malformed or duplicate block mode entry: $entry"
        }
    }
    return $names
}

function Get-Quantile([double[]]$Values, [double]$Quantile) {
    if ($Values.Count -eq 0) {
        throw 'Cannot calculate a quantile of zero values'
    }
    $ordered = @($Values | Sort-Object)
    $index = [int][Math]::Ceiling($Quantile * $ordered.Count) - 1
    return $ordered[[Math]::Max(0, [Math]::Min($index, $ordered.Count - 1))]
}

function Assert-ResumePlan([object]$Existing, [string]$ExpectedId,
                           [string]$ExpectedStage, [string]$ExpectedCodecPath,
                           [string]$ExpectedCodecHash, [string]$ExpectedDatasetPath,
                           [string[]]$ExpectedFiles, [int[]]$ExpectedScopes,
                           [int[]]$ExpectedBlockSizes, [string[]]$ExpectedPolicies,
                           [int[]]$ExpectedRepeats, [int]$ExpectedFastThreadCount) {
    if ([string]$Existing.experiment_id -cne $ExpectedId -or
        [string]$Existing.stage -cne $ExpectedStage) {
        throw "Resume metadata does not match requested experiment: $ExpectedId"
    }
    if (-not [string]::Equals([string]$Existing.codec_path, $ExpectedCodecPath,
                               [StringComparison]::OrdinalIgnoreCase) -or
        [string]$Existing.codec_sha256 -cne $ExpectedCodecHash -or
        -not [string]::Equals([string]$Existing.dataset_path, $ExpectedDatasetPath,
                               [StringComparison]::OrdinalIgnoreCase)) {
        throw 'Resume codec or dataset identity differs from the existing package'
    }
    if ($null -eq $Existing.PSObject.Properties['fast_thread_count'] -or
        [int]$Existing.fast_thread_count -ne $ExpectedFastThreadCount) {
        throw 'Resume Fast worker count differs from the existing package'
    }
    $pairs = @(
        @(@($Existing.files), @($ExpectedFiles)),
        @(@($Existing.scopes_kib | ForEach-Object { [int]$_ }), @($ExpectedScopes)),
        @(@($Existing.block_sizes_kib | ForEach-Object { [int]$_ }), @($ExpectedBlockSizes)),
        @(@($Existing.policies), @($ExpectedPolicies)),
        @(@($Existing.timing_repeats | ForEach-Object { [int]$_ }), @($ExpectedRepeats))
    )
    foreach ($pair in $pairs) {
        $actual = @($pair[0])
        $expected = @($pair[1])
        if ($actual.Count -ne $expected.Count) {
            throw 'Resume matrix dimensions differ from the existing package'
        }
        for ($index = 0; $index -lt $actual.Count; ++$index) {
            if ([string]$actual[$index] -cne [string]$expected[$index]) {
                throw 'Resume matrix dimensions differ from the existing package'
            }
        }
    }
}

function Assert-CompletedMatrixPackage([string]$PackagePath,
                                       [int]$ExpectedRows) {
    $matrixPath = Join-Path $PackagePath 'matrix_rows.csv'
    $summaryPath = Join-Path $PackagePath 'summary.json'
    if (-not (Test-Path -LiteralPath $matrixPath -PathType Leaf) -or
        -not (Test-Path -LiteralPath $summaryPath -PathType Leaf)) {
        throw "Completed package is missing matrix evidence: $PackagePath"
    }
    $rows = @(Import-Csv -LiteralPath $matrixPath -Encoding UTF8)
    if ($rows.Count -ne $ExpectedRows) {
        throw "Completed package has unexpected matrix row count: $PackagePath"
    }
    foreach ($row in $rows) {
        if ($row.status -cne 'COMPLETE' -or $row.roundtrip -cne 'PASS') {
            throw "Completed package contains a non-passing row: $PackagePath"
        }
    }
    $rankerIdentities = @($rows | ForEach-Object {
        if ([string]::IsNullOrWhiteSpace($_.ranker_version) -or
            [string]::IsNullOrWhiteSpace($_.ranker_crc32) -or
            [string]::IsNullOrWhiteSpace($_.ranker_sha256)) {
            throw "Completed package is missing ranker identity: $PackagePath"
        }
        "$($_.ranker_version)|$($_.ranker_crc32)|$($_.ranker_sha256)"
    } | Sort-Object -Unique)
    if ($rankerIdentities.Count -ne 1) {
        throw "Completed package contains multiple ranker identities: $PackagePath"
    }
    $summary = Get-Content -LiteralPath $summaryPath -Raw -Encoding UTF8 |
        ConvertFrom-Json
    if ([string]$summary.status -cne 'COMPLETE') {
        throw "Completed package summary is not COMPLETE: $PackagePath"
    }
}

$requestedFiles = @()
foreach ($entry in $SilesiaFiles) {
    $requestedFiles += ([string]$entry).Split(',') |
        Where-Object { -not [string]::IsNullOrWhiteSpace([string]$_) }
}
if ($requestedFiles.Count -eq 0) {
    $files = @($allFiles)
}
else {
    $files = @()
    foreach ($requestedFile in $requestedFiles) {
        $canonical = @($allFiles | Where-Object {
            [string]::Equals($_, [string]$requestedFile,
                [System.StringComparison]::OrdinalIgnoreCase)
        })
        if ($canonical.Count -ne 1 -or $files -contains $canonical[0]) {
            throw "Invalid or duplicate Silesia file: $requestedFile"
        }
        $files += $canonical[0]
    }
}
$scopes = @($ScopesKiB | Sort-Object -Unique)
$blockSizes = @($BlockSizesKiB | Sort-Object -Unique)
if ($files.Count -eq 0 -or $scopes.Count -eq 0 -or $blockSizes.Count -eq 0) {
    throw 'At least one file, scope, and block size is required'
}
if ($Stage -eq 'e5-router' -and $FastThreadCount -ne 1) {
    throw 'FastThreadCount is only valid for the e6-fast stage'
}
if (-not $ListOnly -and -not $AuthorizeRuntimeExperiment) {
    throw 'Runtime is disabled. Use -ListOnly:$false and -AuthorizeRuntimeExperiment together.'
}
if (-not $ListOnly -and $requestedFiles.Count -eq 0 -and -not $AllowAllFiles) {
    throw 'Refusing an implicit all-file runtime matrix; use -SilesiaFiles or -AllowAllFiles.'
}

if ([string]::IsNullOrWhiteSpace($ExperimentId)) {
    $ExperimentId = "hybridzip-r2-$Stage-$([DateTime]::UtcNow.ToString('yyyyMMdd-HHmmss-fff'))-$([Guid]::NewGuid().ToString('N').Substring(0, 8))"
}
if ($ExperimentId -cnotmatch '^[a-z0-9-]+$') {
    throw "ExperimentId must match ^[a-z0-9-]+`$: $ExperimentId"
}

$policies = if ($Stage -eq 'e5-router') {
    @('auto', 'auto-k2', 'auto-k4', 'auto-k8')
}
else {
    @('fast')
}
$timingRepeats = if ($Stage -eq 'e5-router') { @(1) } else { @(0, 1, 2, 3) }
$threadCount = if ($Stage -eq 'e6-fast') { $FastThreadCount } else { 1 }
$jobs = New-Object System.Collections.Generic.List[object]
foreach ($blockSize in $blockSizes) {
    foreach ($policy in $policies) {
        foreach ($repeat in $timingRepeats) {
            $jobs.Add([pscustomobject]@{
                BlockSizeKiB = $blockSize
                Policy = $policy
                TimingRepeat = $repeat
            })
        }
    }
}

$plan = [ordered]@{
    experiment_id = $ExperimentId
    stage = $Stage
    runtime_started = $false
    files = $files
    scopes_kib = $scopes
    block_sizes_kib = $blockSizes
    policies = $policies
    timing_repeats = $timingRepeats
    fast_thread_count = $threadCount
    child_packages = $jobs.Count
    cases_per_child = $files.Count * $scopes.Count
    codec_invocations = $jobs.Count * $files.Count * $scopes.Count * 2
    runtime_authorization_required = $true
    resume_requested = [bool]$Resume
    command = "& $($MyInvocation.MyCommand.Path) -Stage $Stage -FastThreadCount $threadCount -ListOnly:`$false -AuthorizeRuntimeExperiment ..."
}
if ($ListOnly) {
    $plan | ConvertTo-Json -Depth 5
    $global:LASTEXITCODE = 0
    return
}

$CodecPath = [System.IO.Path]::GetFullPath($CodecPath)
$DatasetPath = Get-NormalizedDirectoryPath $DatasetPath
$OutputRoot = Get-NormalizedDirectoryPath $OutputRoot
$packagePath = Join-Path $OutputRoot $ExperimentId
if (-not (Test-Path -LiteralPath $CodecPath -PathType Leaf)) {
    throw "Codec executable not found: $CodecPath"
}
if (-not (Test-Path -LiteralPath $DatasetPath -PathType Container)) {
    throw "Dataset directory not found: $DatasetPath"
}
foreach ($file in $files) {
    $source = Join-Path $DatasetPath $file
    if (-not (Test-Path -LiteralPath $source -PathType Leaf) -or
        (Get-Item -LiteralPath $source).Length -lt 128KB) {
        throw "Missing or too-short Silesia source: $source"
    }
}
$codecHash = Get-Sha256 $CodecPath
$childRoot = Join-Path $packagePath 'children'
if (Test-Path -LiteralPath $packagePath) {
    if (-not $Resume) {
        throw "Refusing to overwrite matrix package: $packagePath"
    }
    $metadataPath = Join-Path $packagePath 'experiment.json'
    if (-not (Test-Path -LiteralPath $metadataPath -PathType Leaf) -or
        -not (Test-Path -LiteralPath $childRoot -PathType Container)) {
        throw "Resume package is missing required metadata: $packagePath"
    }
    $plan = Get-Content -LiteralPath $metadataPath -Raw -Encoding UTF8 |
        ConvertFrom-Json
    Assert-ResumePlan -Existing $plan -ExpectedId $ExperimentId `
        -ExpectedStage $Stage -ExpectedCodecPath $CodecPath `
        -ExpectedCodecHash $codecHash -ExpectedDatasetPath $DatasetPath `
        -ExpectedFiles $files -ExpectedScopes $scopes -ExpectedBlockSizes $blockSizes `
        -ExpectedPolicies $policies -ExpectedRepeats $timingRepeats `
        -ExpectedFastThreadCount $threadCount
    if ([string]$plan.status -ceq 'COMPLETE') {
        Assert-CompletedMatrixPackage -PackagePath $packagePath `
            -ExpectedRows ($jobs.Count * $files.Count * $scopes.Count)
        Write-Host "Experiment already complete: $packagePath"
        return
    }
    if ($null -eq $plan.PSObject.Properties['resume_count']) {
        $plan | Add-Member -NotePropertyName resume_count -NotePropertyValue 0
    }
    $plan.resume_count = [int]$plan.resume_count + 1
    if ($null -eq $plan.PSObject.Properties['resumed_at']) {
        $plan | Add-Member -NotePropertyName resumed_at -NotePropertyValue ''
    }
    $plan.resumed_at = [DateTime]::UtcNow.ToString('o')
    $plan.status = 'RUNNING'
}
else {
    New-Item -ItemType Directory -Path $packagePath | Out-Null
    New-Item -ItemType Directory -Path $childRoot | Out-Null
    $plan.codec_path = $CodecPath
    $plan.codec_sha256 = $codecHash
    $plan.dataset_path = $DatasetPath
    $plan.runtime_started = $true
}
Write-Utf8Json (Join-Path $packagePath 'experiment.json') $plan

$matrixRows = New-Object System.Collections.Generic.List[object]
try {
    $jobIndex = 0
    foreach ($job in $jobs) {
        ++$jobIndex
        $childId = "$ExperimentId-$($job.Policy)-b$($job.BlockSizeKiB)-r$($job.TimingRepeat)"
        $arguments = @{
            CodecPath = $CodecPath
            DatasetPath = $DatasetPath
            OutputRoot = $childRoot
            ExperimentId = $childId
            Profile = 'r2'
            R2Mode = $job.Policy
            ProcessTimeoutSeconds = $ProcessTimeoutSeconds
            ScopesKiB = $scopes
            BlockSizeKiB = $job.BlockSizeKiB
            ThreadCount = $threadCount
            SilesiaFiles = $files
        }
        if ($AllowAllFiles) {
            $arguments.AllowAllFiles = $true
        }
        $childPath = Join-Path $childRoot $childId
        if ($Resume -and (Test-Path -LiteralPath $childPath -PathType Container)) {
            $arguments.Resume = $true
        }
        & $childRunner @arguments
        if (-not $?) {
            throw "Child runner failed: $childId"
        }

        $childRows = @(Import-Csv -LiteralPath (Join-Path $childPath 'results.csv') -Encoding UTF8)
        if ($childRows.Count -ne ($files.Count * $scopes.Count)) {
            throw "Unexpected child row count for $childId"
        }
        foreach ($row in $childRows) {
            if ($row.status -cne 'COMPLETE' -or $row.roundtrip -cne 'PASS') {
                throw "Child row is not COMPLETE/PASS: $childId $($row.file) $($row.scope_kib)"
            }
            $logPath = Join-Path $childPath (
                'logs\{0}KiB\{1}.encode.stdout.log' -f $row.scope_kib, $row.file)
            $telemetry = Get-R2Telemetry $logPath
            if ($telemetry.Workers -ne $threadCount) {
                throw "Fast worker count mismatch for $childId $($row.file) $($row.scope_kib): expected $threadCount, found $($telemetry.Workers)"
            }
            $record = [ordered]@{
                stage = $Stage
                policy = $job.Policy
                block_size_kib = $job.BlockSizeKiB
                timing_repeat = $job.TimingRepeat
                warmup = ($Stage -eq 'e6-fast' -and $job.TimingRepeat -eq 0)
                fast_thread_count = $threadCount
                telemetry_worker_count = $telemetry.Workers
                candidates_evaluated = $telemetry.Candidates
                full_oracle_evaluated = $telemetry.FullOracle
                ranker_version = $telemetry.RankerVersion
                ranker_crc32 = $telemetry.RankerCrc32
                ranker_sha256 = $telemetry.RankerSha256
                candidate_modes = $telemetry.CandidateModes
            }
            foreach ($property in $row.PSObject.Properties) {
                $record[$property.Name] = $property.Value
            }
            $matrixRows.Add([pscustomobject]$record)
        }
        Write-Csv (Join-Path $packagePath 'matrix_rows.csv') $matrixRows.ToArray()
        Write-Host ("[{0}/{1}] {2} block={3}KiB repeat={4} complete" -f `
            $jobIndex, $jobs.Count, $job.Policy, $job.BlockSizeKiB, $job.TimingRepeat)
    }

    $inputGroups = $matrixRows | Group-Object file, scope_kib
    foreach ($group in $inputGroups) {
        $hashes = @($group.Group.input_sha256 | Sort-Object -Unique)
        if ($hashes.Count -ne 1) {
            throw "Same-input SHA-256 mismatch for $($group.Name)"
        }
    }
    $rankerIdentities = @($matrixRows | ForEach-Object {
        "$($_.ranker_version)|$($_.ranker_crc32)|$($_.ranker_sha256)"
    } | Sort-Object -Unique)
    if ($rankerIdentities.Count -ne 1) {
        throw 'Matrix contains multiple fixed-point ranker model identities'
    }
    $rankerParts = $rankerIdentities[0].Split('|')
    $rankerModel = [ordered]@{
        version = $rankerParts[0]
        crc32 = $rankerParts[1]
        sha256 = $rankerParts[2]
    }

    $summaryRows = New-Object System.Collections.Generic.List[object]
    if ($Stage -eq 'e5-router') {
        foreach ($group in ($matrixRows | Group-Object block_size_kib, file, scope_kib)) {
            $rows = @($group.Group)
            $reference = @($rows | Where-Object { $_.policy -eq 'auto' })
            if ($reference.Count -ne 1 -or [int]$reference[0].full_oracle_evaluated -ne 1) {
                throw "Missing full-Auto reference for $($group.Name)"
            }
            $referenceModes = Get-BlockModeNames $reference[0].block_types
            foreach ($policy in @('auto-k2', 'auto-k4', 'auto-k8')) {
                $shortlist = @($rows | Where-Object { $_.policy -eq $policy })
                if ($shortlist.Count -ne 1) {
                    throw "Missing $policy row for $($group.Name)"
                }
                $candidateIds = Get-ModeIds $shortlist[0].candidate_modes
                $selectedCoverage = $true
                foreach ($modeName in $referenceModes) {
                    $modeId = [array]::IndexOf($r2BlockModes, $modeName)
                    if ($modeId -lt 0 -or -not $candidateIds.Contains($modeId)) {
                        $selectedCoverage = $false
                    }
                }
                $summaryRows.Add([pscustomobject][ordered]@{
                    stage = $Stage
                    block_size_kib = [int]$shortlist[0].block_size_kib
                    file = $shortlist[0].file
                    scope_kib = [int]$shortlist[0].scope_kib
                    input_sha256 = $shortlist[0].input_sha256
                    ranker_version = $shortlist[0].ranker_version
                    ranker_crc32 = $shortlist[0].ranker_crc32
                    ranker_sha256 = $shortlist[0].ranker_sha256
                    policy = $policy
                    reference_kind = 'full-auto-reference'
                    full_auto_archive_bytes = [int64]$reference[0].archive_bytes
                    shortlist_archive_bytes = [int64]$shortlist[0].archive_bytes
                    regret_vs_full_auto_bytes = [int64]$shortlist[0].archive_bytes - [int64]$reference[0].archive_bytes
                    full_auto_selected_modes = $reference[0].block_types
                    shortlist_candidate_modes = $shortlist[0].candidate_modes
                    full_auto_selected_mode_covered = $selectedCoverage
                    tie_aware_winner_recall_available = $false
                    candidates_evaluated = [int]$shortlist[0].candidates_evaluated
                    encode_seconds = [double]$shortlist[0].encode_seconds
                    decode_seconds = [double]$shortlist[0].decode_seconds
                    peak_ram_mib = [double]$shortlist[0].peak_ram_mib
                    roundtrip = $shortlist[0].roundtrip
                })
            }
        }
        $retained = @($summaryRows.ToArray())
        $aggregate = [ordered]@{
            status = 'COMPLETE'
            evidence_boundary = 'Reference regret is against current full Auto. Tie-aware forced-mode oracle recall requires a separately completed forced-mode ledger.'
            rows = $retained.Count
            selected_mode_coverage = if ($retained.Count -eq 0) { 0 } else {
                (@($retained | Where-Object full_auto_selected_mode_covered).Count / $retained.Count)
            }
            aggregate_regret_vs_full_auto_bytes = @($retained | ForEach-Object { [int64]$_.regret_vs_full_auto_bytes } | Measure-Object -Sum).Sum
            ranker_model = $rankerModel
        }
    }
    else {
        $retained = @($matrixRows | Where-Object { -not $_.warmup })
        foreach ($group in ($retained | Group-Object block_size_kib, scope_kib, fast_thread_count)) {
            $rows = @($group.Group)
            $inputBytes = @($rows | ForEach-Object { [int64]$_.input_bytes } | Measure-Object -Sum).Sum
            $encodeSeconds = @($rows | ForEach-Object { [double]$_.encode_seconds } | Measure-Object -Sum).Sum
            $decodeSeconds = @($rows | ForEach-Object { [double]$_.decode_seconds } | Measure-Object -Sum).Sum
            $summaryRows.Add([pscustomobject][ordered]@{
                stage = $Stage
                block_size_kib = [int]$rows[0].block_size_kib
                scope_kib = [int]$rows[0].scope_kib
                fast_thread_count = [int]$rows[0].fast_thread_count
                ranker_version = $rows[0].ranker_version
                ranker_crc32 = $rows[0].ranker_crc32
                ranker_sha256 = $rows[0].ranker_sha256
                retained_samples = $rows.Count
                input_bytes = [int64]$inputBytes
                encode_mb_per_s = if ($encodeSeconds -gt 0) { $inputBytes / $encodeSeconds / 1000000.0 } else { 0 }
                decode_mb_per_s = if ($decodeSeconds -gt 0) { $inputBytes / $decodeSeconds / 1000000.0 } else { 0 }
                encode_p50_ms = 1000.0 * (Get-Quantile @($rows | ForEach-Object { [double]$_.encode_seconds }) 0.50)
                encode_p95_ms = 1000.0 * (Get-Quantile @($rows | ForEach-Object { [double]$_.encode_seconds }) 0.95)
                decode_p50_ms = 1000.0 * (Get-Quantile @($rows | ForEach-Object { [double]$_.decode_seconds }) 0.50)
                decode_p95_ms = 1000.0 * (Get-Quantile @($rows | ForEach-Object { [double]$_.decode_seconds }) 0.95)
                peak_ram_mib = @($rows | ForEach-Object { [double]$_.peak_ram_mib } | Measure-Object -Maximum).Maximum
                byte_exact = (@($rows | Where-Object { $_.roundtrip -eq 'PASS' }).Count -eq $rows.Count)
            })
        }
        $aggregate = [ordered]@{
            status = 'COMPLETE'
            evidence_boundary = 'Fast-policy warmup repeat 0 is retained in matrix_rows.csv and excluded from timing summaries. Each row records the requested and telemetry-reported Fast worker count.'
            rows = $summaryRows.Count
            cpu_floor_mb_per_s = 0.16
            all_rows_byte_exact = @($summaryRows | Where-Object byte_exact).Count -eq $summaryRows.Count
            ranker_model = $rankerModel
        }
    }
    Write-Csv (Join-Path $packagePath 'summary_rows.csv') $summaryRows.ToArray()
    Write-Utf8Json (Join-Path $packagePath 'summary.json') $aggregate
    $plan.status = 'COMPLETE'
    Write-Utf8Json (Join-Path $packagePath 'experiment.json') $plan
}
catch {
    $plan.status = 'FAILED'
    $plan.failure = $_.Exception.Message
    Write-Utf8Json (Join-Path $packagePath 'experiment.json') $plan
    throw
}

Write-Host "Experiment complete: $packagePath"
