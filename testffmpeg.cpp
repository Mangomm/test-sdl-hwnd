#include "testffmpeg.h"

static SDL_bool CreateWindowAndRenderer(test_player *p, Uint32 window_flags, const char *driver)
{
    SDL_RendererInfo info;
    SDL_bool useEGL = (SDL_bool)(driver && SDL_strcmp(driver, "opengles2") == 0);

    FILE *ff = NULL;
    if(!ff){
        ff = fopen("111video-ios.h264", "wb+");
    }
    if(ff){
        fwrite("1", 1, 1, ff);
    }

// please see here!!!
#define USING_HANDLE

    SDL_SetHint(SDL_HINT_RENDER_DRIVER, driver);
    if (useEGL) {
#ifndef USING_HANDLE
        SDL_SetHint(SDL_HINT_VIDEO_FORCE_EGL, "1");// SDL_SetHint(SDL_HINT_VIDEO_FORCE_EGL, "1") cannot be used with SDL_CreateWindowFrom
#endif
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    } else {
        SDL_SetHint(SDL_HINT_VIDEO_FORCE_EGL, "0");
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, 0);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    }
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 6);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);

#ifndef USING_HANDLE
    if (SDL_CreateWindowAndRenderer(WINDOW_WIDTH, WINDOW_HEIGHT, window_flags, &p->window, &p->renderer) < 0) {
        return SDL_FALSE;
    }
#else

    // tyy code
    SDL_SetHint("SDL_HINT_VIDEO_WINDOW_SHARE_PIXEL_FORMAT", "1");
    p->window = SDL_CreateWindowFrom(p->hwnd);
    if(!p->window){
        return SDL_FALSE;
    }

    p->renderer = SDL_CreateRenderer(p->window, NULL, SDL_RENDERER_ACCELERATED);
    if(!p->renderer){
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Created renderer failed, %s\n", SDL_GetError());
        return SDL_FALSE;
    }
#endif

    if (SDL_GetRendererInfo(p->renderer, &info) == 0) {
        SDL_Log("Created renderer %s\n", info.name);
    }

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");

#ifdef HAVE_EGL
    if (useEGL) {
        const char *extensions = eglQueryString(eglGetCurrentDisplay(), EGL_EXTENSIONS);
        SDL_Log("[ extensions ]： %s\n", extensions);
        if (SDL_strstr(extensions, "EGL_EXT_image_dma_buf_import") != NULL) {
            p->has_EGL_EXT_image_dma_buf_import = SDL_TRUE;
        }

        if (SDL_GL_ExtensionSupported("GL_OES_EGL_image")) {
            p->glEGLImageTargetTexture2DOESFunc = (PFNGLEGLIMAGETARGETTEXTURE2DOESPROC)eglGetProcAddress("glEGLImageTargetTexture2DOES");
        }

        p->glActiveTextureARBFunc = (PFNGLACTIVETEXTUREARBPROC)SDL_GL_GetProcAddress("glActiveTextureARB");

        if (p->has_EGL_EXT_image_dma_buf_import &&
            p->glEGLImageTargetTexture2DOESFunc &&
            p->glActiveTextureARBFunc) {
            p->has_eglCreateImage = SDL_TRUE;
        }
    }
#endif /* HAVE_EGL */

#ifdef __APPLE__
    has_videotoolbox_output = SetupVideoToolboxOutput(renderer);
#endif

#ifdef __WIN32__
    d3d11_device = SDL_GetRenderD3D11Device(renderer);
    if (d3d11_device) {
        ID3D11Device_GetImmediateContext(d3d11_device, &d3d11_context);
    }
#endif

    return SDL_TRUE;
}

