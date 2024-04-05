load("@rules_foreign_cc//foreign_cc:defs.bzl", "configure_make")

package(default_visibility = ["//visibility:public"])

filegroup(
    name = "all_srcs",
    srcs = glob(["**"]),
)

configure_make(
    name = "dejagnu_configure",
    lib_source = ":all_srcs",
    out_binaries = ["runtest"],
    out_data_dirs = [
        "share",
    ],
    visibility = ["//visibility:public"],
)

# filegroup(name = "dejagnu_runtest",
#           srcs = [":dejagnu_configure"],

#           output_group = "runtest")
