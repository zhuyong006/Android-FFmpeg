package com.sunmi.audio;

import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.media.MediaPlayer;
import android.os.Bundle;
import android.app.Activity;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.TextView;

public class MainActivity extends AppCompatActivity {
    //声名变量
    private Button start=null;
    private Button pause=null;
    private Button stop=null;
    private TextView state=null;
    private MediaPlayer mp3;
    private Boolean flag=false; //设置标记，false表示正在播放

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        //取得各按钮组件
        start=(Button) super.findViewById(R.id.start);
        pause=(Button) super.findViewById(R.id.pause);
        stop=(Button) super.findViewById(R.id.stop);
        state=(TextView)super.findViewById(R.id.state);
        //为每个按钮设置单击事件
        start.setOnClickListener(new OnClickListenerStart());
        pause.setOnClickListener(new OnClickListenerPause());
        stop.setOnClickListener(new OnClickListenerStop());
        mp3= new MediaPlayer();    //创建一个MediaPlayer对象
        //在res下新建一个raw文件夹把一首歌放到此文件夹中并用英文命名
        mp3 = MediaPlayer.create(MainActivity.this,R.raw.gem);
    }
    //各按钮单击事件的实现如下
    //开始播放
    private class OnClickListenerStart implements OnClickListener{
        //implementsOnClickListener为实现OnClickListener接口
        @Override
        //重写onClic事件
        public void onClick(View v)
        {
            //执行的代码，其中可能有异常。一旦发现异常，则立即跳到catch执行。否则不会执行catch里面的内容
            try
            {
                if (mp3!=null)
                {
                    mp3.stop();
                }
                mp3.prepare();         //进入到准备状态
                mp3.start();          //开始播放
                state.setText("Playing");  //改变输出信息为“Playing”，下同
            } catch (Exception e)
            {
                state.setText(e.toString());//以字符串的形式输出异常
                e.printStackTrace();  //在控制台（control）上打印出异常
            }
        }
    }
    //暂停播放
    private class OnClickListenerPause implements OnClickListener{
        @Override
        public void onClick(View v)
        {
            try
            {
                if (flag==false) //若flag为false，则表示此时播放器的状态为正在播放
                {
                    mp3.pause();
                    flag=true;
                    state.setText("pause");
                }
                else if(flag==true){
                    mp3.start();    //开始播放
                    flag=false;     //重新设置flag为false
                    state.setText("Playing");
                }
            } catch (Exception e)
            {
                state.setText(e.toString());
                e.printStackTrace();
            }
        }
    }
    //停止播放
    private class OnClickListenerStop implements OnClickListener{
        @Override
        public void onClick(View v)
        {
            try
            {
                if (mp3!=null)
                {
                    mp3.stop();
                    state.setText("stop");
                }
            } catch (Exception e)
            {
                state.setText(e.toString());
                e.printStackTrace();
            }
        }
    }
    //重写暂停状态事件
    protected void onPause(){
        try
        {
            mp3.release();   //释放音乐资源
        } catch (Exception e)
        {
            state.setText(e.toString());
            e.printStackTrace();
        }
        super.onPause();
    }
}
