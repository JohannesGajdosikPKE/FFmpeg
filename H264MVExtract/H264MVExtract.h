#ifndef H264_MV_EXTRACT_H_
#define H264_MV_EXTRACT_H_

#if defined (_WIN32) 
  #if defined(H264MVExtract_COMPILE_DLL)
    #define  H264MVExtract_EXPORT __declspec(dllexport)
  #else
    #define  H264MVExtract_EXPORT __declspec(dllimport)
  #endif
#else
  #define H264MVExtract_EXPORT
#endif

#ifdef __cplusplus
extern "C" {
#endif

struct AVMotionVector; /* from libavutil/motion_vector.h */

typedef void (*H264MVExtract_CB)(void *user_data,
                                 long long int pts,
                                 const struct AVMotionVector *mvs,
                                 int nr_of_mvs);

H264MVExtract_EXPORT
void *H264MVExtract_CreateContext(void *user_data,H264MVExtract_CB cb);

H264MVExtract_EXPORT
void H264MVExtract_DestroyContext(void *context);

  // call at end of decoding in order to get the last decoded motion vectors:
H264MVExtract_EXPORT
int H264MVExtract_FlushContext(void *context);

H264MVExtract_EXPORT
int H264MVExtract_DecodeFrame(void *context,
                              long long int pts,
                              const unsigned char *data,int size);

#ifdef __cplusplus
}
#endif

#endif
