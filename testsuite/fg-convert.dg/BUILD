load("//testsuite:dejagnu.bzl", "dejagnu_test")

dejagnu_test(
    name = "fg-convert",
    size = "small",
    srcs = glob(["**/*.exp"]),
    data = glob(
        [
            "**/*.in",
            "**/*.golden",
            "include*.d/**",
        ],
        exclude_directories = 0,
    ),
    tool_exec = "//input/filtergen/t:convert",
    deps = ["//testsuite/lib:fg-convert-dg"],
)
