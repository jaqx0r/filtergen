#/bin/sh

set -x

if ! [ -x /bin/runtest ]; then
  echo "/bin/runtest not found, install dejagnu" | tee -a $TEST_INFRASTRUCTURE_FAILURE_FILE
  exit 127
fi

echo -n "pwd: "
pwd
/bin/runtest --version

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

/bin/runtest --xml --status --all --debug -v -v --tool {tool} --srcdir {srcdir} --outdir $TEST_UNDECLARED_OUTPUTS_DIR
