load("@rules_rust//rust:defs.bzl", "rust_library", "rust_test")

rust_library(
    name = "filtergen_lib",
    srcs = ["filtergen.rs"],
    deps = [
        "//lib:flow",
        "@crates//:winnow",
    ],
)

rust_test(
    name = "filtergen_test",
    crate = ":filtergen_lib",
    deps = [
        "@crates//:pretty_assertions",
        "@crates//:yare",
    ],
)
