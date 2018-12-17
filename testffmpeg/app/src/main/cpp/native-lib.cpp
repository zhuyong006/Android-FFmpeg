#include <jni.h>
#include <string>
#include <android/log.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>

}

#define LOGW(...) __android_log_print(ANDROID_LOG_WARN,"testffmpeg",__VA_ARGS__)

#define srcWidth 640
#define srcHeight 480

#define outWidth 640
#define outHeight 480

//extern "C" 是不能少的，同时需要正确的返回函数的返回值，否则会出错
extern "C"
JNIEXPORT void JNICALL
Java_com_sunmi_testffmpeg_UsbCamera_start_1preview(JNIEnv *env, jobject instance, jobject surface) {


    // TODO
    FILE *src_file =fopen("/sdcard/yuv420.yuv", "rb");
    if(src_file == NULL)
        LOGW("yuv420.yuv open failed !");

    //设置输入视频文件的大小和视频格式
    const int src_w=srcWidth,src_h=srcHeight;

    AVPixelFormat src_pixfmt=AV_PIX_FMT_YUV420P;

    //获取输入视频文件每个像素占有的BIT数
    int src_bpp=av_get_bits_per_pixel(av_pix_fmt_desc_get(src_pixfmt));



    //设置输出图像的大小和格式
    const int dst_w=outWidth,dst_h=outHeight;

    AVPixelFormat dst_pixfmt=AV_PIX_FMT_RGBA;

    //获取输出视频文件每个像素占有的BIT数
    int dst_bpp=av_get_bits_per_pixel(av_pix_fmt_desc_get(dst_pixfmt));



    //Structures

    uint8_t *src_data[4];

    int src_linesize[4];



    uint8_t *dst_data[4];

    int dst_linesize[4];




    struct SwsContext *img_convert_ctx;
    //用于临时存放yuv420p文件的buf
    uint8_t *temp_buffer=(uint8_t *)new char[src_w*src_h*src_bpp/8];


    int frame_idx=0;

    int ret=0;
    /*通过输入宽和高以及像素格式分配输入缓冲，对于yuv420p的格式：
     * src_data[0]：代表Y通道；src_linesize[0]：对齐后的Y宽度，可能会大于输入的图像宽度
     * src_data[1]：代表U通道；src_linesize[1]：对齐后的U宽度
     * src_data[2]：代表V通道；src_linesize[2]：对齐后的V宽度
     * align：输入宽度需要对齐到多少
     */
    ret= av_image_alloc(src_data, src_linesize,src_w, src_h, src_pixfmt, 1);

    if (ret< 0) {

        LOGW( "Could not allocate source image\n");

        return ;

    }

    /*通过输出宽和高以及像素格式分配输入缓冲，对于RGBA的格式：
     * dst_data[0]：代表RGBA通道；dst_linesize[0]：对齐后的RGBA宽度(W*H*4)，可能会大于输入的图像宽度
     * dst_data[1]：RGBA是非平面数据
     * dst_data[2]：RGBA是非平面数据
     * align：输出宽度需要对齐到多少
     */
    ret = av_image_alloc(dst_data, dst_linesize,dst_w, dst_h, dst_pixfmt, 1);

    if (ret< 0) {

        LOGW( "Could not allocate destination image\n");

        return ;

    }


    //-----------------------------

    //Init Method 1
    //分配图像转换的上下文
    img_convert_ctx =sws_alloc_context();

    //Set Value
    //SWS_BICUBIC代表的是图像转换的其中一种算法
    av_opt_set_int(img_convert_ctx,"sws_flags",SWS_BICUBIC|SWS_PRINT_INFO,0);
    //设置图像转换输入源的宽
    av_opt_set_int(img_convert_ctx,"srcw",src_w,0);
    //设置图像转换输入源的高
    av_opt_set_int(img_convert_ctx,"srch",src_h,0);
    //设置图像转换输入源的格式
    av_opt_set_int(img_convert_ctx,"src_format",src_pixfmt,0);

    //'0' for MPEG (Y:0-235);'1' for JPEG (Y:0-255)
    /*
     * FFmpeg中可以通过使用av_opt_set()设置“src_range”和“dst_range”来设置输入和输出的YUV的取值范围。
     * 如果“dst_range”字段设置为“1”的话，则代表输出的YUV的取值范围遵循“jpeg”标准；
     * 如果“dst_range”字段设置为“0”的话，则代表输出的YUV的取值范围遵循“mpeg”标准。
     * 下面记录一下YUV的取值范围的概念。与RGB每个像素点的每个分量取值范围为0-255不同（每个分量占8bit），YUV取值范围有两种：
     * （1）以Rec.601为代表（还包括BT.709 / BT.2020）的广播电视标准中，Y的取值范围是16-235，U、V的取值范围是16-240。FFmpeg中称之为“mpeg”范围。
     * （2）以JPEG为代表的标准中，Y、U、V的取值范围都是0-255。FFmpeg中称之为“jpeg” 范围。
     */
    av_opt_set_int(img_convert_ctx,"src_range",1,0);
    //设置图像转换输出图像的宽
    av_opt_set_int(img_convert_ctx,"dstw",dst_w,0);
    //设置图像转换输出图像的高
    av_opt_set_int(img_convert_ctx,"dsth",dst_h,0);
    //设置图像转换输出图像的目标格式
    av_opt_set_int(img_convert_ctx,"dst_format",dst_pixfmt,0);
    //同上设置输出图像遵循Mjpeg还是Jpeg
    av_opt_set_int(img_convert_ctx,"dst_range",1,0);
    //初始化图像转换上下文
    sws_init_context(img_convert_ctx,NULL,NULL);



    //显示窗口初始化
    ANativeWindow *nwin = ANativeWindow_fromSurface(env,surface);
    //设置显示窗口的格式
    ANativeWindow_setBuffersGeometry(nwin,outWidth,outHeight,WINDOW_FORMAT_RGBA_8888);
    //定义窗口buf
    ANativeWindow_Buffer wbuf;

    while(1)

    {
        //从源图像中读取一帧的数据到temp_buffer中
        if (fread(temp_buffer, 1, src_w*src_h*src_bpp/8, src_file) != src_w*src_h*src_bpp/8){
            break;
        }



        switch(src_pixfmt){

            case AV_PIX_FMT_GRAY8:{

                memcpy(src_data[0],temp_buffer,src_w*src_h);

                break;

            }

            case AV_PIX_FMT_YUV420P:{
                //通道转换，YUV420P有三个通道需要转换
                memcpy(src_data[0],temp_buffer,src_w*src_h);                    //Y

                memcpy(src_data[1],temp_buffer+src_w*src_h,src_w*src_h/4);      //U

                memcpy(src_data[2],temp_buffer+src_w*src_h*5/4,src_w*src_h/4);  //V

                break;

            }

            case AV_PIX_FMT_YUV422P:{
                //通道转换，YUV422P有三个通道需要转换
                memcpy(src_data[0],temp_buffer,src_w*src_h);                    //Y

                memcpy(src_data[1],temp_buffer+src_w*src_h,src_w*src_h/2);      //U

                memcpy(src_data[2],temp_buffer+src_w*src_h*3/2,src_w*src_h/2);  //V

                break;

            }

            case AV_PIX_FMT_YUV444P:{

                memcpy(src_data[0],temp_buffer,src_w*src_h);                    //Y

                memcpy(src_data[1],temp_buffer+src_w*src_h,src_w*src_h);        //U

                memcpy(src_data[2],temp_buffer+src_w*src_h*2,src_w*src_h);      //V

                break;

            }

            case AV_PIX_FMT_YUYV422:{

                memcpy(src_data[0],temp_buffer,src_w*src_h*2);                  //Packed

                break;

            }

            case AV_PIX_FMT_RGB24:{

                memcpy(src_data[0],temp_buffer,src_w*src_h*3);                  //Packed

                break;

            }

            default:{

                printf("Not Support Input Pixel Format.\n");

                break;

            }

        }


        //将数据转换,YUV420P转换为RGBA格式
        sws_scale(img_convert_ctx, src_data, src_linesize, 0, src_h, dst_data, dst_linesize);

        LOGW("Finish process frame %5d\n",frame_idx);

        frame_idx++;

        ANativeWindow_lock(nwin,&wbuf,0);
        //wbuf.bits：对应的就是surface的buf
        uint8_t *dst = (uint8_t*)wbuf.bits;
        //因为RGBA是非平面的数据，所以转换后数据是存放在dst_data[0]中的
        memcpy(dst,dst_data[0],outWidth*outHeight*4);
        ANativeWindow_unlockAndPost(nwin);


    }



    sws_freeContext(img_convert_ctx);



    free(temp_buffer);
    av_freep(&src_data[0]);

    av_freep(&dst_data[0]);

    return;
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_com_sunmi_testffmpeg_MainActivity_open(JNIEnv *env, jobject instance, jstring url_,
                                            jobject handle) {
    const char *url = env->GetStringUTFChars(url_, 0);
    // TODO

    //Parameters



out:
    env->ReleaseStringUTFChars(url_, url);
    return JNI_TRUE;
}


//extern "C" 是不能少的，同时需要正确的返回函数的返回值，否则会出错
extern "C"
jstring
Java_com_sunmi_testffmpeg_MainActivity_stringFromJNI(
        JNIEnv* env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    hello += avcodec_configuration();
    return env->NewStringUTF(hello.c_str());
}
