#ifndef lint
static	char sccsid[] = "@(#)space.c 1.1 86/09/25 SMI"; /* from UCB 4.1 6/27/83 */
#endif

# include "con.h"
space(x0,y0,x1,y1){
	botx = -2047.;
	boty = -2047.;
	obotx = x0;
	oboty = y0;
	scalex = 4096./(x1-x0);
	scaley = 4096./(y1-y0);
}
