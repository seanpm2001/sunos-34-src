#ifdef sccsid
static	char sccsid[] = "@(#)CSatand.cs 1.1 86/09/25 SMI"; /* from UCB X.X XX/XX/XX */
#endif

#include "libmdefs.h"

/******************************

This code was derived from

 ******************************/

/* ATAN2(Y,X)
 * RETURN ARG (X+iY)
 * DOUBLE PRECISION (VAX D format 56 bits, IEEE DOUBLE 53 BITS)
 * CODED IN C BY K.C. NG, 1/8/85; REVISED BY K.C. NG, 2/7/85.
 *
 * Required system supported functions :
 *	copysign(x,y)
 *	scalb(x,y)
 *	logb(x)
 *	
 * Method :
 *	1. Reduce y to positive by atan2(y,x)=-atan2(-y,x).
 *	2. Reduce x to positive by (if x and y are unexceptional): 
 *		ARG (x+iy) = arctan(y/x)   	   ... if x > 0,
 *		ARG (x+iy) = pi - arctan[y/(-x)]   ... if x < 0,
 *	3. According to the integer k=4t+0.25 truncated , t=y/x, the argument 
 *	   is further reduced to one of the following intervals and the 
 *	   arctangent of y/x is evaluated by the corresponding formula:
 *
 *         [0,7/16]	   atan(y/x) = t - t^3*(a1+t^2*(a2+...(a10+t^2*a11)...)
 *	   [7/16,11/16]    atan(y/x) = atan(1/2) + atan( (y-x/2)/(x+y/2) )
 *	   [11/16,19/16]   atan(y/x) = atan( 1 ) + atan( (y-x)/(x+y) )
 *	   [19/16,39/16]   atan(y/x) = atan(3/2) + atan( (y-1.5x)/(x+1.5y) )
 *	   [39/16,INF]     atan(y/x) = atan(INF) + atan( -x/y )
 *
 * Special cases:
 * Notations: atan2(y,x) == ARG (x+iy) == ARG(x,y).
 *
 *	ARG(+(anything), +-0) is +-0  ;
 *	ARG(-(anything), +-0) is +-PI ;
 *	ARG( 0, +-(anything but 0) ) is +-PI/2;
 *	ARG( +INF,+-(anything but INF) ) is +-0 ;
 *	ARG( -INF,+-(anything but INF) ) is +-PI;
 *	ARG( (anything but INF),+-INF ) is +-PI/2;
 *	ARG( +-INF,+INF ) is +-PI/4 ;
 *	ARG( +-INF,-INF ) is +-3PI/4;
 *	ARG( NAN , (anything but 0 and INF) ) is NAN;
 *	ARG( (anything but 0 and INF), NAN ) is NAN.
 *
 * Accuracy:
 *	atan2(y,x) returns 
 *			PI/pi * the exact ARG (x+iy) 
 *	nearly rounded, where
 *
 *	Decimal:
 *		pi = 3.141592653589793 23846264338327 ..... 
 *    53 bits   PI = 3.141592653589793 115997963 ..... ,
 *    56 bits   PI = 3.141592653589793 227020265 ..... ,  
 *
 *	Hexadecimal:
 *		pi = 3.243F6A8885A308D313198A2E....
 *    53 bits   PI = 3.243F6A8885A30  =  2 * 1.921FB54442D18	error=.276ulps
 *    56 bits   PI = 3.243F6A8885A308 =  4 * .C90FDAA22168C2    error=.206ulps
 *	
 *	For atan2(y,1.0) = arctan(y), the accuracy is better than 0.89 ulps 
 *	compared with PI/pi*ARG(1+iy) according to an error analysis. The 
 *	maximum error observed under Z. Liu's test on is 0.86 ulps.
 *
 * Note:
 *	We regard the machine PI (the true pi rounded) as if the actual
 *	value of pi for all the trig and inverse trig functions. In general, 
 *	if trig is one the sin, cos, tan, then trig(y) returns the exact 
 *	trig(y*pi/PI) nearly rounded; correspondingly, arctrig return 
 *	the exact arctrig(y)*PI/pi nearly rounded. These guarantee the 
 *	trig functions have period PI, and trig(arctrig(x)) returns x 
 *	nearly rounded for any x.
 *	
 * Constants:
 * The hexadecimal values are the intended ones for the following constants.
 * The decimal values may be used, provided that the compiler will convert
 * from decimal to binary accurately enough to produce the hexadecimal values
 * shown.
 */

