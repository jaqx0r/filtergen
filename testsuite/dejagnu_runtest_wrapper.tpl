#/usr/bin/env bash

set -x

# --- begin runfiles.bash initialization v3 ---
# Copy-pasted from the Bazel Bash runfiles library v3.
set -uo pipefail; set +e; f=bazel_tools/tools/bash/runfiles/runfiles.bash
source "${RUNFILES_DIR:-/dev/null}/$f" 2>/dev/null || \
  source "$(grep -sm1 "^$f " "${RUNFILES_MANIFEST_FILE:-/dev/null}" | cut -f2- -d' ')" 2>/dev/null || \
  source "$0.runfiles/$f" 2>/dev/null || \
  source "$(grep -sm1 "^$f " "$0.runfiles_manifest" | cut -f2- -d' ')" 2>/dev/null || \
  source "$(grep -sm1 "^$f " "$0.exe.runfiles_manifest" | cut -f2- -d' ')" 2>/dev/null || \
  { echo>&2 "ERROR: cannot find $f"; exit 1; }; f=; set -e
# --- end runfiles.bash initialization v3 ---


echo -n "pwd: "
pwd

runtest=$(rlocation "{runtest}")

export DEJAGNULIBS="{libdir}"


$runtest --version

ls -laLR

set -e

cat >site.exp <<EOF
set tool {tool}
set srcdir {srcdir}
set objdir `pwd`
info exists env(COVERAGE_DIR)
EOF

cat site.exp

$runtest --global_init site.exp --status --all --debug -v -v --tool {tool} --srcdir {srcdir} --outdir $TEST_UNDECLARED_OUTPUTS_DIR


echo "post run directory"
ls -laLR
