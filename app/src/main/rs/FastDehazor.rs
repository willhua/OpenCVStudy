#pragma version(1)
#pragma rs java_package_name(com.willhua.opencvstudy.rs)
#pragma rs_fp_relaxed


static const int WIN_RADIUS = 15;
static const int GUID_WIN_RADIUS = WIN_RADIUS * 4;

void init()
{

}

//
uchar RS_KERNEL GetDarkChannel(uchar4 in)
{
    return (in.r < in.g ) ? (in.r < in.b ? in.r : in.b) : (in.g < in.b ? in.g : in.b);
 /*   uchar out = in.r;
    if(out > in.g) out = in.g;
    if(out > in.b) out = in.b;
    return out;  */
}