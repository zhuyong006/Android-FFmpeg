package com.sunmi.player;

import android.content.Context;
import android.opengl.GLSurfaceView;
import android.util.AttributeSet;
import android.util.Log;
import android.view.SurfaceHolder;

/**
 * Created by Administrator on 2018/12/14 0014.
 */

public class UsbCamera extends GLSurfaceView implements Runnable,SurfaceHolder.Callback{

    public UsbCamera(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    @Override
    public void run() {
        boolean re = true;

        re = usb_camera_init();
        if(re == false) {
            Log.e("ffmpeg", "usb_camera_init failed");
            return ;
        }
        re = usb_camera_stream_on();
        if(re == false) {
            Log.e("ffmpeg", "usb_camera_stream_on failed");
            return ;
        }

        start_preview(getHolder().getSurface());
    }

    @Override
    public void surfaceCreated(SurfaceHolder var1){

        new Thread(this).start();

    }

    @Override
    public void surfaceChanged(SurfaceHolder var1, int var2, int var3, int var4){

    }

    @Override
    public void surfaceDestroyed(SurfaceHolder var1){

    }
    public native boolean usb_camera_init();
    public native boolean usb_camera_stream_on();
    public native void start_preview(Object surface);
}
