load("@rules_bison//bison:bison.bzl", "bison")
load("@rules_flex//flex:flex.bzl", "flex")

cc_library(
    name = "in_filtergen",
    srcs = [
        "glue.c",
        "include.c",
        "resolver.c",
        ":parser",
        ":scanner",
    ],
    hdrs = [
        "include.h",
        "resolver.h",
    ],
    defines = ["_GNU_SOURCE"],
    visibility = ["//:__subpackages__"],
    deps = [
        ":ast",
        "//:error",
        "//:filter",
        "//:icmpent",
    ],
)

cc_library(
    name = "ast",
    hdrs = ["ast.h"],
    deps = ["//input:sourcepos"],
)

bison(
    name = "parser",
    src = "parser.y",
)

flex(
    name = "scanner",
    src = "scanner.l",
)
