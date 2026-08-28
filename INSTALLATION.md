# Building on Linux

NZBGet Lite supports native Linux builds only. Install a C++20 compiler,
CMake, Git, and the development packages for Boost, libxml2, OpenSSL,
and zlib, then run:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DENABLE_TESTS=ON
cmake --build build -j4
ctest --test-dir build --output-on-failure
sudo cmake --install build
```

The first build downloads pinned `par2-turbo` and `rapidyenc` sources.