static double 
athfhi =  4.6364760900080609352E-1    , /*Hex  2^ -2   *  1.DAC670561BB4F */
athflo =  4.6249969567426939759E-18   , /*Hex  2^-58   *  1.5543B8F253271 */
PIo4   =  7.8539816339744827900E-1    , /*Hex  2^ -1   *  1.921FB54442D18 */
at1fhi =  9.8279372324732905408E-1    , /*Hex  2^ -1   *  1.F730BD281F69B */
at1flo = -2.4407677060164810007E-17   , /*Hex  2^-56   * -1.C23DFEFEAE6B5 */
PIo2   =  1.5707963267948965580E0     , /*Hex  2^  0   *  1.921FB54442D18 */
PI     =  3.1415926535897931160E0     , /*Hex  2^  1   *  1.921FB54442D18 */
a1     =  3.3333333333333942106E-1    , /*Hex  2^ -2   *  1.55555555555C3 */
a2     = -1.9999999999979536924E-1    , /*Hex  2^ -3   * -1.9999999997CCD */
a3     =  1.4285714278004377209E-1    , /*Hex  2^ -3   *  1.24924921EC1D7 */
a4     = -1.1111110579344973814E-1    , /*Hex  2^ -4   * -1.C71C7059AF280 */
a5     =  9.0908906105474668324E-2    , /*Hex  2^ -4   *  1.745CE5AA35DB2 */
a6     = -7.6919217767468239799E-2    , /*Hex  2^ -4   * -1.3B0FA54BEC400 */
a7     =  6.6614695906082474486E-2    , /*Hex  2^ -4   *  1.10DA924597FFF */
a8     = -5.8358371008508623523E-2    , /*Hex  2^ -5   * -1.DE125FDDBD793 */
a9     =  4.9850617156082015213E-2    , /*Hex  2^ -5   *  1.9860524BDD807 */
a10    = -3.6700606902093604877E-2    , /*Hex  2^ -5   * -1.2CA6C04C6937A */
a11    =  1.6438029044759730479E-2    ; /*Hex  2^ -6   *  1.0D52174A1BB54 */

double CSatand(y)
double  y;
{  
	static double one=1.0, half=0.5, small=1.0E-9, big=1.0E18;
	double t,z,hi,lo;
	int k, yminus;

	yminus = dminus(y) ;
	y=fabs(y); t=y;

	if (t < 2.4375) {		 

	/* truncate 4(t+1/16) to integer for branching */
	    k = 4 * (t+0.0625);
	    switch (k) {

	    /* t is in [0,7/16] */
	    case 0:                    
	    case 1:
		if (t < small) 
		      if (yminus) 
			return(-y) ;
			else
			return(y) ;

		hi = 0.0;  lo = 0.0;  break;

	    /* t is in [7/16,11/16] */
	    case 2:                    
		hi = athfhi; lo = athflo;
		z = 2.0;
		t = ( (y+y) - 1.0 ) / ( z +  y ); break;

	    /* t is in [11/16,19/16] */
	    case 3:                    
	    case 4:
		hi = PIo4; lo = 0.0;
		t = ( y - 1.0 ) / ( 1.0 + y ); break;

	    /* t is in [19/16,39/16] */
	    default:                   
		hi = at1fhi; lo = at1flo;
		z = y-1.0; y=y+y+y; t = 2.0;
		t = ( (z+z)-1.0 ) / ( t + y ); break;
	    }
	}
	/* end of if (t < 2.4375) */

	else                           
	{
	    hi = PIo2; lo = 0.0;

	    /* t is in [2.4375, big] */
	    if (t <= big)  t = - 1.0 / y;

	    /* t is in [big, INF] */
	    else          
	      { if (t > big ) t = 0.0; /* else t = y = Nan */}
	}
    /* end of argument reduction */

    /* compute atan(t) for t in [-.4375, .4375] */
	z = t*t;
	z = t*(z*(a1+z*(a2+z*(a3+z*(a4+z*(a5+z*(a6+z*(a7+z*(a8+
			z*(a9+z*(a10+z*a11)))))))))));
	z = lo - z; z += t; z += hi;

	if (yminus) 
		return(-z) ;
	else
		return(z) ;
}

