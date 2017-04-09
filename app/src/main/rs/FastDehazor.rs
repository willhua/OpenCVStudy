#pragma version(1)
#pragma rs java_package_name(com.willhua.opencvstudy.rs)
#pragma rs_fp_relaxed

#pragma rs reduce(getSum) accumulator(getSumAccumulator) combiner(getSumCombiner)

static const float MAX_P = 0.9f;
static const int WIN_RADIUS = 15;
static const int GUID_WIN_RADIUS = WIN_RADIUS * 4;

static rs_allocation gDarkChan;
static rs_allocation gMave;  //均值滤波的输出
static rs_allocation gAccum;  //均值滤波时的累加值
static float gMav;
static uint gAir;
static float gDivAir;
static rs_allocation gTable;
static rs_allocation gRawImage;
static rs_allocation gLx;
float gP;
int gWidth;
int gHeight;
int gRadius;


void init()
{

}

uchar RS_KERNEL getDarkChannel(uchar4 in)
{
    return (in.r < in.g ) ? (in.r < in.b ? in.r : in.b) : (in.g < in.b ? in.g : in.b);
}

int RS_KERNEL getRowSum(uint v, uint32_t x)
{
    int sum = 0;
    for(int i = 0; i < gWidth; ++i)
    {
        sum += rsGetElementAt_char(gDarkChan, x, i);
    }
    return sum;
}

int RS_KERNEL getColumnSum(uint v, uint32_t x)
{
    int sum = 0;
    for(int i = 0; i < gWidth; ++i)
    {
        sum += rsGetElementAt_char(gDarkChan, i, x);
    }
    return sum;
}

void RS_KERNEL accumColumn(uchar v, uint32_t x)
{
    uint sum = 0;
    for(int i = 0; i < gHeight; ++i)
    {
        sum += rsGetElementAt_uchar(gDarkChan, x, i);
        rsSetElementAt_uint(gAccum, sum, x, i);
    }
}

uint RS_KERNEL diffColumn(uint v, int x, int y)
{
    if(y < gRadius + 1)
    {
        return v;
    }
    else if (y < gHeight - gRadius - 1)
    {
        return rsGetElementAt_uint(gAccum, x, y + gRadius) - rsGetElementAt_uint(gAccum, x, y - gRadius - 1);
    }
    else
    {
        return rsGetElementAt_uint(gAccum, x, gHeight - 1) - rsGetElementAt_uint(gAccum, x, y - gRadius - 1);
    }
}

void RS_KERNEL accumRow(uint v, int x)
{
    uint sum = 0;
    for(int i = 0; i < gWidth; ++i)
    {
        //前面diff column的值放在gMave中
        sum += rsGetElementAt_uchar(gMave, i, x);
        rsSetElementAt_uint(gAccum, sum, i, x);
    }
}

uint RS_KERNEL diffRow(uint v, int x, int y)
{
    if(x < gRadius + 1)
    {
        return v;
    }
    else if (x < gWidth - gRadius - 1)
    {
        return rsGetElementAt_uint(gAccum, x + gRadius, y) - rsGetElementAt_uint(gAccum, x - gRadius - 1, y);
    }
    else
    {
        return rsGetElementAt_uint(gAccum, gWidth - 1, y) - rsGetElementAt_uint(gAccum, x - gRadius - 1, y);
    }
}

/* static void RS_KERNEL accumRow(uint v, int x)
{
    uint sum = 0;
    for(int i = 0; i < gWidth; ++i)
    {
        sum += rsGetElementAt_uchar(gDarkChan, i, x);
        rsSetElementAt_uint(gAccum, sum, i, x);
    }
} */

uchar RS_KERNEL getLx(float v1, uchar v2)
{
    v1 = v1 * gP;
    if(v1 < v2) return (uchar)v1;
    return v2;
}

