
---
title: Complex number arithmetic
date: 15-01-2013
imgpath: /media/complex-numbers-arithmetics
tags:
    - math
    - xaos
    - fractals
---

In this post we will skip elementary mathematical operations (i.e. addition, substraction, division, multiplication) and focus only on functions with complex number arguments (e.q. trigonometric functions, logarithms etc.). But before that, we should write down basic definitions and equations.

Algebraic form of complex number

![](./img1.gif)

Geometric representation of complex number

![](./img2.gif)

Euler's exponential form of complex number

![](./img3.gif)

and Euler's formulas

![](./img4.gif)

![](./img5.gif)

![](./img6.gif)

![](./img7.gif)


Now you can do some math transformation to get what we want...

### Logarithms

#### exponential function of complex number

![](./img8.gif)

#### complex natural logarithm

![](./img9.gif)

#### complex logarithm of any base

![](./img10.gif)


### Trigonometry

using Euler's formulas for complex argument we can get

![](./img11.gif)

and

![](./img12.gif)

and also

![](./img13.gif)

<P>
and by setting ![](./img14.gif) in above equations, we get these relations

![](./img15.gif)

using trigonometrical realtions for sine and cosine we can get

#### sine functions for complex numbers

![](./img16.gif)

#### cosine functions for complex numbers

![](./img17.gif)

#### tangent functions for complex numbers

![](./img18.gif)

in next step we will get hyperbolic trigonometry functions, using some well known realations (similar to those used previously).

#### hyperbolic sine functions for complex numbers

![](./img19.gif)

#### hyperbolic cosine functions for complex numbers

![](./img20.gif)

#### hyperbolic tangent functions for complex numbers

![](./img21.gif)

### Power functions

there are three different ways to evaluate power functions in complex plane, we get one way for complex
number to the integer power (de Moivre relation), other for complex number to the real power and quite similar complex number to the complex number.
complex number to integer power (de Moivre)

![](./img22.gif)

#### real exponents with complex bases

![](./img23.gif)

#### complex exponents with complex bases

![](./img24.gif)

The last case is when we want to calculate

#### complex exponents of integer (or real) bases

![](./img25.gif)

---

*In the sffe source* — these identities are what the complex back-ends
implement. The **GSL** build (`lib/src/sffe_cmplx_gsl.c`) delegates to the
`gsl_complex_*` routines (`gsl_complex_exp`, `gsl_complex_log`,
`gsl_complex_sin/cos/tan`, `gsl_complex_sinh/cosh/tanh`, `gsl_complex_pow`,
etc.). The **x87 FPU** build implements them by hand in
`lib/src/asm/cmplx.S` / `cmplx.asm` (`sffecexp`, `sffecln`, `sffeclog`,
`sffecsin`, `sffeccos`, `sffectan`, `sffecsinh`, …, `sffeccpow`,
`sffecpowd`, `sffecpowi`, `sffecpowc`, `sffecsqrt`); the de&nbsp;Moivre
*n*-th root above is `sffecrtni` / `sfrtni`. Note that a few GSL entries are
still stubs or approximations (e.g. `sfatan2`, `sfceil`/`sffloor`, `sfsqr`),
and the complex back-ends currently lag the refactored core — see the
project `CLAUDE.md` for the current state.
