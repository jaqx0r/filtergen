load("@rules_cc//cc:cc_library.bzl", "cc_library")

cc_library(
    name = "out_iptablesrestore",
    srcs = ["fg-iptablesrestore.c"],
    visibility = ["//:__subpackages__"],
    deps = [
        "//:filter",
        "//:flags",
        "//:gen",
        "//:oputs",
        "//:util",
    ],
)
