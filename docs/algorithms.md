# Algorithms

## Parser (Phase 2)

**Idea:** tokenize, then Dijkstra shunting-yard to an AST (not RPN output).

**Serial:** numbers and identifiers go to a value stack; operators go to an operator stack; `)` drains until `(`. Function names sit on the operator stack and become `Call` nodes when their `)` is seen.

**Precedence:** `^` (4, right), unary minus (3), `*` `/` (2), `+` `-` (1). So `-2^2 = -4` and `2^3^2 = 512`.

**Parallel:** none. Parsing a single expression is not a useful OpenMP workload.

**Complexity:** O(n) tokens.

## Evaluator (Phase 2)

**Serial:** recurse on the AST. Elementary functions follow `EvalContext.backend` (`std`, `taylor`, `cordic`).

**Angle mode:** `sin`/`cos`/`tan` convert degrees → radians on input; `asin`/`acos`/`atan` convert radians → degrees on output.

**Domain errors:** `/0`, `ln(x<=0)`, `sqrt(x<0)`, `tan` when cosine is ~0, `fact` of non-integers.

Brent's lemma is still Phase 9.

## Elementary functions (Phase 3)

**Taylor:** argument reduction, then Maclaurin / atanh-style series (Newton for sqrt after `frexp`). Default 40 terms.

**CORDIC:** circular rotation for sin/cos, vectoring for atan, hyperbolic rotation for exp/ln (then sinh/cosh from exp). This is a **software loop**, not a pipelined FPGA CORDIC.

**sqrt + CORDIC:** unsupported (no linear CORDIC here).

**Parallel:** `batch_sin` / `batch_cos` / `batch_exp` / `batch_ln` with `parallel for` when `n >= parallel_cutoff`. Independent outputs, no shared accumulators.

**Not parallel:** a 40-term Taylor sum (too small; cutoff 256).

## Integration (Phase 4)

**Serial:** composite trapezoid (O(h^2)), Simpson 1/3 (O(h^4), even panels; odd n is rounded up), Gauss–Legendre orders 2–5 on `n` subintervals, adaptive Simpson with `|S_left+S_right-S|/15 <= tol`, Monte Carlo.

**Parallel:** panel loops use `reduction(+:sum)`. Adaptive uses OpenMP tasks on subintervals (global `tol` split in half). 2D/3D: outer 1D is parallel, inner is serial (no nested OpenMP). Monte Carlo: one `mt19937` per thread, seed `seed + tid*997`.

**Tests:** ∫ 4/(1+x²) dx on [0,1] = π, ∫ sin x on [0,π] = 2, ∫ exp(-x²) on [0,1] vs erf.

## Differentiation (Phase 5)

**Finite differences:** forward O(h), backward O(h), central O(h^2). Small h → round-off; large h → truncation. The step-size study should show a U-shaped error curve.

**Forward AD:** `Dual {value, derivative}` with product/quotient/chain rules. One sweep per independent variable for a gradient. Reverse-mode tape is **not** implemented (shared accumulations are harder to parallelize).

**Parallel:** independent Dual seeds (one per variable) with `parallel for`. Does not reduce arithmetic; it only reduces elapsed time.
