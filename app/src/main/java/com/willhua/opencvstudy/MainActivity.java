package com.willhua.opencvstudy;

import android.app.Activity;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.content.res.AssetManager;
import android.database.Cursor;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.os.Bundle;
import android.os.Environment;
import android.Manifest.permission;
import android.provider.MediaStore;
import android.provider.Settings;
import android.renderscript.Allocation;
import android.renderscript.RenderScript;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.SeekBar;
import android.widget.TextView;
import android.widget.Toast;

import com.willhua.opencvstudy.rs.ScriptC_FastDehazor;
import com.willhua.opencvstudy.rs.ScriptC_Rgb2Yuv;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.concurrent.Executor;
import java.util.concurrent.Executors;
import java.util.concurrent.atomic.AtomicBoolean;

import butterknife.BindView;
import butterknife.ButterKnife;

import static android.renderscript.Allocation.USAGE_SCRIPT;
import static android.renderscript.Allocation.USAGE_SHARED;


public class MainActivity extends Activity {
    private static final String TAG = "MainActivity";
    private static final int PICK_PIC = 1111;

    @BindView(R.id.tv)
    TextView mInfo;
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
    @BindView(R.id.btn_auto)
    Button mBtnStart;
    @BindView(R.id.btn_cho)
    Button mBtnCho;


