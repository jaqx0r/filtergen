package(default_visibility = [":__subpackages__"])

cc_library(
    name = "core",
    srcs = [
        "error.c",
        "fg-util.c",
        "filter.c",
        "icmpent.c",
    ],
    hdrs = [
        "error.h",
        "filter.h",
        "icmpent.h",
        "util.h",
    ],
    deps = ["//input:sourcepos"],
)

cc_library(
    name = "factoriser",
    srcs = ["factoriser.c"],
    hdrs = ["factoriser.h"],
)
