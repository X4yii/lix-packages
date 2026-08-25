#include "math.h"
#include <stdint.h>

double fabs(double x) {
    union { double d; uint64_t u; } u = { .d = x };
    u.u &= 0x7fffffffffffffffull;
    return u.d;
}

float fabsf(float x) {
    union { float f; uint32_t u; } u = { .f = x };
    u.u &= 0x7fffffffu;
    return u.f;
}

double trunc(double x) {
    if (isnan(x) || isinf(x)) return x;
    if (fabs(x) >= 4503599627370496.0) return x;
    return (double)((int64_t)x);
}

float truncf(float x) {
    if (isnan(x) || isinf(x)) return x;
    if (fabsf(x) >= 8388608.0f) return x;
    return (float)((int32_t)x);
}

double floor(double x) {
    double t = trunc(x);
    if (x < 0.0 && x != t) return t - 1.0;
    return t;
}

float floorf(float x) {
    float t = truncf(x);
    if (x < 0.0f && x != t) return t - 1.0f;
    return t;
}

double ceil(double x) {
    double t = trunc(x);
    if (x > 0.0 && x != t) return t + 1.0;
    return t;
}

float ceilf(float x) {
    float t = truncf(x);
    if (x > 0.0f && x != t) return t + 1.0f;
    return t;
}

double round(double x) {
    return (x >= 0.0) ? floor(x + 0.5) : ceil(x - 0.5);
}

float roundf(float x) {
    return (x >= 0.0f) ? floorf(x + 0.5f) : ceilf(x - 0.5f);
}

double sqrt(double x) {
    if (x < 0.0) return NAN;
    if (x == 0.0) return 0.0;
    double res;
    __asm__("sqrtsd %1, %0" : "=x"(res) : "x"(x));
    return res;
}

float sqrtf(float x) {
    if (x < 0.0f) return NAN;
    if (x == 0.0f) return 0.0f;
    float res;
    __asm__("sqrtss %1, %0" : "=x"(res) : "x"(x));
    return res;
}

double fmod(double x, double y) {
    if (y == 0.0 || isnan(x) || isnan(y) || isinf(x)) return NAN;
    if (isinf(y)) return x;
    double q = trunc(x / y);
    return x - q * y;
}

float fmodf(float x, float y) {
    if (y == 0.0f || isnan(x) || isnan(y) || isinf(x)) return NAN;
    if (isinf(y)) return x;
    float q = truncf(x / y);
    return x - q * y;
}

double hypot(double x, double y) {
    double ax = fabs(x);
    double ay = fabs(y);
    if (isinf(ax) || isinf(ay)) return INFINITY;
    return sqrt(ax * ax + ay * ay);
}

float hypotf(float x, float y) {
    float ax = fabsf(x);
    float ay = fabsf(y);
    if (isinf(ax) || isinf(ay)) return INFINITY;
    return sqrtf(ax * ax + ay * ay);
}

double cbrt(double x) {
    if (x == 0.0 || isinf(x) || isnan(x)) return x;
    int neg = (x < 0.0);
    double val = neg ? -x : x;
    double y = exp(log(val) / 3.0);
    y = (2.0 * y + val / (y * y)) / 3.0;
    return neg ? -y : y;
}

float cbrtf(float x) {
    return (float)cbrt((double)x);
}

static double kernel_sin(double x) {
    double x2 = x * x;
    return x * (1.0 + x2 * (-0.166666666666666324348 + x2 * (0.00833333333333224894612 + x2 * (-0.000198412698298579493134 + x2 * (0.00000275573137070700676789 - x2 * 0.000000025050760253406963414)))));
}

static double kernel_cos(double x) {
    double x2 = x * x;
    return 1.0 + x2 * (-0.5 + x2 * (0.0416666666666666019037 + x2 * (-0.00138888888888741095749 + x2 * (0.000024801587289478 + x2 * (-0.00000027557314351390 + x2 * 0.000000002087572321298)))));
}

double sin(double x) {
    if (isnan(x) || isinf(x)) return NAN;
    double rem = fmod(x, 2.0 * M_PI);
    if (rem < 0.0) rem += 2.0 * M_PI;
    if (rem > M_PI) {
        rem -= M_PI;
        if (rem > M_PI_2) return -kernel_cos(rem - M_PI_2);
        return -kernel_sin(rem);
    }
    if (rem > M_PI_2) return kernel_cos(rem - M_PI_2);
    return kernel_sin(rem);
}

float sinf(float x) {
    return (float)sin((double)x);
}

double cos(double x) {
    if (isnan(x) || isinf(x)) return NAN;
    return sin(x + M_PI_2);
}

float cosf(float x) {
    return (float)cos((double)x);
}

double tan(double x) {
    double c = cos(x);
    if (c == 0.0) return NAN;
    return sin(x) / c;
}

float tanf(float x) {
    return (float)tan((double)x);
}

double atan(double x) {
    if (isnan(x)) return NAN;
    if (x == 0.0) return 0.0;
    int neg = (x < 0.0);
    double val = neg ? -x : x;
    double res;
    if (val > 1.0) {
        double inv = 1.0 / val;
        res = M_PI_2 - inv / (1.0 + 0.28 * inv * inv);
    } else {
        res = val / (1.0 + 0.28 * val * val);
    }
    return neg ? -res : res;
}