    AtomicBoolean mProssing = new AtomicBoolean(false);
    String mPrevFile;
    Executor mExecetor = Executors.newSingleThreadExecutor();
    float mP = 1.3f;
    int mRadius = 50;
    String mDirectory = Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DCIM) + "/去雾结果图";
    String mDirectoryAuto = Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DCIM) + "/自动去雾";
    String mBitmapFile = "1920-1080大山" + ".jpg";
    //String mBitmapFile = "1920-1080登山" + ".jpg";
    //String mBitmapFile = "1920-1080田野" + ".jpg";
    //String mBitmapFile = "1920-1080田野" + ".jpg";
    //String mBitmapFile = "1920-1080森林" + ".jpg";
    //String mBitmapFile = "4288-2848" + ".jpg";


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        ButterKnife.bind(this);
        fileCheck();
        viewSet();
        OpenCVMethod.initNative();
        /*
        new Thread(new Runnable() {
            @Override
            public void run() {
                //rgb2yuv();
                //fastDehazorRsTest();
                nativeAlgorithmTest();
            }
        }).start();   */
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        OpenCVMethod.freeNative();
    }

    /**
     *
     * @param stream
     * @param name   完整绝对路径
     */
    void nativeAlgorithmTest(InputStream stream, String name) {
        mProssing.set(true);
        try {
            mPrevFile = name;
            Bitmap bitmap = BitmapFactory.decodeStream(stream);
            stream.close();
            Bitmap bitmap1 = bitmap.copy(Bitmap.Config.ARGB_8888, true);
            setBeforeImage(bitmap1);
            Log.d(TAG, "dehazor start");
            //OpenCVMethod.dehazor(bitmap, bitmap.getWidth(), bitmap.getHeight());
            //OpenCVMethod.fastDehazor(bitmap, bitmap.getWidth(), bitmap.getHeight());
            //Log.d(TAG, "dehazor start  2");
            OpenCVMethod.fastDehazorCV(bitmap, bitmap.getWidth(), bitmap.getHeight(), mRadius, mP);
            Log.d(TAG, "dehazor  end " + bitmap.getWidth() + " *" + bitmap.getHeight());
            setAfterImage(bitmap);
            writeBitmapToFile(bitmap, name);
        } catch (IOException e) {
            e.printStackTrace();
        }
        mProssing.set(false);
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
        Bitmap bitmapout = Bitmap.createBitmap(bitmap.getWidth(), bitmap.getHeight(), Bitmap.Config.ARGB_8888);
        Log.d(TAG, "rs init");
        RenderScript renderScript = RenderScript.create(getApplication());
        ScriptC_FastDehazor scriptC_fastDehazor = new ScriptC_FastDehazor(renderScript);
        Log.d(TAG, "rs init 2");
        Allocation in = Allocation.createFromBitmap(renderScript, bitmap, Allocation.MipmapControl.MIPMAP_NONE, USAGE_SCRIPT);
        Allocation out = Allocation.createFromBitmap(renderScript, bitmapout, Allocation.MipmapControl.MIPMAP_NONE, USAGE_SCRIPT);
        Log.d(TAG, "rs start");
        //  scriptC_fastDehazor.invoke_fastProcess(in, out, bitmap.getWidth(), bitmap.getHeight());
        renderScript.finish();
        Log.d(TAG, "rs end");
        out.copyTo(bitmapout);
        Log.d(TAG, "rs copy end");
        setAfterImage(bitmapout);

        //the test for jni


    }


    native void floatTest();

    static {
        System.loadLibrary("OpenCV");
    }


    void fileCheck() {
        String[] permissions = new String[]{permission.WRITE_EXTERNAL_STORAGE, permission.READ_PHONE_STATE};
        boolean result = true;
        for (String p : permissions) {
            if (PackageManager.PERMISSION_GRANTED != checkSelfPermission(p)) {
                result = false;
                break;
            }
        }
        if (!result) {
            requestPermissions(new String[]{permission.WRITE_EXTERNAL_STORAGE}, 1);
        }

        File file = new File(mDirectory);
        if (!file.exists()) {
            try {
                boolean r = file.mkdirs();
                Log.d(TAG, "mkdir   " + r);
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
        file = new File(mDirectoryAuto);
        if (!file.exists()) {
            try {
                boolean r = file.mkdirs();
                Log.d(TAG, "mkdir   " + r);
            } catch (Exception e) {
                e.printStackTrace();
            }
        }

    }

    /**
     * 保存在源文件的同一路径，只是在后面增加时间和参数
     * @param bitmap
     * @param name   源文件的完整路径名
     */
    void writeBitmapToFile(final Bitmap bitmap, final String name) {

        boolean result = false;
        final File file = new File(name.substring(0,name.lastIndexOf(".")) + "_" + System.currentTimeMillis() + "_" + mRadius + "_" + mP + ".jpg");
        Log.d(TAG, "file " + file);
        if (bitmap != null) {
            try {
                if (!file.exists()) {
                    file.createNewFile();
                }
                result = bitmap.compress(Bitmap.CompressFormat.JPEG, 98, new FileOutputStream(file));
            } catch (Exception e) {
                Log.d(TAG, "write fail: " + e.getMessage());
                e.printStackTrace();
            }
        }
        if (!result) {
            runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    Toast.makeText(getApplicationContext(), "图片保存失败：" + file.getName(), Toast.LENGTH_SHORT).show();
                }
            });
        }

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

    void changUIStatus(final boolean b) {
        mSeekBarP1.setEnabled(b);
        mSeekBarP2.setEnabled(b);
        mBtnStart.setEnabled(b);
        mBtnCho.setEnabled(b);
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, final Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        if (requestCode == PICK_PIC && resultCode == RESULT_OK && data != null) {
            changUIStatus(false);
            mExecetor.execute(new Runnable() {
                @Override
                public void run() {
                    try {
                        runOnUiThread(new Runnable() {
                            @Override
                            public void run() {
                                Toast.makeText(getApplicationContext(), "处理中...", Toast.LENGTH_SHORT).show();
                                mInfo.setText("处理中...");
                            }
                        });
                        Cursor cursor = getContentResolver().query(data.getData(), new String[]{MediaStore.Images.Media.DATA}, null, null, null);
                        cursor.moveToFirst();
                        String file = cursor.getString(0);
                        nativeAlgorithmTest(new FileInputStream(new File(file)), file);
                    } catch (FileNotFoundException e) {
                        e.printStackTrace();
                    }
                    runOnUiThread(new Runnable() {
                        @Override
                        public void run() {
                            changUIStatus(true);
                            mInfo.setText("保存完成");
                        }
                    });
                }
            });

        }
    }

    boolean checkPicture(String name){
        name = name.toLowerCase();
        return name.endsWith(".jpg") || name.endsWith(".jpeg") || name.endsWith(".png");
    }

    void viewSet() {
        final float P_MAX = 2.0f;
        final float P_MIN = 0.5f;
        mSeekBarP1.setProgress((int)((mP - P_MIN) / (P_MAX - P_MIN) * 100));
        mInfoP1.setText(mP + "");
        mSeekBarP2.setProgress(mRadius / 2);
        mInfoP2.setText(mRadius + "");

        mBtnStart.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                changUIStatus(false);
                File file = new File(mDirectoryAuto);
                final File[] files = file.listFiles();
                if(files.length == 0)
                {
                    Toast.makeText(getApplicationContext(), "没找到图片", Toast.LENGTH_SHORT).show();
                    return;
                }
                Toast.makeText(getApplicationContext(), "开始处理。。。", Toast.LENGTH_SHORT).show();
                mInfo.setText("处理中...");
                mExecetor.execute(new Runnable() {
                    @Override
                    public void run() {
                        try {
                            for (File f : files) {
                                if (!f.isDirectory() && checkPicture(f.getAbsolutePath())) {
                                    nativeAlgorithmTest(new FileInputStream(f), f.getAbsolutePath());
                                }
                            }
                        } catch (Exception e) {

                        }
                        runOnUiThread(new Runnable() {
                            @Override
                            public void run() {
                                Toast.makeText(getApplicationContext(), "处理完成！", Toast.LENGTH_SHORT).show();
                                changUIStatus(true);
                                mInfo.setText("处理完成");
                            }
                        });
                    }
                });
            }
        });

        mBtnCho.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Intent intent = new Intent(Intent.ACTION_PICK, MediaStore.Images.Media.EXTERNAL_CONTENT_URI);
                startActivityForResult(intent, PICK_PIC);
            }
        });

        mSeekBarP1.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                mP = (P_MAX - P_MIN) * seekBar.getProgress() / 100 + P_MIN;
                mP = ((int)(mP * 100)) / 100f;
                mInfoP1.setText(mP + "");
                if(!mProssing.get() && !mPrevFile.isEmpty()){
                    mExecetor.execute(new Runnable() {
                        @Override
                        public void run() {
                            try {
                                nativeAlgorithmTest(new FileInputStream(new File(mPrevFile)), mPrevFile);
                            } catch (FileNotFoundException e) {
                                e.printStackTrace();
                            }
                        }
                    });
                }
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
            }
        });

        mSeekBarP2.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                mRadius = 2 * seekBar.getProgress();
                mInfoP2.setText(mRadius + "");
                if(!mProssing.get() && !mPrevFile.isEmpty()){
                    mExecetor.execute(new Runnable() {
                        @Override
                        public void run() {
                            try {
                                nativeAlgorithmTest(new FileInputStream(new File(mPrevFile)), mPrevFile);
                            } catch (FileNotFoundException e) {
                                e.printStackTrace();
                            }
                        }
                    });
                }
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {

            }
        });
    }

}
