#include <cmath>

// this file is a HACK WORKAROUND for a problem with mathlib.a:
// Source Engine is compiled with -ffast-math, which implies -ffinite-math-only
// but, starting with glibc 2.31, at least in Ubuntu 20.04,
// some symbols specifically used for math functions with -ffinite-math-only
// are either gone or just don't link for whatever reason
// so, this file exports some weak symbols for funcs that forward to the non-finite-only funcs

#pragma GCC visibility push(default)
extern "C"
{
    [[gnu::weak]] double __pow_finite   (double base, double exp) { return pow(base, exp); }
    [[gnu::weak]] double __log_finite   (double arg)              { return log(arg);       }
    [[gnu::weak]] double __asin_finite  (double arg)              { return asin(arg);      }
    [[gnu::weak]] double __acos_finite  (double arg)              { return acos(arg);      }
    [[gnu::weak]] double __atan2_finite (double y, double x)      { return atan2(y, x);    }
    [[gnu::weak]] float  __acosf_finite (float arg)               { return acosf(arg);     }
    [[gnu::weak]] float  __atan2f_finite(float y, float x)        { return atan2f(y, x);   }
}
#pragma GCC visibility pop
