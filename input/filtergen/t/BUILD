cc_binary(
    name = "scan",
    testonly = True,
    srcs = ["scan.c"],
    deps = [
        "//:core",
        "//input:sourcepos",
        "//input/filtergen:in_filtergen",
    ],
)

cc_binary(
    name = "parse",
    testonly = True,
    srcs = ["parse.c"],
    deps = [
        "//:core",
        "//input:sourcepos",
        "//input/filtergen:in_filtergen",
    ],
)

cc_binary(
    name = "emit",
    testonly = True,
    srcs = ["emit.c"],
    deps = [
        "//:core",
        "//input:sourcepos",
        "//input/filtergen:in_filtergen",
    ],
)

cc_binary(
    name = "convert",
    testonly = True,
    srcs = ["convert.c"],
    deps = [
        "//:core",
        "//input:sourcepos",
        "//input/filtergen:in_filtergen",
    ],
)

cc_binary(
    name = "factorise",
    testonly = True,
    srcs = ["factorise.c"],
    deps = [
        "//:core",
        "//:factoriser",
        "//input:sourcepos",
        "//input/filtergen:in_filtergen",
    ],
)
