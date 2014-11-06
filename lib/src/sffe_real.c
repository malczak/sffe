#ifdef SFFE_DOUBLE
#include "sffe.h"

#include <math.h>
#include <string.h>

#define sfEPSILON 1e-16
 
/*! IMPLEMENTACJA WSZSYTKICH DOOSTEPNYCH W STANDARDZIE FUNKCJI RZECZYWISTYCH !*/
#ifdef __cplusplus
 extern "C" {
#endif

sfarg *sfadd( sfarg * const p ) /* + */
{
 sfvalue(p) = sfvalue( sfaram2(p) ) + sfvalue( sfaram1(p) );
 return sfaram2(p);
};

sfarg *sfsub( sfarg * const p ) /* - */
{
 sfvalue(p) = sfvalue( sfaram2(p) ) - sfvalue( sfaram1(p) );
 return sfaram2(p);
};

sfarg *sfmul( sfarg * const p ) /* *  */
{
 sfvalue(p) = sfvalue( sfaram2(p) ) * sfvalue( sfaram1(p) );
 return sfaram2(p);
};

sfarg *sfdiv( sfarg * const p ) /*  / */
{
 sfvalue(p) = sfvalue( sfaram2(p) ) / sfvalue( sfaram1(p) );
 return sfaram2(p);
};


sfarg *sfsin( sfarg * const p ) /* sin */
{
 sfvalue(p) = sin( sfvalue( sfaram1(p) ) );
 return sfaram1(p);
};

sfarg *sfcos( sfarg * const p ) /* cos */
{
 sfvalue(p) = cos( sfvalue( sfaram1(p) ) );
 return sfaram1(p);
};

sfarg *sftan( sfarg * const p ) /* tan */
{
 sfvalue(p) = tan( sfvalue( sfaram1(p) ) );
 return sfaram1(p);
};

sfarg *sfcot( sfarg * const p ) /* ctan */
{
 sfvalue(p) = 1.0/tan( sfvalue( sfaram1(p) ) );
 return sfaram1(p);
};


sfarg *sfasin( sfarg * const p ) /* asin */
{
 sfvalue(p) = asin( sfvalue( sfaram1(p) ) );
 return sfaram1(p);
};

sfarg *sfacos( sfarg * const p ) /* acos */
{
 sfvalue(p) = acos( sfvalue( sfaram1(p) ) );
 return sfaram1(p);
};

sfarg *sfatan( sfarg * const p ) /* atan */
{
 sfvalue(p) = atan( sfvalue( sfaram1(p) ) );
 return sfaram1(p);
};

sfarg *sfacot( sfarg * const p ) /* actan */
{
 sfvalue(p) = 1.0/atan( sfvalue( sfaram1(p) ) );
 return sfaram1(p);
};

sfarg *sfatan2( sfarg * const p ) /* atan2 */
{
 sfvalue(p) = atan2( sfvalue( sfaram2(p) ), sfvalue( sfaram1(p) ) );
 return sfaram2(p);
};


sfarg *sfsinh( sfarg * const p ) /* sinh */
{
 sfvalue(p) = sinh( sfvalue( sfaram1(p) ) );
 return sfaram1(p);
};

sfarg *sfcosh( sfarg * const p ) /* cosh */
{
 sfvalue(p) = cosh( sfvalue( sfaram1(p) ) );
 return sfaram1(p);
};

sfarg *sftanh( sfarg * const p ) /* tanh */
{
 sfvalue(p) = tanh( sfvalue( sfaram1(p) ) );
 return sfaram1(p);
};

sfarg *sfcoth( sfarg * const p ) /* ctanh */
{
 sfvalue(p) = 1.0/tanh( sfvalue( sfaram1(p) ) );
 return sfaram1(p);
};


sfarg *sfexp( sfarg * const p ) /* exp */
{
 sfvalue(p) = exp( sfvalue( sfaram1(p) ) );
 return sfaram1(p);
};

sfarg *sflog( sfarg * const p ) /* log */
{
 sfvalue(p) = log( sfvalue( sfaram1(p) ) );
 return sfaram1(p);
};

sfarg *sflog10( sfarg * const p ) /* log10 */
{
 sfvalue(p) = log10( sfvalue( sfaram1(p) ) );
 return sfaram1(p);
};

sfarg *sflogN( sfarg * const p ) /* logN */
{
 sfvalue(p) = log( sfvalue( sfaram1(p) ) )/log( sfvalue( sfaram2(p) ) );;
 return sfaram2(p);
};

sfarg *sfpow( sfarg * const p ) /* pow */
{
 sfvalue(p) = pow( sfvalue( sfaram2(p) ), sfvalue( sfaram1(p) ) );
 return sfaram2(p);
};

sfarg *sfsqr( sfarg * const p ) /* sqr */
{
 sfvalue(p) = sfvalue( sfaram1(p) )*sfvalue( sfaram1(p) );
 return sfaram1(p);
};

sfarg *sfsqrt( sfarg * const p ) /* sqrt */
{
 sfvalue(p) = sqrt( sfvalue( sfaram1(p) ) );
 return sfaram1(p);
};

sfarg *sfceil( sfarg * const p ) /* ceil */
{
 sfvalue(p) = ceil( sfvalue( sfaram1(p) ) );
 return sfaram1(p);
};

sfarg *sffloor( sfarg * const p ) /* floor */
{
 sfvalue(p) = floor( sfvalue( sfaram1(p) ) );
 return sfaram1(p);
};

sfarg *sfabs( sfarg * const p ) /* abs */
{
 sfvalue(p) = fabs( sfvalue( sfaram1(p) ) );
 return sfaram1(p);
};

//const eval
sfarg *sfcPI( sfNumber *cnst ) { *cnst=4*atan(1); };
sfarg *sfcPI2( sfNumber *cnst ) { *cnst=2*atan(1); };
sfarg *sfc2PI( sfNumber *cnst ) { *cnst=8*atan(1); };
sfarg *sfcE( sfNumber *cnst ) { *cnst=exp(1); };
sfarg *sfcEPS( sfNumber *cnst ) { *cnst=sfEPSILON; };
sfarg *sfcRND( sfNumber *cnst ) { *cnst=(double)rand()/(double)RAND_MAX; };

#ifdef __cplusplus
 }
#endif

#endif
