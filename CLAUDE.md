# CLAUDE.md

Guidance for AI assistants working in this repository.

## Project overview

**sffe** ("SegFault Formula Evaluator") is a small, fast math formula
evaluator library written in C. It parses a math expression supplied as a
string (e.g. `"sin(x)^2 + cos(x)^2"`) into an internal flat "stack" of
operations, then evaluates it repeatedly — variables can be re-bound between
evaluations without re-parsing, which is the main performance goal.

It supports three number back-ends selected at compile time:

- **real** (`double`) — pure C, no external dependencies
- **complex via GSL** — uses GNU Scientific Library complex routines
- **complex via x87 asm** — uses a hand-written FPU complex unit (NASM/GAS)

Original author: Mateusz Malczak. Background article:
http://malczak.info/blog/math-parser/

## Repository layout

```
lib/
  include/
    sffe.h              # Public API + core structs; selects a back-end header
    sffe_real.h         # Real (double) build: function/constant declarations
    sffe_cmplx_gsl.h    # Complex build via GSL
    sffe_cmplx_asm.h    # Complex build via x87 asm unit
  src/
    sffe.c              # Parser + evaluator core (back-end independent)
    sffe_real.c         # Real implementations of all functions/constants
    sffe_cmplx_gsl.c    # Complex (GSL) implementations
    sffe_cmplx_asm.c    # Complex (asm) C-side glue
    asm/
      cmplx.S           # x87 complex math, GAS/AT&T syntax (ELF, `_` prefix)
      cmplx.asm         # x87 complex math, NASM syntax (Win/COFF)
      build.sh          # nasm -f elf  cmplx.asm  (legacy; see notes)
      build_win.sh      # nasm -f coff cmplx.asm  (legacy; see notes)
test/
  test_sffe.c           # Unit tests (Check framework)
Makefile                # Builds the real back-end as a static lib + runs tests
Dockerfile              # Minimal Alpine build/test/dev image
docker-compose.yml      # `test` and `dev` convenience services
README.md
```

