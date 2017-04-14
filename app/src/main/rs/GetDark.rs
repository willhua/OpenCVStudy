#pragma version(1)
#pragma rs java_package_name(com.willhua.opencvstudy.rs)
#pragma rs_fp_relaxed




uchar RS_KERNEL getDarkChannel(uchar4 in)
{
    return in.r < in.g ? (in.r < in.b ? in.r : in.b) : (in.g < in.b ? in.g : in.b);
}