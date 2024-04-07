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

load(
    "@bazel_tools//tools/build_defs/repo:git.bzl",
    "git_repository",
)

git_repository(
    name = "bazel_clang_tidy",
    commit = "43bef6852a433f3b2a6b001daecc8bc91d791b92",
    remote = "https://github.com/erenon/bazel_clang_tidy.git",
)
