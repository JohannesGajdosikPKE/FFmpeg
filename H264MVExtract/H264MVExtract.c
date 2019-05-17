/*

Linux:

gcc H264MVExtract.c -O3 -fPIC -c -o H264MVExtract.o -I..
gcc -shared -Wl,-Bsymbolic -Wl,--version-script=H264MVExtract.version_script \
-o libH264MVExtract.so H264MVExtract.o \
../libavcodec/libavcodec.a ../libavutil/libavutil.a

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
               ((H264MVExtract_Context*)context)->frame->pts,
               (const AVMotionVector*)sd->data,
               sd->size/sizeof(AVMotionVector));
      } else {
        ((H264MVExtract_Context*)context)
          ->cb(((H264MVExtract_Context*)context)->user_data,
               ((H264MVExtract_Context*)context)->frame->pts,
               NULL,0);
      }
    }
    av_frame_unref(((H264MVExtract_Context*)context)->frame);
  }
  return 0;
}