float atanf(float x) {
    return (float)atan((double)x);
}

double atan2(double y, double x) {
    if (isnan(x) || isnan(y)) return NAN;
    if (x == 0.0) {
        if (y > 0.0) return M_PI_2;
        if (y < 0.0) return -M_PI_2;
        return 0.0;
    }
    double res = atan(y / x);
    if (x < 0.0) {
        if (y >= 0.0) res += M_PI;
        else res -= M_PI;
    }
    return res;
}

float atan2f(float y, float x) {
    return (float)atan2((double)y, (double)x);
}

double asin(double x) {
    if (x < -1.0 || x > 1.0) return NAN;
    if (x == 1.0) return M_PI_2;
    if (x == -1.0) return -M_PI_2;
    return atan2(x, sqrt(1.0 - x * x));
}

float asinf(float x) {
    return (float)asin((double)x);
}

double acos(double x) {
    if (x < -1.0 || x > 1.0) return NAN;
    return M_PI_2 - asin(x);
}

float acosf(float x) {
    return (float)acos((double)x);
}

double exp(double x) {
    if (isnan(x)) return NAN;
    if (x > 709.782712893384) return INFINITY;
    if (x < -745.1332191019412) return 0.0;
    if (x == 0.0) return 1.0;

    double k = round(x / M_LN2);
    double r = x - k * M_LN2;

    double r2 = r * r;
    double poly = r * (1.0 + r2 * (0.016666666666666666 + r2 * 0.0001984126984126984));
    double num = 2.0 + poly;
    double den = 2.0 - poly;
    double exp_r = num / den;

    int64_t ik = (int64_t)k;
    union { double d; uint64_t u; } scale;
    scale.u = (uint64_t)(ik + 1023) << 52;
    return exp_r * scale.d;
}

float expf(float x) {
    return (float)exp((double)x);
}

double exp2(double x) {
    return exp(x * M_LN2);
}

float exp2f(float x) {
    return (float)exp2((double)x);
}

double log(double x) {
    if (x < 0.0 || isnan(x)) return NAN;
    if (x == 0.0) return -INFINITY;
    if (isinf(x)) return INFINITY;

    union { double d; uint64_t u; } u = { .d = x };
    int64_t exp_bits = (u.u >> 52) & 0x7ff;
    int64_t k = exp_bits - 1023;
    u.u = (u.u & 0x000fffffffffffffull) | 0x3ff0000000000000ull;
    double m = u.d;

    if (m > M_SQRT2) {
        m *= 0.5;
        k += 1;
    }

    double f = m - 1.0;
    double s = f / (2.0 + f);
    double s2 = s * s;
    double r = s2 * (0.6666666666666666 + s2 * (0.4 + s2 * 0.2857142857142857));
    double log_m = f - s * (f - r);

    return (double)k * M_LN2 + log_m;
}

float logf(float x) {
    return (float)log((double)x);
}

double log2(double x) {
    return log(x) / M_LN2;
}

float log2f(float x) {
    return (float)log2((double)x);
}

double log10(double x) {
    return log(x) / M_LN10;
}

float log10f(float x) {
    return (float)log10((double)x);
}

double pow(double x, double y) {
    if (y == 0.0) return 1.0;
    if (isnan(x) || isnan(y)) return NAN;
    if (x == 0.0) {
        if (y < 0.0) return INFINITY;
        return 0.0;
    }
    if (x < 0.0) {
        if (floor(y) == y) {
            double res = exp(y * log(-x));
            if (fmod(y, 2.0) != 0.0) return -res;
            return res;
        }
        return NAN;
    }
    return exp(y * log(x));
}

float powf(float x, float y) {
    return (float)pow((double)x, (double)y);
}

double sinh(double x) {
    return (exp(x) - exp(-x)) / 2.0;
}

float sinhf(float x) {
    return (float)sinh((double)x);
}

double cosh(double x) {
    return (exp(x) + exp(-x)) / 2.0;
}

float coshf(float x) {
    return (float)cosh((double)x);
}

double tanh(double x) {
    double ex = exp(x);
    double emx = exp(-x);
    return (ex - emx) / (ex + emx);
}

float tanhf(float x) {
    return (float)tanh((double)x);
}

int isnan(double x) {
    union { double d; uint64_t u; } u = { .d = x };
    return ((u.u & 0x7fffffffffffffffull) > 0x7ff0000000000000ull);
}

int isinf(double x) {
    union { double d; uint64_t u; } u = { .d = x };
    return ((u.u & 0x7fffffffffffffffull) == 0x7ff0000000000000ull);
}

int isfinite(double x) {
    union { double d; uint64_t u; } u = { .d = x };
    return ((u.u & 0x7fffffffffffffffull) < 0x7ff0000000000000ull);
}

void sincos(double x, double *sin_out, double *cos_out) {
    if (sin_out) *sin_out = sin(x);
    if (cos_out) *cos_out = cos(x);
}

void sincosf(float x, float *sin_out, float *cos_out) {
    if (sin_out) *sin_out = sinf(x);
    if (cos_out) *cos_out = cosf(x);
}

