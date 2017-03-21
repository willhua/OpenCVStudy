package com.willhua.opencvstudy;

/**
 * Created by willhua on 2017/3/11.
 */

public class OpenCVMethod {

    static {
        System.loadLibrary("OpenCV");
    }
    public static native int[] gray(int[] buf, int w, int h);
}
