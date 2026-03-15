<#
.SYNOPSIS
    Stage all changes, commit with a message, and push to the configured remote.

.DESCRIPTION
    A helper script for Windows/PowerShell users that automates the git workflow.
    You can override the commit message, branch, and remote name via named parameters.
    The script fails if it cannot discover the repo root or the given remote is missing.

.PARAMETER Message
    Commit message to use, defaults to "Auto-commit from push.ps1".

.PARAMETER Branch
    Branch to push, defaults to "main".

.PARAMETER Remote
    Remote name to push to, defaults to "origin".

.PARAMETER DryRun
    Show the commands that would run without making changes.
#>
param(
    [string]$Message = "Auto-commit from push.ps1",
    [string]$Branch = "main",
    [string]$Remote = "origin",
    [switch]$DryRun
)

$repoRoot = git rev-parse --show-toplevel 2>$null
if ($LASTEXITCODE -ne 0 -or [string]::IsNullOrWhiteSpace($repoRoot)) {
    Write-Error "Cannot find the git repository root. Run the script inside a git working tree."
    exit 1
}
$repoRoot = $repoRoot.Trim()

Push-Location $repoRoot
try {
    $remoteUrl = git remote get-url $Remote 2>$null
    if ($LASTEXITCODE -ne 0) {
        Write-Error "Remote '$Remote' is not configured. Add it with `git remote add $Remote <url>`."
        exit 1
    }

    $steps = @(
        @{ name = "status"; action = { git status -sb }; text = "git status -sb" },
        @{ name = "add"; action = { git add -A }; text = "git add -A" },
        @{ name = "commit"; action = { git commit -m $Message }; text = "git commit -m \"$Message\"" },
        @{ name = "push"; action = { git push $Remote $Branch }; text = "git push $Remote $Branch" }
    )

    if ($DryRun) {
        Write-Host "Dry run: the script would run the following git commands:"
        foreach ($step in $steps) {
            Write-Host "- $($step.text)"
        }
    }
    else {
        foreach ($step in $steps) {
            & $step.action
        }
    }
}
finally {
    Pop-Location
}
