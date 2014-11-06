#ifdef SFFE_DOUBLE

#ifndef SFFE_DOUBLE_H
#define SFFE_DOUBLE_H

#include "sffe.h"

#define sffnctscount 28
#define sfvarscount 6

 #ifdef __cplusplus
  extern "C" {
 #endif

void sfadd( sfarg * const p ); /*  +  */
void sfsub( sfarg * const p ); /*  -  */
void sfmul( sfarg * const p ); /*  *  */
void sfdiv( sfarg * const p ); /*  /  */
void sfsin( sfarg * const p ); /* sin */
void sfcos( sfarg * const p ); /* cos */
void sftan( sfarg * const p ); /* tan */
void sfcot( sfarg * const p ); /* ctan */
void sfasin( sfarg * const p ); /* asin */
void sfacos( sfarg * const p ); /* acos */
void sfatan( sfarg * const p ); /* atan */
void sfacot( sfarg * const p ); /* actan */
void sfatan2( sfarg * const p ); /* atan2 */
void sfsinh( sfarg * const p ); /* sinh */
void sfcosh( sfarg * const p ); /* cosh */
void sftanh( sfarg * const p ); /* tanh */
void sfcoth( sfarg * const p ); /* ctanh */
void sfexp( sfarg * const p ); /* exp */
void sflog( sfarg * const p ); /* log */
void sflog10( sfarg * const p ); /* log10 */
void sflogN( sfarg * const p ); /* logN */
void sfpow( sfarg * const p ); /* pow */
void sfsqr( sfarg * const p ); /* sqr */
void sfsqrt( sfarg * const p ); /* sqrt */
void sfceil( sfarg * const p ); /* ceil */
void sffloor( sfarg * const p ); /* floor */
void sfabs( sfarg * const p ); /* abs */

//const eval
void sfcPI( sfNumber *cnst );
void sfcPI2( sfNumber *cnst );
void sfc2PI( sfNumber *cnst );
void sfcE( sfNumber *cnst );
void sfcEPS( sfNumber *cnst );
void sfcRND( sfNumber *cnst );


static sffunction sfcmplxfunc[sffnctscount] = 
	{ 
		/* basic real arithmetics */
		{sfpow,2,"^\0"}, {sfadd,2,"+\0"}, {sfsub,2,"-\0"}, {sfmul,2,"*\0"}, {sfdiv,2,"/\0"},
		/* ponizej uwzgledniaj w wyszukaniu funkcji */
		/* trygonometry */
		{sfsin,1,"SIN\0"}, {sfcos,1,"COS\0"}, {sftan,1,"TAN\0"}, {sfcot,1,"COT\0"},
		/* invert trygonometry */
		{sfasin,1,"ASIN\0"}, {sfacos,1,"ACOS\0"}, {sfatan,1,"ATAN\0"}, {sfacot,1,"ACOT\0"},
		{sfatan2,2,"ATAN2\0"}, 
		/* hyperbolic trygonometry */
		{sfsinh,1,"SINH\0"}, {sfcosh,1,"COSH\0"}, {sftanh,1,"TANH\0"}, {sfcoth,1,"COTH\0"},
		/* logs and exps */
		{sfexp,1,"EXP\0"}, {sflog,1,"LOG\0"}, {sflog10,1,"LOG10\0"}, {sflogN,2,"LOGN\0"},
		/* powers */
		{sfpow,2,"POW\0"},{sfsqr,1,"SQR\0"},{sfsqrt,1,"SQRT\0"},
		/* rounds */
		{sfceil,1,"CEIL\0"}, {sffloor,1,"FLOOR\0"}, {sfabs,1,"ABS\0"}
	 };

static char sfcnames[sfvarscount][5] = 
	{"PI\0","PI_2\0","PI2\0","E\0","EPS\0","RND\0"};
static cfptr sfcvals[sfvarscount] = 
	{ sfcPI, sfcPI2, sfc2PI, sfcE, sfcEPS, sfcRND };


 #ifdef __cplusplus
  }
 #endif

#endif
#endif

