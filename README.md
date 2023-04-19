# DDS

### Dependencies
* CMake >= 3.17
* Boost
* [FFMPEG](https://git.ffmpeg.org/ffmpeg.git) build with at least libx264 enabled
* [websocketpp](https://github.com/zaphoyd/websocketpp)
* OpenSSL
* libopencv

``` $ sudo apt-get install make cmake yasm nasm pkg-config libssl-dev libboost-all-dev libx264-dev```

I configured ffmpeg with:

``` $ ./configure  --ld="g++" --enable-gpl --disable-programs --disable-static --enable-shared --enable-libx264```

### Building
```
$ mkdir build
$ cd build
$ cmake ..
$ make
```
---
Rtmp module inspired by [cpp_media_server-1](https://github.com/grandi23/cpp_media_server-1) which is licensed under [MIT](https://opensource.org/licenses/MIT). Copyright (c) 2021 Alex.CR

The class contains a copy of [json](https://github.com/nlohmann/json) from Niels Lohmann which is licensed under [MIT](https://opensource.org/licenses/MIT). Copyright (c) 2013-2022 Niels Lohmann
