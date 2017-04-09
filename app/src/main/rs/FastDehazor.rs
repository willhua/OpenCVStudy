#pragma version(1)
#pragma rs java_package_name(com.willhua.opencvstudy.rs)
#pragma rs_fp_relaxed

#pragma rs reduce(getSum) accumulator(getSumAccumulator) combiner(getSumCombiner)


static const int WIN_RADIUS = 15;
static const int GUID_WIN_RADIUS = WIN_RADIUS * 4;

static rs_allocation gDarkChan;
int gWidth;
int gHeight;


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


static void getSumAccumulator(long *sum, uchar c)
{
    *sum += c;
}

static void getSumCombiner(long *sum, const long * c)
{
    *sum += c;
}

void fastProcess(rs_allocation inImage, rs_allocation outImage, int width, int height)
{
    //      const uint32_t imageWidth = rsAllocationGetDimX(inputImage);
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
    int mav = (int)(sum / LEN); //求得暗通道的元素平均值mav

}


