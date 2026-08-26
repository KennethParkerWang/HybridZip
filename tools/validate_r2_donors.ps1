[CmdletBinding()]
param(
    [string]$WarehouseRoot
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$script:Failures = New-Object System.Collections.Generic.List[string]
$script:CheckCount = 0

$projectRoot = Split-Path $PSScriptRoot -Parent
$mixerRoot = Split-Path $projectRoot -Parent
if ([string]::IsNullOrWhiteSpace($WarehouseRoot)) {
    $WarehouseRoot = Join-Path $mixerRoot "KU\hybridzip-r2"
}

function Add-Failure {
    param([string]$Message)

    $script:Failures.Add($Message)
}

function Assert-Check {
    param(
        [bool]$Condition,
        [string]$Message
    )

    ++$script:CheckCount
    if (-not $Condition) {
        Add-Failure $Message
    }
}

function Get-PropertyValue {
    param(
        [object]$Object,
        [string]$Name
    )

    if ($null -eq $Object) {
        return $null
    }
    $properties = @(
        $Object.PSObject.Properties |
            Where-Object { $_.Name -ceq $Name })
    if ($properties.Count -ne 1) {
        return $null
    }
    return ,$properties[0].Value
}

function Read-JsonFile {
    param([string]$Path)

    if (-not (Test-Path -LiteralPath $Path -PathType Leaf)) {
        Add-Failure "Missing JSON file: $Path"
        return $null
    }

    try {
        $decoder = New-Object System.Text.UTF8Encoding($false, $true)
        $bytes = [System.IO.File]::ReadAllBytes($Path)
        $text = $decoder.GetString($bytes)

        Add-Type -AssemblyName System.Runtime.Serialization
        $reader = [System.Runtime.Serialization.Json.JsonReaderWriterFactory]::CreateJsonReader(
            $bytes, [System.Xml.XmlDictionaryReaderQuotas]::Max)
        try {
            $document = New-Object System.Xml.XmlDocument
            $document.Load($reader)
        }
        finally {
            $reader.Dispose()
        }
        foreach ($objectNode in @($document.SelectNodes("//*[@type='object']"))) {
            $keys = @{}
            foreach ($propertyNode in @($objectNode.ChildNodes)) {
                if ($keys.ContainsKey($propertyNode.LocalName)) {
                    throw "Duplicate JSON key '$($propertyNode.LocalName)'"
                }
                $keys[$propertyNode.LocalName] = $true
            }
        }

        $value = $text | ConvertFrom-Json -ErrorAction Stop
        if ($null -eq $value -or $value -is [System.Array]) {
            Add-Failure "JSON root must be an object: $Path"
            return $null
        }
        return $value
    }
    catch {
        Add-Failure "Invalid JSON or UTF-8 in ${Path}: $($_.Exception.Message)"
        return $null
    }
}

function Get-CanonicalPath {
    param([string]$Path)

    return [System.IO.Path]::GetFullPath($Path).TrimEnd(
        [System.IO.Path]::DirectorySeparatorChar,
        [System.IO.Path]::AltDirectorySeparatorChar)
}

function Get-SafeChildPath {
    param(
        [string]$Root,
        [string]$RelativePath,
        [string]$Label
    )

    if ([string]::IsNullOrWhiteSpace($RelativePath) -or
        [System.IO.Path]::IsPathRooted($RelativePath)) {
        Add-Failure "$Label must be a relative path: $RelativePath"
        return $null
    }

    try {
        $canonicalRoot = Get-CanonicalPath $Root
        $candidate = Get-CanonicalPath (Join-Path $canonicalRoot $RelativePath)
        $prefix = $canonicalRoot + [System.IO.Path]::DirectorySeparatorChar
        $inside = $candidate.StartsWith(
            $prefix, [System.StringComparison]::OrdinalIgnoreCase)
        Assert-Check $inside "$Label escapes its root: $RelativePath"
        if ($inside) {
            return $candidate
        }
    }
    catch {
        Add-Failure "Invalid $Label path '${RelativePath}': $($_.Exception.Message)"
    }
    return $null
}

function Get-RelativeChildPath {
    param(
        [string]$Root,
        [string]$Path,
        [string]$Label
    )

    $canonicalRoot = Get-CanonicalPath $Root
    $canonicalPath = Get-CanonicalPath $Path
    $prefix = $canonicalRoot + [System.IO.Path]::DirectorySeparatorChar
    if (-not $canonicalPath.StartsWith(
            $prefix, [System.StringComparison]::OrdinalIgnoreCase)) {
        throw "$Label is outside its root: $Path"
    }
    return $canonicalPath.Substring($prefix.Length).Replace(
        [char]92, [char]47)
}

function Assert-ExactSet {
    param(
        [object[]]$Expected,
        [object[]]$Actual,
        [string]$Label
    )

    $expectedCount = if ($null -eq $Expected) { 0 } else { $Expected.Count }
    $actualCount = if ($null -eq $Actual) { 0 } else { $Actual.Count }
    Assert-Check ($actualCount -eq $expectedCount) (
        "$Label count is $actualCount; expected $expectedCount")
    foreach ($item in $Expected) {
        Assert-Check ($Actual -ccontains $item) "$Label is missing: $item"
    }
    foreach ($item in $Actual) {
        Assert-Check ($Expected -ccontains $item) "$Label is unknown: $item"
    }
}

function Assert-ManifestShape {
    param(
        [object]$Manifest,
        [string]$Path
    )

    $stringFields = @(
        "name", "url", "revision", "license", "language",
        "download_date", "status")
    foreach ($field in $stringFields) {
        $value = Get-PropertyValue $Manifest $field
        Assert-Check (
            $value -is [string] -and
            -not [string]::IsNullOrWhiteSpace([string]$value)
        ) "$Path field '$field' must be a non-empty string"
    }

    $modules = Get-PropertyValue $Manifest "candidate_modules"
    $validModules = $modules -is [System.Array] -and $modules.Count -gt 0
    if ($validModules) {
        foreach ($module in $modules) {
            if ($module -isnot [string] -or
                [string]::IsNullOrWhiteSpace([string]$module)) {
                $validModules = $false
                break
            }
        }
    }
    Assert-Check $validModules (
        "$Path field 'candidate_modules' must be a non-empty string array")
}

function Assert-Url {
    param(
        [string]$Url,
        [string]$Label
    )

    $uri = $null
    $valid = [System.Uri]::TryCreate(
        $Url, [System.UriKind]::Absolute, [ref]$uri)
    Assert-Check ($valid -and $uri.Scheme -ceq "https") (
        "$Label must be an absolute HTTPS URL: $Url")
}

function Assert-Date {
    param(
        [string]$DateText,
        [string]$Label
    )

    $parsed = [datetime]::MinValue
    $valid = [datetime]::TryParseExact(
        $DateText,
        "yyyy-MM-dd",
        [System.Globalization.CultureInfo]::InvariantCulture,
        [System.Globalization.DateTimeStyles]::None,
        [ref]$parsed)
    Assert-Check ($valid -and $parsed.Date -le [datetime]::Today) (
        "$Label must be a valid, non-future YYYY-MM-DD date: $DateText")
}

function Normalize-GitUrl {
    param([string]$Url)

    $normalized = $Url.Trim().TrimEnd("/")
    if ($normalized.EndsWith(".git", [System.StringComparison]::OrdinalIgnoreCase)) {
        $normalized = $normalized.Substring(0, $normalized.Length - 4)
    }
    return $normalized
}

$expectedDonors = @(
    [pscustomobject]@{
        RelativePath = "compressors/context-mixing/paq8px"
        Provenance = "paq8px.json"
        RootLicense = "GPL-2.0-or-later"
        CentralLicense = "GPL-2.0-or-later"
        SelectedLicense = $null
        ExpectedStatus = "ported"
        SourceCopyAllowed = $true
        PortEvidence = "third_party/paq8px/PROVENANCE.md"
        NoteTokens = @(
            "MatchCore", "src/model/MatchModel.cpp",
            "src/model/MatchInfo.cpp", "tests/paq8px_match_tests.cpp",
            "APM1", "src/APM1.cpp", "paq8px_apm_backend",
            "RecordModel", "RECORD_MODEL_AUDIT.md", "mode 32",
            "paq8px_record_model_backend", "record_model",
            "LinearPredictionModel", "LINEAR_PREDICTION_MODEL_AUDIT.md",
            "mode 33", "paq8px_linear_prediction_backend",
            "SimilarityModel", "SIMILARITY_MODEL_AUDIT.md", "mode 34",
            "paq8px_similarity_backend",
            "SSE", "SSE_AUDIT.md", "mode 35", "APMPost",
            "paq8px_similarity_sse_backend",
            "ContextModelGeneric", "GENERIC_MODEL_AUDIT.md", "mode 36",
            "paq8px_generic_sse_backend",
            "DETECTED_SSE_AUDIT.md", "mode 37",
            "paq8px_block_detector", "paq8px_detected_sse_backend",
            "FiltersDetection.hpp",
            "third_party/paq8px/PROVENANCE.md")
    },
    [pscustomobject]@{
        RelativePath = "compressors/context-mixing/cmix"
        Provenance = "cmix.json"
        RootLicense = "GPL-3.0"
        CentralLicense = "GPL-3.0"
        SelectedLicense = $null
        ExpectedStatus = "ported"
        SourceCopyAllowed = $true
        PortEvidence = "third_party/cmix/MODIFICATIONS.md"
        NoteTokens = @(
            "PPMD", "Online LSTM", "Match", "WRT Dictionary",
            "cmix_word_dictionary_transform",
            "third_party/cmix/MODIFICATIONS.md")
    },
    [pscustomobject]@{
        RelativePath = "compressors/text/brotli"
        Provenance = "brotli.json"
        RootLicense = "MIT"
        CentralLicense = "MIT"
        SelectedLicense = $null
        ExpectedStatus = "ported"
        SourceCopyAllowed = $true
        IdentityKind = "archive"
        ArchiveRelativePath = "compressors/text/brotli-8e10eeb3378f6c459dbaf033ca6727e9816afccb.tar.gz"
        ArchiveUrl = "https://codeload.github.com/google/brotli/tar.gz/8e10eeb3378f6c459dbaf033ca6727e9816afccb"
        ArchiveSha256 = "12E2DA62A51C3D9F148297723A01654BD1CFC6D87B8FC4DAA7FA9E52E546911E"
        PortEvidence = "third_party/brotli/PROVENANCE.md"
        NoteTokens = @(
            "Brotli text", "third_party/brotli",
            "src/r2/representation/brotli_text_transform",
            "r2-brotli-text-32k-20260820")
    },
    [pscustomobject]@{
        RelativePath = "compressors/lz/7zip"
        Provenance = "7zip.json"
        RootLicense = "mixed: LGPL-2.1-or-later, unRAR restrictions, BSD, and file-level public-domain declarations"
        CentralLicense = "mixed: LGPL-2.1-or-later, unRAR restrictions, BSD, and file-level public-domain declarations"
        SelectedLicense = $null
        ExpectedStatus = "ported"
        SourceCopyAllowed = $false
        PortEvidence = "third_party/7zip-lzma/PROVENANCE.md"
        NoteTokens = @(
            "public-domain", "third_party/7zip-lzma",
            "C/Alloc.c", "C/CpuArch.c", "C/LzFind.c",
            "C/LzFindOpt.c", "C/LzmaDec.c", "C/LzmaEnc.c",
            "C/Ppmd.h", "C/Ppmd7.h", "C/Ppmd7.c",
            "C/Ppmd7Enc.c", "C/Ppmd7Dec.c", "mode 27",
            "C/Ppmd8.h", "C/Ppmd8.c", "C/Ppmd8Enc.c",
            "C/Ppmd8Dec.c", "mode 28")
    },
    [pscustomobject]@{
        RelativePath = "compressors/lz/zstd"
        Provenance = "zstd.json"
        RootLicense = "BSD-3-Clause"
        CentralLicense = "BSD-3-Clause OR GPL-2.0"
        SelectedLicense = "BSD-3-Clause"
        ExpectedStatus = "ported"
        SourceCopyAllowed = $true
        PortEvidence = "third_party/zstd/PROVENANCE.md"
        NoteTokens = @("complete-codec", "third_party/zstd")
    },
    [pscustomobject]@{
        RelativePath = "compressors/lz/lz4"
        Provenance = "lz4.json"
        RootLicense = "BSD-2-Clause"
        CentralLicense = "mixed: BSD-2-Clause lib and GPL-2.0-or-later tools"
        SelectedLicense = "BSD-2-Clause"
        ExpectedStatus = "ported"
        SourceCopyAllowed = $true
        PortEvidence = "third_party/lz4/PROVENANCE.md"
        NoteTokens = @(
            "third_party/lz4", "src/r2/entropy/lz4_backend",
            "LZ4 HC level 12", "HZ02 mode 39",
            "r2-lz4-1k-20260821")
    },
    [pscustomobject]@{
        RelativePath = "compressors/bwt/libsais"
        Provenance = "libsais.json"
        RootLicense = "Apache-2.0"
        CentralLicense = "Apache-2.0"
        SelectedLicense = $null
        ExpectedStatus = "ported"
        SourceCopyAllowed = $true
        PortEvidence = "third_party/libsais/PROVENANCE.md"
        NoteTokens = @(
            "libsais_bwt()", "libsais_unbwt()", "third_party/libsais",
            "bwt_transform")
    },
    [pscustomobject]@{
        RelativePath = "compressors/meta/kanzi-cpp"
        Provenance = "kanzi-cpp.json"
        RootLicense = "Apache-2.0"
        CentralLicense = "Apache-2.0"
        SelectedLicense = $null
        ExpectedStatus = "ported"
        SourceCopyAllowed = $true
        PortEvidence = "third_party/kanzi/PROVENANCE.md"
        NoteTokens = @(
            "SBRT MTF", "BwtMtfZstd", "third_party/kanzi",
            "ANS entropy closure", "mode 40", "r2-kanzi-ans-1k-20260826")
    },
    [pscustomobject]@{
        RelativePath = "compressors/meta/zpaq"
        Provenance = "zpaq.json"
        RootLicense = "Unlicense/public domain with embedded MIT divsufsort"
        CentralLicense = "Unlicense/public domain with embedded MIT divsufsort"
        SelectedLicense = $null
        ExpectedStatus = "ported"
        SourceCopyAllowed = $true
        IdentityKind = "snapshot"
        PortEvidence = "third_party/zpaq/PROVENANCE.md"
        NoteTokens = @("ZPAQ", "HZQ1", "mode 29", "third_party/zpaq")
    },
    [pscustomobject]@{
        RelativePath = "compressors/ppm-ctw"
        Provenance = "ctw.json"
        RootLicense = "BSD-3-Clause"
        CentralLicense = "BSD-3-Clause"
        SelectedLicense = $null
        ExpectedStatus = "ported"
        SourceCopyAllowed = $true
        PortEvidence = "third_party/ctw/PROVENANCE.md"
        NoteTokens = @(
            "ctw.go", "ac/willems/willems.go", "HZC1", "mode 30",
            "src/r2/entropy/ctw_backend.cpp",
            "third_party/ctw/PROVENANCE.md")
    },
    [pscustomobject]@{
        RelativePath = "entropy/fse"
        Provenance = "fse.json"
        RootLicense = "BSD-2-Clause"
        CentralLicense = "BSD-2-Clause"
        SelectedLicense = $null
        ExpectedStatus = "ported"
        SourceCopyAllowed = $true
        PortEvidence = "third_party/fse/README.hybridzip.md"
        NoteTokens = @(
            "lib/ dependency closure", "third_party/fse",
            "HZFSE_", "HZHIST_", "HZHUF_")
    },
    [pscustomobject]@{
        RelativePath = "entropy/ryg-rans"
        Provenance = "ryg-rans.json"
        RootLicense = "CC0-1.0"
        CentralLicense = "CC0-1.0"
        SelectedLicense = $null
        ExpectedStatus = "ported"
        SourceCopyAllowed = $true
        PortEvidence = "third_party/rans/PROVENANCE.md"
        NoteTokens = @(
            "scalar rANS", "third_party/rans",
            "src/r2/entropy/rans_backend", "exact payload consumption")
    },
    [pscustomobject]@{
        RelativePath = "transforms/numeric/FastPFOR"
        Provenance = "fastpfor.json"
        RootLicense = "Apache-2.0"
        CentralLicense = "Apache-2.0"
        SelectedLicense = $null
        ExpectedStatus = "ported"
        SourceCopyAllowed = $true
        PortEvidence = "third_party/fastpfor/PROVENANCE.md"
        NoteTokens = @(
            "FastPFor<8>", "third_party/fastpfor",
            "src/r2/entropy/fastpfor_backend", "exact donor consumption")
    },
    [pscustomobject]@{
        RelativePath = "transforms/numeric/apache-arrow"
        Provenance = "apache-arrow.json"
        RootLicense = "Apache-2.0"
        CentralLicense = "Apache-2.0"
        SelectedLicense = $null
        ExpectedStatus = "ported"
        SourceCopyAllowed = $true
        PortEvidence = "third_party/apache-arrow/PROVENANCE.md"
        NoteTokens = @(
            "DELTA_BINARY_PACKED", "delta_binary_packed_transform", "mode 42",
            "third_party/apache-arrow/PROVENANCE.md",
            "r2-delta-binary-packed-zstd-1k-20260826")
    },
    [pscustomobject]@{
        RelativePath = "transforms/image/charls"
        Provenance = "charls.json"
        RootLicense = "BSD-3-Clause"
        CentralLicense = "BSD-3-Clause"
        SelectedLicense = $null
        ExpectedStatus = "ported"
        SourceCopyAllowed = $true
        IdentityKind = "archive"
        ArchiveRelativePath = "transforms/image/charls-c0bae6496fa5d787fbb4698debd1e5decb40cf3a.tar.gz"
        ArchiveUrl = "https://codeload.github.com/team-charls/charls/tar.gz/c0bae6496fa5d787fbb4698debd1e5decb40cf3a"
        ArchiveSha256 = "D61309DA04C64541EE545C167137B173C881FE1648426ED01A884A0CB5CE5615"
        PortEvidence = "third_party/charls/PROVENANCE.md"
        NoteTokens = @(
            "JPEG-LS", "third_party/charls",
            "src/r2/representation/jpegls_transform",
            "r2-jpegls-32k-20260820")
    },
    [pscustomobject]@{
        RelativePath = "transforms/audio/flac"
        Provenance = "flac.json"
        RootLicense = "BSD-3-Clause"
        CentralLicense = "BSD-3-Clause"
        SelectedLicense = $null
        ExpectedStatus = "ported"
        SourceCopyAllowed = $true
        PortEvidence = "third_party/flac/PROVENANCE.md"
        NoteTokens = @(
            "fixed/LPC", "third_party/flac",
            "src/r2/representation/flac_residual_transform",
            "r2-flac-residual-32k-20260820")
    },
    [pscustomobject]@{
        RelativePath = "transforms/audio/wavpack"
        Provenance = "wavpack.json"
        RootLicense = "BSD-3-Clause"
        CentralLicense = "BSD-3-Clause"
        SelectedLicense = $null
        ExpectedStatus = "ported"
        SourceCopyAllowed = $true
        PortEvidence = "third_party/wavpack/PROVENANCE.md"
        NoteTokens = @(
            "lossless PCM packer", "third_party/wavpack",
            "src/r2/entropy/wavpack_backend", "HZW1", "mode 38",
            "r2-wavpack-1k-20260821")
    },
    [pscustomobject]@{
        RelativePath = "neural/online/lstm-compress"
        Provenance = "lstm-compress.json"
        RootLicense = "GPL-3.0"
        CentralLicense = "GPL-3.0"
        SelectedLicense = $null
        ExpectedStatus = "ported"
        SourceCopyAllowed = $true
        PortEvidence = "third_party/lstm-compress/PROVENANCE.md"
        NoteTokens = @(
            "third_party/lstm-compress/port/lstm_compress_donor_port",
            "HLC1", "mode 23", "r2-lstm-compress-donor-port-1k-20260826")
    },
    [pscustomobject]@{
        RelativePath = "neural/online/jax-compress"
        Provenance = "jax-compress.json"
        RootLicense = "Unlicense"
        CentralLicense = "Unlicense"
        SelectedLicense = $null
        ExpectedStatus = "ported"
        SourceCopyAllowed = $true
        IdentityKind = "archive"
        ArchiveRelativePath = "neural/online/jax-compress-77adbc581eb0819a77e47c50ff6ed8ece338e60c.tar.gz"
        ArchiveUrl = "https://codeload.github.com/byronknoll/jax-compress/tar.gz/77adbc581eb0819a77e47c50ff6ed8ece338e60c"
        ArchiveSha256 = "32C12D882FBB9BF67D2F8465D8CA7777EC8918CC7C62CE43D75363500DBDA1A9"
        PortEvidence = "third_party/jax-compress/PROVENANCE.md"
        NoteTokens = @(
            "mode 26", "jax_compress_portable_backend",
            "32F26C0071529F7CDF0B68B41518709AE8D09B050586B1A9896A7C5039F73BE7")
    },
    [pscustomobject]@{
        RelativePath = "neural/shared/bgpt"
        Provenance = "bgpt.json"
        RootLicense = "MIT"
        CentralLicense = "MIT"
        SelectedLicense = $null
        ExpectedStatus = "ported"
        SourceCopyAllowed = $true
        PortEvidence = "third_party/bgpt-shared-prior/PROVENANCE.md"
        NoteTokens = @(
            "mode 25", "fixed-bootstrap",
            "third_party/bgpt-shared-prior/projection.json")
    },
    [pscustomobject]@{
        RelativePath = "neural/shared/lmic"
        Provenance = "lmic.json"
        RootLicense = "Apache-2.0"
        CentralLicense = "Apache-2.0"
        SelectedLicense = $null
        ExpectedStatus = "ported"
        SourceCopyAllowed = $true
        PortEvidence = "third_party/lmic/PROVENANCE.md"
        NoteTokens = @(
            "lmic-arithmetic-frozen-bgpt-v1", "HLM1", "mode 41",
            "src/r2/entropy/lmic_arithmetic_backend", "r2-lmic-arithmetic-1k-20260826")
    }
)

try {
    $WarehouseRoot = Get-CanonicalPath $WarehouseRoot
}
catch {
    Add-Failure "Invalid warehouse root '${WarehouseRoot}': $($_.Exception.Message)"
}

if ($script:Failures.Count -eq 0) {
    Assert-Check (Test-Path -LiteralPath $WarehouseRoot -PathType Container) (
        "Warehouse root does not exist: $WarehouseRoot")
}

if ($script:Failures.Count -eq 0) {
    $provenanceRoot = Join-Path $WarehouseRoot "provenance"
    $compressorsRoot = Join-Path $WarehouseRoot "compressors"
    $entropyRoot = Join-Path $WarehouseRoot "entropy"
    $transformsRoot = Join-Path $WarehouseRoot "transforms"
    Assert-Check (Test-Path -LiteralPath $provenanceRoot -PathType Container) (
        "Missing provenance directory: $provenanceRoot")
    Assert-Check (Test-Path -LiteralPath $compressorsRoot -PathType Container) (
        "Missing compressor inventory directory: $compressorsRoot")
    Assert-Check (Test-Path -LiteralPath $entropyRoot -PathType Container) (
        "Missing entropy inventory directory: $entropyRoot")
    Assert-Check (Test-Path -LiteralPath $transformsRoot -PathType Container) (
        "Missing transform inventory directory: $transformsRoot")
}

if ($script:Failures.Count -eq 0) {
    $expectedProvenance = @($expectedDonors | ForEach-Object { $_.Provenance })
    $actualProvenance = @(
        Get-ChildItem -LiteralPath $provenanceRoot -Filter "*.json" -File |
            Where-Object { $_.Name -ne "warehouse.json" } |
            ForEach-Object { $_.Name })
    Assert-ExactSet $expectedProvenance $actualProvenance "provenance record"

    $warehouseRecordPath = Join-Path $provenanceRoot "warehouse.json"
    $warehouseRecord = Read-JsonFile $warehouseRecordPath
    if ($null -ne $warehouseRecord) {
        $listedDonors = Get-PropertyValue $warehouseRecord "donors"
        Assert-ExactSet $expectedProvenance $listedDonors "warehouse donor list"
        Assert-Check (
            (Get-PropertyValue $warehouseRecord "schema_version") -eq 1
        ) "warehouse schema_version must be 1"
        $recordedWarehouseRoot = [string](
            Get-PropertyValue $warehouseRecord "warehouse_root")
        try {
            Assert-Check (
                (Get-CanonicalPath $recordedWarehouseRoot) -ieq $WarehouseRoot
            ) "warehouse_root does not identify the validated warehouse"
        }
        catch {
            Add-Failure "warehouse_root is invalid: $recordedWarehouseRoot"
        }
        $warehouseDate = [string](
            Get-PropertyValue $warehouseRecord "download_date")
        Assert-Date $warehouseDate "warehouse download_date"
        $architectureHash = Get-PropertyValue $warehouseRecord "architecture_sha256"
        Assert-Check (
            $architectureHash -ceq
            "98E4C59E95E1AD5C98E94D431F674F555F6A50260BDC1FBBB95CC17C7D5F2FB3"
        ) "warehouse architecture SHA-256 is missing or changed"
    }

    $expectedManifestPaths = @(
        $expectedDonors | ForEach-Object {
            Get-CanonicalPath (Join-Path (
                Join-Path $WarehouseRoot $_.RelativePath) "DONOR.json")
        })
    $explicitTransformManifestPaths = @(
        $expectedDonors |
            Where-Object { $_.RelativePath.StartsWith("transforms/") } |
            ForEach-Object {
                Get-CanonicalPath (Join-Path (
                    Join-Path $WarehouseRoot $_.RelativePath) "DONOR.json")
            })
    $inventoryRoots = @(
        (Join-Path $WarehouseRoot "compressors"),
        (Join-Path $WarehouseRoot "entropy"),
        (Join-Path $WarehouseRoot "neural"))
    $actualManifestPaths = @(
        Get-ChildItem -LiteralPath $inventoryRoots -Filter "DONOR.json" -File -Recurse -Force |
            ForEach-Object { Get-CanonicalPath $_.FullName }) +
        $explicitTransformManifestPaths
    Assert-ExactSet $expectedManifestPaths $actualManifestPaths "donor manifest"

    $gitExpectedDonors = @(
    $expectedDonors | Where-Object {
        (Get-PropertyValue $_ "IdentityKind") -cne "archive" -and
        (Get-PropertyValue $_ "IdentityKind") -cne "snapshot"
    })
    $gitRoots = @(
        Get-ChildItem -LiteralPath $inventoryRoots -Directory -Filter ".git" -Recurse -Force |
            ForEach-Object { Get-CanonicalPath $_.Parent.FullName }) +
        @($gitExpectedDonors |
            Where-Object { $_.RelativePath.StartsWith("transforms/") } |
            ForEach-Object {
                Get-CanonicalPath (Join-Path $WarehouseRoot $_.RelativePath)
            })
    $expectedGitRoots = @(
        $gitExpectedDonors | ForEach-Object {
            Get-CanonicalPath (Join-Path $WarehouseRoot $_.RelativePath)
        })
    Assert-ExactSet $expectedGitRoots $gitRoots "donor Git root"

    $gitCommand = Get-Command git -ErrorAction SilentlyContinue
    Assert-Check ($null -ne $gitCommand) "git is required to validate donor revisions"

    $allowedStatuses = @("downloaded", "studied", "ported", "rejected")
    foreach ($expected in $expectedDonors) {
        $rootPath = Get-CanonicalPath (
            Join-Path $WarehouseRoot $expected.RelativePath)
        $manifestPath = Join-Path $rootPath "DONOR.json"
        $centralPath = Join-Path $provenanceRoot $expected.Provenance
        $manifest = Read-JsonFile $manifestPath
        $central = Read-JsonFile $centralPath
        if ($null -eq $manifest -or $null -eq $central) {
            continue
        }

        Assert-ManifestShape $manifest $manifestPath
        Assert-ManifestShape $central $centralPath

        $name = [string](Get-PropertyValue $manifest "name")
        $url = [string](Get-PropertyValue $manifest "url")
        $revision = [string](Get-PropertyValue $manifest "revision")
        $license = [string](Get-PropertyValue $manifest "license")
        $downloadDate = [string](Get-PropertyValue $manifest "download_date")
        $status = [string](Get-PropertyValue $manifest "status")

        Assert-Url $url "$name URL"
        Assert-Date $downloadDate "$name download_date"
        $identityKind = [string](Get-PropertyValue $expected "IdentityKind")
        if ($identityKind -ceq "snapshot") {
            Assert-Check ($revision -cmatch "^[A-Za-z0-9._-]+$") (
                "$name release snapshot revision is invalid: $revision")
        }
        else {
            Assert-Check ($revision -cmatch "^[0-9a-f]{40}$") (
                "$name revision must be a lowercase 40-character Git SHA-1")
        }
        Assert-Check ($allowedStatuses -ccontains $status) (
            "$name status is invalid: $status")
        Assert-Check ($status -ceq $expected.ExpectedStatus) (
            "$name status is '$status'; expected '$($expected.ExpectedStatus)'")
        Assert-Check ($license -ceq $expected.RootLicense) (
            "$name root license is '$license'; expected '$($expected.RootLicense)'")

        foreach ($field in @(
            "name", "url", "revision", "language", "download_date", "status")) {
            $rootValue = [string](Get-PropertyValue $manifest $field)
            $centralValue = [string](Get-PropertyValue $central $field)
            Assert-Check ($rootValue -ceq $centralValue) (
                "$name field '$field' disagrees with $($expected.Provenance)")
        }

        $rootModulesValue = Get-PropertyValue $manifest "candidate_modules"
        $centralModulesValue = Get-PropertyValue $central "candidate_modules"
        $rootModules = if ($rootModulesValue -is [System.Array]) {
            @($rootModulesValue)
        }
        else {
            @()
        }
        $centralModules = if ($centralModulesValue -is [System.Array]) {
            @($centralModulesValue)
        }
        else {
            @()
        }
        $moduleMatch = $rootModules.Count -eq $centralModules.Count
        if ($moduleMatch) {
            for ($index = 0; $index -lt $rootModules.Count; ++$index) {
                if ([string]$rootModules[$index] -cne [string]$centralModules[$index]) {
                    $moduleMatch = $false
                    break
                }
            }
        }
        Assert-Check $moduleMatch (
            "$name candidate_modules disagree with $($expected.Provenance)")

        $centralLicense = [string](Get-PropertyValue $central "license")
        Assert-Check ($centralLicense -ceq $expected.CentralLicense) (
            "$name central license is '$centralLicense'; expected '$($expected.CentralLicense)'")
        if ($null -ne $expected.SelectedLicense) {
            $selected = [string](Get-PropertyValue $central "selected_license")
            Assert-Check ($selected -ceq $expected.SelectedLicense) (
                "$name selected_license is '$selected'; expected '$($expected.SelectedLicense)'")
            Assert-Check ($license -ceq $selected) (
                "$name root license must equal the selected central license")
        }
        else {
            Assert-Check ($license -ceq $centralLicense) (
                "$name root and central licenses disagree")
        }

        $sourceCopyAllowed = Get-PropertyValue $central "source_copy_allowed"
        Assert-Check (
            $sourceCopyAllowed -is [bool] -and
            $sourceCopyAllowed -eq $expected.SourceCopyAllowed
        ) "$name source_copy_allowed does not match its repository boundary"

        if ($null -ne $expected.PortEvidence) {
            $portEvidencePath = Get-SafeChildPath $projectRoot $expected.PortEvidence "$name port evidence"
            if ($null -ne $portEvidencePath) {
                Assert-Check (
                    Test-Path -LiteralPath $portEvidencePath -PathType Leaf
                ) "$name port evidence is missing: $portEvidencePath"
            }
            $notes = [string](Get-PropertyValue $central "notes")
            foreach ($token in @($expected.NoteTokens)) {
                Assert-Check ($notes.Contains([string]$token)) (
                    "$name provenance notes do not identify port evidence '$token'")
            }
        }

        $centralLocalPath = [string](Get-PropertyValue $central "local_path")
        try {
            $canonicalCentralPath = Get-CanonicalPath $centralLocalPath
            Assert-Check ($canonicalCentralPath -ieq $rootPath) (
                "$name central local_path does not identify its donor root")
        }
        catch {
            Add-Failure "$name has invalid central local_path: $centralLocalPath"
        }

        $isArchiveIdentity = (Get-PropertyValue $expected "IdentityKind") -ceq "archive"
        if ($isArchiveIdentity) {
            foreach ($field in @("source_archive_url", "source_archive_sha256")) {
                $rootValue = [string](Get-PropertyValue $manifest $field)
                $centralValue = [string](Get-PropertyValue $central $field)
                Assert-Check ($rootValue -ceq $centralValue) (
                    "$name field '$field' disagrees with $($expected.Provenance)")
            }
            $archiveUrl = [string](Get-PropertyValue $central "source_archive_url")
            $archiveHash = [string](Get-PropertyValue $central "source_archive_sha256")
            Assert-Check ($archiveUrl -ceq $expected.ArchiveUrl) (
                "$name source archive URL differs from the pinned codeload URL")
            Assert-Check ($archiveHash -ceq $expected.ArchiveSha256) (
                "$name source archive SHA-256 differs from the pinned value")
            $archivePath = Get-SafeChildPath $WarehouseRoot (
                [string](Get-PropertyValue $central "source_archive_relative_path")) "$name source archive"
            if ($null -ne $archivePath) {
                Assert-Check (Test-Path -LiteralPath $archivePath -PathType Leaf) (
                    "$name source archive is missing: $archivePath")
                if (Test-Path -LiteralPath $archivePath -PathType Leaf) {
                    $actualArchiveHash = (
                        Get-FileHash -LiteralPath $archivePath -Algorithm SHA256).Hash
                    Assert-Check ($actualArchiveHash -ceq $archiveHash) (
                        "$name source archive SHA-256 is $actualArchiveHash; expected $archiveHash")
                }
            }
        }
        elseif ((Get-PropertyValue $expected "IdentityKind") -ceq "snapshot") {
            $snapshotFiles = @(
                [pscustomobject]@{ RelativePath = "libzpaq.cpp"; Hash = "151EB6BD83CB6C6F5261D64B1DB49358710F844EE1A2AA4B9CB63E17319DF122" },
                [pscustomobject]@{ RelativePath = "libzpaq.h"; Hash = "08BD9CE17CE018468E35721E2C6A8BD13C0C5E397CE4E9C90C52AEC389662F79" },
                [pscustomobject]@{ RelativePath = "COPYING"; Hash = "927B5FEDA84F7A7F2063998B124829182967F54B954DB2C3569E8BD07958BF07" }
            )
            foreach ($snapshot in $snapshotFiles) {
                $snapshotPath = Get-SafeChildPath $rootPath $snapshot.RelativePath "$name snapshot file"
                if ($null -ne $snapshotPath) {
                    Assert-Check (Test-Path -LiteralPath $snapshotPath -PathType Leaf) (
                        "$name snapshot file is missing: $snapshotPath")
                    if (Test-Path -LiteralPath $snapshotPath -PathType Leaf) {
                        $actualSnapshotHash = (Get-FileHash -LiteralPath $snapshotPath -Algorithm SHA256).Hash
                        Assert-Check ($actualSnapshotHash -ceq $snapshot.Hash) (
                            "$name snapshot hash for $($snapshot.RelativePath) is $actualSnapshotHash; expected $($snapshot.Hash)")
                    }
                }
            }
        }
        elseif ($null -ne $gitCommand) {
            $headOutput = @(& git -c "safe.directory=$rootPath" -C $rootPath rev-parse HEAD 2>&1)
            $headExit = $LASTEXITCODE
            Assert-Check ($headExit -eq 0) "$name Git HEAD could not be read"
            if ($headExit -eq 0) {
                $head = [string]$headOutput[0]
                Assert-Check ($head.Trim() -ceq $revision) (
                    "$name Git HEAD is $($head.Trim()); expected $revision")
            }

            $originOutput = @(& git -c "safe.directory=$rootPath" -C $rootPath remote get-url origin 2>&1)
            $originExit = $LASTEXITCODE
            Assert-Check ($originExit -eq 0) "$name Git origin could not be read"
            if ($originExit -eq 0) {
                $origin = Normalize-GitUrl ([string]$originOutput[0])
                Assert-Check ($origin -ieq (Normalize-GitUrl $url)) (
                    "$name Git origin '$origin' does not match '$url'")
            }
        }

        $evidenceRelative = [string](
            Get-PropertyValue $central "license_evidence")
        $expectedEvidenceHash = [string](
            Get-PropertyValue $central "license_evidence_sha256")
        Assert-Check ($expectedEvidenceHash -cmatch "^[0-9A-F]{64}$") (
            "$name license_evidence_sha256 must be 64 uppercase hex characters")
        $evidencePath = Get-SafeChildPath $rootPath $evidenceRelative "$name license evidence"
        if ($null -ne $evidencePath) {
            Assert-Check (Test-Path -LiteralPath $evidencePath -PathType Leaf) (
                "$name license evidence is missing: $evidencePath")
            if (Test-Path -LiteralPath $evidencePath -PathType Leaf) {
                $actualEvidenceHash = (
                    Get-FileHash -LiteralPath $evidencePath -Algorithm SHA256).Hash
                Assert-Check ($actualEvidenceHash -ceq $expectedEvidenceHash) (
                    "$name license evidence SHA-256 is $actualEvidenceHash; expected $expectedEvidenceHash")

                if ($expected.Provenance -ceq "cmix.json") {
                    $copying = [System.IO.File]::ReadAllText($evidencePath)
                    Assert-Check (
                        $copying -match
                        "GNU GENERAL PUBLIC LICENSE\s+Version 3, 29 June 2007"
                    ) "cmix COPYING does not identify GNU GPL version 3"
                }
            }
        }
    }
}

$lz4SourceRoot = Join-Path $WarehouseRoot "compressors/lz/lz4/lib"
$lz4VendoredRoot = Join-Path $projectRoot "third_party/lz4"
$lz4Closure = @(
    [pscustomobject]@{ Path = "LICENSE"; Hash = "8B58C446121A109CCF32EDC094BBA3010A3D85E4EE3702950DB55E4D3E87736C" },
    [pscustomobject]@{ Path = "lz4.c"; Hash = "9396F7DE527BC8435DE9C7569FB7998E56545A84B4F3C2D808C0235C01774539" },
    [pscustomobject]@{ Path = "lz4.h"; Hash = "26B82EFC53D1570F3B54EEF02E9C4764C1AD374FF03CAC04E2CED5EA4D4C552F" },
    [pscustomobject]@{ Path = "lz4hc.c"; Hash = "126CAFAFDB91767E6E55238298A910903851B35B2CEE27CE80AE2280469EE232" },
    [pscustomobject]@{ Path = "lz4hc.h"; Hash = "E43824E8A9BA16F54100C4CCBCCFA5782A858CA9AB83C48AAC303FEA3E76E21E" }
)
foreach ($entry in $lz4Closure) {
    $sourcePath = Join-Path $lz4SourceRoot $entry.Path
    $vendoredPath = Join-Path $lz4VendoredRoot $entry.Path
    Assert-Check (Test-Path -LiteralPath $sourcePath -PathType Leaf) (
        "LZ4 donor file is missing: $($entry.Path)")
    Assert-Check (Test-Path -LiteralPath $vendoredPath -PathType Leaf) (
        "LZ4 vendored file is missing: $($entry.Path)")
    if ((Test-Path -LiteralPath $sourcePath -PathType Leaf) -and
        (Test-Path -LiteralPath $vendoredPath -PathType Leaf)) {
        $sourceHash = (Get-FileHash -LiteralPath $sourcePath -Algorithm SHA256).Hash
        $vendoredHash = (Get-FileHash -LiteralPath $vendoredPath -Algorithm SHA256).Hash
        Assert-Check ($sourceHash -ceq $entry.Hash) (
            "LZ4 donor hash changed: $($entry.Path)")
        Assert-Check ($vendoredHash -ceq $entry.Hash) (
            "LZ4 vendored hash changed: $($entry.Path)")
    }
}

$paq8pxGenericFiles = @(
    "file/File.cpp",
    "file/File.hpp",
    "file/FileDisk.cpp",
    "file/FileDisk.hpp",
    "file/fileUtils.hpp",
    "file/OpenFromMyFolder.cpp",
    "file/OpenFromMyFolder.hpp",
    "HashElementForMatchPositions.hpp",
    "LargeIndirectContext.hpp",
    "LargeStationaryMap.cpp",
    "LargeStationaryMap.hpp",
    "model/CharGroupModel.cpp",
    "model/CharGroupModel.hpp",
    "model/ChartModel.cpp",
    "model/ChartModel.hpp",
    "model/DmcForest.cpp",
    "model/DmcForest.hpp",
    "model/DmcModel.cpp",
    "model/DmcModel.hpp",
    "model/DmcNode.cpp",
    "model/DmcNode.hpp",
    "model/ExeModel.cpp",
    "model/ExeModel.hpp",
    "model/IndirectModel.cpp",
    "model/IndirectModel.hpp",
    "model/MatchInfo.cpp",
    "model/MatchInfo.hpp",
    "model/MatchModel.cpp",
    "model/MatchModel.hpp",
    "model/NestModel.cpp",
    "model/NestModel.hpp",
    "model/NormalModel.cpp",
    "model/NormalModel.hpp",
    "model/SparseBitModel.cpp",
    "model/SparseBitModel.hpp",
    "model/SparseMatchModel.cpp",
    "model/SparseMatchModel.hpp",
    "model/SparseModel.cpp",
    "model/SparseModel.hpp",
    "model/WordModel.cpp",
    "model/WordModel.hpp",
    "model/WordModelInfo.cpp",
    "model/WordModelInfo.hpp",
    "model/XMLModel.cpp",
    "model/XMLModel.hpp",
    "MTFList.cpp",
    "MTFList.hpp",
    "StateMapPair.cpp",
    "StateMapPair.hpp",
    "String.cpp",
    "String.hpp",
    "text/Cache.hpp",
    "text/English.cpp",
    "text/English.hpp",
    "text/EnglishStemmer.cpp",
    "text/EnglishStemmer.hpp",
    "text/Entry.hpp",
    "text/French.cpp",
    "text/French.hpp",
    "text/FrenchStemmer.cpp",
    "text/FrenchStemmer.hpp",
    "text/German.cpp",
    "text/German.hpp",
    "text/GermanStemmer.cpp",
    "text/GermanStemmer.hpp",
    "text/Language.hpp",
    "text/Paragraph.hpp",
    "text/Segment.hpp",
    "text/Sentence.hpp",
    "text/Stemmer.cpp",
    "text/Stemmer.hpp",
    "text/TextModel.cpp",
    "text/TextModel.hpp",
    "text/Word.cpp",
    "text/Word.hpp",
    "text/WordEmbeddingDictionary.cpp",
    "text/WordEmbeddingDictionary.hpp"
)
Assert-Check ($paq8pxGenericFiles.Count -eq 77) (
    "PAQ8px Generic closure allowlist must contain 77 files")
$paq8pxSourceRoot = Join-Path $WarehouseRoot (
    "compressors/context-mixing/paq8px/src")
$paq8pxVendoredRoot = Join-Path $projectRoot (
    "third_party/paq8px/record_model")
foreach ($relativePath in $paq8pxGenericFiles) {
    $sourcePath = Join-Path $paq8pxSourceRoot $relativePath
    $vendoredPath = Join-Path $paq8pxVendoredRoot $relativePath
    $sourceExists = Test-Path -LiteralPath $sourcePath -PathType Leaf
    $vendoredExists = Test-Path -LiteralPath $vendoredPath -PathType Leaf
    Assert-Check $sourceExists (
        "PAQ8px Generic upstream file is missing: $relativePath")
    Assert-Check $vendoredExists (
        "PAQ8px Generic vendored file is missing: $relativePath")
    if ($sourceExists -and $vendoredExists) {
        $sourceHash = (Get-FileHash -LiteralPath $sourcePath -Algorithm SHA256).Hash
        $vendoredHash = (Get-FileHash -LiteralPath $vendoredPath -Algorithm SHA256).Hash
        Assert-Check ($vendoredHash -ceq $sourceHash) (
            "PAQ8px Generic vendored hash differs from upstream: $relativePath")
    }
}

$paq8pxAdaptedHashes = @{
    "APMPost.cpp" = [pscustomobject]@{
        Source = "8F8186C50A15597A9D8ABBF112B67B0B98B4806868C9AE1447DEBC35BCD75697"
        Vendored = "0E91D4129434BA0EEB0F291CAB3F412639B5C4A7FCB9B4D488F5A03C94EFC940"
    }
    "LMS.cpp" = [pscustomobject]@{
        Source = "FCEBE6FAE8DBFE6D3BF290F88A6FA2BC97A6BBC79FFAE8416F079ADC95D37AB5"
        Vendored = "47DFE777104470736C77E7C44CBE27B35F7FC79828907E0638A22D74335E0F8F"
    }
    "MixerFactory.cpp" = [pscustomobject]@{
        Source = "9A834CA548CF8725CAAC68AA0DB4BCA3F425D0AF46A6AB513927D9C02C01CDD2"
        Vendored = "9BF8A86EF0C82D1419806363C926B6E77ADEBA3B5ED03A70CB7A03E28D3850EA"
    }
    "MixerFactory.hpp" = [pscustomobject]@{
        Source = "F889B043E927E30FBE353824B40AA7ED58B3961EAF6297B4B1C49D833EA1F31E"
        Vendored = "385C8A14E565FE9C58AA8D89346E7A741DE532C2A2FD00EAB8CF8795AD7BE526"
    }
    "model/SimilarityEmaFunctionsFactory.cpp" = [pscustomobject]@{
        Source = "62D91BC94A4FE54D405EF8DF480FAA8FAD8595199CE1623F17D66131015ABE16"
        Vendored = "1CE778890DD827FB725207980881FE6446D26D08239A6C9516A2EAD58B2223FE"
    }
    "model/SimilarityEmaFunctionsFactory.hpp" = [pscustomobject]@{
        Source = "CAC493F507E2656A9220750AA9B058537D8C47E1C70FF0578C935CD1793814EF"
        Vendored = "9F17F82A0A50B721C62A14319D48F4B3F0C76EA49CBF43C8051A7340FFC438E2"
    }
    "OLS_factory.cpp" = [pscustomobject]@{
        Source = "9C622D3EB47CAD7478B5F9D8A6740C0FD8EC027C57ABEE001F68856C17CFB492"
        Vendored = "9D7577FCBE76F4C50F51B161ECF873E73D19AF16DCED2CE76B0CA6757D0CFC38"
    }
    "Shared.cpp" = [pscustomobject]@{
        Source = "324D35D3FBA1C5796D18197FDE93AC0DEE864FE10269B649B67DAA21794C1EB7"
        Vendored = "ECE706D8AB4B764CD33C50ECCFFF9CD6540C694E4DD16B485FE112E465E91060"
    }
}

$paq8pxSourceFiles = @(
    Get-ChildItem -LiteralPath $paq8pxSourceRoot -Recurse -File |
        ForEach-Object {
            Get-RelativeChildPath $paq8pxSourceRoot $_.FullName (
                "PAQ8px source file")
        })
$paq8pxVendoredFiles = @(
    Get-ChildItem -LiteralPath $paq8pxVendoredRoot -Recurse -File |
        ForEach-Object {
            Get-RelativeChildPath $paq8pxVendoredRoot $_.FullName (
                "PAQ8px vendored file")
        })
Assert-Check ($paq8pxSourceFiles.Count -eq 310) (
    "PAQ8px pinned source tree must contain 310 files")
Assert-Check ($paq8pxVendoredFiles.Count -eq 310) (
    "PAQ8px vendored source snapshot must contain 310 files")
Assert-ExactSet $paq8pxSourceFiles $paq8pxVendoredFiles (
    "PAQ8px complete source snapshot")

foreach ($relativePath in $paq8pxSourceFiles) {
    $sourcePath = Join-Path $paq8pxSourceRoot $relativePath
    $vendoredPath = Join-Path $paq8pxVendoredRoot $relativePath
    $sourceHash = (Get-FileHash -LiteralPath $sourcePath -Algorithm SHA256).Hash
    $vendoredHash = (Get-FileHash -LiteralPath $vendoredPath -Algorithm SHA256).Hash
    if ($paq8pxAdaptedHashes.ContainsKey($relativePath)) {
        $expected = $paq8pxAdaptedHashes[$relativePath]
        Assert-Check ($sourceHash -ceq $expected.Source) (
            "PAQ8px adapted upstream hash changed: $relativePath")
        Assert-Check ($vendoredHash -ceq $expected.Vendored) (
            "PAQ8px adapted vendored hash changed: $relativePath")
    }
    else {
        Assert-Check ($vendoredHash -ceq $sourceHash) (
            "PAQ8px unadapted source differs from upstream: $relativePath")
    }
}
Assert-Check ($paq8pxAdaptedHashes.Count -eq 8) (
    "PAQ8px adapted source allowlist must contain eight files")

$paq8pxFilterSourceRoot = Join-Path $paq8pxSourceRoot "filter"
$paq8pxFilterVendoredRoot = Join-Path $projectRoot (
    "third_party/paq8px/block_detection/upstream/filter")
$paq8pxFilterSourceFiles = @(
    Get-ChildItem -LiteralPath $paq8pxFilterSourceRoot -Recurse -File |
        ForEach-Object {
            Get-RelativeChildPath $paq8pxFilterSourceRoot $_.FullName (
                "PAQ8px filter source file")
        })
$paq8pxFilterVendoredFiles = @(
    Get-ChildItem -LiteralPath $paq8pxFilterVendoredRoot -Recurse -File |
        ForEach-Object {
            Get-RelativeChildPath $paq8pxFilterVendoredRoot $_.FullName (
                "PAQ8px retained filter file")
        })
Assert-Check ($paq8pxFilterSourceFiles.Count -eq 25) (
    "PAQ8px pinned filter tree must contain 25 files")
Assert-ExactSet $paq8pxFilterSourceFiles $paq8pxFilterVendoredFiles (
    "PAQ8px retained filter tree")
foreach ($relativePath in $paq8pxFilterSourceFiles) {
    $sourcePath = Join-Path $paq8pxFilterSourceRoot $relativePath
    $vendoredPath = Join-Path $paq8pxFilterVendoredRoot $relativePath
    Assert-Check (
        (Get-FileHash -LiteralPath $sourcePath -Algorithm SHA256).Hash -ceq
        (Get-FileHash -LiteralPath $vendoredPath -Algorithm SHA256).Hash
    ) "PAQ8px retained filter differs from upstream: $relativePath"
}

$paq8pxFiltersSource = Join-Path $paq8pxFilterSourceRoot "Filters.hpp"
$paq8pxFiltersAdapter = Join-Path $projectRoot (
    "third_party/paq8px/block_detection/FiltersDetection.hpp")
Assert-Check (
    (Get-FileHash -LiteralPath $paq8pxFiltersSource -Algorithm SHA256).Hash -ceq
    "B4CD6613F8B3DBDE2EC8FA224F007A0BC27ED9B1ABE8070B3130AF25622EAF1F"
) "PAQ8px detector upstream Filters.hpp hash changed"
Assert-Check (
    (Get-FileHash -LiteralPath $paq8pxFiltersAdapter -Algorithm SHA256).Hash -ceq
    "91D8F5A17E7B7603A08B76BAFD28AB3CB7116271BCFB94EE9E9D50BA440D9791"
) "PAQ8px detector adapter hash changed"
Assert-Check (Test-Path -LiteralPath (
    Join-Path $projectRoot "third_party/paq8px/DETECTED_SSE_AUDIT.md") -PathType Leaf) (
    "PAQ8px detected-SSE audit is missing")

if ($script:Failures.Count -gt 0) {
    Write-Host (
        "FAIL: R2 donor validation found {0} problem(s) across {1} checks." -f
        $script:Failures.Count, $script:CheckCount) -ForegroundColor Red
    foreach ($failure in $script:Failures) {
        Write-Host "  - $failure" -ForegroundColor Red
    }
    exit 1
}

Write-Host (
        "PASS: validated 21 donor manifests, 18 port evidence records, " +
        "17 Git revisions/origins, 1 release snapshot, and 3 source archives, plus 21 license evidence hashes across " +
    "$script:CheckCount checks.") -ForegroundColor Green
exit 0
