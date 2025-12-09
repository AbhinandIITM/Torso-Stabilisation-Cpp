cc_library(
    name = "torch",
    hdrs = glob([
        "include/**/*.h",
        "include/**/*.hpp",
    ]),
    includes = [
        "include",
        "include/torch/csrc/api/include",
    ],
   

    # Only C++ libs, no libtorch_python.so
    srcs = [
        "lib/libc10.so",
        "lib/libc10_cuda.so",        
        "lib/libtorch.so",
        "lib/libtorch_cpu.so",
        "lib/libtorch_cuda.so",      
        # Add .a variants here if you really need static link
        # "lib/libc10.a",
        # "lib/libtorch.a",
    ],

    visibility = ["//visibility:public"],
)
