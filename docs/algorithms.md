# Algorithms

## Parser (Phase 2)

**Idea:** tokenize, then Dijkstra shunting-yard to an AST (not RPN output).

**Serial:** numbers and identifiers go to a value stack; operators go to an operator stack; `)` drains until `(`. Function names sit on the operator stack and become `Call` nodes when their `)` is seen.

**Precedence:** `^` (4, right), unary minus (3), `*` `/` (2), `+` `-` (1). So `-2^2 = -4` and `2^3^2 = 512`.

**Parallel:** none. Parsing a single expression is not a useful OpenMP workload.

**Complexity:** O(n) tokens.

## Evaluator (Phase 2)

**Serial:** recurse on the AST. Trig uses `std::sin` etc. until Phase 3.

**Angle mode:** `sin`/`cos`/`tan` convert degrees → radians on input; `asin`/`acos`/`atan` convert radians → degrees on output.

**Domain errors:** `/0`, `ln(x<=0)`, `sqrt(x<0)`, `tan` when cosine is ~0, `fact` of non-integers.

Brent's lemma is still Phase 9.
