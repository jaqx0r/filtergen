load("@rules_rust//rust:defs.bzl", "rust_binary", "rust_library", "rust_test")

package(default_visibility = ["//:__subpackages__"])

rust_library(
    name = "lib",
    srcs = ["lib.rs"],
)

rust_test(
    name = "lib_test",
    crate = ":lib",
)

rust_binary(
    name = "filtergen",
    srcs = ["main.rs"],
    deps = [
        ":build_info",
        "@crates//:clap",
    ],
)

genrule(
    name = "gen_build_info",
    outs = ["build_info.rs"],
    cmd = "echo \"#![allow(dead_code)]\" > $@; sed -E 's/(\\S+) (\\S*)/pub const \\1: \\&str = \"\\2\";/' < bazel-out/stable-status.txt >> $@",
    stamp = True,
)

rust_library(
    name = "build_info",
    srcs = ["build_info.rs"],
)

rust_library(
    name = "output",
    srcs = ["output.rs"],
)
