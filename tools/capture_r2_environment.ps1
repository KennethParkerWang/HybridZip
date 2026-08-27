[CmdletBinding()]
param(
    [string]$CodecPath = '',
    [string]$OutputPath = '',
    [switch]$ListOnly
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

function Get-FirstCommandLine([string]$Name, [string[]]$Arguments) {
    try {
        $command = Get-Command $Name -ErrorAction Stop | Select-Object -First 1
        $lines = @(& $command.Source @Arguments 2>&1)
        return [string]::Join("`n", @($lines | Select-Object -First 4))
    }
    catch {
        return ''
    }
}

function Get-Sha256OrEmpty([string]$Path) {
    if ([string]::IsNullOrWhiteSpace($Path) -or
        -not (Test-Path -LiteralPath $Path -PathType Leaf)) {
        return ''
    }
    return (Get-FileHash -LiteralPath $Path -Algorithm SHA256).Hash.ToUpperInvariant()
}

function Get-GitText([string[]]$Arguments) {
    try {
        return ([string]::Join("`n", @(& git -C $script:repoRoot @Arguments 2>$null))).Trim()
    }
    catch {
        return ''
    }
}

$repoRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..'))
if (-not [string]::IsNullOrWhiteSpace($CodecPath)) {
    $CodecPath = [IO.Path]::GetFullPath($CodecPath)
}

$os = $null
$processors = @()
$memory = $null
$gpus = @()
try { $os = Get-CimInstance Win32_OperatingSystem -ErrorAction Stop } catch {}
try { $processors = @(Get-CimInstance Win32_Processor -ErrorAction Stop) } catch {}
try { $memory = Get-CimInstance Win32_ComputerSystem -ErrorAction Stop } catch {}
try { $gpus = @(Get-CimInstance Win32_VideoController -ErrorAction Stop) } catch {}

$powerPlan = Get-FirstCommandLine 'powercfg.exe' @('/getactivescheme')
$nvidiaSmi = Get-FirstCommandLine 'nvidia-smi.exe' @(
    '--query-gpu=name,driver_version,memory.total,pci.bus_id', '--format=csv,noheader')
$sourceRevision = Get-GitText @('rev-parse', 'HEAD')
$sourceDirty = -not [string]::IsNullOrWhiteSpace(
    (Get-GitText @('status', '--porcelain')))

$fingerprintMaterial = [ordered]@{
    schema_version = 1
    os = [ordered]@{
        caption = if ($null -eq $os) { '' } else { [string]$os.Caption }
        version = if ($null -eq $os) { '' } else { [string]$os.Version }
        build_number = if ($null -eq $os) { '' } else { [string]$os.BuildNumber }
        architecture = if ($null -eq $os) { '' } else { [string]$os.OSArchitecture }
    }
    cpu = @($processors | ForEach-Object {
        [ordered]@{
            name = [string]$_.Name
            cores = [int]$_.NumberOfCores
            logical_processors = [int]$_.NumberOfLogicalProcessors
            max_clock_mhz = [int]$_.MaxClockSpeed
        }
    })
    memory = [ordered]@{
        total_physical_bytes = if ($null -eq $memory) { 0 } else { [int64]$memory.TotalPhysicalMemory }
    }
    gpu = @($gpus | ForEach-Object {
        [ordered]@{
            name = [string]$_.Name
            driver_version = [string]$_.DriverVersion
            video_processor = [string]$_.VideoProcessor
            adapter_ram_bytes = [int64]$_.AdapterRAM
        }
    })
    nvidia_smi = $nvidiaSmi
    active_power_plan = $powerPlan
    compiler = [ordered]@{
        cmake_version = Get-FirstCommandLine 'cmake.exe' @('--version')
        cl_version = Get-FirstCommandLine 'cl.exe' @()
    }
    codec = [ordered]@{
        path = $CodecPath
        sha256 = Get-Sha256OrEmpty $CodecPath
    }
    source = [ordered]@{
        revision = $sourceRevision
        dirty = $sourceDirty
    }
}
$fingerprintJson = $fingerprintMaterial | ConvertTo-Json -Depth 8 -Compress
$sha256 = [Security.Cryptography.SHA256]::Create()
try {
    $fingerprint = ([BitConverter]::ToString(
        $sha256.ComputeHash([Text.Encoding]::UTF8.GetBytes($fingerprintJson))
    ) -replace '-', '')
}
finally {
    $sha256.Dispose()
}
$result = [ordered]@{
    schema_version = 1
    captured_at_utc = [DateTime]::UtcNow.ToString('o')
    fingerprint_sha256 = $fingerprint
    fingerprint_material = $fingerprintMaterial
}

if ($ListOnly) {
    $result | ConvertTo-Json -Depth 10
    return
}
if ([string]::IsNullOrWhiteSpace($OutputPath)) {
    $result | ConvertTo-Json -Depth 10
    return
}
$OutputPath = [IO.Path]::GetFullPath($OutputPath)
$outputDirectory = [IO.Path]::GetDirectoryName($OutputPath)
if (-not (Test-Path -LiteralPath $outputDirectory -PathType Container)) {
    throw "Environment output directory does not exist: $outputDirectory"
}
if (Test-Path -LiteralPath $OutputPath) {
    throw "Refusing to overwrite environment manifest: $OutputPath"
}
[IO.File]::WriteAllText(
    $OutputPath, (($result | ConvertTo-Json -Depth 10) + "`n"),
    [Text.UTF8Encoding]::new($false))
$result | ConvertTo-Json -Depth 10
