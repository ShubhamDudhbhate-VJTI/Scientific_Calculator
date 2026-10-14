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

## Linear Algebra (Phase 6)

**Matrix representation:** row-major dense storage in `std::vector<double>`.

**Serial operations:**
- Addition/subtraction: element-wise O(mn).
- Transpose: O(mn) with index reordering.
- Multiplication (naive i-j-k): O(mnk) flops, poor cache locality for large matrices.
- Multiplication (i-k-j): reordering improves cache reuse by keeping A(i,k) in registers.
- Multiplication (tiled): block size 64 by default; processes tiles to fit in L1/L2 cache.
- LU decomposition with partial pivoting: O(n³) for n×n matrix; selects max pivot in column to improve numerical stability.
- Determinant: via LU decomposition (product of U's diagonal, adjusted by pivot sign).
- Inverse: Gaussian elimination on augmented matrix [A|I] → [I|A⁻¹].
- Solve Ax=b: LU decomposition + forward substitution (Ly = Pb) + backward substitution (Ux = y).
- Rank: count non-zero diagonal entries in U after LU (tolerance 1e-10).

**Parallel multiplication:**
- `multiply_naive`: `parallel for` over rows i, each thread computes full row of C.
- `multiply_ikj`: `parallel for` over rows i, inner loop over k then j (better cache locality).
- `multiply_tiled`: `parallel for` with dynamic schedule over tiles; each thread processes independent tiles. Tile size chosen to balance overhead and cache fit.

**Parallel LU:** Currently serial. Parallelizing Gaussian elimination requires careful synchronization: pivot selection is sequential, but row updates under a pivot can be parallelized. This is left as future work (requires task dependencies or wavefront pattern).

**Numerical stability:**
- Partial pivoting reduces growth factor.
- Tolerance 1e-10 for rank detection and singularity checks.
- Inverse uses augmented matrix method; singular matrices throw exception.

**Tests:**
- A × I = A
- A × A⁻¹ ≈ I (within 1e-9)
- LU: PA = LU where P is permutation from partial pivoting (L lower triangular with ones, U upper triangular)
- Solve: A × x = b verified by substitution
- Determinant of 2×2 matches formula, Iₙ has det=1
- Singular matrices detected and throw

## Solvers (Phase 7)

**Root finding methods:**

**Bisection:** Guaranteed convergence for continuous functions on interval [a,b] where f(a)*f(b) < 0. Halves interval each iteration. Linear convergence (O(log(1/ε)) iterations).

**Newton-Raphson:** Uses derivative: x_{n+1} = x_n - f(x_n)/f'(x_n). Quadratic convergence (error squares each iteration) when near root. Requires derivative function; can diverge if initial guess is poor.

**Secant:** Derivative-free approximation of Newton: x_{n+1} = x_n - f(x_n)*(x_n - x_{n-1})/(f(x_n) - f(x_{n-1})). Superlinear convergence (~1.618 order). Needs two initial points.

**False position (regula falsi):** Hybrid of bisection and secant. Uses secant interpolation but keeps bracketing property. Guaranteed convergence like bisection, but can be slow for some functions.

**Parallel root finding:** Not implemented. Ensemble root finding (finding multiple roots simultaneously) could be parallelized with OpenMP tasks.

**ODE solvers:**

**Euler method:** First-order explicit: y_{n+1} = y_n + h*f(t_n, y_n). Local error O(h²), global error O(h). Simple but inaccurate for stiff systems.

**Runge-Kutta 4 (RK4):** Fourth-order explicit: uses four function evaluations per step. Local error O(h⁵), global error O(h⁴). Much more accurate than Euler for same step size.

**Adaptive RK4:** Automatically adjusts step size based on error estimate (comparing full step vs two half steps). Reduces step when error exceeds tolerance, increases when error is small. Efficient for problems with varying dynamics.

**System of ODEs:** Both Euler and RK4 support vector-valued systems. For n equations, each step evaluates f for all n variables.

**Parallel ODE ensemble:** Not implemented. Could parallelize solving multiple ODEs with different initial conditions (parameter sweeps) using OpenMP.

**Numerical considerations:**
- Newton method checks for near-zero derivative to avoid division by zero
- Adaptive step size has min/max bounds to prevent too small/large steps
- System solvers store results interleaved: t[0], y1[0], y2[0], ..., t[1], y1[1], y2[1], ...

**Tests:**
- Bisection finds roots of x²-4 at ±2
- Newton finds root of cos(x)-x near 0.739
- Secant converges to same roots as Newton
- False position maintains bracketing property
- Euler approximates dy/dt = -2y with ~2% error at h=0.01
- RK4 approximates same ODE with <1e-6 error at h=0.1
- Adaptive RK4 takes fewer steps than fixed-step for same accuracy
- System solvers handle coupled ODEs correctly

## Big Integer (Phase 8)

**Representation:** Limb-based big integer using base 2^32 (32-bit limbs). Little-endian limb order. Sign stored separately.

**Operations:**
- Addition: Schoolbook addition with carry propagation across limbs. O(n) where n is number of limbs.
- Subtraction: Schoolbook subtraction with borrow propagation. Handles sign changes.
- Multiplication: Schoolbook O(n²) multiplication. For each limb pair, compute product and add with carry.
- Division: Long division algorithm. O(n²) worst case.
- Modulo: Computed as remainder after division.
- Bitwise operations: AND, OR, XOR, NOT, left shift, right shift. Operate limb-by-limb.
- Comparison: Sign-aware comparison. For same sign, compare limb-by-limb from most significant.

**Numerical considerations:**
- Normalization removes leading zero limbs after each operation
- Negative zero is normalized to positive zero
- Hex string input/output for easy debugging
- Decimal conversion uses repeated division by 10

**Parallel big integer:** Not implemented. Karatsuba multiplication or FFT-based multiplication could be parallelized, but schoolbook is sufficient for this project.

**Statistics:**
- Limb size: 32 bits
- Max limb value: 2^32 - 1
- Limb count: dynamic based on number size

**Tests:**
- Default constructor creates zero
- Int constructor handles positive/negative values
- Hex constructor parses 0x prefix
- Comparison operators work correctly
- Addition/subtraction/multiplication/division/modulo
- Bitwise operations (AND, OR, XOR, shifts)
- Large numbers (64-bit and beyond)
- Statistics return correct limb info

## Parallel Evaluation (Phase 9)

**Batch evaluation:** Evaluates the same expression with different variable values in parallel using OpenMP.

**batch_eval_x:** Evaluates expression f(x) for a vector of x values. Parses expression once, then evaluates in parallel with different x values.

**batch_eval_exprs:** Evaluates multiple different expressions with the same context. Parses all expressions first, then evaluates in parallel.

**param_sweep:** Evaluates expression f(x, param) for different parameter values. Useful for sensitivity analysis and parameter studies.

**Implementation:**
- Expression parsing is serial (done once before parallel evaluation)
- Each evaluation gets its own EvalContext copy to avoid race conditions
- Dynamic scheduling for load balancing
- Error handling per-evaluation (doesn't stop entire batch)

**Parallel strategy:**
- Independent evaluations → embarrassingly parallel
- No shared state between evaluations
- OpenMP parallel for with dynamic schedule

**Tests:**
- Batch eval with linear expression 2x+1
- Batch eval with quadratic x²
- Batch eval multiple expressions
- Parameter sweep for a*x
- Error handling for division by zero

## CPU Benchmarks (Phase 10a)

**Benchmark suite:** Measures serial vs parallel performance for key operations.

**Workloads:**
- Matrix multiply (128x128): naive vs tiled multiplication
- Integration (100k panels): Simpson rule serial vs parallel
- Batch sin (100k values): serial vs parallel batch evaluation
- Root finding (100 instances): independent bisection calls
- ODE solver (100 instances): independent RK4 calls

**Timing:** Uses std::chrono::high_resolution_clock for millisecond precision.

**Output:** CSV file with columns: name, serial_ms, parallel_ms, threads, speedup.

**Speedup calculation:** speedup = serial_time / parallel_time.

**Usage:** `./psc_bench --threads N --output benchmarks.csv`

**Notes:**
- Root finding and ODE solving are inherently sequential for a single instance
- Parallel versions run independent instances to show ensemble speedup
- Matrix multiply and integration show true algorithmic parallelism
- Batch operations show data parallelism

