"""Dejagnu test runner."""

DejaGNULibraryInfo = provider(
    doc = "DejaGNU Provider",
)

def _dejagnu_lib_impl(ctx):
    """Implementation of dejagnu lib."""
    runfiles = ctx.runfiles(files = ctx.files.srcs)
    runfiles = runfiles.merge_all([
        dep[DefaultInfo].default_runfiles
        for dep in ctx.attr.deps
    ])
    return [
        DefaultInfo(runfiles = runfiles),
        DejaGNULibraryInfo(),
    ]

dejagnu_library = rule(
    implementation = _dejagnu_lib_impl,
    attrs = {
        "srcs": attr.label_list(
            doc = "Sources for this library.",
            allow_files = [".exp"],
        ),
        "deps": attr.label_list(providers = [DejaGNULibraryInfo]),
    },
)

def _dejagnu_test_impl(ctx):
    """Implementation of dejagnu test suite."""
    executable_path = "{name}_/{name}".format(name = ctx.label.name)
    executable = ctx.actions.declare_file(executable_path)

    ctx.actions.expand_template(
        template = ctx.file._runtest_template,
        output = executable,
        substitutions = {
            "{tool}": ctx.label.name,
            "{srcdir}": ctx.files.srcs[0].dirname,
            "{runtest}": ctx.executable._runtest.path,
            "{libdir}": ctx.executable._runtest.dirname + "/dejagnu_configure/share/dejagnu",
        },
        is_executable = True,
    )

    # gather runfiles
    files = [
        ctx.file._runtest,
        ctx.file.tool_exec,
    ]
    files.extend(ctx.files._runtest_libs)
    files.extend(ctx.files.srcs)
    files.extend(ctx.files.data)
    runfiles = ctx.runfiles(files = files)
    runfiles = runfiles.merge_all([
        dep[DefaultInfo].default_runfiles
        for dep in ctx.attr.deps
    ])

    test_env = {}
    if ctx.configuration.coverage_enabled:
        # Bazel’s coverage runner
        # (https://github.com/bazelbuild/bazel/blob/6.4.0/tools/test/collect_coverage.sh)
        # needs a binary called “lcov_merge.”  Its location is passed in the
        # LCOV_MERGER environmental variable.  For builtin rules, this variable
        # is set automatically based on a magic “$lcov_merger” or
        # “:lcov_merger” attribute, but it’s not possible to create such
        # attributes in Starlark.  Therefore we specify the variable ourselves.
        # Note that the coverage runner runs in the runfiles root instead of
        # the execution root, therefore we use “path” instead of “short_path.”
        runfiles = runfiles.merge(
            ctx.attr._lcov_merger[DefaultInfo].default_runfiles,
        )
        test_env["LCOV_MERGER"] = ctx.executable._lcov_merger.path

        # C/C++ coverage instrumentation needs another binary that wraps gcov;
        # see
        # https://github.com/bazelbuild/bazel/blob/6.4.0/tools/test/collect_coverage.sh#L183.
        # This is normally set from a hidden “$collect_cc_coverage” attribute;
        # see
        # https://github.com/bazelbuild/bazel/blob/6.4.0/src/main/java/com/google/devtools/build/lib/analysis/test/TestActionBuilder.java#L256-L261.
        # We also need to inject its location here, like above.
        runfiles = runfiles.merge(
            ctx.attr._collect_cc_coverage[DefaultInfo].default_runfiles,
        )
        test_env["CC_CODE_COVERAGE_SCRIPT"] = ctx.executable._collect_cc_coverage.path

    return [
        DefaultInfo(
            runfiles = runfiles,
            executable = executable,
        ),
        testing.TestEnvironment(test_env),
        coverage_common.instrumented_files_info(
            ctx,
            source_attributes = ["srcs"],
            dependency_attributes = ["deps", "data", "tool_exec"],
        ),
    ]

dejagnu_test = rule(
    implementation = _dejagnu_test_impl,
    attrs = {
        "srcs": attr.label_list(
            allow_files = [".exp"],
            doc = "Main test file for dejagnu runtests.",
        ),
        "deps": attr.label_list(providers = [DejaGNULibraryInfo]),
        "data": attr.label_list(
            allow_files = True,
        ),
        "tool_exec": attr.label(
            executable = True,
            doc = "Binary target under test.",
            cfg = "exec",
            allow_single_file = True,
        ),
        "_runtest": attr.label(
            default = "@org_gnu_dejagnu//:runtest",
            executable = True,
            allow_single_file = True,
            cfg = "exec",
        ),
        "_runtest_libs": attr.label(
            default = "@org_gnu_dejagnu//:dejagnu_configure",
        ),
        "_runtest_template": attr.label(
            default = ":dejagnu_runtest_wrapper.tpl",
            allow_single_file = True,
        ),
        # Magic coverage attributes.  This is only partially documented
        # (https://bazel.build/rules/lib/coverage#output_generator), but we can
        # take over the values from
        # https://github.com/bazelbuild/bazel/blob/7.0.0-pre.20231018.3/src/main/starlark/builtins_bzl/common/python/py_test_bazel.bzl.
        "_lcov_merger": attr.label(
            default = configuration_field("coverage", "output_generator"),
            executable = True,
            cfg = "exec",
        ),
        "_collect_cc_coverage": attr.label(
            default = Label("@bazel_tools//tools/test:collect_cc_coverage"),
            executable = True,
            cfg = "exec",
        ),
    },
    test = True,
)
