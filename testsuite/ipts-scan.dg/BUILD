load("@rules_dejagnu//dejagnu:defs.bzl", "dejagnu_test")

dejagnu_test(
    name = "ipts-scan",
    size = "small",
    srcs = glob(["**/*.exp"]),
    data = glob(
        [
            "**/*.in",
        ],
        exclude_directories = 0,
    ),
    tool_exec = "//input/iptables-save/t:scan",
    deps = ["//testsuite/lib:ipts-scan-dg"],
)
