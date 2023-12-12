load("@hedron_compile_commands//:refresh_compile_commands.bzl", "refresh_compile_commands")

licenses(["notice"])  # Apache 2.0

package(default_visibility = ["//visibility:public"])

config_setting(
    name = "llvm_compiler",
    values = {
        "compiler": "llvm",
    },
)

config_setting(
    name = "windows",
    values = {
        "cpu": "x64_windows",
    },
)

refresh_compile_commands(
    name = "refresh_compile_commands",

    # Specify the targets of interest.
    # For example, specify a dict of targets and any flags required to build.
    targets = {
        "//common/...": "",
        "//example/...": "",
        "//hessian2/...": "",
        "//test_hessian/...": "",
    },
)