static Uint32 GetTextureFormat(enum AVPixelFormat format)
{
    switch (format) {
    case AV_PIX_FMT_RGB8:
        return SDL_PIXELFORMAT_RGB332;
    case AV_PIX_FMT_RGB444:
        return SDL_PIXELFORMAT_RGB444;
    case AV_PIX_FMT_RGB555:
        return SDL_PIXELFORMAT_RGB555;
    case AV_PIX_FMT_BGR555:
        return SDL_PIXELFORMAT_BGR555;
    case AV_PIX_FMT_RGB565:
        return SDL_PIXELFORMAT_RGB565;
    case AV_PIX_FMT_BGR565:
        return SDL_PIXELFORMAT_BGR565;
    case AV_PIX_FMT_RGB24:
        return SDL_PIXELFORMAT_RGB24;
    case AV_PIX_FMT_BGR24:
        return SDL_PIXELFORMAT_BGR24;
    case AV_PIX_FMT_0RGB32:
        return SDL_PIXELFORMAT_XRGB8888;
    case AV_PIX_FMT_0BGR32:
        return SDL_PIXELFORMAT_XBGR8888;
    case AV_PIX_FMT_NE(RGB0, 0BGR):
        return SDL_PIXELFORMAT_RGBX8888;
    case AV_PIX_FMT_NE(BGR0, 0RGB):
        return SDL_PIXELFORMAT_BGRX8888;
    case AV_PIX_FMT_RGB32:
        return SDL_PIXELFORMAT_ARGB8888;
    case AV_PIX_FMT_RGB32_1:
        return SDL_PIXELFORMAT_RGBA8888;
    case AV_PIX_FMT_BGR32:
        return SDL_PIXELFORMAT_ABGR8888;
    case AV_PIX_FMT_BGR32_1:
        return SDL_PIXELFORMAT_BGRA8888;
    case AV_PIX_FMT_YUV420P:
        return SDL_PIXELFORMAT_IYUV;
    case AV_PIX_FMT_YUYV422:
        return SDL_PIXELFORMAT_YUY2;
    case AV_PIX_FMT_UYVY422:
        return SDL_PIXELFORMAT_UYVY;
    default:
        return SDL_PIXELFORMAT_UNKNOWN;
    }
}

static SDL_bool SupportedPixelFormat(test_player* p, enum AVPixelFormat format)
{
//    if (!software_only) {
    if (!0) {
        if (p->has_eglCreateImage &&
            (format == AV_PIX_FMT_VAAPI || format == AV_PIX_FMT_DRM_PRIME)) {
            return SDL_TRUE;
        }
#ifdef __APPLE__
        if (has_videotoolbox_output && format == AV_PIX_FMT_VIDEOTOOLBOX) {
            return SDL_TRUE;
        }
#endif
#ifdef __WIN32__
        if (d3d11_device && format == AV_PIX_FMT_D3D11) {
            return SDL_TRUE;
        }
#endif
    }

    if (GetTextureFormat(format) != SDL_PIXELFORMAT_UNKNOWN) {
        return SDL_TRUE;
    }
    return SDL_FALSE;
}

static enum AVPixelFormat GetSupportedPixelFormat(AVCodecContext *s, const enum AVPixelFormat *pix_fmts)
{
    const enum AVPixelFormat *p;

    for (p = pix_fmts; *p != AV_PIX_FMT_NONE; p++) {
        if (SupportedPixelFormat((test_player*)s->opaque, *p)) {
            /* We support this format */
            break;
        }
    }

    if (*p == AV_PIX_FMT_NONE) {
        SDL_Log("Couldn't find a supported pixel format:\n");
        for (p = pix_fmts; *p != AV_PIX_FMT_NONE; p++) {
            SDL_Log("    %s\n", av_get_pix_fmt_name(*p));
        }
    }

    return *p;
}

