load("//testsuite:dejagnu.bzl", "dejagnu_test")

dejagnu_test(
    name = "ipts-parse",
    srcs = glob(["**/*.exp"]),
    data = glob(
        [
            "**/*.in",
        ],
    ),
    tool_exec = "//input/iptables-save/t:parse",
    deps = ["//testsuite/lib:ipts-parse-dg"],
)
