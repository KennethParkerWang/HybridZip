[CmdletBinding()]
param(
    [string]$DatasetPath = 'F:\paq8px\silesia',
    [string]$OutputPath = ''
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

$repoRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..'))
if ([string]::IsNullOrWhiteSpace($OutputPath)) {
    $OutputPath = Join-Path $repoRoot 'bench\manifests\silesia-leading-32-64-128.tsv'
}
$DatasetPath = [IO.Path]::GetFullPath($DatasetPath)
$OutputPath = [IO.Path]::GetFullPath($OutputPath)

$files = @(
    'dickens', 'mozilla', 'mr', 'nci', 'ooffice', 'osdb',
    'reymont', 'samba', 'sao', 'webster', 'x-ray', 'xml'
)
$scopes = @(32, 64, 128)

function Get-LeadingPrefixSha256([string]$Path, [int]$PrefixBytes) {
    $stream = [IO.File]::Open($Path, [IO.FileMode]::Open, [IO.FileAccess]::Read,
        [IO.FileShare]::Read)
    try {
        if ($stream.Length -lt $PrefixBytes) {
            throw "Source is shorter than requested prefix: $Path"
        }
        $buffer = New-Object byte[] $PrefixBytes
        $offset = 0
        while ($offset -lt $PrefixBytes) {
            $read = $stream.Read($buffer, $offset, $PrefixBytes - $offset)
            if ($read -eq 0) {
                throw "Unexpected end of source while reading prefix: $Path"
            }
            $offset += $read
        }
    }
    finally {
        $stream.Dispose()
    }

    $sha256 = [Security.Cryptography.SHA256]::Create()
    try {
        return [BitConverter]::ToString($sha256.ComputeHash($buffer)).Replace('-', '')
    }
    finally {
        $sha256.Dispose()
    }
}

function Write-Utf8NoBomNewFile([string]$Path, [string]$Content) {
    if (Test-Path -LiteralPath $Path) {
        throw "Refusing to overwrite existing manifest: $Path"
    }
    $directory = Split-Path -Parent $Path
    if (-not (Test-Path -LiteralPath $directory -PathType Container)) {
        New-Item -ItemType Directory -Path $directory -Force | Out-Null
    }
    $temporaryFileName = '.{0}.{1}.tmp' -f ([IO.Path]::GetFileName($Path)),
        ([Guid]::NewGuid().ToString('N'))
    $temporaryPath = Join-Path $directory $temporaryFileName
    try {
        [IO.File]::WriteAllText($temporaryPath, $Content, (New-Object Text.UTF8Encoding($false)))
        [IO.File]::Move($temporaryPath, $Path)
    }
    finally {
        if (Test-Path -LiteralPath $temporaryPath -PathType Leaf) {
            [IO.File]::Delete($temporaryPath)
        }
    }
}

if (-not (Test-Path -LiteralPath $DatasetPath -PathType Container)) {
    throw "Silesia dataset directory not found: $DatasetPath"
}
if (Test-Path -LiteralPath $OutputPath) {
    throw "Refusing to overwrite existing manifest: $OutputPath"
}

$presentFiles = @(Get-ChildItem -LiteralPath $DatasetPath -File | ForEach-Object Name)
if ($presentFiles.Count -ne $files.Count -or
    (@($presentFiles | Sort-Object) -join '|') -ne (@($files | Sort-Object) -join '|')) {
    throw 'Dataset must contain exactly the canonical 12 Silesia files'
}

$rows = New-Object System.Collections.Generic.List[object]
$caseOrder = 0
foreach ($file in $files) {
    $sourcePath = Join-Path $DatasetPath $file
    if (-not (Test-Path -LiteralPath $sourcePath -PathType Leaf)) {
        throw "Silesia source file not found: $sourcePath"
    }
    $source = Get-Item -LiteralPath $sourcePath
    $sourceHash = (Get-FileHash -LiteralPath $sourcePath -Algorithm SHA256).Hash.ToUpperInvariant()
    foreach ($scopeKiB in $scopes) {
        $prefixBytes = $scopeKiB * 1024
        if ($source.Length -lt $prefixBytes) {
            throw "Silesia source is shorter than ${scopeKiB}KiB: $sourcePath"
        }
        $caseOrder++
        $rows.Add([pscustomobject][ordered]@{
            manifest_version = 1
            case_order = $caseOrder
            case_id = ('{0}-leading-{1}k' -f $file, $scopeKiB)
            file = $file
            scope_kib = $scopeKiB
            prefix_bytes = $prefixBytes
            input_policy = 'leading-prefix-v1'
            source_path = $source.FullName
            source_bytes = [Int64]$source.Length
            source_sha256 = $sourceHash
            prefix_sha256 = Get-LeadingPrefixSha256 -Path $source.FullName -PrefixBytes $prefixBytes
        })
    }
}

$tsv = (($rows | ConvertTo-Csv -Delimiter "`t" -NoTypeInformation) -join "`r`n") + "`r`n"
Write-Utf8NoBomNewFile -Path $OutputPath -Content $tsv

[pscustomobject][ordered]@{
    manifest_path = $OutputPath
    rows = $rows.Count
    files = $files.Count
    scopes_kib = [string]::Join(',', $scopes)
    input_policy = 'leading-prefix-v1'
    generated_utc = [DateTime]::UtcNow.ToString('o')
} | ConvertTo-Json -Compress
