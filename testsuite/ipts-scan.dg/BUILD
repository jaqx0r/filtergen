load("//testsuite:dejagnu.bzl", "dejagnu_test")

dejagnu_test(
    name = "ipts-scan",
    size = "small",
    srcs = glob(["**/*.exp"]),
    data = glob(
        [
            "**/*.in",
            "include*.d/**",
        ],
        exclude_directories = 0,
    ),
    tool_exec = "//input/iptables-save/t:scan",
    deps = ["//testsuite/lib:ipts-scan-dg"],
)
