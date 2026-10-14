#pragma once

#include "psc/eval/eval.hpp"

namespace psc {

// Forward-mode automatic differentiation: value and one directional derivative.
struct Dual {
  double v = 0.0;
  double d = 0.0;
};

inline Dual operator+(Dual a, Dual b) { return {a.v + b.v, a.d + b.d}; }
inline Dual operator-(Dual a, Dual b) { return {a.v - b.v, a.d - b.d}; }
inline Dual operator-(Dual a) { return {-a.v, -a.d}; }
inline Dual operator*(Dual a, Dual b) { return {a.v * b.v, a.d * b.v + a.v * b.d}; }
Dual operator/(Dual a, Dual b);
Dual pow_dual(Dual a, Dual b);

Dual dsin(Dual x, AngleMode angle);
Dual dcos(Dual x, AngleMode angle);
Dual dtan(Dual x, AngleMode angle);
Dual dasin(Dual x, AngleMode angle);
Dual dacos(Dual x, AngleMode angle);
Dual datan(Dual x, AngleMode angle);
Dual dsinh(Dual x);
Dual dcosh(Dual x);
Dual dtanh(Dual x);
Dual dln(Dual x);
Dual dlog10(Dual x);
Dual dexp(Dual x);
Dual dsqrt(Dual x);
Dual dabs(Dual x);
Dual dfact(Dual x);

using DualEnv = std::unordered_map<std::string, Dual>;

Dual evaluate_dual(const Expr& ast, const DualEnv& vars, AngleMode angle);
Dual evaluate_dual_string(const std::string& input, const DualEnv& vars, AngleMode angle);

}  // namespace psc