uint RS_KERNEL getRowMax4(uint v, int x)
{
    uchar mv = 0;
    for(int i = 0; i < gWidth; ++i)
    {
        uchar4 uc4 = rsGetElementAt_uchar4(gRawImage, i, x);
        if(uc4.r > mv) mv = uc4.r;
        if(uc4.g > mv) mv = uc4.g;
        if(uc4.b > mv) mv = uc4.b;
    }
    return (uint)mv;
}

uint RS_KERNEL getRowMax(uint v, int x)
{
    uchar mv = 0;
    for(int i = 0; i < gWidth; ++i)
    {
        uchar uc = rsGetElementAt_uchar(gDarkChan, i, x);
        if(uc > mv) mv = uc;
    }
    return (uint)mv;
}

uchar RS_KERNEL getTable(uchar v, int x, int y)
{
    return (uchar)((x - y) / (1 - y * gDivAir));
}

uchar4 RS_KERNEL setResult(uchar4 in, uint32_t x, uint32_t y)
{
    uchar4 out;
    uchar lx = rsGetElementAt_uchar(gLx, x, y);
    out.r = rsGetElementAt_uchar(gTable, in.r, lx);
    out.g = rsGetElementAt_uchar(gTable, in.g, lx);
    out.b = rsGetElementAt_uchar(gTable, in.b, lx);
    out.a = in.a;
    return out;
}

static void getSumAccumulator(long *sum, uchar c)
{
    *sum += c;
}

static void getSumCombiner(long *sum, const long * c)
{
    *sum += c;
}

static void copy(rs_allocation des, rs_allocation src, int width, int height)
{
    rsAllocationCopy2DRange(des, 0, 0, 0, NULL, width, height, src, 0, 0, 0, NULL);
}

void fastProcess(rs_allocation inImage, int width, int height)
{
    //      const uint32_t imageWidth = rsAllocationGetDimX(inputImage);
    gRawImage = inImage;
    gWidth = width;
    gHeight = height;
    int LEN = gWidth * gHeight;
    gDarkChan = rsCreateAllocation_uchar(width, height);
    rsForEach(getDarkChannel, inImage, gDarkChan);   //获得暗通道darkChan
    rs_allocation rowSum = rsCreateAllocation_uint(height); //用来存储每一行的累加值
    rsForEach(getRowSum, rowSum, rowSum);  //获取暗通道的每一行的累加值到rowSum中
    long sum = 0;
    for(int i = 0; i < height; ++i)
    {
        sum += rsGetElementAt_int(rowSum, i);
    }
    gMav = sum / LEN / 255.0f; //求得暗通道的元素平均值mav
    gP = gP * gMav;
    if(gP > MAX_P) gP = MAX_P;

    //均值滤波
    gMave = rsCreateAllocation_uint(width, height);
    gAccum = rsCreateAllocation_uint(width, height);
    //copy(mave, gDarkChan, width, height);
    rs_allocation columns = rsCreateAllocation_uint(width);
    rsForEach(accumColumn, columns);  //y方向累加
    rsForEach(diffColumn, gAccum, gMave);  //y diff
    rsForEach(accumRow, rowSum);
    rsForEach(diffRow, gAccum, gMave);  //x diff

    //步骤5
    gLx = rsCreateAllocation_uchar(gWidth, gHeight);
    rsForEach(getLx, gMave, gDarkChan, gLx);


    //步骤6
    rsForEach(getRowMax4, rowSum, rowSum);
    uint max1 = 0;
    for(int i = 0; i < gHeight; ++i)
    {
        uint v = rsGetElementAt_uint(rowSum, i);
        if(v > max1) max1 = v;
    }
    rsForEach(getRowMax, rowSum, rowSum);
    uint max2 = 0;
    for(int i = 0; i < gHeight; ++i)
    {
        uint v = rsGetElementAt_uint(rowSum, i);
        if(v > max2) max2 = v;
    }
    gAir = (max1 + max2) / 2;
    gDivAir = 1.0f / gAir;
    gTable = rsCreateAllocation_uchar(256, 256);
    rsForEach(getTable, gTable, gTable);
    rsForEach(setResult, inImage, inImage);
}


