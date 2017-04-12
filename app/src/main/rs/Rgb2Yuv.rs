#pragma version(1)
#pragma rs java_package_name(com.willhua.opencvstudy.rs)
#pragma rs_fp_relaxed

rs_allocation yr;
rs_allocation yg;
rs_allocation yb;
rs_allocation ur;
rs_allocation ub;
rs_allocation ug;
rs_allocation vr;
rs_allocation vb;
rs_allocation vg;

rs_allocation yuv;

int gwidth;
int gheight;

void initalloc()
{
    yr = rsCreateAllocation_uchar(256);
    yg = rsCreateAllocation_uchar(256);
    yb = rsCreateAllocation_uchar(256);
    ur = rsCreateAllocation_uchar(256);
    ug = rsCreateAllocation_uchar(256);
    ub = rsCreateAllocation_uchar(256);
    vr = rsCreateAllocation_uchar(256);
    vg = rsCreateAllocation_uchar(256);
    vb = rsCreateAllocation_uchar(256);
    yuv = rsCreateAllocation_uchar(1920, 1820 + (1820 >> 1));
}

void RS_KERNEL initTable(uchar v, int x)
{
    rsSetElementAt_uchar(yr, (uchar)((x * 1224) >> 12), x);
    rsSetElementAt_uchar(yg, (uchar)((x * 2404) >> 12), x);
    rsSetElementAt_uchar(yb, (uchar)((x * 467) >> 12), x);
    rsSetElementAt_uchar(ur, (uchar)((x * 602) >> 12), x);
    rsSetElementAt_uchar(ug, (uchar)((x * 1183) >> 12), x);
    rsSetElementAt_uchar(ub, (uchar)((x * 1785) >> 12), x);
    rsSetElementAt_uchar(vr, (uchar)((x * 2519) >> 12), x);
    rsSetElementAt_uchar(vg, (uchar)((x * 2109) >> 12), x);
    rsSetElementAt_uchar(vb, (uchar)((x * 409) >> 12), x);
}

void RS_KERNEL toYUV(uchar4 in, int x, int y)
{
    uchar yy = rsGetElementAt_uchar(yr, in.r) + rsGetElementAt_uchar(yg, in.g) + rsGetElementAt_uchar(yb, in.b);
    rsSetElementAt_uchar(yuv, yy, x, y);
    if((x & 1) == 0 && (y & 1) == 0)
    {
        uchar u = rsGetElementAt_uchar(ub, in.b) - rsGetElementAt_uchar(ur, in.r) - rsGetElementAt_uchar(ug, in.g);
        uchar v = rsGetElementAt_uchar(vr, in.r) - rsGetElementAt_uchar(vg, in.g) - rsGetElementAt_uchar(vb, in.b);
        y = y >> 1 + gheight;
        rsSetElementAt_uchar(yuv, x, y);
        rsSetElementAt_uchar(yuv, x + 1, y);
    }
}

void rgb2yuv(rs_allocation in, int w, int h)
{
    gwidth = w;
    gheight = h;
    rsDebug("initalloc ", 1);
    initalloc();
    rsDebug("initalloc ", 2);
    rs_allocation ta = rsCreateAllocation_uchar(256);
    rsForEach(initTable, ta);
    rsDebug("initalloc intitable end", 3);
    rsForEach(toYUV, in);
    rsDebug("initalloc toyuv end", 4);
}