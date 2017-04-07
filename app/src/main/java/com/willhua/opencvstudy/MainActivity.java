package com.willhua.opencvstudy;

import android.app.Activity;
import android.content.res.AssetManager;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.os.Bundle;
import android.util.Log;
import android.widget.ImageView;

import java.io.FilterInputStream;
import java.io.IOException;
import java.io.InputStream;

import butterknife.BindView;
import butterknife.ButterKnife;

public class MainActivity extends Activity {
    private static final String TAG = "MainActivity";

    @BindView(R.id.before)
    ImageView mBeforeImage;
    @BindView(R.id.after)
    ImageView mAfterImage;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        ButterKnife.bind(this);
        new Thread(new Runnable() {
            @Override
            public void run() {
           /*     Bitmap bitmap = BitmapFactory.decodeResource(getResources(), R.drawable.pic);
                long time = System.currentTimeMillis();
                mBeforeImage.setImageBitmap(bitmap);
                int w = bitmap.getWidth(), h = bitmap.getHeight();
                int[] pix = new int[w * h];
                bitmap.getPixels(pix, 0, w, 0, 0, w, h);
                long time2 = System.currentTimeMillis();
                int [] resultPixes=OpenCVMethod.gray(pix,w,h);
                time2 = System.currentTimeMillis() - time2;
                Bitmap result = Bitmap.createBitmap(w,h, Bitmap.Config.RGB_565);
                result.setPixels(resultPixes, 0, w, 0, 0,w, h);
                Log.d(TAG, "opencvtime:" + time2 + " " + (System.currentTimeMillis() - time ) + "BITMAP:" + result.getWidth() + "*" + result.getHeight());
                mAfterImage.setImageBitmap(result);

                floatTest(); */
                //  img.setImageBitmap(result);

                AssetManager am = getAssets();
                try {
                    String file = "1000-667" + ".jpg";
                    InputStream fis = am.open(file);
                    Bitmap bitmap = BitmapFactory.decodeStream(fis);
                    fis.close();
                    fis = am.open(file);
                    Bitmap bitmap1 = BitmapFactory.decodeStream(fis);
                    mBeforeImage.setImageBitmap(bitmap1);
                 //   OpenCVMethod.dehazor(bitmap, bitmap.getWidth(), bitmap.getHeight());
                    OpenCVMethod.fastDehazor(bitmap, bitmap.getWidth(), bitmap.getHeight());
                    Log.d(TAG, "dehazor  " + bitmap.getWidth() + " *" + bitmap.getHeight());
                    mAfterImage.setImageBitmap(bitmap);
                    fis.close();
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        }).run();
    }

    native void floatTest();
}
