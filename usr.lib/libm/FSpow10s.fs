c       .data
c       .asciz  "@(#)FSpow10s.fs 1.1 86/09/25 Copyr 1986 Sun Micro"
c       .even
c       .text

c       Copyright (c) 1986 by Sun Microsystems, Inc.

	real*4 function FSpow10s ( x )
	real*4 x

	integer n
	real*4 xx, FFscales

	real*4 log210
	real*8 loge2, loge10
c	log2(10)
	parameter (log210 = 		3.32192 80948 87362 34787 e0)
c	loge(2)
	parameter (loge2 = 		0.6931471805599453094     d0)
c	loge(10)
	parameter (loge10 = 		2.30258 50929 94045 68401 d0)

	n = nint(x * log210)
	xx = dble(x) * loge10 - dble(n) * loge2
	FSpow10s = FFscales( exp(xx), n)
	end
