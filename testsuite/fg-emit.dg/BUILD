load("//testsuite:dejagnu.bzl", "dejagnu_test")

dejagnu_test(
    name = "fg-emit",
    srcs = glob(["**/*.exp"]),
    data = glob(
        [
            "**/*.in",
            "**/*.golden",
        ],
    ),
    tool_exec = "//input/filtergen/t:emit",
    deps = ["//testsuite/lib:fg-emit-dg"],
)
