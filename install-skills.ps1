param(
    [ValidateSet("opencode", "opencode-global", "reasonix", "claude", "codex", "gemini", "cursor", "all")]
    [string]$Target = "all",
    [string]$ProjectRoot = $PSScriptRoot
)

$ErrorActionPreference = "Stop"

# Source: canonical skill folders live next to this script.
$SrcDir = Join-Path $PSScriptRoot "cw32-framework"
$SkillNames = @("cw32-framework", "cw32l010", "cw32l011", "cw32l012")

# Resolve per-agent destination directories.
# Format: name -> @{ Type = "project"|"global"; Path = script-block returning dir }
function Get-Dest($Agent) {
    $homeDir = $HOME
    switch ($Agent) {
        "opencode"        { return (Join-Path $ProjectRoot ".opencode\skills") }
        "opencode-global" { return (Join-Path $homeDir ".config\opencode\skills") }
        "reasonix"        { return (Join-Path $homeDir ".reasonix\skills") }
        "claude"          { return (Join-Path $homeDir ".claude\skills") }
        "codex"           { return (Join-Path $homeDir ".codex\skills") }
        "gemini"          { return (Join-Path $homeDir ".gemini\skills") }
        "cursor"          { return (Join-Path $ProjectRoot ".cursor\skills") }
        default           { throw "Unknown agent: $Agent" }
    }
}

function Install-One($Agent) {
    $dest = Get-Dest $Agent
    Write-Host "Installing CW32L0xx skills -> $dest"
    New-Item -ItemType Directory -Path $dest -Force | Out-Null
    foreach ($name in $SkillNames) {
        $src = Join-Path $PSScriptRoot $name
        if (-not (Test-Path -LiteralPath $src)) {
            Write-Warning "Missing source: $src (skipped)"
            continue
        }
        $dst = Join-Path $dest $name
        if (Test-Path -LiteralPath $dst) {
            Write-Host "  $name : exists, replacing"
            Remove-Item -LiteralPath $dst -Recurse -Force
        }
        Copy-Item -LiteralPath $src -Destination $dst -Recurse
        Write-Host "  $name : OK"
    }
}

$agents = if ($Target -eq "all") {
    @("opencode", "opencode-global", "reasonix", "claude", "codex", "gemini", "cursor")
} else {
    @($Target)
}

foreach ($a in $agents) {
    Install-One $a
}

Write-Host "Done. Restart your agent (skills are read at startup)."
Write-Host "Usage:  powershell -ExecutionPolicy Bypass -File install-skills.ps1 -Target reasonix"
