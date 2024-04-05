#/bin/sh

set -x

echo -n "pwd: "
pwd
{runtest} --version

ls -laLR

set -e

cat >site.exp <<EOF
set tool {tool}
set srcdir {srcdir}
set objdir `pwd`
info exists env(COVERAGE_DIR)
EOF

cat site.exp

export DEJAGNULIBS="{libdir}"

{runtest} --global_init site.exp --status --all --debug -v -v --tool {tool} --srcdir {srcdir} --outdir $TEST_UNDECLARED_OUTPUTS_DIR
