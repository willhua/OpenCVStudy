#pragma version(1)
#pragma rs java_package_name(com.willhua.opencvstudy.rs)
#pragma rs_fp_relaxed

typedef struct Point{
    float x;
    float y;
} *point;

static int mW;
static int mH;
struct Point mPoint;
long mTime;

#pragma rs reduce(getSum) accumulator(getSumAccumulator) combiner(getSumCombiner)

static void getSumAccumulator(long *sum, uchar c){
    *sum += c;
}

static void getSumCombiner(long *sum, const long * c){
    *sum += c;
}

void init(){
    rsDebug("hi, rs init is call", 1);
}

void setParam(int w, int h){
    mW = w; mH = h;
}

static uchar getInvert(uchar c){
    return 255 - c;
}

uchar4 RS_KERNEL invert(uchar4 in, uint32_t x, uint32_t y){
    uchar4 outdata;
    outdata.r = 255 - in.r;
    outdata.g = 255 - in.g;
    outdata.b = getInvert(in.b);
    return outdata;
}