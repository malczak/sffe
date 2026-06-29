---
title: Math parser
date: 16-01-2013
imgpath: /media/math-parser
tags:
    - c
    - math
    - xaos
    - fractals
---

I'm a fractal geek. To discover new Mandelbrot sets I needed to create a complex numbers math parser. So that I can just type in a set formula and quickly check if it is interesting or not. Because drawing of a single fractal (or its fragment) is always a computationally challenging task, parser had to be fast.

I have created easy to extend math parser, that is able to efficiently evaluate math formulas in complex number space. It also includes two complex number arithmetic implementations - nasm fpu (for x86) and GSL.

Parser is currently used in [XaoS](http://matek.hu/xaos/doku.php) fast interactive fractal zoomer and create custom formulas.


Here are some notes and details on how it is build.


### Basic assumptions

First step was to create a simple math parser for real numbers. Keeping in mind, that it has to be fast and will be used only for function evaluation I made these basic assumptions

 * ignore variables modification (assignment)

 * ignore all logical operations

 * set of numbers independent (&#x211d;, &#x2102;);

 * precision independent

 * easy to extend

 * make it fast

### More assumptions

Basically speaking its all about transforming math expression

~~~
x * sin( 2 * x / y )
~~~

into corresponding stack notation of any kind (prefix or reverse Polish notation)

~~~
x y / 2 * sin 2 * (rpn)
x 2 x * y / sin * (prefix)
~~~

Arithmetic notations is then transformed into bytecode (or operators stack/tree) and expression value can be evaluated.

Problem with this simplified approach is that stack needs to be fixed (or even rebuild) every time we want to calculate expression value. Because parser is going to be used in loops, this is something we should avoid. Internal stack stucture should never be modified. There should be no extra steps before and after expression value is evaluated. Moreover there should be no memory operation during this operation.

The whole process can be divided into to two parts with two corresponding modules

 * **parser** - used only once to parse input expression and transform it into bytecode

 * **evaluator** - evaluate the value of the expression using input variables

I don't really care how fast parser is. I only need it to be reliable. To be honest it is slow, complicated and a bit memory consuming.

I have only focused on evaluator implementation. Its internal structure should fulfill this set of rules.

 * expession evaluation should be an in-situ operation.

 * evaluator should never change its state - in terms of its internal structure (no stack manipulation) and in terms of consumed memory.

 * changing the input variables should not result in any memory operations.


### Implementation

#### Parser module

Parser is responsible for translating the input formula string into some kind of bytecode.

  * Lexical and syntactic analysis - use a simple context-free grammar for math expressions. Implementation depends on set of numbers we are working with (&#x211d; or &#x2102;)

  * Tokenization - covert input expression
  ~~~
  x * sin( 2 * x / y )
  ~~~
  into tokenized form and keep track of all tokens
  ~~~
  n * f( n * n / n )
  ~~~

  * Bytecode generator (eq. operation stack builder) - covert tokenized expression into bytecode
  ~~~ markdown
   +---+---+---+---+---+---+---+---+
   | n | n | n | * | n | / | f | * |
   +---+---+---+---+---+---+---+---+
  ~~~

In the current source (`lib/src/sffe.c`) the parser actually runs in three passes over the string, all inside `sffe_parse`:

  1. **Normalize.** Upper-case the whole input (names are matched
     case-insensitively), strip spaces, turn `[` `]` into `(` `)` and a decimal
     `,` into `.`, collapse runs of unary/binary signs (`++--1` &rarr; `1`,
     `-+-2` &rarr; `+2`) and verify the brackets are balanced.

  2. **Tokenize.** Walk the cleaned string and classify each token as `n`
     (a number, a built-in constant or a variable), `f` (a function) or an
     operator. This is also where *implicit multiplication* is injected: `3x`,
     `2sin(x)`, `)(` and `3(...)` all get a `*` spliced in, and a unary minus
     in front of a group is turned either into the literal `-1` (a leading
     `-(...)`) or into a tight negation function (an operator-preceded
     `... ^ -(...)` / `... / -(...)`).

  3. **Build the stack notation.** A shunting-yard operator stack (with the
     operator priorities from `sf_priority`) turns the token stream into the
     flat RPN array that the evaluator consumes.

#### Evaluator module

Evaluator is just a bytecode interpreter. It takes input variables and evaluates expression value. In this particular case bytecode is just a variation of operations stack rather that a typical bytecode. It cannot be saved and reused in compiled form.

##### Internal structure

The parser produces two contiguous arrays and nothing else has to be touched at
evaluation time:

  * `parser->args` &mdash; one **value slot** (`sfarg`) per operand *and* one per
    operation result, laid out in **RPN order**. Each slot carries a pointer to
    its `sfNumber` value and a back-pointer `parg` to the previous slot in the
    chain.

  * `parser->oprs` &mdash; one **operation** (`sfopr`) per operator/function, in
    evaluation order. Each holds a pointer to the result slot it writes and the
    function pointer to call.

~~~ c
typedef struct sfargument__ {
    struct sfargument__ *parg;  /* previous slot in the argument chain */
    sfvartype            type;
    sfNumber            *value; /* the actual number lives here */
} sfarg;

typedef struct {
    sfarg *arg;                 /* result slot for this operation       */
    sffptr fnc;                 /* function to apply (SFFE_DIRECT_FPTR)  */
} sfopr;
~~~

A function reads its arguments right-to-left by walking the `parg` chain
(`sfaram1(p)` is `p->parg`, `sfaram2(p)` is `p->parg->parg`, &hellip;), writes
its result through `sfvalue(p)` and returns the slot the chain should continue
from. So the `parg` links *are* the operand stack &mdash; but they live inside the
single preallocated `args` array, so "pushing" and "popping" is nothing more
than rewriting a couple of pointers in place.

For `2+3*4` the parser emits (this is the real `SFFE_DEVEL` trace):

~~~
| compiled expr.: |n+n*n|
| stack not.: nnn*+
| numbers:  2 3 4 -1 -1     <- args[0..4]; the two -1 are result slots
| operations: *  then  +    <- oprs in evaluation order
~~~

`args` is `[2][3][4][R*][R+]`. The `*` result lands right after its rightmost
operand (slot `R*` = index 3), and the `+` result after *its* rightmost operand
(slot `R+` = index 4). Initially every slot links back one step:
`R+ -> R* -> 4 -> 3 -> 2`.

Evaluating walks `oprs` once:

  * **`*`** reads `4` and `3` (the two slots before `R*`), writes `12` into
    `R*`, and **relinks** `R*` to point past the consumed `3` and `4`, straight
    at the `2`. The chain is now `R+ -> R* -> 2`.

  * **`+`** reads `R*` (`12`) and `2`, writes `14` into `R+`.

The consumed operands were "popped" simply by moving `R*`'s back-pointer &mdash; no
memory was allocated or freed, and the leaf operand slots (`2`, `3`, `4`) were
never modified. That last point is what makes variable rebinding free: a
variable's slot holds a pointer to *your* `sfNumber`, so changing the input is
just writing to your own memory and calling `sffe_eval` again.

Finally the heart of math parser implementation.

~~~ c
sfNumber sffe_eval(sffe * const parser)
{
  register sfopr *optr  = parser->oprs;
  register sfopr *optro = optr;
  register sfopr *optrl = optr + parser->oprCount;
  for (; optr != optrl; optr += 1, optro += 1)
  {
    optro->arg->parg = optro->arg - 1;          /* (A) reset chain head    */
    sfarg *arg = optr->arg;
#ifdef SFFE_DIRECT_FPTR
    arg->parg = optr->fnc(arg, NULL)->parg;     /* (B) apply + splice out  */
#else
    arg->parg = optr->fnc->fptr(arg, optr->fnc->payload)->parg;
#endif
  };
  return *(parser->result);
}
~~~

Line **(A)** restores the result slot's back-pointer to its physically previous
slot &mdash; this is needed because a *previous* evaluation left it pointing
somewhere else (line B). Line **(B)** calls the operation, then sets the result
slot's `parg` to whatever the function returns, splicing the just-consumed
operands out of the chain for the operations that follow. The whole loop is two
pointer writes plus one indirect call per operation, over a contiguous array.

Crucially, **two consecutive `sffe_eval` calls need no fix-up of the RPN
structure in between.** Every operation only ever rewrites its *own* result
slot's `parg` (both line A and line B write `optr->arg->parg`, nothing else) and
the leaf operand slots are never touched at all. Line A re-seeds that one link
at the top of each operation's turn, so the pass is self-contained and
idempotent: the evaluator leaves the `args`/`oprs` arrays in a state the *next*
call simply re-normalizes on the fly. You can therefore call `sffe_eval`
back-to-back, or spin it in a tight loop rebinding variables between calls, with
zero intervening work &mdash; no reset pass, no rebuild, no allocation. This is
exactly the "no extra steps before and after expression is evaluated" guarantee
from the assumptions above.

(The two cursors `optr`/`optro` advance together and are equal on every
iteration &mdash; a historical leftover; one cursor would do.)

### Why it is fast

  * **Parse once, evaluate millions of times.** All the slow work &mdash; lexing,
    the shunting-yard, every `malloc`/`realloc` &mdash; happens a single time in
    `sffe_parse`. The fractal inner loop only ever touches `sffe_eval`.

  * **No allocation, no branching, in-situ.** `sffe_eval` performs zero memory
    operations and has no data-dependent branches; the "stack" is emulated by
    relinking pointers inside one preallocated array.

  * **Cache-friendly layout.** `args` and `oprs` are flat contiguous arrays
    walked front-to-back. There is no tree to traverse and no heap-scattered
    nodes &mdash; the `parg` chain hops only within the `args` block.

  * **Free variable rebinding.** Variables are bound *by pointer*. Changing an
    input is a single store into caller-owned memory; the structure never
    changes, so loops that sweep `x`/`y` across a plane cost nothing extra.

  * **Direct function pointers.** With `SFFE_DIRECT_FPTR` (the default) each
    operation is a direct `fnc(arg, NULL)` call &mdash; no dereference through a
    descriptor struct, no payload handling.

  * **Type chosen at compile time.** `sfNumber` resolves to `double`,
    `gsl_complex` or the x87 `cmplx` struct at build time, so there is no
    runtime dispatch on the number kind &mdash; the same tight core serves &#x211d;
    and &#x2102;.

### Pros and cons

**Pros**

  * Parse-once / evaluate-many design that is genuinely fast in tight loops.
  * Real and complex back-ends from one core; precision/number-set is a
    compile-time choice.
  * Easy to extend with new constants and `sffe_regfunc` user functions.
  * Tiny and dependency-free for the real build (drop in `sffe.c` +
    `sffe_real.c`); no runtime allocation during evaluation.
  * Self-contained, idempotent evaluation: consecutive `sffe_eval` calls need
    no structural fix-up between them, so it drops straight into a tight loop.
  * Handy conveniences: case-insensitive names, implicit multiplication
    (`3x`, `2sin(x)`, `3(...)`), `[` `]` brackets and `,` decimals.

**Cons / things to know**

  * **One parser instance is not reentrant.** `sffe_eval` rewrites the `parg`
    links inside `args` as it runs, so a single parser cannot be evaluated from
    two threads at once. (Separate instances are fully independent &mdash; give each
    worker thread its own parser.)
  * **Error messages are opt-in.** `sffe_parse` returns a non-zero
    `enum sffe_error` code, but it only fills `parser->errormsg` *if you have
    already pointed it at a buffer* &mdash; the library never allocates one. Out of
    the box the message is dropped and only the code survives.
  * **`^` is left-associative** here (`2^3^2` evaluates to `64`, not the
    conventional `512`). Numeric unary minus also binds into the number, so
    `-2^2 == 4`.
  * **By design there are no assignments, comparisons, logical operators or
    conditionals** &mdash; it evaluates a pure arithmetic expression and nothing
    more.
  * **No domain checking.** `1/0`, `log(-1)` etc. just produce `inf`/`nan`.
  * **The default build ignores user-function `payload`.** Because
    `SFFE_DIRECT_FPTR` is hard-coded on, functions are called with `NULL`
    instead of the payload passed to `sffe_regfunc`.
  * **The complex back-ends currently lag the refactored core.** The GSL/asm
    function signatures don't yet match the new `sffptr` typedef, so only the
    real back-end builds cleanly today; the asm unit is also 32-bit x87 only.
  * The parser itself is, in the author's words, "slow, complicated and a bit
    memory consuming" &mdash; fine, since it runs only once.

### Summary

 * parser can be used for any set of numbers and precision

 * parser is easy to extend (new functions, constants)

 * evaluator can be reused with different input variables

 * expression evaluation is an in-situ operation (in terms of structure and memory usage)



### Code

Source code for both &#x211d; and &#x2102; is available on [github](https://github.com/malczak/sffe).

For complex numbers implemetation details please read these posts

 * [Complex number aritmetics]({{site.url}}/blog/2013/01/15/complex-numbers-arithmetics)

 * [Complex number aritmetics using FPU]().


### Example

Parse debug trace

~~~markdown
|-----------------------------------------
+ > lib/src/sffe.c[504] - parsing
|-----------------------------------------
| input (len.=30): |(-1+2*3(sin(x+1)-1/x))/(2*x+1)|
| check (len.=30): |(-1+2*3(SIN(X+1)-1/X))/(2*X+1)|
| compiled expr.: |(n+n*n*(f(n+n)-n/n))/(n*n+n)|
| operations: 10
| numbers,vars: 10
| stack not.: nnn*nn+fnn/-*+nn*n+/
| numbers:  -1 2 3 -1 -1 1 -1 -1 1 -1 -1 -1 -1 -1 2 -1 -1 1 -1 -1
| functions fnctbl: [+] [*] [*] [SIN] [+] [-] [/] [/] [*] [+]
| functions used ptrs: __addr_dump__
| compiled in  0.000094 s
|-----------------------------------------
+ < lib/src/sffe.c[1020] - parsing
|-----------------------------------------
~~~

In the trace above note how `3(sin(...))` became `n*n*(f(...))`: the tokenizer
inserted the implicit multiplications. The leading `-1` stays a plain number
`n`, and the ten `-1` placeholders in the `numbers:` line are exactly the ten
operation result slots waiting to be filled by `sffe_eval`.
