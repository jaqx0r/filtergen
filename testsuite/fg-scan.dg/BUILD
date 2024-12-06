load("@rules_dejagnu//dejagnu:defs.bzl", "dejagnu_test")

dejagnu_test(
    name = "fg-scan",
    size = "small",
    srcs = glob(["**/*.exp"]),
    data = glob(
        [
            "**/*.in",
            "include*.d/**",
        ],
        exclude_directories = 0,
    ) + ["include"],
    tool_exec = "//input/filtergen/t:scan",
    deps = ["//testsuite/lib:fg-scan-dg"],
)
