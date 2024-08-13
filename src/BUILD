load("@rules_rust//rust:defs.bzl", "rust_library", "rust_test")

package(default_visibility = ["//:__subpackages__"])

rust_library(
    name = "lib",
    srcs = ["lib.rs"],
)

rust_test(
    name = "lib_test",
    crate = ":lib",
)
