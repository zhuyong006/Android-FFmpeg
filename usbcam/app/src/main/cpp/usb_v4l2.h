//
// Created by Administrator on 2018/12/17 0017.
//

#ifndef TESTFFMPEG_USB_V4L2_H
#define TESTFFMPEG_USB_V4L2_H
#include <android/log.h>


class usb_v4l2 {
public:
    virtual int init_V4L2(int w,int h);
    virtual int gather_picture_init();
    virtual int gather_on();
    virtual int gather_off();
    virtual int video_getframe(int i);
    virtual int video_enqueue(int i);

private:
    int width;
    int height;
};

#define VIDEO_DEV    "/dev/video0"    //视频设备
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN,"testffmpeg",__VA_ARGS__)


struct frame_buf{  //一帧图像缓冲区
    int length[4];
    void * start[4];
};

extern struct frame_buf frame_buf;

#endif //TESTFFMPEG_USB_V4L2_H
