cc_binary(
    name = "scan",
    testonly = True,
    srcs = ["scan.c"],
    deps = [
        "//:core",
        "//input:sourcepos",
        "//input/iptables-save:in_iptables_save",
    ],
)

cc_binary(
    name = "parse",
    testonly = True,
    srcs = ["parse.c"],
    deps = [
        "//:core",
        "//input:sourcepos",
        "//input/iptables-save:in_iptables_save",
    ],
)

cc_binary(
    name = "emit",
    testonly = True,
    srcs = ["emit.c"],
    deps = [
        "//:core",
        "//input:sourcepos",
        "//input/iptables-save:in_iptables_save",
    ],
)
