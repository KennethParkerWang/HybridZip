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
            "PPMD", "Online LSTM", "Match",
            "third_party/cmix/MODIFICATIONS.md")
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
            "C/LzFindOpt.c", "C/LzmaDec.c", "C/LzmaEnc.c")
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
        RelativePath = "compressors/meta/kanzi-cpp"
        Provenance = "kanzi-cpp.json"
        RootLicense = "Apache-2.0"
        CentralLicense = "Apache-2.0"
        SelectedLicense = $null
        ExpectedStatus = "downloaded"
        SourceCopyAllowed = $true
        PortEvidence = $null
        NoteTokens = @()
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
        ExpectedStatus = "downloaded"
        SourceCopyAllowed = $true
        PortEvidence = $null
        NoteTokens = @()
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
    Assert-Check (Test-Path -LiteralPath $provenanceRoot -PathType Container) (
        "Missing provenance directory: $provenanceRoot")
    Assert-Check (Test-Path -LiteralPath $compressorsRoot -PathType Container) (
        "Missing compressor inventory directory: $compressorsRoot")
    Assert-Check (Test-Path -LiteralPath $entropyRoot -PathType Container) (
        "Missing entropy inventory directory: $entropyRoot")
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
    $inventoryRoots = @(
        (Join-Path $WarehouseRoot "compressors"),
        (Join-Path $WarehouseRoot "entropy"))
    $actualManifestPaths = @(
        Get-ChildItem -LiteralPath $inventoryRoots -Filter "DONOR.json" -File -Recurse -Force |
            ForEach-Object { Get-CanonicalPath $_.FullName })
    Assert-ExactSet $expectedManifestPaths $actualManifestPaths "donor manifest"

    $gitRoots = @(
        Get-ChildItem -LiteralPath $inventoryRoots -Directory -Filter ".git" -Recurse -Force |
            ForEach-Object { Get-CanonicalPath $_.Parent.FullName })
    $expectedGitRoots = @(
        $expectedDonors | ForEach-Object {
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
        Assert-Check ($revision -cmatch "^[0-9a-f]{40}$") (
            "$name revision must be a lowercase 40-character Git SHA-1")
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

        if ($null -ne $gitCommand) {
            $headOutput = @(& git -C $rootPath rev-parse HEAD 2>&1)
            $headExit = $LASTEXITCODE
            Assert-Check ($headExit -eq 0) "$name Git HEAD could not be read"
            if ($headExit -eq 0) {
                $head = [string]$headOutput[0]
                Assert-Check ($head.Trim() -ceq $revision) (
                    "$name Git HEAD is $($head.Trim()); expected $revision")
            }

            $originOutput = @(& git -C $rootPath remote get-url origin 2>&1)
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
    "PASS: validated 7 donor manifests, 5 port evidence records, " +
    "7 Git revisions/origins, and 7 license evidence hashes across " +
    "$script:CheckCount checks.") -ForegroundColor Green
exit 0
