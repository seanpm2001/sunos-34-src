/*
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#ifndef lint
static char sccsid[] = "@(#)cont.c 1.1 86/09/25 SMI"; /* from UCB 5.1 5/7/85 */
#endif not lint

#include "gigi.h"

cont(xi,yi)
int xi,yi;
{
	currentx = xsc(xi);
	currenty = ysc(yi);
	printf("V[%d,%d]",currentx, currenty);
}
