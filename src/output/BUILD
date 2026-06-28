load("@rules_rust//rust:defs.bzl", "rust_library", "rust_test")

package(default_visibility = ["//:__subpackages__"])

rust_library(
    name = "nftables",
    srcs = ["nftables.rs"],
    deps = ["//src:output"],
)

rust_test(
    name = "nftables_test",
    crate = ":nftables",
)
