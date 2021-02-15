load("@rules_cc//cc:defs.bzl", "cc_binary")
load("//bazel:cc_opts.bzl", "default_copts")

cc_library(
    name = "capp",
    hdrs = glob(["capp/**/*.h"]),
    visibility = ["//visibility:public"],
    copts = default_copts(),
    deps = [
        "@com_github_fmtlib_fmt//:lib",
    ]
)

cc_library(
    name = "tests",
    srcs = glob(["test/*.cc"]),
    hdrs = glob(["test/*.h"]),
    visibility = ["//visibility:private"],
    copts = default_copts(),
    testonly = True,
    deps = [
        "@com_github_catchorg_catch2//:lib",
        ":capp",
    ]
)

cc_test(
    name = "test",
    copts = default_copts(),
    linkopts = ["-lpthread"],
    deps = [
        ":tests",
        "@com_github_catchorg_catch2//:main",
    ],
)
