load("@rules_dejagnu//dejagnu:defs.bzl", "dejagnu_test")

dejagnu_test(
    name = "filtergen",
    size = "small",
    srcs = glob(["**/*.exp"]),
    data = glob(
        [
            "*.filter",
        ],
    ) + ["//examples"],
    tool_exec = "//:filtergen",
    deps = ["//testsuite/lib:filtergen-dg"],
)
