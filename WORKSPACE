load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

http_archive(
    name = "com_github_storypku_bazel_iwyu",
    integrity = "sha256-r/2ZySFZDTvZAyQ0/lvbW4zrIFX6BBQPEMO/Sweh9oQ=",
    strip_prefix = "bazel_iwyu-0.20",
    urls = [
        "https://github.com/storypku/bazel_iwyu/archive/0.20.tar.gz",
    ],
)

load("@com_github_storypku_bazel_iwyu//bazel:dependencies.bzl", "bazel_iwyu_dependencies")

bazel_iwyu_dependencies()
