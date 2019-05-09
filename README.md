This code is a fork of the popular ffmpeg. The goal is to crate a library that can do nothing but extract the motion vectors of H264 streams at high speed.

I configure it like that:

 ./configure --enable-static --disable-everything --disable-filters --disable-hwaccels --enable-decoder=h264 --disable-libv4l2 --disable-vdpau --disable-xvmc --disable-bzlib --disable-zlib --disable-doc --disable-network --disable-lzo --disable-swresample --disable-swscale --disable-autodetect --disable-iconv --disable-indev=alsa --disable-outdev=alsa --disable-libv4l2 --disable-indev=v4l2 --disable-outdev=v4l2 --disable-outdev=xv --disable-indev=xcbgrab --disable-outdev=sdl2 --disable-programs
 
