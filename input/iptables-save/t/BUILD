package(default_visibility = ["//:__subpackages__"])

cc_binary(
    name = "scan",
    testonly = True,
    srcs = ["scan.c"],
    deps = [
        "//input:sourcepos",
        "//input/iptables-save:in_iptables_save",
    ],
)

cc_binary(
    name = "parse",
    testonly = True,
    srcs = ["parse.c"],
    deps = [
        "//input:sourcepos",
        "//input/iptables-save:in_iptables_save",
    ],
)

cc_binary(
    name = "emit",
    testonly = True,
    srcs = ["emit.c"],
    deps = [
        "//input:sourcepos",
        "//input/iptables-save:in_iptables_save",
    ],
)
