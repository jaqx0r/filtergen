"""Dejagnu test runner."""

def _dejagnu_test_impl(ctx):
    """Implementation of dejagnu test suite."""
    executable_path = "{name}_/{name}".format(name = ctx.label.name)
    executable = ctx.actions.declare_file(executable_path)

    tool = ctx.label.name
    srcdir = ctx.files.srcs[0].dirname

    content = _runtest(tool = tool, srcdir = srcdir)
    ctx.actions.write(
        output = executable,
        content = content,
    )

    # gather runfiles
    runfiles = ctx.runfiles(files = ctx.files.srcs + ctx.files.data + ctx.files.tool_exec + ctx.files.deps)

    return [
        DefaultInfo(
            runfiles = runfiles,
            executable = executable,
        ),
    ]

dejagnu_test = rule(
    implementation = _dejagnu_test_impl,
    attrs = {
        "srcs": attr.label_list(
            allow_files = [".exp"],
            doc = "Main test file for dejagnu runtests.",
        ),
        "deps": attr.label_list(allow_files = True),
        "data": attr.label_list(
            allow_files = True,
        ),
        "tool_exec": attr.label(),
    },
    test = True,
)

def _runtest(tool, srcdir):
    """Generates the runtest wrapper script."""
    return DEJAGNU_TEST_RUNNER_TEMPLATE.format(tool = tool, srcdir = srcdir)

DEJAGNU_TEST_RUNNER_TEMPLATE = """
set -o errexit

if ! [ -x /bin/runtest ]; then
  echo "/bin/runtest not found, install dejagnu" | tee -a $TEST_INFRASTRUCTURE_FAILURE_FILE
  exit 127
fi

cleanup () {{
  mv $TEST_UNDECLARED_OUTPUTS_DIR/{tool}.xml $XML_OUTPUT_FILE
}}

trap cleanup EXIT

/bin/runtest --xml --status -all --debug -v -v --tool {tool} --srcdir {srcdir} --outdir $TEST_UNDECLARED_OUTPUTS_DIR
"""
