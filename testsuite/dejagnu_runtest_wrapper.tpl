#/bin/sh

set -x

echo -n "pwd: "
pwd
{runtest} --version

ls -laLR

cleanup () {
  mv $TEST_UNDECLARED_OUTPUTS_DIR/{tool}.xml $XML_OUTPUT_FILE
}

trap cleanup EXIT

set -e

cat >site.exp <<EOF
set tool {tool}
set srcdir {srcdir}
set objdir `pwd`
EOF

cat site.exp

export DEJAGNULIBS="{libdir}"

{runtest} --global_init site.exp --xml --status --all --debug -v -v --tool {tool} --srcdir {srcdir} --outdir $TEST_UNDECLARED_OUTPUTS_DIR
