package com.willhua.opencvstudy;

import android.graphics.Bitmap;

/**
 * Created by willhua on 2017/3/11.
 */

public class OpenCVMethod {

    static {
        System.loadLibrary("OpenCV");
    }
    public static native int[] gray(int[] buf, int w, int h);
    public static native void dehazor(Bitmap bitmap, int width, int height);
}