static AVCodecContext *OpenVideoStream(test_player* p, AVFormatContext *ic, int stream, const AVCodec *codec)
{
    AVStream *st = ic->streams[stream];
    AVCodecParameters *codecpar = st->codecpar;
    AVCodecContext *context;
    const AVCodecHWConfig *config;
    enum AVHWDeviceType type;
    int i;
    int result;

    SDL_Log("Video stream: %s %dx%d\n", avcodec_get_name(codec->id), codecpar->width, codecpar->height);

    context = avcodec_alloc_context3(NULL);
    if (!context) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "avcodec_alloc_context3 failed");
        return NULL;
    }

    result = avcodec_parameters_to_context(context, ic->streams[stream]->codecpar);
    if (result < 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "avcodec_parameters_to_context failed: %s\n", av_err2str(result));
        avcodec_free_context(&context);
        return NULL;
    }
    context->pkt_timebase = ic->streams[stream]->time_base;

    /* Look for supported hardware accelerated configurations */
    i = 0;
    while (!context->hw_device_ctx &&
           (config = avcodec_get_hw_config(codec, i++)) != NULL) {
#if 0
        SDL_Log("Found %s hardware acceleration with pixel format %s\n", av_hwdevice_get_type_name(config->device_type), av_get_pix_fmt_name(config->pix_fmt));
#endif

        if (!(config->methods & AV_CODEC_HW_CONFIG_METHOD_HW_DEVICE_CTX) ||
            !SupportedPixelFormat(p, config->pix_fmt)) {
            continue;
        }

        type = AV_HWDEVICE_TYPE_NONE;
        while (!context->hw_device_ctx &&
               (type = av_hwdevice_iterate_types(type)) != AV_HWDEVICE_TYPE_NONE) {
            if (type != config->device_type) {
                continue;
            }

#ifdef __WIN32__
            if (type == AV_HWDEVICE_TYPE_D3D11VA) {
                AVD3D11VADeviceContext *device_context;

                context->hw_device_ctx = av_hwdevice_ctx_alloc(type);

                device_context = (AVD3D11VADeviceContext *)((AVHWDeviceContext *)context->hw_device_ctx->data)->hwctx;
                device_context->device = d3d11_device;
                ID3D11Device_AddRef(device_context->device);
                device_context->device_context = d3d11_context;
                ID3D11DeviceContext_AddRef(device_context->device_context);

                result = av_hwdevice_ctx_init(context->hw_device_ctx);
                if (result < 0) {
                    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create hardware device context: %s", av_err2str(result));
                } else {
                    SDL_Log("Using %s hardware acceleration with pixel format %s\n", av_hwdevice_get_type_name(config->device_type), av_get_pix_fmt_name(config->pix_fmt));
                }
            } else
#endif
            {
                result = av_hwdevice_ctx_create(&context->hw_device_ctx, type, NULL, NULL, 0);
                if (result < 0) {
                    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create hardware device context: %s", av_err2str(result));
                } else {
                    SDL_Log("Using %s hardware acceleration with pixel format %s\n", av_hwdevice_get_type_name(config->device_type), av_get_pix_fmt_name(config->pix_fmt));
                }
            }
        }
    }

    /* Allow supported hardware accelerated pixel formats */
    context->opaque     = p;
    context->get_format = GetSupportedPixelFormat;

    result = avcodec_open2(context, codec, NULL);
    if (result < 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't open codec %s: %s", avcodec_get_name(context->codec_id), av_err2str(result));
        avcodec_free_context(&context);
        return NULL;
    }

    //SDL_SetWindowSize(window, codecpar->width, codecpar->height);

    return context;
}

static AVCodecContext *OpenAudioStream(test_player* p, AVFormatContext *ic, int stream, const AVCodec *codec)
{
    AVStream *st = ic->streams[stream];
    AVCodecParameters *codecpar = st->codecpar;
    AVCodecContext *context;
    int result;

    SDL_Log("Audio stream: %s %d channels, %d Hz\n", avcodec_get_name(codec->id), codecpar->ch_layout.nb_channels, codecpar->sample_rate);

    context = avcodec_alloc_context3(NULL);
    if (!context) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "avcodec_alloc_context3 failed\n");
        return NULL;
    }

    result = avcodec_parameters_to_context(context, ic->streams[stream]->codecpar);
    if (result < 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "avcodec_parameters_to_context failed: %s\n", av_err2str(result));
        avcodec_free_context(&context);
        return NULL;
    }
    context->pkt_timebase = ic->streams[stream]->time_base;

    result = avcodec_open2(context, codec, NULL);
    if (result < 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't open codec %s: %s", avcodec_get_name(context->codec_id), av_err2str(result));
        avcodec_free_context(&context);
        return NULL;
    }

    SDL_AudioSpec spec = { SDL_AUDIO_F32, codecpar->ch_layout.nb_channels, codecpar->sample_rate };
    p->audio = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_OUTPUT, &spec, NULL, NULL);
    if (p->audio) {
        SDL_ResumeAudioDevice(SDL_GetAudioStreamDevice(p->audio));
    } else {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't open audio: %s", SDL_GetError());
    }
    return context;
}

static SDL_AudioFormat GetAudioFormat(enum AVSampleFormat format)
{
    switch (format) {
    case AV_SAMPLE_FMT_U8:
    case AV_SAMPLE_FMT_U8P:
        return SDL_AUDIO_U8;
    case AV_SAMPLE_FMT_S16:
    case AV_SAMPLE_FMT_S16P:
        return SDL_AUDIO_S16;
    case AV_SAMPLE_FMT_S32:
    case AV_SAMPLE_FMT_S32P:
        return SDL_AUDIO_S32;
    case AV_SAMPLE_FMT_FLT:
    case AV_SAMPLE_FMT_FLTP:
        return SDL_AUDIO_F32;
    default:
        /* Unsupported */
        return 0;
    }
}

static SDL_bool IsPlanarAudioFormat(enum AVSampleFormat format)
{
    switch (format) {
    case AV_SAMPLE_FMT_U8P:
    case AV_SAMPLE_FMT_S16P:
    case AV_SAMPLE_FMT_S32P:
    case AV_SAMPLE_FMT_FLTP:
    case AV_SAMPLE_FMT_DBLP:
    case AV_SAMPLE_FMT_S64P:
        return SDL_TRUE;
    default:
        return SDL_FALSE;
    }
}

