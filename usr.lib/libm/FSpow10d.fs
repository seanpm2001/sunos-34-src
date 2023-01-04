c       .data
c       .asciz  "@(#)FSpow10d.fs 1.1 86/09/25 Copyr 1986 Sun Micro"
c       .even
c       .text

c       Copyright (c) 1986 by Sun Microsystems, Inc.

	real*8 function FSpow10d ( x )
	real*8 x

	integer n
	real*8 dn, xx, FFscaled

	real*8 log210, log102a, loge10m2, loge10log102b
c	log2(10)
	parameter (log210 = 		3.32192 80948 87362 34787 d0)
c	log10(2) rounded to 40 significant bits = .9a20 9a84 fc h-1
	parameter (log102a = 		3.010299956640665187d-1)
c	loge(10)-2
	parameter (loge10m2 = 		0.30258 50929 94045 68401 d0)
c	loge(10) * (log10(2)-log102a) = loge10 * -8.532344317057106531E-14 
	parameter (loge10log102b = 	-1.964644883274815485d-13)

	n = nint(x * log210)
	dn = n
	xx = x - dn * log102a
	xx = (xx + xx) + (loge10m2 * xx - dn * loge10log102b)
	FSpow10d = FFscaled( exp(xx), n)
	end
