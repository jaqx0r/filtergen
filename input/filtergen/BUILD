load("@rules_bison//bison:bison.bzl", "bison_cc_library")
load("@rules_flex//flex:flex.bzl", "flex")

cc_library(
    name = "in_filtergen",
    srcs = [
        "glue.c",
        "include.c",
        ":scanner",
    ],
    hdrs = [
        "include.h",
    ],
    defines = ["_GNU_SOURCE"],
    visibility = ["//:__subpackages__"],
    deps = [
        ":ast",
        ":parser",
        ":resolver",
        "//:error",
    ],
)

cc_library(
    name = "ast",
    hdrs = ["ast.h"],
    deps = ["//input:sourcepos"],
)

bison_cc_library(
    name = "parser",
    src = "parser.y",
    deps = [
        ":ast",
        "//:error",
        "//input:sourcepos",
    ],
)

cc_library(
    name = "resolver",
    srcs = ["resolver.c"],
    hdrs = ["resolver.h"],
    deps = [
        ":ast",
        "//:filter",
        "//:icmpent",
    ],
)

flex(
    name = "scanner",
    src = "scanner.l",
)
