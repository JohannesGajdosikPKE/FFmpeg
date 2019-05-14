This code is a fork of the popular ffmpeg. The goal is to crate a library that can do nothing but extract the motion vectors of H264 streams at high speed.

I configure it like that:

 ./configure --enable-static --disable-everything --disable-doc --disable-swresample --disable-avfilter --disable-avdevice --disable-avformat --disable-swscale --disable-autodetect --disable-programs --enable-decoder=h264 --enable-parser=mpegvideo --disable-iconv
