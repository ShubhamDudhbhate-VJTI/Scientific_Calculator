#pragma once

namespace psc {

double taylor_sin(double x, int iters);
double taylor_cos(double x, int iters);
double taylor_tan(double x, int iters);
double taylor_exp(double x, int iters);
double taylor_ln(double x, int iters);
double taylor_sqrt(double x, int iters);
double taylor_sinh(double x, int iters);
double taylor_cosh(double x, int iters);
double taylor_atan(double x, int iters);

double cordic_sin(double x, int iters);
double cordic_cos(double x, int iters);
double cordic_tan(double x, int iters);
double cordic_atan(double x, int iters);
double cordic_exp(double x, int iters);
double cordic_ln(double x, int iters);
double cordic_sinh(double x, int iters);
double cordic_cosh(double x, int iters);

}  // namespace psc
