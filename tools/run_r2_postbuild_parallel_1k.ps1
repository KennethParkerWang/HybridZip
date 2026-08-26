[CmdletBinding()]
param(
    [string]$CodecPath = (Join-Path $PSScriptRoot '..\build\Release\hybridzip.exe'),
    [string]$InputPath = (Join-Path $PSScriptRoot '..\results\smoke\r2-postbuild-1k-20260826\input.bin'),
    [string]$SmokeRoot = (Join-Path $PSScriptRoot '..\results\smoke'),
    [string]$CodecSha256 = 'FDE6F9ABC0F831CC9E35BF6B53C24654E06FBB2EE232856924E211A17B04A75B',
    [int]$TimeoutSeconds = 60
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'
[System.Threading.Thread]::CurrentThread.CurrentCulture =
    [System.Globalization.CultureInfo]::InvariantCulture
[System.Threading.Thread]::CurrentThread.CurrentUICulture =
    [System.Globalization.CultureInfo]::InvariantCulture

$CodecPath = [IO.Path]::GetFullPath($CodecPath)
$InputPath = [IO.Path]::GetFullPath($InputPath)
$SmokeRoot = [IO.Path]::GetFullPath($SmokeRoot)
if (-not (Test-Path -LiteralPath $CodecPath -PathType Leaf)) {
    throw "Codec not found: $CodecPath"
}
if (-not (Test-Path -LiteralPath $InputPath -PathType Leaf)) {
    throw "Input not found: $InputPath"
}
$actualCodecSha256 = (Get-FileHash -LiteralPath $CodecPath -Algorithm SHA256).Hash
if ($actualCodecSha256 -ne $CodecSha256) {
    throw "Codec SHA-256 mismatch. expected=$CodecSha256 actual=$actualCodecSha256"
}
$inputBytes = [IO.File]::ReadAllBytes($InputPath)
if ($inputBytes.Length -ne 1024) {
    throw "Expected exactly 1024 input bytes, got $($inputBytes.Length)"
}
$inputSha256 = (Get-FileHash -LiteralPath $InputPath -Algorithm SHA256).Hash

$modes = @(
    [pscustomobject]@{ Id = 11; Name = 'bitshuffle-zstd' },
    [pscustomobject]@{ Id = 10; Name = 'shuffle-zstd' },
    [pscustomobject]@{ Id = 9; Name = 'x86-bcj-zstd' },
    [pscustomobject]@{ Id = 8; Name = 'bwt-rlt-zstd' },
    [pscustomobject]@{ Id = 7; Name = 'bwt-mtf-zstd' },
    [pscustomobject]@{ Id = 6; Name = 'bwt-zstd' },
    [pscustomobject]@{ Id = 5; Name = 'donor-match' },
    [pscustomobject]@{ Id = 4; Name = 'lzma' },
    [pscustomobject]@{ Id = 3; Name = 'fse' },
    [pscustomobject]@{ Id = 2; Name = 'zstd' },
    [pscustomobject]@{ Id = 1; Name = 'predictive' },
    [pscustomobject]@{ Id = 0; Name = 'stored' }
)

$runTag = Get-Date -Format 'yyyyMMdd-HHmmss'
$workerScript = {
    param($WorkerName, $WorkerModes, $CodecPath, $InputPath, $SmokeRoot,
        $CodecSha256, $InputSha256, $TimeoutSeconds, $RunTag)

    Set-StrictMode -Version Latest
    $ErrorActionPreference = 'Stop'
    $invariant = [System.Globalization.CultureInfo]::InvariantCulture
    [System.Threading.Thread]::CurrentThread.CurrentCulture = $invariant
    [System.Threading.Thread]::CurrentThread.CurrentUICulture = $invariant

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
        $stopwatch = [Diagnostics.Stopwatch]::StartNew()
        $process = Start-Process -FilePath $Executable -ArgumentList $argumentList `
            -RedirectStandardOutput $StdoutPath -RedirectStandardError $StderrPath `
            -WindowStyle Hidden -PassThru
        $timedOut = $false
        while (-not $process.HasExited) {
            if ($stopwatch.Elapsed.TotalSeconds -gt $Timeout) {
                $timedOut = $true
                $process.Kill()
                break
            }
            [void](Start-Sleep -Milliseconds 25)
        }
        [void]$process.WaitForExit()
        $stopwatch.Stop()
        [void]$process.Refresh()
        if ($timedOut) {
            throw "Process timed out after $Timeout seconds: $Executable"
        }
        $exitCode = [int]$process.ExitCode
        $elapsedSeconds = [double]$stopwatch.Elapsed.TotalSeconds
        return ,([pscustomobject]@{
            ExitCode = $exitCode
            Seconds = $elapsedSeconds
        })
    }

    function Find-ExistingEvidence($Mode) {
        foreach ($path in Get-ChildItem -LiteralPath $SmokeRoot -Recurse `
                -Filter 'verification.json' -File -ErrorAction SilentlyContinue) {
            try {
                $record = Get-Content -LiteralPath $path.FullName -Raw | ConvertFrom-Json
                if ([int]$record.mode -eq $Mode.Id -and
                    [string]$record.codec_sha256 -eq $CodecSha256 -and
                    [int]$record.input_bytes -eq 1024 -and
                    [bool]$record.byte_exact) {
                    return $path.FullName
                }
            }
            catch {
                continue
            }
        }
        return $null
    }

    foreach ($mode in $WorkerModes) {
        $existing = Find-ExistingEvidence $mode
        if ($null -ne $existing) {
            Write-Output ("[$WorkerName] SKIP mode {0} {1}: {2}" -f
                $mode.Id, $mode.Name, $existing)
            continue
        }

        $directory = Join-Path $SmokeRoot ("r2-postbuild-{0}-mode{1}-1k-{2}-parallel" -f
                $mode.Name, $mode.Id, $RunTag)
        if (Test-Path -LiteralPath $directory) {
            throw "Refusing to overwrite existing output: $directory"
        }
        $archivePath = Join-Path $directory 'input.hz2'
        $decodedPath = Join-Path $directory 'decoded.bin'
        $encodeStdout = Join-Path $directory 'encode.stdout.log'
        $encodeStderr = Join-Path $directory 'encode.stderr.log'
        $decodeStdout = Join-Path $directory 'decode.stdout.log'
        $decodeStderr = Join-Path $directory 'decode.stderr.log'
        New-Item -ItemType Directory -Path $directory -Force | Out-Null

        try {
            $encodeResult = @(Invoke-MeasuredProcess $CodecPath `
                @('c', '--profile=r2', "--r2-mode=$($mode.Name)", $InputPath, $archivePath) `
                $encodeStdout $encodeStderr $TimeoutSeconds)
            $encode = $encodeResult | Select-Object -Last 1
            if ($encode.ExitCode -ne 0) {
                throw "encode exit code $($encode.ExitCode)"
            }
            if (-not (Test-Path -LiteralPath $archivePath -PathType Leaf)) {
                $failure = [ordered]@{
                    mode = $mode.Id
                    mode_name = $mode.Name
                    input_path = $InputPath
                    input_bytes = 1024
                    codec_sha256 = $CodecSha256
                    reason = 'encoder exited successfully but produced no archive'
                    generated_utc = [DateTime]::UtcNow.ToString('o')
                    worker = $WorkerName
                }
                $failureJson = $failure | ConvertTo-Json -Depth 4
                [IO.File]::WriteAllText((Join-Path $directory 'failure.json'),
                    $failureJson, (New-Object Text.UTF8Encoding($false)))
                Write-Output ("[{0}] FAIL-SKIP mode {1} {2}: no archive produced; continuing" -f
                    $WorkerName, $mode.Id, $mode.Name)
                continue
            }
            $decodeResult = @(Invoke-MeasuredProcess $CodecPath `
                @('d', $archivePath, $decodedPath) `
                $decodeStdout $decodeStderr $TimeoutSeconds)
            $decode = $decodeResult | Select-Object -Last 1
            if ($decode.ExitCode -ne 0) {
                throw "decode exit code $($decode.ExitCode)"
            }
            $archiveBytes = (Get-Item -LiteralPath $archivePath).Length
            $decodedBytes = (Get-Item -LiteralPath $decodedPath).Length
            $decodedSha256 = (Get-FileHash -LiteralPath $decodedPath -Algorithm SHA256).Hash
            $record = [ordered]@{
                mode = $mode.Id
                mode_name = $mode.Name
                input_path = $InputPath
                archive_path = $archivePath
                decoded_path = $decodedPath
                input_bytes = 1024
                archive_bytes = $archiveBytes
                decoded_bytes = $decodedBytes
                bpb = ($archiveBytes * 8.0) / 1024.0
                encode_seconds = $encode.Seconds
                decode_seconds = $decode.Seconds
                encode_exit_code = $encode.ExitCode
                decode_exit_code = $decode.ExitCode
                input_sha256 = $InputSha256
                decoded_sha256 = $decodedSha256
                byte_exact = ($decodedBytes -eq 1024 -and $decodedSha256 -eq $InputSha256)
                codec_sha256 = $CodecSha256
                generated_utc = [DateTime]::UtcNow.ToString('o')
                worker = $WorkerName
            }
            if (-not $record.byte_exact) {
                throw "byte-exact roundtrip failed"
            }
            $json = $record | ConvertTo-Json -Depth 4
            [IO.File]::WriteAllText((Join-Path $directory 'verification.json'), $json,
                (New-Object Text.UTF8Encoding($false)))
            Write-Output ("[{0}] PASS mode {1} {2}: archive={3} B bpb={4:N6} encode={5:N3}s decode={6:N3}s" -f
                $WorkerName, $mode.Id, $mode.Name, $archiveBytes, $record.bpb,
                $encode.Seconds, $decode.Seconds)
        }
        catch {
            Write-Output ("[{0}] FAIL mode {1} {2}: {3}" -f
                $WorkerName, $mode.Id, $mode.Name, $_.Exception.Message)
            throw
        }
    }
}

$lanes = @(
    [pscustomobject]@{ Name = 'lane-a'; Modes = @($modes[0], $modes[3], $modes[6], $modes[9]) },
    [pscustomobject]@{ Name = 'lane-b'; Modes = @($modes[1], $modes[4], $modes[7], $modes[10]) },
    [pscustomobject]@{ Name = 'lane-c'; Modes = @($modes[2], $modes[5], $modes[8], $modes[11]) }
)

$jobs = @()
foreach ($lane in $lanes) {
    $jobs += Start-Job -Name "r2-$($lane.Name)" -ScriptBlock $workerScript -ArgumentList @(
        $lane.Name, $lane.Modes, $CodecPath, $InputPath, $SmokeRoot,
        $CodecSha256, $inputSha256, $TimeoutSeconds, $runTag)
}

try {
    while (@($jobs | Where-Object State -in @('NotStarted', 'Running')).Count -gt 0) {
        foreach ($job in $jobs) {
            $jobState = (Get-Job -Id $job.Id).State
            Write-Output ("[{0}] state={1}" -f $job.Name, $jobState)
        }
        Start-Sleep -Seconds 2
    }
    $failures = @()
    foreach ($job in $jobs) {
        try {
            Receive-Job -Job $job -ErrorAction Stop | ForEach-Object {
                Write-Output $_
            }
            if ($job.State -ne 'Completed') {
                $failures += "$($job.Name) ended in state $($job.State)"
            }
        }
        catch {
            $failures += "$($job.Name): $($_.Exception.Message)"
        }
    }
    if ($failures.Count -gt 0) {
        throw ("Parallel worker failure(s): " + ($failures -join '; '))
    }
}
finally {
    $jobs | Remove-Job -Force -ErrorAction SilentlyContinue
}

Write-Output "Parallel 1 KiB gate complete. run_tag=$runTag"
