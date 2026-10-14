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
