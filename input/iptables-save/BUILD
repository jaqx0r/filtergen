load("@rules_bison//bison:bison.bzl", "bison")
load("@rules_flex//flex:flex.bzl", "flex")

cc_library(
    name = "in_iptables_save",
    srcs = [
        "glue.c",
        ":parser",
        ":scanner",
    ],
    hdrs = ["ast.h"],
    defines = ["_GNU_SOURCE"],
    visibility = ["//:__subpackages__"],
    deps = [
        "//:error",
        "//:filter",
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
