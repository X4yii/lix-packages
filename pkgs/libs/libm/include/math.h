#ifndef _MATH_H
#define _MATH_H

#define M_E         2.71828182845904523536028747135266250
#define M_LOG2E     1.44269504088896340735992468100189214
#define M_LOG10E    0.434294481903251827651128918916605082
#define M_LN2       0.693147180559945309417232121458176568
#define M_LN10      2.30258509299404568401799145468436421
#define M_PI        3.14159265358979323846264338327950288
#define M_PI_2      1.57079632679489661923132169163975144
#define M_PI_4      0.785398163397448309615660845819875721
#define M_1_PI      0.318309886183790671537767526745028724
#define M_2_PI      0.636619772367581343075535053490057448
#define M_2_SQRTPI  1.12837916709551257389615890312154517
#define M_SQRT2     1.41421356237309504880168872420969808
#define M_SQRT1_2   0.707106781186547524400844362104849039

#define HUGE_VAL    (__builtin_huge_val())
#define HUGE_VALF   (__builtin_huge_valf())
#define INFINITY    (__builtin_inff())
#define NAN         (__builtin_nanf(""))

double fabs(double x);
float fabsf(float x);

double ceil(double x);
float ceilf(float x);

double floor(double x);
float floorf(float x);

double trunc(double x);
float truncf(float x);

double round(double x);
float roundf(float x);

double fmod(double x, double y);
float fmodf(float x, float y);

double sqrt(double x);
float sqrtf(float x);

double cbrt(double x);
float cbrtf(float x);

double hypot(double x, double y);
float hypotf(float x, float y);

double exp(double x);
float expf(float x);

double exp2(double x);
float exp2f(float x);

double log(double x);
float logf(float x);

double log2(double x);
float log2f(float x);

double log10(double x);
float log10f(float x);

double pow(double x, double y);
float powf(float x, float y);

double sin(double x);
float sinf(float x);

double cos(double x);
float cosf(float x);

void sincos(double x, double *sin, double *cos);
void sincosf(float x, float *sin, float *cos);


double tan(double x);
float tanf(float x);

double asin(double x);
float asinf(float x);

double acos(double x);
float acosf(float x);

double atan(double x);
float atanf(float x);

double atan2(double y, double x);
float atan2f(float y, float x);

double sinh(double x);
float sinhf(float x);

double cosh(double x);
float coshf(float x);

double tanh(double x);
float tanhf(float x);

int isnan(double x);
int isinf(double x);
int isfinite(double x);

#endif
