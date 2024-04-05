load("@rules_bison//bison:bison.bzl", "bison")
load("@rules_flex//flex:flex.bzl", "flex")

cc_library(
    name = "in_filtergen",
    srcs = [
        "glue.c",
        "include.c",
        ":parser",
        ":scanner",
    ],
    hdrs = [
        "include.h",
    ],
    defines = ["_GNU_SOURCE"],
    visibility = ["//:__subpackages__"],
    deps = [
        ":ast",
        ":resolver",
        "//:error",
    ],
)

cc_library(
    name = "ast",
    hdrs = ["ast.h"],
    deps = ["//input:sourcepos"],
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

bison(
    name = "parser",
    src = "parser.y",
)

flex(
    name = "scanner",
    src = "scanner.l",
)
