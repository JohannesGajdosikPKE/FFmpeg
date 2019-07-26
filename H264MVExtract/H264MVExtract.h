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
                                 int width,int height,
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
