load("//testsuite:dejagnu.bzl", "dejagnu_test")

dejagnu_test(
    name = "filtergen",
    srcs = glob(["**/*.exp"]),
    data = glob(
        [
            "**/*.in",
            "include*.d/**",
        ],
        exclude_directories = 0,
    ),
    tool_exec = "//:filtergen",
    deps = ["//testsuite/lib:filtergen-dg"],
)
