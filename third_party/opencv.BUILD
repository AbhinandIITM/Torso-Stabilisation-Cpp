load("@rules_foreign_cc//foreign_cc:defs.bzl", "cmake")

filegroup(
    name = "all_srcs",
    srcs = glob(["**"]),
    visibility = ["//visibility:public"],
)

# Rename the rule name to "opencv" so consumers can reference it as @opencv//:opencv
cmake(
    name = "opencv", # <-- CHANGE THIS NAME
    lib_source = ":all_srcs",
    # Defines the static libraries we expect CMake to generate.
    out_static_libs = [
        "libopencv_core.a",
        "libopencv_imgproc.a",
        "libopencv_imgcodecs.a",
        "libopencv_highgui.a",  # Include if you need UI/Window functions
        "libopencv_videoio.a",  # Include if you need VideoCapture
        "libopencv_video.a",
        "libopencv_features2d.a",
        "libopencv_flann.a",
        "libopencv_calib3d.a",
    ],
    # Configure CMake build flags here:
    cache_entries = {
        "CMAKE_BUILD_TYPE": "Release",
        "BUILD_SHARED_LIBS": "OFF",  # Build static libs
        "BUILD_LIST": "core,highgui,imgcodecs,imgproc,video,videoio,features2d,flann,calib3d",
        # ... other options ...
    },
    # The output target will be discoverable as @opencv//:opencv
    visibility = ["//visibility:public"],
)