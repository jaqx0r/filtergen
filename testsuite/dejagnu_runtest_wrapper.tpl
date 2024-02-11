#/bin/sh

set -e

if ! [ -x /bin/runtest ]; then
  echo "/bin/runtest not found, install dejagnu" | tee -a $TEST_INFRASTRUCTURE_FAILURE_FILE
  exit 127
fi

/bin/runtest --version

cleanup () {
  mv $TEST_UNDECLARED_OUTPUTS_DIR/{tool}.xml $XML_OUTPUT_FILE
}

trap cleanup EXIT

/bin/runtest --xml --status --all --debug -v -v --tool {tool} --srcdir {srcdir} --outdir $TEST_UNDECLARED_OUTPUTS_DIR