The library is also meant to be **droppable directly into a consumer
project** without the build system: add `lib/src/sffe.c` plus exactly one
back-end `.c` file, and put `lib/include` on the include path. The asm
`build.sh` scripts are legacy ("`../Makefile` already contains this
compilation") and refer to a Makefile that does not exist in this repo — the
root `Makefile` here is new and covers only the real back-end.

## Build configuration (compile-time macros)

Behavior is controlled entirely by preprocessor defines (see the header of
`lib/include/sffe.h`). Pick exactly one number back-end:

| Macro            | Effect                                                       |
|------------------|--------------------------------------------------------------|
| `SFFE_REAL`      | Real `double` parser. Implies `SFFE_DOUBLE`.                 |
| `SFFE_CMPLX_GSL` | Complex parser using GSL. Implies `SFFE_COMPLEX`. Needs GSL. |
| `SFFE_CMPLX_ASM` | Complex parser using the asm unit. Implies `SFFE_COMPLEX`.   |
| `SFFE_DEVEL`     | Verbose parse/eval tracing + error printing to stdout.       |
| `SFFE_DIRECT_FPTR` | Store/call raw function pointers, skipping the `payload`. **Hard-coded to `1` in `sffe.h`.** |
| `SFFE_DLL`       | Windows DLL export decoration.                               |

`sfNumber` is the scalar type and resolves per back-end: `double` for real,
`gsl_complex` for GSL, and a `{double r, i;}` struct (`cmplx`) for asm.

> **Gotcha — the real back-end needs `SFFE_DOUBLE` explicitly.** `sffe_real.c`
> and `sffe_real.h` guard their entire bodies with `#ifdef SFFE_DOUBLE`
> *before* `sffe.h` is included, so `-DSFFE_REAL` alone (which only turns into
> `SFFE_DOUBLE` *inside* `sffe.h`) compiles `sffe_real.c` to an empty object
> and you get undefined references to `sfcmplxfunc`/`sfcnames`/`sfcvals`.
> Always pass **both** `-DSFFE_REAL -DSFFE_DOUBLE` (this is what the `Makefile`
> does).

Example compile lines (invoke the compiler directly when not using the
`Makefile`):

```sh
# Real build
cc -DSFFE_REAL -DSFFE_DOUBLE -Ilib/include -c lib/src/sffe.c lib/src/sffe_real.c

# Complex build via GSL
cc -DSFFE_CMPLX_GSL -Ilib/include -c lib/src/sffe.c lib/src/sffe_cmplx_gsl.c $(pkg-config --cflags gsl)

# Complex build via asm (assemble the FPU unit first; 32-bit x87)
nasm -f elf lib/src/asm/cmplx.asm -o cmplx.o      # or use cmplx.S with gas
cc -m32 -DSFFE_CMPLX_ASM -Ilib/include -c lib/src/sffe.c lib/src/sffe_cmplx_asm.c
```

Note the asm back-end is **32-bit x87** (uses `%ebp`/`%esp`, `fldl`, etc.)
and will not assemble/link cleanly on a 64-bit-only toolchain without `-m32`.

## Building & testing

A `Makefile` and an Alpine-based Docker setup cover the **real back-end**
(the complex back-ends are not wired in — see "Known issues"):

```sh
make           # build build/libsffe.a (static lib, real back-end)
make test      # build the lib + compile/run the Check test suite
make clean     # remove build/

# Docker (minimal Alpine with gcc, make and the Check framework):
docker compose run --rm test   # build + run tests in a container
docker compose run --rm dev     # interactive shell, source bind-mounted
```

Tests live in `test/test_sffe.c` and use **Check**
(https://libcheck.github.io/check/), a widely used C unit-testing framework,
located at link time via `pkg-config`. The current suite has one stub case
asserting `2+2*2 == 6`; add new cases with `START_TEST`/`tcase_add_test`. The
`Makefile` compiles with `-Wall -Wextra`; the existing code emits a number of
`unused parameter`/`missing initializer` warnings (leftovers from the
`SFFE_DIRECT_FPTR` refactor) that are harmless.

## Public API (`sffe.h`)

```c
sffe       *sffe_alloc(void);                                  // create evaluator
void        sffe_free(sffe **parser);                          // destroy
int         sffe_parse(sffe **parser, const char *expression); // parse; 0 == OK
sfNumber    sffe_eval(sffe *const parser);                     // evaluate, returns result

// Variables (bind C storage to a name used in expressions)
sfvariable *sffe_var(sffe *const parser, const char *name);    // lookup, NULL if absent
sfvariable *sffe_regvar(sffe **parser, sfNumber *vptr, const char *name);
void        sffe_regvars(sffe **parser, unsigned int n, sfNumber **vptrs, char *const *names);
sfNumber   *sffe_setvar(sffe **parser, sfNumber val, const char *name);

// User-defined functions
void       *sffe_regfunc(sffe **parser, const char *name, unsigned int parcnt,
                         sffptr funptr, void *payload);
```

Typical usage: `sffe_alloc` → `sffe_regvar`/`sffe_regfunc` → `sffe_parse`
once → set variable storage → `sffe_eval` many times → `sffe_free`.

`parser->result`, `parser->errormsg`, and `parser->expression` are
read-only outputs. `sffe_parse` returns `0` on success or an
`enum sffe_error` value, and writes a human-readable message into
`parser->errormsg` on failure.

## How it works (parser internals in `sffe.c`)

`sffe_parse` runs in distinct phases (search for `PHASE` comments):

1. **Phase 1 — normalize.** Uppercases the input, strips spaces, converts
   `[` `]` to `(` `)`, decimal `,` to `.`, collapses runs of unary/binary
   `+`/`-` operators, and checks bracket balance.
2. **Phase 2 — tokenize / lexical analysis.** Walks the cleaned string,
   classifying tokens into `n` (number/const/variable), `f` (function), and
   operators. Numbers go through `sffe_donum` (a small flag-driven state
   machine); complex literals `{re;im}` go through `sffe_docmplx`. Names are
   resolved against variables (`sffe_variable`), user functions
   (`userfunction`), built-in functions (`sffe_function`), and built-in
   constants (`sffe_const`). Implicit multiplication (e.g. `2x`, `)(`, `3sin`)
   is injected here.
3. **Phase 3 — build stack notation.** A shunting-yard-style operator stack
   (`struct __expression` / `struct _operator`, with operator priority from
   `sf_priority`) turns the token stream into the flat `parser->oprs` array of
   `sfopr` (each = an argument slot + a function pointer). Nested brackets push
   new stacks; `;` separates function arguments and decrements the allowed
   parameter count.

`sffe_eval` is a tight loop over `parser->oprs`: it relinks each operation's
argument chain and calls the stored function pointer. With `SFFE_DIRECT_FPTR`
(the default) it calls `optr->fnc(arg, NULL)` directly; otherwise it goes
through `optr->fnc->fptr(arg, payload)`.

### Function/constant tables (per back-end)

Each back-end `.c` defines, indexed identically to the header counts
(`sffnctscount`, `sfvarscount`):

- `const sffunction sfcmplxfunc[]` — function pointer, parameter count, name.
  **The first 5 entries are the operators `^ + - * /` in that exact order** —
  `sffe_operator()` indexes them by position, so do not reorder them.
- `const char sfcnames[][5]` — built-in constant/variable names
  (`PI`, `PI_2`, `PI2`, `E`, `EPS`/`I`, `RND`).
- `const cfptr sfcvals[]` — matching constant-evaluator function pointers.

A function takes its arguments right-to-left via the `parg` chain
(`sfaram1(p)`, `sfaram2(p)`, …), writes through `sfvalue(p)`, and returns the
slot pointer to continue the chain. Names are matched **case-insensitively**
(input is uppercased in Phase 1; stored names are uppercased via
`sf_strdup`).

## Conventions

- **Language/style:** C (C99-ish: VLAs, `for`-loop declarations, `//`
  comments). 2-space indentation in the recently refactored `sffe.c`; older
  back-end files use tabs. Match the style of the file you are editing.
- **Naming:** public API is `sffe_*`; built-in math functions are `sf<name>`
  (`sfadd`, `sfsin`); constant evaluators are `sfc<NAME>` (`sfcPI`).
- **Memory:** the parser owns `args`, `oprs`, `variables`, `userf`, and the
  normalized `expression`. `sfvar_type_managed_ptr` slots are freed by the
  library; `sfvar_type_ptr` slots point at caller-owned storage and are not.
  `sffe_clear` resets per-parse state; `sffe_free` tears everything down.
- **Errors:** add to `enum sffe_error` and to both `sffe_print_error`
  (debug) and `sffe_setup_error` (message) switch statements.
- The macros `sfset`, `pop_expression`, `insert_fnc_slot`, `append_token`,
  etc. are local to `sffe_parse`; understand them before touching Phase 3.

## Git workflow

- Active development branch for assistant work: `claude/claude-md-docs-euomln`.
  Commit there and push with `git push -u origin <branch>`; do not push to
  `master` without explicit permission.
- Do not open pull requests unless explicitly asked.
- Commit messages in history are short and imperative (often numbered lists).

## Known issues / gotchas

- **Complex back-ends are out of sync with the refactored core.** The
  `sffptr` typedef and the real back-end (`sffe_real.c`) were refactored so
  every math function takes `(sfarg *const p, void *payload)`. The GSL and
  asm back-ends (`sffe_cmplx_gsl.c`, `sffe_cmplx_asm.c`, and their headers)
  still declare/define the old single-argument form `(sfarg *const p)`.
  Because `sfcmplxfunc[]` initializes `sffptr` fields with these mismatched
  pointers, **the complex builds will not compile cleanly** (incompatible
  function pointer types) and would be undefined behavior if forced through.
  Any work on complex expressions should start by bringing these signatures
  back in line with `sffptr`. This is the most likely source of the
  long-standing "complex parsing" bugs.
- Several complex (GSL) functions are stubs or approximations:
  `sfatan2` ignores its inputs, `sfceil`/`sffloor` are no-ops, `sfsqr` is
  implemented as `pow(z, z)` rather than `z*z`, and the table maps
  `POWI`/`POWDC` to plain `sfpow`.
- **Unary minus before a group** is handled in two ways by Phase 2. A
  *leading* `-(...)` (at the start of an expression or after `(`) is emitted
  as the literal `-1` plus an implicit multiplication — `-(a)` → `-1*(a)` —
  which gives the conventional precedence `-(a)^n == -(a^n)`. An
  *operator-preceded* `-(...)` (after `* / ^`) is instead injected as a tight
  negation function (`sffe_neg` / `sffe_neg_func` in `sffe.c`), so
  `2^-(1+1) == 2^(-(1+1)) == 0.25` and `2/-(1+1) == 2/(-(1+1)) == -1`. This
  split is covered by `test_unary_minus_group_*` in `test/test_sffe.c`. (An
  earlier version emitted `-1*(a)` in *all* cases, which was wrong under `^`
  and `/`.) Note: numeric unary minus like `-2^2` is unaffected and still
  parses the sign into the number (`(-2)^2 == 4`).
- `sf_priority` and the operator-table ordering are tightly coupled; the
  first five `sfcmplxfunc` entries must stay `^ + - * /`.
- Test coverage is minimal — only the real back-end has a build target
  (`make test`) and a single stub case so far. When touching the complex
  back-ends, add tests and a build path for them too rather than relying on a
  throwaway driver.
</content>
</invoke>
