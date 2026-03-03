#!/usr/bin/env bash
set -euo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
WORKDIR="$(mktemp -d)"
cleanup() {
  rm -rf "$WORKDIR"
}
trap cleanup EXIT

cp "$REPO_ROOT/tests/Stellar/None/flarebin_vpl.in" "$WORKDIR/vpl.in"
cp "$REPO_ROOT/tests/Stellar/None/flarebin_star.in" "$WORKDIR/star.in"

cd "$WORKDIR"

"$REPO_ROOT/bin/vplanet" vpl.in > run1.stdout 2> run1.stderr
cp flarebin_smoke.log flarebin_smoke.log.run1
cp flarebin_smoke.star.forward flarebin_smoke.star.forward.run1

"$REPO_ROOT/bin/vplanet" vpl.in > run2.stdout 2> run2.stderr

cmp -s flarebin_smoke.log flarebin_smoke.log.run1
cmp -s flarebin_smoke.star.forward flarebin_smoke.star.forward.run1

if rg -n -i '(^|[^a-z])nan([^a-z]|$)|(^|[^a-z])inf([^a-z]|$)' \
  flarebin_smoke.log flarebin_smoke.star.forward run1.stderr run2.stderr; then
  echo "Found NaN/Inf in flarebin outputs."
  exit 1
fi

rg -q 'Active Modules: (FLAREBIN STELLAR|STELLAR FLAREBIN)' flarebin_smoke.log
rg -q 'FlareBinPStoch' flarebin_smoke.log
rg -q 'FlareBinMuActive' flarebin_smoke.log
rg -q 'FlareBinPAnyActive' flarebin_smoke.log
rg -q 'FlareBinEStochMin' flarebin_smoke.log
rg -q 'FlareBinItemplate' flarebin_smoke.log

echo "Flarebin smoke and determinism checks passed."
