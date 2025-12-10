cc_library(
    name = "asio",
    hdrs = glob([
        "asio/include/**/*.hpp",
        "asio/include/**/*.ipp",
    ]),
    includes = ["asio/include"],
    defines = [
        "ASIO_STANDALONE",
        "ASIO_NO_DEPRECATED",
        "ASIO_HEADER_ONLY",
    ],
    visibility = ["//visibility:public"],
)