static void InterleaveAudio(test_player* p, AVFrame *frame, const SDL_AudioSpec *spec)
{
    int c, n;
    int samplesize = SDL_AUDIO_BYTESIZE(spec->format);
    int framesize = SDL_AUDIO_FRAMESIZE(*spec);
    Uint8 *data = (Uint8 *)SDL_malloc(frame->nb_samples * framesize);
    if (!data) {
        return;
    }

    /* This could be optimized with SIMD and not allocating memory each time */
    for (c = 0; c < spec->channels; ++c) {
        const Uint8 *src = frame->data[c];
        Uint8 *dst = data + c * samplesize;
        for (n = frame->nb_samples; n--; ) {
            SDL_memcpy(dst, src, samplesize);
            src += samplesize;
            dst += framesize;
        }
    }
    SDL_PutAudioStreamData(p->audio, data, frame->nb_samples * framesize);
    SDL_free(data);
}

static void HandleAudioFrame(test_player* p, AVFrame *frame)
{
    if (p->audio) {
        SDL_AudioSpec spec = { GetAudioFormat((enum AVSampleFormat)frame->format), frame->ch_layout.nb_channels, frame->sample_rate };
        SDL_SetAudioStreamFormat(p->audio, &spec, NULL);

        if (frame->ch_layout.nb_channels > 1 && IsPlanarAudioFormat((enum AVSampleFormat)frame->format)) {
            InterleaveAudio(p, frame, &spec);
        } else {
            SDL_PutAudioStreamData(p->audio, frame->data[0], frame->nb_samples * SDL_AUDIO_FRAMESIZE(spec));
        }
    }
}

static void SetYUVConversionMode(AVFrame *frame)
{
    SDL_YUV_CONVERSION_MODE mode = SDL_YUV_CONVERSION_AUTOMATIC;
    if (frame && (frame->format == AV_PIX_FMT_YUV420P || frame->format == AV_PIX_FMT_YUYV422 || frame->format == AV_PIX_FMT_UYVY422)) {
        if (frame->color_range == AVCOL_RANGE_JPEG)
            mode = SDL_YUV_CONVERSION_JPEG;
        else if (frame->colorspace == AVCOL_SPC_BT709)
            mode = SDL_YUV_CONVERSION_BT709;
        else if (frame->colorspace == AVCOL_SPC_BT470BG || frame->colorspace == AVCOL_SPC_SMPTE170M)
            mode = SDL_YUV_CONVERSION_BT601;
    }
    SDL_SetYUVConversionMode(mode); /* FIXME: no support for linear transfer */
}

static SDL_bool GetTextureForMemoryFrame(test_player* p, AVFrame *frame, SDL_Texture **texture)
{
    int texture_width = 0, texture_height = 0;
    Uint32 texture_format = SDL_PIXELFORMAT_UNKNOWN;
    Uint32 frame_format = GetTextureFormat((enum AVPixelFormat)frame->format);

    if (*texture) {
        SDL_QueryTexture(*texture, &texture_format, NULL, &texture_width, &texture_height);
    }
    if (!*texture || frame_format != texture_format || frame->width != texture_width || frame->height != texture_height) {
        if (*texture) {
            SDL_DestroyTexture(*texture);
        }

        *texture = SDL_CreateTexture(p->renderer, frame_format, SDL_TEXTUREACCESS_STREAMING, frame->width, frame->height);
        if (!*texture) {
            return SDL_FALSE;
        }
    }

    switch (frame_format) {
    case SDL_PIXELFORMAT_IYUV:
        if (frame->linesize[0] > 0 && frame->linesize[1] > 0 && frame->linesize[2] > 0) {
            SDL_UpdateYUVTexture(*texture, NULL, frame->data[0], frame->linesize[0],
                                                   frame->data[1], frame->linesize[1],
                                                   frame->data[2], frame->linesize[2]);
        } else if (frame->linesize[0] < 0 && frame->linesize[1] < 0 && frame->linesize[2] < 0) {
            SDL_UpdateYUVTexture(*texture, NULL, frame->data[0] + frame->linesize[0] * (frame->height                    - 1), -frame->linesize[0],
                                                   frame->data[1] + frame->linesize[1] * (AV_CEIL_RSHIFT(frame->height, 1) - 1), -frame->linesize[1],
                                                   frame->data[2] + frame->linesize[2] * (AV_CEIL_RSHIFT(frame->height, 1) - 1), -frame->linesize[2]);
        }
        SetYUVConversionMode(frame);
        break;
    default:
        if (frame->linesize[0] < 0) {
            SDL_UpdateTexture(*texture, NULL, frame->data[0] + frame->linesize[0] * (frame->height - 1), -frame->linesize[0]);
        } else {
            SDL_UpdateTexture(*texture, NULL, frame->data[0], frame->linesize[0]);
        }
        break;
    }
    return SDL_TRUE;
}

