#!/usr/bin/env bash
# Push helper script for UNIX-like systems.
# Usage:
#   ./scripts/push.sh [-m "commit message"] [-b branch] [-r remote] [--dry-run]

set -euo pipefail

message="Auto-commit from push.sh"
branch="main"
remote="origin"
dry_run=0

while [[ $# -gt 0 ]]; do
  case $1 in
    -m|--message)
      message="$2"
      shift 2
      ;;
    -b|--branch)
      branch="$2"
      shift 2
      ;;
    -r|--remote)
      remote="$2"
      shift 2
      ;;
    --dry-run)
      dry_run=1
      shift
      ;;
    *)
      echo "Unknown argument: $1"
      exit 1
      ;;
  esac
done

repo_root=$(git rev-parse --show-toplevel 2>/dev/null || true)
if [[ -z "$repo_root" ]]; then
  echo "Not inside a git repository."
  exit 1
fi

cd "$repo_root"

if ! git remote get-url "$remote" >/dev/null 2>&1; then
  echo "Remote '$remote' is not configured; add it with:"
  echo "  git remote add $remote <url>"
  exit 1
fi

commands=(
  "git status -sb"
  "git add -A"
  "git commit -m \"$message\""
  "git push $remote $branch"
)

if [[ "$dry_run" -eq 1 ]]; then
  echo "Dry run enabled. Commands that would run:"
  for cmd in "${commands[@]}"; do
    echo "  $cmd"
  done
  exit 0
fi

for cmd in "${commands[@]}"; do
  eval "$cmd"
done
