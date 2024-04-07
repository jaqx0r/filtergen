load("//testsuite:dejagnu.bzl", "dejagnu_library")

package(
    default_visibility = ["//:__subpackages__"],
)

dejagnu_library(
    name = "filtergen-dg",
    srcs = ["filtergen-dg.exp"],
)

dejagnu_library(
    name = "fg-scan-dg",
    srcs = ["fg-scan-dg.exp"],
)

dejagnu_library(
    name = "fg-parse-dg",
    srcs = ["fg-parse-dg.exp"],
)

dejagnu_library(
    name = "compare",
    srcs = ["compare.exp"],
)

dejagnu_library(
    name = "fg-emit-dg",
    srcs = [
        "fg-emit-dg.exp",
    ],
    deps = [":compare"],
)

dejagnu_library(
    name = "fg-convert-dg",
    srcs = [
        "fg-convert-dg.exp",
    ],
    deps = [":compare"],
)

dejagnu_library(
    name = "ipts-scan-dg",
    srcs = ["ipts-scan-dg.exp"],
)

dejagnu_library(
    name = "ipts-parse-dg",
    srcs = ["ipts-parse-dg.exp"],
)
