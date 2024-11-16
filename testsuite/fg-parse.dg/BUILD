load("//testsuite:dejagnu.bzl", "dejagnu_test")

dejagnu_test(
    name = "fg-parse",
    size = "small",
    srcs = glob(["**/*.exp"]),
    data = glob(
        [
            "**/*.in",
        ],
    ) + ["include.conf"],
    tool_exec = "//input/filtergen/t:parse",
    deps = ["//testsuite/lib:fg-parse-dg"],
)
