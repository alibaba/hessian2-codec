load(
    "@com_google_absl//absl:copts/GENERATED_copts.bzl",
    "ABSL_GCC_EXCEPTIONS_FLAGS",
    "ABSL_GCC_FLAGS",
    "ABSL_GCC_TEST_FLAGS",
    "ABSL_LLVM_EXCEPTIONS_FLAGS",
    "ABSL_LLVM_FLAGS",
    "ABSL_LLVM_TEST_FLAGS",
    "ABSL_MSVC_EXCEPTIONS_FLAGS",
    "ABSL_MSVC_FLAGS",
    "ABSL_MSVC_LINKOPTS",
    "ABSL_MSVC_TEST_FLAGS",
)

package(default_visibility = ["//visibility:public"])

cc_binary(
    name = "demo",
    srcs = [
        "demo.cc",
    ],
    copts = ABSL_GCC_FLAGS + ["-std=c++14"],
    deps = [
        "@com_alibaba_hessian2_codec//hessian2:codec_impl_lib",
        "@com_alibaba_hessian2_codec//hessian2/basic_codec:basic_codec_lib",
    ],
)
