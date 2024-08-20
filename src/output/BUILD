package(default_visibility = ["//:__subpackages__"])
load("@rules_rust//rust:defs.bzl", "rust_binary", "rust_library", "rust_test")

rust_library(name = "nftables",
             srcs = ["nftables.rs"])

rust_test(name = "nftables_test",
          crate = ":nftables")
