package(
    default_visibility = ["//:__subpackages__"],
)

cc_library(
    name = "sourcefile",
    srcs = ["sourcefile.c"],
    hdrs = [
        "sourcefile.h",
    ],
    defines = ["_GNU_SOURCE"],
    deps = [
        ":sourcepos",
        "//:error",
    ],
)

cc_library(
    name = "sourcepos",
    srcs = ["sourcepos.c"],
    hdrs = [
        "input.h",
        "sourcepos.h",
    ],
    defines = ["_GNU_SOURCE"],
)
