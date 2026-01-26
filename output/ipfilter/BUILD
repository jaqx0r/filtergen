load("@rules_cc//cc:cc_library.bzl", "cc_library")

cc_library(
    name = "out_ipfilter",
    srcs = ["fg-ipfilter.c"],
    visibility = ["//:__subpackages__"],
    deps = [
        "//:filter",
        "//:gen",
        "//:oputs",
        "//:util",
    ],
)
