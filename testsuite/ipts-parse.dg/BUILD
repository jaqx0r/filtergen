load("@rules_dejagnu//dejagnu:defs.bzl", "dejagnu_test")

dejagnu_test(
    name = "ipts-parse",
    size = "small",
    srcs = glob(["**/*.exp"]),
    data = glob(
        [
            "**/*.in",
        ],
    ),
    tool_exec = "//input/iptables-save/t:parse",
    deps = ["//testsuite/lib:ipts-parse-dg"],
)
