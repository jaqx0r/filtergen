load("@buildifier_prebuilt//:rules.bzl", "buildifier")

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

cc_binary(
    name = "filtergen",
    srcs = [
        "fg-iptrestore.c",
        "filtergen.c",
        "gen.c",
    ],
    defines = ["VERSION=\\\"TODO\\\""],
    deps = [
        "//input/filtergen:in_filtergen",
        "//input/iptables-save:in_iptables_save",
        "//output/cisco:out_cisco",
        "//output/filtergen:out_filtergen",
        "//output/ipchains:out_ipchains",
        "//output/ipfilter:out_ipfilter",
        "//output/iptables:out_iptables",
        "//output/iptablesrestore:out_iptablesrestore",
    ],
)

buildifier(
    name = "buildifier",
    exclude_patterns = [
        "./.git/*",
    ],
    lint_mode = "fix",
    mode = "fix",
)
