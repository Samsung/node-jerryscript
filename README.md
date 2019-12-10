
# node-jerryscript

## Build & test instructions
1. Run the appropriate configure command:

  * x86 configuration:

```sh
$ ./configure --without-dtrace --without-etw --without-perfctr \
    --without-ssl --without-inspector --without-intl --without-snapshot \
    --dest-cpu x86 --without-bundled-v8
```

  * x64 configuration:

```sh
$ ./configure --without-dtrace --without-etw --without-perfctr \
    --without-ssl --without-inspector --without-intl --without-snapshot \
    --dest-cpu x64 --without-bundled-v8
```

  * ARM32 cross-compile configuration:

```sh
$ CC=arm-linux-gnueabihf-gcc CXX=arm-linux-gnueabihf-g++ \
    ./configure --without-dtrace --without-etw --without-perfctr --without-ssl \
    --without-inspector --without-intl --without-snapshot --dest-cpu arm \
    --cross-compiling --dest-os=linux --with-arm-float-abi=hard --with-arm-fpu=neon \
    --without-bundled-v8
```

The `--without-bundled-v8` option was hijacked to force build Node with JerryScript.

2. Run `make` . Optionally use `make -j4`

3. After build execute the following steps for a "hello" output:

```sh
$ echo "console.log('hello');" >> test.js
$ ./node test.js
```
