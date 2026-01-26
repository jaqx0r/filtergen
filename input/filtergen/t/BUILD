load("@rules_cc//cc:cc_binary.bzl", "cc_binary")

package(default_visibility = ["//:__subpackages__"])

cc_binary(
    name = "scan",
    testonly = True,
    srcs = ["scan.c"],
    deps = [
        "//input:sourcefile",
        "//input:sourcepos",
        "//input/filtergen:in_filtergen",
    ],
)

cc_binary(
    name = "parse",
    testonly = True,
    srcs = ["parse.c"],
    deps = [
        "//input:sourcefile",
        "//input:sourcepos",
        "//input/filtergen:in_filtergen",
    ],
)

cc_binary(
    name = "emit",
    testonly = True,
    srcs = ["emit.c"],
    deps = [
        "//input:sourcefile",
        "//input:sourcepos",
        "//input/filtergen:in_filtergen",
    ],
)

cc_binary(
    name = "convert",
    testonly = True,
    srcs = ["convert.c"],
    deps = [
        "//:filter",
        "//input:sourcepos",
        "//input/filtergen:in_filtergen",
    ],
)

cc_binary(
    name = "factorise",
    testonly = True,
    srcs = ["factorise.c"],
    deps = [
        "//:factoriser",
        "//input:sourcepos",
        "//input/filtergen:in_filtergen",
    ],
)
