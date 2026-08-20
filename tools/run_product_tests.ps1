[CmdletBinding()]
param(
    [string]$CodecPath = (Join-Path $PSScriptRoot '..\build\Release\hybridzip.exe'),
    [string]$ResultsRoot = (Join-Path $PSScriptRoot '..\results'),
    [int]$TimeoutSeconds = 1800
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'
$invariant = [System.Globalization.CultureInfo]::InvariantCulture
[System.Threading.Thread]::CurrentThread.CurrentCulture = $invariant
[System.Threading.Thread]::CurrentThread.CurrentUICulture = $invariant

function Get-Sha256([string]$Path) {
    return (Get-FileHash -LiteralPath $Path -Algorithm SHA256).Hash
}

function Write-Utf8NoBomAtomic([string]$Path, [string[]]$Lines) {
    $parent = Split-Path -Parent $Path
    if (-not (Test-Path -LiteralPath $parent -PathType Container)) {
        New-Item -ItemType Directory -Path $parent -Force | Out-Null
    }
    $temporary = "$Path.tmp-$PID"
    $encoding = New-Object System.Text.UTF8Encoding($false)
    [System.IO.File]::WriteAllLines($temporary, $Lines, $encoding)
    Move-Item -LiteralPath $temporary -Destination $Path -Force
}

function Write-Prefix([string]$Source, [string]$Destination, [int]$Bytes) {
    $input = [System.IO.File]::OpenRead($Source)
    try {
        if ($input.Length -lt $Bytes) {
            throw "Source is shorter than requested prefix: $Source"
        }
        $buffer = New-Object byte[] $Bytes
        $offset = 0
        while ($offset -lt $Bytes) {
            $read = $input.Read($buffer, $offset, $Bytes - $offset)
            if ($read -eq 0) {
                throw "Unexpected end of source: $Source"
            }
            $offset += $read
        }
        [System.IO.File]::WriteAllBytes($Destination, $buffer)
    }
    finally {
        $input.Dispose()
    }
}

function Quote-Argument([string]$Value) {
    if ($Value -notmatch '[\s"]') {
        return $Value
    }
    return '"' + $Value.Replace('"', '\"') + '"'
}

function Invoke-MeasuredProcess {
    param(
        [string]$Executable,
        [string[]]$Arguments,
        [string]$StdoutPath,
        [string]$StderrPath,
        [int]$Timeout
    )

    $argumentList = @($Arguments | ForEach-Object { Quote-Argument $_ })
    $stopwatch = [System.Diagnostics.Stopwatch]::StartNew()
    $process = Start-Process -FilePath $Executable `
        -ArgumentList $argumentList `
        -RedirectStandardOutput $StdoutPath `
        -RedirectStandardError $StderrPath `
        -WindowStyle Hidden `
        -PassThru
    $processHandle = $process.Handle
    $peakBytes = 0L
    $timedOut = $false

    while (-not $process.HasExited) {
        try {
            $process.Refresh()
            $peakBytes = [Math]::Max($peakBytes, $process.PeakWorkingSet64)
        }
        catch {
        }
        if ($stopwatch.Elapsed.TotalSeconds -gt $Timeout) {
            $timedOut = $true
            $process.Kill()
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
    if ($timedOut) {
        throw "Process timed out after $Timeout seconds: $Executable"
    }
    if ($null -eq $process.ExitCode) {
        throw "Process exit code unavailable (handle $processHandle): $Executable"
    }

    return [pscustomobject]@{
        ExitCode = $process.ExitCode
        Seconds = $stopwatch.Elapsed.TotalSeconds
        PeakMiB = $peakBytes / 1MB
    }
}

function New-DirectSpec([string[]]$Arguments, [string]$OutputPath) {
    return [pscustomobject]@{
        Arguments = $Arguments
        StdoutIsOutput = $false
        OutputPath = $OutputPath
    }
}

function New-StdoutSpec([string[]]$Arguments, [string]$OutputPath) {
    return [pscustomobject]@{
        Arguments = $Arguments
        StdoutIsOutput = $true
        OutputPath = $OutputPath
    }
}

function Get-CodecSpecs($Codec, [string]$InputPath, [string]$ArchivePath,
        [string]$DecodedPath) {
    switch ($Codec.Name) {
        'HybridZip' {
            return @(
                (New-DirectSpec @('c', $InputPath, $ArchivePath) $ArchivePath),
                (New-DirectSpec @('d', $ArchivePath, $DecodedPath) $DecodedPath)
            )
        }
        'gzip' {
            return @(
                (New-StdoutSpec @('-9', '-c', $InputPath) $ArchivePath),
                (New-StdoutSpec @('-d', '-c', $ArchivePath) $DecodedPath)
            )
        }
        'zstd' {
            return @(
                (New-DirectSpec @('-19', '-T1', '-f', $InputPath, '-o', $ArchivePath) $ArchivePath),
                (New-DirectSpec @('-d', '-f', $ArchivePath, '-o', $DecodedPath) $DecodedPath)
            )
        }
        'brotli' {
            return @(
                (New-DirectSpec @('-q', '11', '-f', '-o', $ArchivePath, $InputPath) $ArchivePath),
                (New-DirectSpec @('-d', '-f', '-o', $DecodedPath, $ArchivePath) $DecodedPath)
            )
        }
        'xz' {
            return @(
                (New-StdoutSpec @('-9e', '-T1', '-c', $InputPath) $ArchivePath),
                (New-StdoutSpec @('-d', '-c', $ArchivePath) $DecodedPath)
            )
        }
        '7-Zip' {
            return @(
                (New-DirectSpec @('a', '-t7z', '-mx=9', '-mmt=1', '-bd', '-bso0', '-bsp0', $ArchivePath, $InputPath) $ArchivePath),
                (New-StdoutSpec @('e', '-so', '-bd', '-bso0', '-bsp0', $ArchivePath) $DecodedPath)
            )
        }
        'PAQ8PX' {
            return @(
                (New-DirectSpec @('-1', $InputPath, $ArchivePath) $ArchivePath),
                (New-DirectSpec @('-d', $ArchivePath, $DecodedPath) $DecodedPath)
            )
        }
        default {
            throw "Unsupported codec: $($Codec.Name)"
        }
    }
}

function Invoke-CodecRoundtrip($Codec, $Case, [string]$InputPath,
        [string]$CodecRoot) {
    $caseRoot = Join-Path $CodecRoot $Case.Id
    $archivePath = Join-Path $caseRoot ('archive' + $Codec.Extension)
    $decodedPath = Join-Path $caseRoot 'decoded.bin'
    $logRoot = Join-Path $caseRoot 'logs'
    New-Item -ItemType Directory -Path $logRoot -Force | Out-Null
    $specs = Get-CodecSpecs $Codec $InputPath $archivePath $decodedPath

    $encodeStdout = if ($specs[0].StdoutIsOutput) {
        $archivePath
    } else {
        Join-Path $logRoot 'encode.stdout.log'
    }
    $encode = Invoke-MeasuredProcess -Executable $Codec.Path `
        -Arguments $specs[0].Arguments `
        -StdoutPath $encodeStdout `
        -StderrPath (Join-Path $logRoot 'encode.stderr.log') `
        -Timeout $TimeoutSeconds
    if ($encode.ExitCode -ne 0) {
        throw "$($Codec.Name) encode failed for $($Case.Id): exit $($encode.ExitCode)"
    }
    if (-not (Test-Path -LiteralPath $archivePath -PathType Leaf)) {
        throw "$($Codec.Name) did not create archive for $($Case.Id)"
    }

    $decodeStdout = if ($specs[1].StdoutIsOutput) {
        $decodedPath
    } else {
        Join-Path $logRoot 'decode.stdout.log'
    }
    $decode = Invoke-MeasuredProcess -Executable $Codec.Path `
        -Arguments $specs[1].Arguments `
        -StdoutPath $decodeStdout `
        -StderrPath (Join-Path $logRoot 'decode.stderr.log') `
        -Timeout $TimeoutSeconds
    if ($decode.ExitCode -ne 0) {
        throw "$($Codec.Name) decode failed for $($Case.Id): exit $($decode.ExitCode)"
    }

    $inputItem = Get-Item -LiteralPath $InputPath
    $archiveItem = Get-Item -LiteralPath $archivePath
    $decodedItem = Get-Item -LiteralPath $decodedPath
    $roundtrip = ($inputItem.Length -eq $decodedItem.Length) -and
        ((Get-Sha256 $InputPath) -eq (Get-Sha256 $decodedPath))
    if (-not $roundtrip) {
        throw "$($Codec.Name) roundtrip mismatch for $($Case.Id)"
    }

    $sizeMiB = $inputItem.Length / 1MB
    return [pscustomobject]@{
        File = $Case.FileName
        Type = $Case.Type
        OriginalBytes = $inputItem.Length
        CompressedBytes = $archiveItem.Length
        Ratio = $archiveItem.Length / $inputItem.Length
        Bpb = ($archiveItem.Length * 8.0) / $inputItem.Length
        CompressSeconds = $encode.Seconds
        DecompressSeconds = $decode.Seconds
        CompressMBps = if ($encode.Seconds -gt 0) { $sizeMiB / $encode.Seconds } else { 0 }
        DecompressMBps = if ($decode.Seconds -gt 0) { $sizeMiB / $decode.Seconds } else { 0 }
        PeakMemoryMB = [Math]::Max($encode.PeakMiB, $decode.PeakMiB)
        Roundtrip = 'PASS'
    }
}

$repoRoot = [System.IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..'))
$CodecPath = [System.IO.Path]::GetFullPath($CodecPath)
$ResultsRoot = [System.IO.Path]::GetFullPath($ResultsRoot)
$generatedRoot = Join-Path $ResultsRoot 'generated\product'
$inputRoot = Join-Path $generatedRoot 'inputs'

$cases = @(
    [pscustomobject]@{Id='plain-text'; FileName='plain-text.bin'; Type='plain_text'; Source='E:\MIXER\silesia\dickens'; Bytes=16384; Selection='first 16 KiB'},
    [pscustomobject]@{Id='source-code'; FileName='source-code.cpp'; Type='source_code'; Source='E:\MIXER\KU\cmix-upstream\src\mixer\lstm.cpp'; Bytes=0; Selection='complete file'},
    [pscustomobject]@{Id='json-xml'; FileName='json-xml.xml'; Type='json_xml'; Source='E:\MIXER\silesia\xml'; Bytes=16384; Selection='first 16 KiB'},
    [pscustomobject]@{Id='binary-executable'; FileName='binary-executable.bin'; Type='binary_executable'; Source=$CodecPath; Bytes=16384; Selection='first 16 KiB'},
    [pscustomobject]@{Id='database-records'; FileName='database-records.bin'; Type='database_binary_records'; Source='E:\MIXER\silesia\osdb'; Bytes=16384; Selection='first 16 KiB'},
    [pscustomobject]@{Id='image-xray'; FileName='image-xray.bin'; Type='image'; Source='E:\MIXER\silesia\x-ray'; Bytes=16384; Selection='first 16 KiB'},
    [pscustomobject]@{Id='audio-media'; FileName='audio-media.wav'; Type='audio_media'; Source='E:\MIXER\KU\product-corpus\sample.wav'; Bytes=16384; Selection='first 16 KiB'},
    [pscustomobject]@{Id='compressed-high-entropy'; FileName='compressed-high-entropy.hz'; Type='compressed_high_entropy'; Source=(Join-Path $repoRoot 'build\phase5_128k\dickens.128KiB.hz'); Bytes=16384; Selection='first 16 KiB'},
    [pscustomobject]@{Id='large-mixed'; FileName='large-mixed.bin'; Type='large_mixed'; Source='E:\MIXER\silesia\mozilla'; Bytes=131072; Selection='first 128 KiB'}
)

$codecs = @(
    [pscustomobject]@{Name='HybridZip'; Version='1.0.0-profile-v1'; Path=$CodecPath; Parameters='profile-v1'; Extension='.hz'},
    [pscustomobject]@{Name='gzip'; Version='1.14'; Path='C:\Program Files\Git\usr\bin\gzip.exe'; Parameters='-9'; Extension='.gz'},
    [pscustomobject]@{Name='zstd'; Version='1.5.7'; Path='D:\anaconda\Library\bin\zstd.exe'; Parameters='-19 -T1'; Extension='.zst'},
    [pscustomobject]@{Name='brotli'; Version='1.2.0'; Path='C:\msys64\usr\bin\brotli.exe'; Parameters='-q 11'; Extension='.br'},
    [pscustomobject]@{Name='xz'; Version='5.6.4'; Path='D:\anaconda\Library\bin\xz.exe'; Parameters='-9e -T1'; Extension='.xz'},
    [pscustomobject]@{Name='7-Zip'; Version='26.00'; Path='C:\Program Files\7-Zip\7z.exe'; Parameters='-t7z -mx=9 -mmt=1'; Extension='.7z'},
    [pscustomobject]@{Name='PAQ8PX'; Version='v216'; Path='F:\paq8px\experiment\build\paq8px.exe'; Parameters='-1'; Extension='.paq8px216'}
)

foreach ($path in @($CodecPath, $cases.Source, $codecs.Path)) {
    if (-not (Test-Path -LiteralPath $path -PathType Leaf)) {
        throw "Required input or executable not found: $path"
    }
}
foreach ($path in @($generatedRoot,
        (Join-Path $ResultsRoot 'product_manifest.tsv'),
        (Join-Path $ResultsRoot 'product_test.tsv'),
        (Join-Path $ResultsRoot 'baseline_test.tsv'),
        (Join-Path $ResultsRoot 'baseline_tools.tsv'))) {
    if (Test-Path -LiteralPath $path) {
        throw "Refusing to overwrite existing product-test output: $path"
    }
}

New-Item -ItemType Directory -Path $inputRoot -Force | Out-Null
$manifestRows = New-Object System.Collections.Generic.List[object]
foreach ($case in $cases) {
    $inputPath = Join-Path $inputRoot $case.FileName
    if ($case.Bytes -eq 0) {
        Copy-Item -LiteralPath $case.Source -Destination $inputPath
    }
    else {
        Write-Prefix -Source $case.Source -Destination $inputPath -Bytes $case.Bytes
    }
    $sourceItem = Get-Item -LiteralPath $case.Source
    $inputItem = Get-Item -LiteralPath $inputPath
    $manifestRows.Add([pscustomobject][ordered]@{
        file = $case.FileName
        type = $case.Type
        source_path = [System.IO.Path]::GetFullPath($case.Source)
        source_bytes = $sourceItem.Length
        input_bytes = $inputItem.Length
        input_sha256 = Get-Sha256 $inputPath
        selection = $case.Selection
    })
}
Write-Utf8NoBomAtomic -Path (Join-Path $ResultsRoot 'product_manifest.tsv') `
    -Lines @($manifestRows | ConvertTo-Csv -NoTypeInformation -Delimiter "`t")

$toolRows = foreach ($codec in $codecs) {
    [pscustomobject][ordered]@{
        codec = $codec.Name
        version = $codec.Version
        executable_path = [System.IO.Path]::GetFullPath($codec.Path)
        executable_sha256 = Get-Sha256 $codec.Path
        parameters = $codec.Parameters
    }
}
Write-Utf8NoBomAtomic -Path (Join-Path $ResultsRoot 'baseline_tools.tsv') `
    -Lines @($toolRows | ConvertTo-Csv -NoTypeInformation -Delimiter "`t")

$hybridResults = New-Object System.Collections.Generic.List[object]
$baselineResults = New-Object System.Collections.Generic.List[object]
$completed = 0
$total = $cases.Count * $codecs.Count
foreach ($codec in $codecs) {
    $codecRoot = Join-Path $generatedRoot $codec.Name
    foreach ($case in $cases) {
        $inputPath = Join-Path $inputRoot $case.FileName
        $measurement = Invoke-CodecRoundtrip $codec $case $inputPath $codecRoot
        ++$completed
        Write-Host ("[{0}/{1}] {2} / {3}: PASS" -f $completed, $total, $codec.Name, $case.Id)
        if ($codec.Name -eq 'HybridZip') {
            $hybridResults.Add([pscustomobject][ordered]@{
                file = $measurement.File
                type = $measurement.Type
                original_bytes = $measurement.OriginalBytes
                compressed_bytes = $measurement.CompressedBytes
                ratio = $measurement.Ratio
                bpb = $measurement.Bpb
                compress_seconds = $measurement.CompressSeconds
                decompress_seconds = $measurement.DecompressSeconds
                compress_MBps = $measurement.CompressMBps
                decompress_MBps = $measurement.DecompressMBps
                peak_memory_MB = $measurement.PeakMemoryMB
            })
            Write-Utf8NoBomAtomic -Path (Join-Path $ResultsRoot 'product_test.tsv') `
                -Lines @($hybridResults | ConvertTo-Csv -NoTypeInformation -Delimiter "`t")
        }
        else {
            $baselineResults.Add([pscustomobject][ordered]@{
                codec = $codec.Name
                version = $codec.Version
                parameters = $codec.Parameters
                file = $measurement.File
                type = $measurement.Type
                original_bytes = $measurement.OriginalBytes
                compressed_bytes = $measurement.CompressedBytes
                ratio = $measurement.Ratio
                bpb = $measurement.Bpb
                compress_seconds = $measurement.CompressSeconds
                decompress_seconds = $measurement.DecompressSeconds
                compress_MBps = $measurement.CompressMBps
                decompress_MBps = $measurement.DecompressMBps
                peak_memory_MB = $measurement.PeakMemoryMB
                roundtrip = $measurement.Roundtrip
            })
            Write-Utf8NoBomAtomic -Path (Join-Path $ResultsRoot 'baseline_test.tsv') `
                -Lines @($baselineResults | ConvertTo-Csv -NoTypeInformation -Delimiter "`t")
        }
    }
}

if ($hybridResults.Count -ne $cases.Count) {
    throw "HybridZip result count is $($hybridResults.Count), expected $($cases.Count)"
}
if ($baselineResults.Count -ne (($codecs.Count - 1) * $cases.Count)) {
    throw "Baseline result count is $($baselineResults.Count), expected $(($codecs.Count - 1) * $cases.Count)"
}
Write-Host "Product test complete: $ResultsRoot"
