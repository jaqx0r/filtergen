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
        "ast.h",
        "include.h",
        "resolver.h",
    ],
    defines = ["_GNU_SOURCE"],
    visibility = ["//:__subpackages__"],
    deps = [
        "//:core",
        "//:error",
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