static SDL_bool GetTextureForDRMFrame(test_player* p, AVFrame *frame, SDL_Texture **texture)
{
#ifdef HAVE_EGL
    const AVDRMFrameDescriptor *desc = (const AVDRMFrameDescriptor *)frame->data[0];
    int i, j, image_index, num_planes;
    EGLDisplay display = eglGetCurrentDisplay();

    /* FIXME: Assuming NV12 data format */
    num_planes = 0;
    for (i = 0; i < desc->nb_layers; ++i) {
        num_planes += desc->layers[i].nb_planes;
    }
    if (num_planes != 2) {
        SDL_SetError("Expected NV12 frames with 2 planes, instead got %d planes", num_planes);
        return SDL_FALSE;
    }

    if (*texture) {
        /* Free the previous texture now that we're about to render a new one */
        SDL_DestroyTexture(*texture);
    } else {
        /* First time set up for NV12 textures */
        SDL_SetHint("SDL_RENDER_OPENGL_NV12_RG_SHADER", "1");

        SetYUVConversionMode(frame);
    }

    *texture = SDL_CreateTexture(p->renderer, SDL_PIXELFORMAT_NV12, SDL_TEXTUREACCESS_STATIC, frame->width, frame->height);
    if (!*texture) {
        return SDL_FALSE;
    }

//    float brightness = 1.0;
//    SDL_SetTextureColorMod(*texture, brightness * 255, brightness * 255, brightness * 255);

    /* Bind the texture for importing */
    SDL_GL_BindTexture(*texture, NULL, NULL);

    /* import the frame into OpenGL */
    image_index = 0;
    for (i = 0; i < desc->nb_layers; ++i) {
        const AVDRMLayerDescriptor *layer = &desc->layers[i];
        for (j = 0; j < layer->nb_planes; ++j) {
            static const uint32_t formats[ 2 ] = { DRM_FORMAT_R8, DRM_FORMAT_GR88 };
            const AVDRMPlaneDescriptor *plane = &layer->planes[j];
            const AVDRMObjectDescriptor *object = &desc->objects[plane->object_index];
            EGLAttrib img_attr[] = {
                EGL_LINUX_DRM_FOURCC_EXT,      formats[i],
                EGL_WIDTH,                     frame->width  / ( image_index + 1 ),  /* half size for chroma */
                EGL_HEIGHT,                    frame->height / ( image_index + 1 ),
                EGL_DMA_BUF_PLANE0_FD_EXT,     object->fd,
                EGL_DMA_BUF_PLANE0_OFFSET_EXT, plane->offset,
                EGL_DMA_BUF_PLANE0_PITCH_EXT,  plane->pitch,
                EGL_NONE
            };
            // 根据desc创建pImage图片，此时pImage就是我们解码后的数据，因为上面有：desc = (const AVDRMFrameDescriptor *)frame->data[0];
            EGLImage pImage = eglCreateImage(display, EGL_NO_CONTEXT, EGL_LINUX_DMA_BUF_EXT, NULL, img_attr);

            // 激活纹理单元，激活纹理单元意味着选择当前正在操作的纹理单元，然后你可以在这个纹理单元上执行各种纹理相关的操作，比如绑定纹理、设置纹理参数、上传纹理数据等。
            p->glActiveTextureARBFunc(GL_TEXTURE0_ARB + image_index);
            // 将pImage的数据上传到GL_TEXTURE_2D纹理
            p->glEGLImageTargetTexture2DOESFunc(GL_TEXTURE_2D, pImage);
            ++image_index;
        }
    }

    SDL_GL_UnbindTexture(*texture);

    return SDL_TRUE;
#else
    return SDL_FALSE;
#endif
}

static SDL_bool GetTextureForVAAPIFrame(test_player* p, AVFrame *frame, SDL_Texture **texture)
{
    AVFrame *drm_frame;
    SDL_bool result = SDL_FALSE;

    drm_frame = av_frame_alloc();
    if (drm_frame) {
        drm_frame->format = AV_PIX_FMT_DRM_PRIME;
        if (av_hwframe_map(drm_frame, frame, 0) == 0) {
            result = GetTextureForDRMFrame(p, drm_frame, texture);
        } else {
            SDL_SetError("Couldn't map hardware frame");
        }
        av_frame_free(&drm_frame);
    } else {
        SDL_OutOfMemory();
    }
    return result;
}

