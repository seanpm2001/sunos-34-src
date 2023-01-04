
/*
 * static char     fpasccsid[] = "@(#)linpack.c 1.1 2/28/86 Copyright Sun Microsystems";
 */
lin_pack_test()
{
	void slinsub_() ;
	void dlinsub_() ;
double *residn, *resid, *eps, *x11, *xn1 ;
double presidn, presid, peps, px11, pxn1 ;
double dresidn, dresid, deps, dx11, dxn1 ;
double sresidn, sresid, seps, sx11, sxn1 ;

sresidn = 1.59605330360994160e+00 ;
sresid  = 3.80277633666992190e-05 ;
seps    = 1.19209289550781250e-07 ;
sx11    = -1.38282775878906250e-05 ;
sxn1    = -7.51018524169921880e-06 ;
dresidn = 1.67117300351187210e+00 ;
dresid  = 7.41628980449604570e-14 ;
deps    = 2.22044604925031310e-16 ;
dx11    = -1.49880108324396130e-14 ;
dxn1    = -1.89848137210901770e-14 ;
residn = & presidn ;
resid = & presid ;
eps = & peps ;
x11 = & px11 ;
xn1 = & pxn1 ;
slinsub_(residn, resid, eps, x11, xn1) ;
/*printf(" Single %25.16e %25.16e %25.16e %25.16e %25.16e\n",*residn, *resid, *eps, *x11, *xn1) ; */
if (!((*residn == sresidn) && (*resid == sresid) && (*eps == seps) && 
	(*x11 == sx11) && (*xn1 == sxn1))) 
	return(-1);

dlinsub_(residn, resid, eps, x11, xn1) ;
/*printf(" Double %25.16e %25.16e %25.16e %25.16e %25.16e\n",*residn, *resid, *eps, *x11, *xn1) ; */
if ((*residn == dresidn) && (*resid == dresid) && (*eps == deps) && 
	(*x11 == dx11) && (*xn1 == dxn1)) 
		return(0);
/*	printf(" double PASSES \n") ; */
else{
/*	printf(" double FAILS \n") ; */
	return(-1);
    }
}

