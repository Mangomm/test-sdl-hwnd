#ifndef _TEST_FFMPEG_H
#define _TEST_FFMPEG_H

/*
  Copyright (C) 1997-2023 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely.
*/
/* Simple program:  Display a video with a sprite bouncing around over it
 *
 * For a more complete video example, see ffplay.c in the ffmpeg sources.
 */

#include <string>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_test.h>

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/pixdesc.h>
}

#define HAVE_EGL

#ifdef HAVE_EGL
#include <SDL3/SDL_opengl.h>
#include <SDL3/SDL_opengles2.h>
#include <SDL3/SDL_egl.h>

extern "C"
{
#include <libavutil/hwcontext_drm.h>
}

#ifndef fourcc_code
#define fourcc_code(a, b, c, d) ((uint32_t)(a) | ((uint32_t)(b) << 8) | ((uint32_t)(c) << 16) | ((uint32_t)(d) << 24))
#endif
#ifndef DRM_FORMAT_R8
#define DRM_FORMAT_R8 fourcc_code('R', '8', ' ', ' ')
#endif
#ifndef DRM_FORMAT_GR88
#define DRM_FORMAT_GR88 fourcc_code('G', 'R', '8', '8')
#endif
#endif

#ifdef __APPLE__
#include "testffmpeg_videotoolbox.h"
#endif

#ifdef __WIN32__
#define COBJMACROS
#include <libavutil/hwcontext_d3d11va.h>
#endif /* __WIN32__ */


#define WINDOW_WIDTH  1920
#define WINDOW_HEIGHT 1080

/* test video in testvideo dir of project, Please enter your absolute path */
/**
 * @brief file Although this video can be played using it, if you look carefully,
 * comment out SDL_SetHint(SDL_HINT_VIDEO_FORCE_EGL, "1"),
 * then call SDL_CreateWindowFrom(), p->has_EGL_EXT_image_dma_buf_import has the value SDL_FALSE,
 * which plays because the video format is yuv420p, so call GetTextureForMemoryFrame().
 */
//static const char *file = "/home/admin1/xxx/test-sdl-hwnd/testvideo/time-60s.mp4";
/**
 * @brief file The video format is yuvj420p, which is not supported by sdl and cannot be played
 */
static const char *file = "/home/admin1/xxx/test-sdl-hwnd/testvideo/video-ios.h264";

typedef struct test_player{
    // ffmpeg
    AVFormatContext *ic;
    int audio_stream;
    int video_stream;
    const AVCodec *audio_decoder;
    const AVCodec *video_decoder;
    AVCodecContext *audio_context;
    AVCodecContext *video_context;

    // sdl
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_AudioStream *audio;
    SDL_Texture *video_texture;
    Uint64 video_start;
    SDL_bool software_only;
    SDL_bool has_eglCreateImage;
#ifdef HAVE_EGL
    SDL_bool has_EGL_EXT_image_dma_buf_import;
    PFNGLACTIVETEXTUREARBPROC glActiveTextureARBFunc;
    PFNGLEGLIMAGETARGETTEXTURE2DOESPROC glEGLImageTargetTexture2DOESFunc;
#endif
    /* play thread */
    SDL_Thread* play_tid;
    /* play handle */
    void* hwnd;

    /* abort play */
    int done;

} test_player;


test_player*            ff_create_player();
void                    ff_free_player(test_player* p);
int                     ff_play_player(test_player* p, const char* file, void* hwnd);
void                    ff_stop_player(test_player* p);

#endif // _TEST_FFMPEG_H