static SDL_bool GetTextureForD3D11Frame(AVFrame *frame, SDL_Texture **texture)
{
#ifdef __WIN32__
    int texture_width = 0, texture_height = 0;
    ID3D11Texture2D *pTexture = (ID3D11Texture2D *)frame->data[0];
    UINT iSliceIndex = (UINT)(uintptr_t)frame->data[1];

    D3D11_TEXTURE2D_DESC desc;
    SDL_zero(desc);
    ID3D11Texture2D_GetDesc(pTexture, &desc);
    if (desc.Format != DXGI_FORMAT_NV12) {
        SDL_SetError("Unsupported texture format, expected DXGI_FORMAT_NV12, got %d", desc.Format);
        return SDL_FALSE;
    }

    if (*texture) {
        SDL_QueryTexture(*texture, NULL, NULL, &texture_width, &texture_height);
    }
    if (!*texture || (UINT)texture_width != desc.Width || (UINT)texture_height != desc.Height) {
        if (*texture) {
            SDL_DestroyTexture(*texture);
        } else {
            /* First time set up for NV12 textures */
            SetYUVConversionMode(frame);
        }

        *texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_NV12, SDL_TEXTUREACCESS_STATIC, desc.Width, desc.Height);
        if (!*texture) {
            return SDL_FALSE;
        }
    }

    IDXGIResource *dxgi_resource = SDL_GetTextureDXGIResource(*texture);
    if (!dxgi_resource) {
        return SDL_FALSE;
    }

    ID3D11Resource *dx11_resource = NULL;
    HRESULT result = IDXGIResource_QueryInterface(dxgi_resource, &SDL_IID_ID3D11Resource, (void **)&dx11_resource);
    IDXGIResource_Release(dxgi_resource);
    if (FAILED(result)) {
        SDL_SetError("Couldn't get texture ID3D11Resource interface: 0x%x", result);
        return SDL_FALSE;
    }
    ID3D11DeviceContext_CopySubresourceRegion(d3d11_context, dx11_resource, 0, 0, 0, 0, (ID3D11Resource *)pTexture, iSliceIndex, NULL);
    ID3D11Resource_Release(dx11_resource);

    return SDL_TRUE;
#else
    return SDL_FALSE;
#endif
}

static SDL_bool GetTextureForFrame(test_player* p, AVFrame *frame, SDL_Texture **texture)
{
    switch (frame->format) {
    case AV_PIX_FMT_VAAPI:
        return GetTextureForVAAPIFrame(p, frame, texture);
    case AV_PIX_FMT_DRM_PRIME:
        return GetTextureForDRMFrame(p, frame, texture);
    case AV_PIX_FMT_D3D11:
        return GetTextureForD3D11Frame(frame, texture);
    default:
        return GetTextureForMemoryFrame(p, frame, texture);
    }
}


static void DisplayVideoTexture(test_player* p, AVFrame *frame)
{
    /* Update the video texture */
    GetTextureForFrame(p, frame, &p->video_texture);

    // 上面已经将AVFrame数据通过egl+opengl弄到video_texture，那么可以将纹理弄到渲染器
    if (frame->linesize[0] < 0) {
        SDL_RenderTextureRotated(p->renderer, p->video_texture, NULL, NULL, 0.0, NULL, SDL_FLIP_VERTICAL);
    } else {
        SDL_RenderTexture(p->renderer, p->video_texture, NULL, NULL);
    }
}

static void DisplayVideoToolbox(AVFrame *frame)
{
#ifdef __APPLE__
    SDL_Rect viewport;
    SDL_GetRenderViewport(renderer, &viewport);
    DisplayVideoToolboxFrame(renderer, frame->data[3], 0, 0, frame->width, frame->height, viewport.x, viewport.y, viewport.w, viewport.h);
#endif
}


static void DisplayVideoFrame(test_player* p, AVFrame *frame)
{
    switch (frame->format) {
    case AV_PIX_FMT_VIDEOTOOLBOX:
        DisplayVideoToolbox(frame);
        break;
    default:
        DisplayVideoTexture(p, frame);
        break;
    }
}

static void HandleVideoFrame(test_player* p, AVFrame *frame, double pts)
{
    /* Quick and dirty PTS handling */
    if (!p->video_start) {
        p->video_start = SDL_GetTicks();
    }
    double now = (double)(SDL_GetTicks() - p->video_start) / 1000.0;
    while (now < pts - 0.001) {
        SDL_Delay(1);
        now = (double)(SDL_GetTicks() - p->video_start) / 1000.0;
    }

    DisplayVideoFrame(p, frame);

    /* Render any bouncing balls */
    //MoveSprite();

    SDL_RenderPresent(p->renderer);
}

