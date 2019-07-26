/*
 * Author: Johannes Gajdosik
 * copyright (c) 2019 PKE Holding
 *
 * This file is part of libH264MVExtract.
 *
 * libH264MVExtract is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * libH264MVExtract is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with libH264MVExtract; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */



/*

Linux:

gcc H264MVExtract.c -O3 -fPIC -c -o H264MVExtract.o -I..
gcc -shared -Wl,-Bsymbolic -Wl,--version-script=H264MVExtract.version_script \
-o libH264MVExtract.so H264MVExtract.o \
../libavcodec/libavcodec.a ../libavutil/libavutil.a
strip --strip-all libH264MVExtract.so

Windows crosscompilation:

x86_64-w64-mingw32-gcc H264MVExtract.c -O3 -c -o H264MVExtract.o -I.. \
-DH264MVExtract_COMPILE_DLL
x86_64-w64-mingw32-gcc -shared -Wl,-Bsymbolic -o H264MVExtract.dll \
H264MVExtract.o ../libavcodec/libavcodec.a ../libavutil/libavutil.a
x86_64-w64-mingw32-strip --strip-all H264MVExtract.dll
x86_64-w64-mingw32-dlltool -d H264MVExtract.def -l H264MVExtract.lib -D H264MVExtract.dll

*/

#include "H264MVExtract.h"

#include <libavutil/motion_vector.h>
#include <libavformat/avformat.h>

#include <stdio.h>
#include <string.h>

typedef struct {
  AVCodecContext *codec_context;
  void *user_data;
  H264MVExtract_CB cb;
  AVPacket pkt;
  AVFrame *frame;
} H264MVExtract_Context;

H264MVExtract_EXPORT
void *H264MVExtract_CreateContext(void *user_data,H264MVExtract_CB cb) {
  AVCodec *codec;
  H264MVExtract_Context *rval;
  int ret;
  if (!cb) return 0;
  codec = avcodec_find_decoder(AV_CODEC_ID_H264);
  if (!codec) return 0;
  rval = (H264MVExtract_Context*)malloc(sizeof(H264MVExtract_Context));
  if (!rval) return 0;
  memset(rval,0,sizeof(H264MVExtract_Context));
  rval->frame = av_frame_alloc();
  if (!rval->frame) {
    fprintf(stderr,"av_frame_alloc failed\n");
    free(rval);
    return 0;
  }
  av_init_packet(&rval->pkt);
  rval->pkt.data = 0;
  rval->pkt.size = 0;
  rval->codec_context = avcodec_alloc_context3(codec);
  if (!rval->codec_context) {
    av_frame_free(&rval->frame);
    free(rval);
    return 0;
  }
  rval->codec_context->flags2 |= AV_CODEC_FLAG2_EXPORT_MVS;
  ret = avcodec_open2(rval->codec_context,codec,NULL);
  if (ret < 0) {
    fprintf(stderr,"avcodec_open2 failed: %s\n",av_err2str(ret));
    avcodec_free_context(&rval->codec_context);
    av_frame_free(&rval->frame);
    free(rval);
    return 0;
  }
  rval->user_data = user_data;
  rval->cb = cb;
  return rval;
}

H264MVExtract_EXPORT void H264MVExtract_DestroyContext(void *context) {
  avcodec_free_context(&((H264MVExtract_Context*)context)->codec_context);
  av_packet_unref(&((H264MVExtract_Context*)context)->pkt);
  av_frame_free(&((H264MVExtract_Context*)context)->frame);
  free(context);
}

H264MVExtract_EXPORT int H264MVExtract_FlushContext(void *context) {
  return H264MVExtract_DecodeFrame(context,0,0,0);
}

H264MVExtract_EXPORT
int H264MVExtract_DecodeFrame(void *context,
                              long long int pts,
                              const unsigned char *data,int size) {
  ((H264MVExtract_Context*)context)->pkt.pts = pts;
  ((H264MVExtract_Context*)context)->pkt.data = (unsigned char*)data;
  ((H264MVExtract_Context*)context)->pkt.size = size;
  int ret = avcodec_send_packet(
              ((H264MVExtract_Context*)context)->codec_context,
              &((H264MVExtract_Context*)context)->pkt);
  if (ret < 0) {
    fprintf(stderr,"avcodec_send_packet failed: %s\n",av_err2str(ret));
    return ret;
  }

  while (ret >= 0) {
    ret = avcodec_receive_frame(
            ((H264MVExtract_Context*)context)->codec_context,
            ((H264MVExtract_Context*)context)->frame);
    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) break;
    if (ret < 0) {
      fprintf(stderr,"avcodec_receive_frame failed: %s\n",av_err2str(ret));
      return ret;
    }
    {
      const AVFrameSideData *sd
        = av_frame_get_side_data(((H264MVExtract_Context*)context)->frame,
                                 AV_FRAME_DATA_MOTION_VECTORS);
      if (sd) {
        ((H264MVExtract_Context*)context)
          ->cb(((H264MVExtract_Context*)context)->user_data,
               ((H264MVExtract_Context*)context)->codec_context->width,
               ((H264MVExtract_Context*)context)->codec_context->height,
               ((H264MVExtract_Context*)context)->frame->pts,
               (const AVMotionVector*)sd->data,
               sd->size/sizeof(AVMotionVector));
      } else {
        ((H264MVExtract_Context*)context)
          ->cb(((H264MVExtract_Context*)context)->user_data,
               ((H264MVExtract_Context*)context)->codec_context->width,
               ((H264MVExtract_Context*)context)->codec_context->height,
               ((H264MVExtract_Context*)context)->frame->pts,
               NULL,0);
      }
    }
    av_frame_unref(((H264MVExtract_Context*)context)->frame);
  }
  return 0;
}
