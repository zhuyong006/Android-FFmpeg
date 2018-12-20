//
// Created by Administrator on 2018/12/17 0017.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/mman.h>
#include <linux/videodev2.h>
#include <sys/inotify.h>
#include "usb_v4l2.h"


int g_videofd = -1;        //设备描述符

struct v4l2_buffer buf;              //驱动中的一帧图像缓存，对应命令VIDIOC_QUERYBUF
struct frame_buf frame_buf;

//投放一个空的视频缓冲区到视频缓冲区输入队列中 ；
int usb_v4l2:: video_enqueue(int i)
{
    buf.index = i;
    if(ioctl(g_videofd, VIDIOC_QBUF, &buf) < 0){
        LOGW("enqueue failed");
        return -1;
    }

    return 0;
}
//摄像头设备信息查询及配置
int usb_v4l2:: init_V4L2(int w,int h)
{
    struct v4l2_capability cap ;//视频设备的功能，对应命令VIDIOC_QUERYCAP
    struct v4l2_fmtdesc stfmt; //当前视频支持的格式，对应命令VIDIOC_ENUM_FMT
    struct v4l2_format  fmt;   //当前驱动的频捕获格式式，对应命令VIDIOC_G_FMT、VIDIOC_S_FMT

    //1).打开摄像头设备
    g_videofd = open(VIDEO_DEV, O_RDWR);
    if(g_videofd == -1){
        LOGW("open dev failed");
        return -1;
    }

    //2).查询摄像头设备的基本信息及功能
    if(ioctl(g_videofd, VIDIOC_QUERYCAP, &cap) < 0){
        LOGW("ioctl failed");
        return -1;
    }else{

        LOGW("driver:\t\t%s\n",cap.driver);
        LOGW("card:\t\t%s\n",cap.card);
        LOGW("bus_info:\t%s\n",cap.bus_info);
        LOGW("version:\t%d\n",cap.version);
        LOGW("capabilities:\t%#x\n",cap.capabilities);
        if((cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) == V4L2_CAP_VIDEO_CAPTURE)
        {
            LOGW("Device %s: supports capture.\n", VIDEO_DEV);
        }

        if((cap.capabilities & V4L2_CAP_STREAMING) == V4L2_CAP_STREAMING)
        {
            LOGW("Device %s: supports streaming.\n", VIDEO_DEV);
        }
    }

    //3).列举摄像头所支持像素格式。
    memset(&stfmt, 0, sizeof(stfmt));
    stfmt.index = 0;
    stfmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    LOGW("device support:\n");
    while(ioctl(g_videofd, VIDIOC_ENUM_FMT, &stfmt) != -1){
        LOGW("\t%d:  %s \n\n", stfmt.index++, stfmt.description);
        //stfmt.index++;
    }

    //4).设置当前驱动的频捕获格式
    memset(&fmt, 0 ,sizeof(fmt));
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
    fmt.fmt.pix.height = w;
    fmt.fmt.pix.width = h;
    fmt.fmt.pix.field = V4L2_FIELD_INTERLACED;
    ioctl(g_videofd, VIDIOC_S_FMT, &fmt);
    LOGW("Size: %d,%d\n", fmt.fmt.pix.width, fmt.fmt.pix.height);
    LOGW("Video stored type: %d\n",fmt.fmt.pix.pixelformat);
    if(fmt.fmt.pix.height != 480){
        LOGW("%s,%d: Unable to set format\n",__func__,__LINE__);
        return -1;
    }

    return 0;
}


//采集初始化
int usb_v4l2:: gather_picture_init()
{
    struct v4l2_requestbuffers reqbuf;//申请帧缓存，对应命令VIDIOC_REQBUFS

    int i = 0;

    //1).向驱动申请帧缓存
    reqbuf.count = 4;// 缓存数量，也就是说在缓存队列里保持多少张照片
    reqbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    reqbuf.memory = V4L2_MEMORY_MMAP;
    if(ioctl(g_videofd, VIDIOC_REQBUFS, &reqbuf)==-1)
    {
        LOGW("VIDEO_REQBUFS");
        return -1;
    }

    //2).获取申请的每个缓存的信息，并mmap到用户空间
    for (i = 0; i < reqbuf.count; i++)
    {
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;
        if (ioctl (g_videofd, VIDIOC_QUERYBUF, &buf) == -1)
        {
            LOGW("query buffer error\n");
            return -1;
        }
        frame_buf.length[i] = buf.length;
        frame_buf.start[i] = mmap(NULL, buf.length, PROT_READ |PROT_WRITE, MAP_SHARED, g_videofd, buf.m.offset);
        if(frame_buf.start[i] == MAP_FAILED)
        {
            LOGW("buffer map error:%s,%d\n", __func__, __LINE__);
            return-1;
        }
        video_enqueue(i);//帧缓存入队
    }

    return 0;

}

//开始采集
int usb_v4l2:: gather_on()
{
    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if(ioctl(g_videofd, VIDIOC_STREAMON, &type) < 0){
        LOGW("stream on");
        return -1;
    }
    return 0;
}

//采集结束
int usb_v4l2:: gather_off()
{
    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if(ioctl(g_videofd, VIDIOC_STREAMOFF, &type) < 0){
        LOGW("stream on");
        return -1;
    }
    return 0;
}

//取出视频缓冲区的输出队列中取得一个已经
//保存有一帧视频数据的视频缓冲区；
int usb_v4l2:: video_getframe(int i)
{
    buf.index = i;
    if(ioctl(g_videofd, VIDIOC_DQBUF, &buf) < 0){
        LOGW("release failed");
        return -1;
    }
    return 0;
}
