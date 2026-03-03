## CI diagnosis for PR #1

PR #1 failures were a CI configuration/coverage mix, not a single scientific regression: the original PR matrix included unstable full-suite checks that failed on baseline-sensitive benchmarks already failing on `main`, and one docs deployment failed at publish time due missing token write permissions (`git` exit 128 in deploy). In flarebin-specific validation, there was also a real module-plumbing bug: `InitializeBodyFlareBin` reset parsed FLAREBIN options before module verification, so even valid flarebin inputs failed with `dFlareBinEmax must be > dFlareBinEmin`.

CI was kept fast and deterministic by keeping PR checks to smoke coverage and moving deep diagnostics to scheduled/manual workflows. For flarebin, a dedicated PR-fast smoke+determinism check now runs `bin/vplanet` on a minimal STELLAR+FLAREBIN case, runs it twice, enforces byte-identical outputs, checks for NaN/Inf, and verifies flarebin outputs are present. This protects flarebin development directly without adding long-running physics matrices to PR gating.

## What runs where

- Pull requests:
  - `tests-linux` smoke pytest subset on `ubuntu-latest` + Python `3.11`.
  - `tests-linux` flarebin smoke+determinism check (`tests/ci_flarebin_smoke.sh`).
  - macOS smoke workflows on `macos-latest` (Python `3.11` and `3.12`).
- Nightly/manual:
  - Linux smoke workflows on Python `3.11` and `3.12`.
  - floating-point sweep workflow.
  - sanitizer workflow.
