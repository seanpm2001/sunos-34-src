#ifndef lint
static char sccsid[] = "@(#)setsegattpas.c 1.1 86/09/25 Copyr 1983 Sun Micro";
#endif

/*
 Copyright (c) 1983 by Sun Microsystems, Inc.
*/

int set_segment_detectability();
int set_segment_highlighting();
int set_segment_image_transformation_2();
int set_segment_image_translate_2();
int set_segment_visibility();

int setsegdetectable(segname, detectbl)
int segname, detectbl;
	{
	return(set_segment_detectability(segname, detectbl));
	}

int setseghighlight(segname, highlght)
int segname, highlght;
	{
	return(set_segment_highlighting(segname, highlght));
	}

int setsegimgxform2(segname, sx, sy, a, tx, ty)
int segname;
double sx, sy, a, tx, ty;
	{
	return(set_segment_image_transformation_2(segname,sx,sy,a,tx,ty));
	}

int setsegimgxlate2(segname, tx, ty)
int segname;
double tx, ty;
	{
	return(set_segment_image_translate_2(segname, tx, ty));
	}

int setsegvisibility(segname, visbilty)
int segname, visbilty;
	{
	return(set_segment_visibility(segname, visbilty));
	}