test_player* ff_create_player(){
    test_player *p = (test_player*)malloc(sizeof(test_player));
    if(!p)
        return NULL;

    // ffmpeg
    p->ic = NULL;
    p->audio_stream = -1;
    p->video_stream = -1;
    p->audio_decoder = NULL;
    p->video_decoder = NULL;
    p->audio_context = NULL;
    p->video_context = NULL;

    // sdl
    p->window = NULL;
    p->renderer = NULL;
    p->audio = NULL;
    p->video_texture = NULL;
    p->video_start = 0;
    p->software_only = (SDL_bool)0;
    p->has_eglCreateImage = (SDL_bool)0;
#ifdef HAVE_EGL
    p->has_EGL_EXT_image_dma_buf_import = (SDL_bool)0;
    p->glActiveTextureARBFunc = NULL;
    p->glEGLImageTargetTexture2DOESFunc = NULL;
#endif
    /* play thread */
    p->play_tid = NULL;
    /* play handle */
    p->hwnd = NULL;

    /* abort play */
    p->done = 0;

    return p;
}

void ff_free_player(test_player* p){
    if(p){
        free(p);
        p = NULL;
    }
}

int ff_play_thread_player(void* arg){
    static int is_init_sdl = 0;
    int return_code = -1;
    Uint32 window_flags;
    int result = -1;

    AVPacket *pkt = NULL;
    AVFrame *frame = NULL;
    SDL_bool flushing = SDL_FALSE;
    SDL_bool decoded = SDL_FALSE;
    double first_pts = -1.0;

    test_player *p = (test_player *)arg;

//    if(!is_init_sdl){
        if (SDL_Init(SDL_INIT_AUDIO|SDL_INIT_VIDEO) < 0) {
            return_code = 2;
            goto quit;
        }
        is_init_sdl = 1;
//    }

    window_flags = SDL_WINDOW_HIDDEN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY;
#ifdef __APPLE__
    window_flags |= SDL_WINDOW_METAL;
#elif !defined(__WIN32__)
    window_flags |= SDL_WINDOW_OPENGL;
#endif
#ifdef HAVE_EGL
    /* Try to create an EGL compatible window for DRM hardware frame support */
    if (!p->window) {
        CreateWindowAndRenderer(p, window_flags, "opengles2");
    }
#endif
#ifdef __APPLE__
    if (!window) {
        CreateWindowAndRenderer(window_flags, "metal");
    }
#endif
#ifdef __WIN32__
    if (!window) {
        CreateWindowAndRenderer(window_flags, "direct3d11");
    }
#endif
//    if (!p->window) {
//        if (!CreateWindowAndRenderer(p, window_flags, NULL)) {
//            return_code = 2;
//            goto quit;
//        }
//    }

    if (SDL_SetWindowTitle(p->window, file) < 0) {
        SDL_Log("SDL_SetWindowTitle: %s", SDL_GetError());
    }

    /* Open the media file */
    result = avformat_open_input(&p->ic, file, NULL, NULL);
    if (result < 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't open %s: %d", file, result);
        return_code = 4;
        goto quit;
    }
    p->video_stream = av_find_best_stream(p->ic, AVMEDIA_TYPE_VIDEO, -1, -1, &p->video_decoder, 0);
    if (p->video_stream >= 0) {
        p->video_context = OpenVideoStream(p, p->ic, p->video_stream, p->video_decoder);
        if (!p->video_context) {
            return_code = 4;
            goto quit;
        }
    }
    p->audio_stream = av_find_best_stream(p->ic, AVMEDIA_TYPE_AUDIO, -1, p->video_stream, &p->audio_decoder, 0);
    if (p->audio_stream >= 0) {
        p->audio_context = OpenAudioStream(p, p->ic, p->audio_stream, p->audio_decoder);
        if (!p->audio_context) {
            return_code = 4;
            goto quit;
        }
    }

    pkt = av_packet_alloc();
    if (!pkt) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "av_packet_alloc failed");
        return_code = 4;
        goto quit;
    }
    frame = av_frame_alloc();
    if (!frame) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "av_frame_alloc failed");
        return_code = 4;
        goto quit;
    }

    /* We're ready to go! */
    SDL_ShowWindow(p->window);

    /* Main render loop */
    p->done = 0;

    while (!p->done) {
        SDL_Event event;

        /* Check for events */
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT || event.type == SDL_EVENT_KEY_DOWN) {
                p->done = 1;
            }
        }

        if (!flushing) {
            result = av_read_frame(p->ic, pkt);
            if (result < 0) {
                SDL_Log("End of stream, finishing decode\n");
                if (p->audio_context) {
                    avcodec_flush_buffers(p->audio_context);
                }
                if (p->video_context) {
                    avcodec_flush_buffers(p->video_context);
                }
                flushing = SDL_TRUE;
            } else {
                if (pkt->stream_index == p->audio_stream) {
                    result = avcodec_send_packet(p->audio_context, pkt);
                    if (result < 0) {
                        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "avcodec_send_packet(audio_context) failed: %s", av_err2str(result));
                    }
                } else if (pkt->stream_index == p->video_stream) {
                    result = avcodec_send_packet(p->video_context, pkt);
                    if (result < 0) {
                        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "avcodec_send_packet(video_context) failed: %s", av_err2str(result));
                    }
                }
                av_packet_unref(pkt);
            }
        }

        decoded = SDL_FALSE;
        if (p->audio_context) {
            while (avcodec_receive_frame(p->audio_context, frame) >= 0) {
                HandleAudioFrame(p, frame);
                decoded = SDL_TRUE;
            }
            if (flushing) {
                /* Let SDL know we're done sending audio */
                SDL_FlushAudioStream(p->audio);
            }
        }
        if (p->video_context) {
            while (avcodec_receive_frame(p->video_context, frame) >= 0) {
                double pts = ((double)frame->pts * p->video_context->pkt_timebase.num) / p->video_context->pkt_timebase.den;
                if (first_pts < 0.0) {
                    first_pts = pts;
                }
                pts -= first_pts;

                HandleVideoFrame(p, frame, pts);
                decoded = SDL_TRUE;
            }
        } else {
            /* Update video rendering */
            SDL_SetRenderDrawColor(p->renderer, 0xA0, 0xA0, 0xA0, 0xFF);
            SDL_RenderClear(p->renderer);
//            MoveSprite();
            SDL_RenderPresent(p->renderer);
        }

        if (flushing && !decoded) {
            if (SDL_GetAudioStreamQueued(p->audio) > 0) {
                /* Wait a little bit for the audio to finish */
                SDL_Delay(10);
            } else {
                p->done = 1;
            }
        }
    }


    return_code = 0;
