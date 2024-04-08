load("@buildifier_prebuilt//:rules.bzl", "buildifier")
load("//:subst.bzl", "subst_template")

package(default_visibility = [":__subpackages__"])

VERSION = "0.13"

cc_library(
    name = "filter",
    srcs = ["filter.c"],
    hdrs = ["filter.h"],
    deps = [
        ":error",
        ":util",
        "//input:sourcefile",
        "//input:sourcepos",
    ],
)

cc_library(
    name = "error",
    srcs = ["error.c"],
    hdrs = ["error.h"],
    deps = ["//input:sourcepos"],
)

cc_library(
    name = "factoriser",
    srcs = ["factoriser.c"],
    hdrs = ["factoriser.h"],
)

cc_library(
    name = "icmpent",
    srcs = ["icmpent.c"],
    hdrs = ["icmpent.h"],
)

cc_library(
    name = "util",
    srcs = [
        "fg-util.c",
    ],
    hdrs = [
        "fg-util.h",
        "util.h",
    ],
)

cc_binary(
    name = "filtergen",
    srcs = [
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

SUBSTITUTIONS = {
    "@sysconfdir@": "/etc",
    "@pkgexdir@": "/usr/share/doc/filergen/examples",
    "@sbindir@": "/usr/sbin",
    "@VERSION@": VERSION,
}

subst_template(
    name = "fgadm",
    src = "fgadm.in",
    substitutions = SUBSTITUTIONS,
)

subst_template(
    name = "fgadm.conf",
    src = "fgadm.conf.in",
    substitutions = SUBSTITUTIONS,
)

subst_template(
    name = "rules.filter",
    src = "rules.filter.in",
    substitutions = SUBSTITUTIONS,
)

subst_template(
    name = "filtergen.spec",
    src = "filtergen.spec.in",
    substitutions = SUBSTITUTIONS,
)

buildifier(
    name = "buildifier",
    exclude_patterns = [
        "./.git/*",
    ],
    lint_mode = "fix",
    mode = "fix",
)
