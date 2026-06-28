load("@rules_rust//rust:defs.bzl", "rust_library", "rust_test")

rust_library(
    name = "filtergen_lib",
    srcs = ["filtergen.rs"],
    deps = ["@crates//:nom"],
)

rust_test(
    name = "filtergen_test",
    crate = ":filtergen_lib",
    deps = ["@crates//:pretty_assertions"],
)
