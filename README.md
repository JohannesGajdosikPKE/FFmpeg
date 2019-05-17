This code is a fork of the popular ffmpeg. The goal is to crate a library that can do nothing but extract the motion vectors of H264 streams at high speed.

I build it like that:

Linux:

 ./configure --enable-static --disable-everything --disable-doc --disable-swresample --disable-avfilter --disable-avdevice --disable-avformat --disable-swscale --disable-autodetect --disable-programs --enable-decoder=h264 --enable-parser=mpegvideo --disable-iconv
 make -j8

Windows crosscompiling:

 ./configure --arch=x86_64 --target-os=mingw32 --cross-prefix=x86_64-w64-mingw32- --enable-static --disable-everything --disable-doc --disable-swresample --disable-avfilter --disable-avdevice --disable-avformat --disable-swscale --disable-autodetect --disable-programs --enable-decoder=h264 --enable-parser=mpegvideo --disable-iconv
 make -j8

In the H264MVExtract subdirectory there is a wrapper library and a test program.
Compile instructions are in the respective source files.
