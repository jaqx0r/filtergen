load("@rules_cc//cc:cc_library.bzl", "cc_library")

cc_library(
    name = "out_iptables",
    srcs = ["fg-iptables.c"],
    visibility = ["//:__subpackages__"],
    deps = [
        "//:filter",
        "//:flags",
        "//:gen",
        "//:oputs",
        "//:util",
    ],
)
