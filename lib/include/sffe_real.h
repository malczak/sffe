/*/////////////////////////////////////////////////////////////////////////////////////
// project : sFFe ( SegFault (or Segmentation Fault :) ) formula evalutaor )
// author  : Mateusz Malczak ( mateusz@malczak.info )
// wpage   : 
/////////////////////////////////////////////////////////////////////////////////////*/
#ifndef SFFE_DOUBLE_H
#define SFFE_DOUBLE_H

#ifdef SFFE_DOUBLE

#include "sffe.h"

#define sffnctscount 28
#define sfvarscount 6

 #ifdef __cplusplus
  extern "C" {
 #endif

sfarg *sfadd( sfarg * const p ); /*  +  */
sfarg *sfsub( sfarg * const p ); /*  -  */
sfarg *sfmul( sfarg * const p ); /*  *  */
sfarg *sfdiv( sfarg * const p ); /*  /  */
sfarg *sfsin( sfarg * const p ); /* sin */
sfarg *sfcos( sfarg * const p ); /* cos */
sfarg *sftan( sfarg * const p ); /* tan */
sfarg *sfcot( sfarg * const p ); /* ctan */
sfarg *sfasin( sfarg * const p ); /* asin */
sfarg *sfacos( sfarg * const p ); /* acos */
sfarg *sfatan( sfarg * const p ); /* atan */
sfarg *sfacot( sfarg * const p ); /* actan */
sfarg *sfatan2( sfarg * const p ); /* atan2 */
sfarg *sfsinh( sfarg * const p ); /* sinh */
sfarg *sfcosh( sfarg * const p ); /* cosh */
sfarg *sftanh( sfarg * const p ); /* tanh */
sfarg *sfcoth( sfarg * const p ); /* ctanh */
sfarg *sfexp( sfarg * const p ); /* exp */
sfarg *sflog( sfarg * const p ); /* log */
sfarg *sflog10( sfarg * const p ); /* log10 */
sfarg *sflogN( sfarg * const p ); /* logN */
sfarg *sfpow( sfarg * const p ); /* pow */
sfarg *sfsqr( sfarg * const p ); /* sqr */
sfarg *sfsqrt( sfarg * const p ); /* sqrt */
sfarg *sfceil( sfarg * const p ); /* ceil */
sfarg *sffloor( sfarg * const p ); /* floor */
sfarg *sfabs( sfarg * const p ); /* abs */

//const eval
void sfcPI( sfNumber *cnst );
void sfcPI2( sfNumber *cnst );
void sfc2PI( sfNumber *cnst );
void sfcE( sfNumber *cnst );
void sfcEPS( sfNumber *cnst );
void sfcRND( sfNumber *cnst );

 #ifdef __cplusplus
  }
 #endif

/* all available function (function pointer, number of parameters, name )*/
extern const sffunction sfcmplxfunc[sffnctscount];
/* all available buildin variables */
extern const char sfcnames[sfvarscount][5];
/* available variables function pointers */
extern const cfptr sfcvals[sfvarscount];

#endif
#endif