quit:
#ifdef __APPLE__
    CleanupVideoToolboxOutput();
#endif
#ifdef __WIN32__
    if (d3d11_context) {
        ID3D11DeviceContext_Release(d3d11_device);
        d3d11_context = NULL;
    }
    if (d3d11_device) {
        ID3D11Device_Release(d3d11_device);
        d3d11_device = NULL;
    }
#endif
//    SDL_free(positions);
//    SDL_free(velocities);
    av_frame_free(&frame);
    av_packet_free(&pkt);
    avcodec_free_context(&p->audio_context);
    avcodec_free_context(&p->video_context);
    avformat_close_input(&p->ic);
    SDL_DestroyRenderer(p->renderer);
    SDL_HideWindow(p->window);
    SDL_DestroyWindow(p->window);
    SDL_Quit();
//    SDLTest_CommonDestroyState(state);
    return return_code;
}

int ff_play_player(test_player* p, const char* file, void* hwnd){
    if(!p)
        return -1;

    p->hwnd = hwnd;
    p->play_tid = SDL_CreateThread(ff_play_thread_player, "play_thread", p);
    if(!p->play_tid){
        return -2;
    }

    return 0;
}

void ff_stop_player(test_player* p){
    p->done = 1;
    if(p->play_tid){
        SDL_WaitThread(p->play_tid, NULL);
        p->play_tid = NULL;
    }

    // reset default value

    // ffmpeg
    p->ic = NULL;
    p->audio_stream = -1;
    p->video_stream = -1;
    p->audio_decoder = NULL;
    p->video_decoder = NULL;
    p->audio_context = NULL;
    p->video_context = NULL;

    // sdl
    p->window = NULL;
    p->renderer = NULL;
    p->audio = NULL;
    p->video_texture = NULL;
    p->video_start = 0;
    p->software_only = (SDL_bool)0;
    p->has_eglCreateImage = (SDL_bool)0;
#ifdef HAVE_EGL
    p->has_EGL_EXT_image_dma_buf_import = (SDL_bool)0;
    p->glActiveTextureARBFunc = NULL;
    p->glEGLImageTargetTexture2DOESFunc = NULL;
#endif
    /* play thread */
    p->play_tid = NULL;
    /* play handle */
    p->hwnd = NULL;

    /* abort play */
    p->done = 0;
}
