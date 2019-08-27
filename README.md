# Build & Test

```sh
$ cmake -Bbuild -H. -DCMAKE_TOOLCHAIN_FILE=i686.toolchain.cmake
$ make -C build/
$ ./build/demo
```

# To build also with v8

```sh
$ cmake -Bbuild -H. -DUSE_V8=1 \
    -DUSE_V8_LIB_DIR=/mnt/work/projects/v8jerry/node/out/Release/obj.target/deps/v8/src/ \
    -DUSE_V8_HEADER_DIR=/mnt/work/projects/v8jerry/node/deps/v8/include/
```

`USE_V8=1` will build additional binaries with `v8_` prefix.
`USE_V8_LIB_DIR` should point to the dir where the node.js have it's build v8 archives (eg.: libv8_base.a).
`USE_V8_HEADER_DIR` should point to the dir where the v8 headers are (eg.: v8.h).
