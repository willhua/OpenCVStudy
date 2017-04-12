package com.willhua.opencvstudy;

import android.app.Activity;
import android.content.res.AssetManager;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.os.Bundle;
import android.renderscript.Allocation;
import android.renderscript.RenderScript;
import android.util.Log;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.SeekBar;
import android.widget.TextView;

import com.willhua.opencvstudy.rs.ScriptC_FastDehazor;
import com.willhua.opencvstudy.rs.ScriptC_Rgb2Yuv;

import java.io.IOException;
import java.io.InputStream;

import butterknife.BindView;
import butterknife.ButterKnife;

import static android.renderscript.Allocation.USAGE_SHARED;


public class MainActivity extends Activity {
    private static final String TAG = "MainActivity";

    @BindView(R.id.before)
    ImageView mBeforeImage;
    @BindView(R.id.after)
    ImageView mAfterImage;
    @BindView(R.id.param1)
    SeekBar mSeekBarP1;
    @BindView(R.id.info_param1)
    TextView mInfoP1;
    @BindView(R.id.param2)
    SeekBar mSeekBarP2;
    @BindView(R.id.info_param2)
    TextView mInfoP2;
    @BindView(R.id.btn)
    Button mBtnStart;

    String mBitmapFile = "1920-1080田野" + ".jpg";
    //String mBitmapFile = "1920-1080森林" + ".jpg";
    //String mBitmapFile = "haze" + ".jpg";
    //String mBitmapFile = "4288-2848" + ".jpg";


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        ButterKnife.bind(this);
        floatTest();
        new Thread(new Runnable() {
            @Override
            public void run() {
                //rgb2yuv();
                //fastDehazorRsTest();
                nativeAlgorithmTest();
            }
        }).start();
    }

    void setAfterImage(final Bitmap bitmap) {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                mAfterImage.setImageBitmap(bitmap);
            }
        });
    }

    void setBeforeImage(final Bitmap bitmap) {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                mBeforeImage.setImageBitmap(bitmap);
            }
        });
    }

    native void floatTest();

    static {
        System.loadLibrary("OpenCV");
    }

    void nativeAlgorithmTest() {
        try {
            AssetManager am = getAssets();
            InputStream fis = am.open(mBitmapFile);
            Bitmap bitmap = BitmapFactory.decodeStream(fis);
            fis.close();
            fis = am.open(mBitmapFile);
            Bitmap bitmap1 = BitmapFactory.decodeStream(fis);
            setBeforeImage(bitmap1);
            Log.d(TAG, "dehazor start");
            OpenCVMethod.dehazor(bitmap, bitmap.getWidth(), bitmap.getHeight());
            //OpenCVMethod.fastDehazor(bitmap, bitmap.getWidth(), bitmap.getHeight());
            Log.d(TAG, "dehazor start  2");
            //OpenCVMethod.fastDehazorCV(bitmap, bitmap.getWidth(), bitmap.getHeight(), 100);
            Log.d(TAG, "dehazor  end " + bitmap.getWidth() + " *" + bitmap.getHeight());
            setAfterImage(bitmap);
            fis.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    void rgb2yuv() {
        AssetManager am = getAssets();
        InputStream fis = null;
        Bitmap bitmap = null;
        try {
            fis = am.open(mBitmapFile);
            bitmap = BitmapFactory.decodeStream(fis);
            fis.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
        Log.d(TAG, " init  1");
        RenderScript renderScript = RenderScript.create(getApplication());
        Log.d(TAG, " init  2");
        ScriptC_Rgb2Yuv scriptC_rgb2Yuv = new ScriptC_Rgb2Yuv(renderScript);
        scriptC_rgb2Yuv.invoke_initEnv();
        Log.d(TAG, " init  3");
        Allocation in = Allocation.createFromBitmap(renderScript, bitmap, Allocation.MipmapControl.MIPMAP_NONE,
                USAGE_SHARED);
        Log.d(TAG, " init  4");
        in.copyFrom(bitmap);
        Log.d(TAG, " init  5");
        //scriptC_rgb2Yuv.invoke_rgb2yuv(in, bitmap.getWidth(), bitmap.getHeight());
        scriptC_rgb2Yuv.forEach_toYUV(in);
        renderScript.finish();
        Log.d(TAG, " init  6");

        Log.d(TAG, " init  400000000");
        in.copyFrom(bitmap);
        Log.d(TAG, " init  5");
        scriptC_rgb2Yuv.forEach_toYUV(in);
        //scriptC_rgb2Yuv.invoke_rgb2yuv(in, bitmap.getWidth(), bitmap.getHeight());
        renderScript.finish();
        Log.d(TAG, " init  6");

        Log.d(TAG, " init  40000000000");
        in.copyFrom(bitmap);
        Log.d(TAG, " init  5");
        scriptC_rgb2Yuv.forEach_toYUV(in);
        //scriptC_rgb2Yuv.invoke_rgb2yuv(in, bitmap.getWidth(), bitmap.getHeight());
        Log.d(TAG, " init  6");
        renderScript.finish();
        Log.d(TAG, " init  7");

        Log.d(TAG, " init  40000000000");
        in.copyFrom(bitmap);
        Log.d(TAG, " init  5");
        scriptC_rgb2Yuv.forEach_toYUV(in);
        //scriptC_rgb2Yuv.invoke_rgb2yuv(in, bitmap.getWidth(), bitmap.getHeight());
        Log.d(TAG, " init  6");
        renderScript.finish();
        Log.d(TAG, " init  7");


        Log.d(TAG, " init  40000000000");
        in.copyFrom(bitmap);
        Log.d(TAG, " init  5");
        scriptC_rgb2Yuv.forEach_toYUV(in);
        //scriptC_rgb2Yuv.invoke_rgb2yuv(in, bitmap.getWidth(), bitmap.getHeight());
        Log.d(TAG, " init  6");
        renderScript.finish();
        Log.d(TAG, " init  7");
    }

    void fastDehazorRsTest() {
        AssetManager am = getAssets();
        InputStream fis = null;
        Bitmap bitmap = null;
        try {
            fis = am.open(mBitmapFile);
            bitmap = BitmapFactory.decodeStream(fis);
            fis.close();
            setBeforeImage(bitmap);
        } catch (IOException e) {
            e.printStackTrace();
        }

        //the test code for rs
        Log.d(TAG, "rs init");
        RenderScript renderScript = RenderScript.create(getApplication());
        ScriptC_FastDehazor scriptC_fastDehazor = new ScriptC_FastDehazor(renderScript);
        Log.d(TAG, "rs init 2");
        Allocation in = Allocation.createFromBitmap(renderScript, bitmap, Allocation.MipmapControl.MIPMAP_NONE, USAGE_SHARED);
        Log.d(TAG, "rs start");
        //scriptC_fastDehazor.forEach_getDarkChannel(in, out);
        scriptC_fastDehazor.invoke_fastProcess(in, bitmap.getWidth(), bitmap.getHeight());
        renderScript.finish();
        Log.d(TAG, "rs end");
        in.copyTo(bitmap);
        Log.d(TAG, "rs copy end");
        setAfterImage(bitmap);

        //the test for jni


    }
}
