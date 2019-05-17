/*

Linux:

g++ -O3 -o H264MVExtractTest H264MVExtractTest.cpp \
-L. -lH264MVExtract -lpthread
export LD_LIBRARY_PATH=`pwd`

*/

#include "H264MVExtract.h"

#include <libavutil/motion_vector.h>

#include <time.h>

#include <stdio.h>
#include <stdlib.h>

static long long int prev_time = -0x8000000000000000LL;
extern "C" {
long long int GetNow(void) {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC_COARSE,&ts);
  long long int rval = ts.tv_sec*1000000000LL+ts.tv_nsec;
  if (rval <= prev_time) rval = prev_time+1;
  prev_time = rval;
  return rval;
}

long long int h264_decode_frame_sum[10] = {0,0,0,0,0,0,0,0,0,0};
long long int decode_nal_units_sum[10] = {0,0,0,0,0,0,0,0,0,0};
long long int decode_slice_sum[10] = {0,0,0,0,0,0,0,0,0,0};
long long int finalize_frame_sum[10] = {0,0,0,0,0,0,0,0,0,0};
long long int decode_mb_cavlc_sum[20] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
}

static inline unsigned long long int ReverseBits(unsigned long long int x) {
  unsigned long long int rval = 0;
  for (int i=0;i<64;i++) {
    rval <<= 1;
    rval |= (x & 1ULL);
    x >>= 1;
  }
  return rval;
}


static void print_motion_vectors_data(const AVMotionVector *mv){
  printf("%2d x %2d | src:(%4d,%4d) | dst:(%4d,%4d) | dx:%4d | dy:%4d | motion_x:%4d | motion_y:%4d | motion_scale:%4d | 0x%Lx |\n",
      mv->w,
      mv->h,
      mv->src_x,
      mv->src_y,
      mv->dst_x,
      mv->dst_y,
      mv->dst_x - mv->src_x,
      mv->dst_y - mv->src_y,
      mv->motion_x,
      mv->motion_y,
      mv->motion_scale,
      (long long unsigned int)(mv->flags));
}

static void MV_CB(void *user_data,
                  long long int pts,
                  const struct AVMotionVector *mvs,
                  int nr_of_mvs) {
  int i;
  pts = ReverseBits(pts);
  static long long int prev_pts = 0;
//printf("received %Ld %Ld: %d\n",pts,pts-prev_pts,nr_of_mvs);
  prev_pts = pts;
  for (i=0;i<nr_of_mvs;i++) {
    const AVMotionVector *mv = &mvs[i];
    print_motion_vectors_data(mv);
//    printf("%2d,%2d,%2d,%4d,%4d,%4d,%4d,0x%lx\n",
//           mv->source, mv->w, mv->h,
//           mv->src_x, mv->src_y,
//           mv->dst_x, mv->dst_y, mv->flags);
  }
}


#define DATA_SIZE (512*1024*1024)

int main(int argc, char *argv[]) {
  if (argc != 2) {
    fprintf(stderr,
            "Usage:   %s <fname-template>\n"
            "Example: mkdir frame_dir\n"
            "         AvCopy -ssm 1 video.mp4 frame_dir/%%06d.frame\n"
            "         %s frame_dir/%%06d.frame\n\n",
            argv[0],argv[0]);
    return 1;
  }
  int i;
  char fname[2048];
  FILE *f;
  size_t size;
  unsigned char *data = (unsigned char *)malloc(DATA_SIZE);
  if (!data) return 1;
  void *context = H264MVExtract_CreateContext(NULL,&MV_CB);
  long long int pts = 1000000LL*86400LL*365*50;
  for (int i=0;i<1000;i++) {
    snprintf(fname,sizeof(fname),argv[1],i);
    f = fopen(fname,"rb");
    if (!f) {
      fprintf(stderr,"fopen(\"%s\",\"rb\") failed\n",fname);
      break;
    }
    size = fread(data,1,DATA_SIZE,f);
    if (size >= DATA_SIZE) {
      fprintf(stderr,"file \"%s\" too big",fname);
      break;
    }
    if (size <= 0) {
      fprintf(stderr,"fread(\"%s\") failed\n",fname);
      break;
    }
    if (0 > H264MVExtract_DecodeFrame(context,ReverseBits(pts),data,size)) {
      fprintf(stderr,"H264MVExtract_DecodeFrame(\"%s\") failed\n",fname);
      break;
    }
    pts += 31753;
    fclose(f);
  }
  if (0 > H264MVExtract_FlushContext(context)) {
    fprintf(stderr,"H264MVExtract_FlushContext failed\n");
  }
  H264MVExtract_DestroyContext(context);
for (i=0;i<10;i++) printf("h264_decode_frame_sum[%2d]:%12Ld\n",i,h264_decode_frame_sum[i]);
for (i=0;i<10;i++) printf("decode_nal_units_sum[%2d] :%12Ld\n",i,decode_nal_units_sum[i]);
for (i=0;i<10;i++) printf("decode_slice_sum[%2d]     :%12Ld\n",i,decode_slice_sum[i]);
for (i=0;i< 3;i++) printf("finalize_frame_sum[%2d]   :%12Ld\n",i,finalize_frame_sum[i]);
for (i=0;i<20;i++) printf("decode_mb_cavlc_sum[%2d]  :%12Ld\n",i,decode_mb_cavlc_sum[i]);
  if (1) {
    long long int t0 = -GetNow();
    for (i=0;i<1000000;i++) {
      GetNow();
      GetNow();
      GetNow();
      GetNow();
      GetNow();
      GetNow();
      GetNow();
      GetNow();
      GetNow();
      GetNow();
    }
    t0 += GetNow();
    printf("GetNow: %g secs\n",1e-16*t0);
  }
  return 0;
}

