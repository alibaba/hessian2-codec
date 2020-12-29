load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

def load_third_party_repositories():
    # abseil-cpp
    http_archive(
        name = "com_google_absl",
        urls = ["https://github.com/abseil/abseil-cpp/archive/61c9bf3e3e1c28a4aa6d7f1be4b37fd473bb5529.tar.gz"], # 2019-06-05
        strip_prefix = "abseil-cpp-61c9bf3e3e1c28a4aa6d7f1be4b37fd473bb5529",
        sha256 = "7ddf863ddced6fa5bf7304103f9c7aa619c20a2fcf84475512c8d3834b9d14fa",
    )

    http_archive(
        name = "com_google_googletest",
        urls = ["https://github.com/google/googletest/archive/8b6d3f9c4a774bef3081195d422993323b6bb2e0.zip"],  # 2019-03-05
        strip_prefix = "googletest-8b6d3f9c4a774bef3081195d422993323b6bb2e0",
        sha256 = "d21ba93d7f193a9a0ab80b96e8890d520b25704a6fac976fe9da81fffb3392e3",
    )

    http_archive(
        name = "com_github_fmtlib_fmt",
        urls = ["https://github.com/fmtlib/fmt/archive/6.0.0.tar.gz"],
        strip_prefix = "fmt-6.0.0",
        sha256 = "f1907a58d5e86e6c382e51441d92ad9e23aea63827ba47fd647eacc0d3a16c78",
        build_file = "//bazel/external:fmtlib.BUILD",
    )