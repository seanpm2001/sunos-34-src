#! /bin/sh
#
#	@(#)lint.sh 1.1 86/09/25 SMI; from UCB 1.5 04/09/83
#
L=/usr/lib/lint/lint T=/usr/tmp/lint.$$ PATH=/bin:/usr/bin O="-C -Dlint"
X= P=unix LL=/usr/lib/lint C=
trap "rm -f $T; exit" 1 2 15
for A in $*
do
	case $A in
	-*n*)	P= ;;
	esac
	case $A in
	*.ln)	cat $A >>$T ;;
	-l*)	cat $LL/llib$A.ln >>$T ;;
	-C?*)	P= C=`echo $A | sed -e s/-C/llib-l/` ; X="$X -L -C$C" ;;
	-[IDOU]*)	O="$O $A" ;;
	-X)	LL=/usr/src/usr.bin/lint L=/usr/src/usr.bin/lint/lpass ;;
	-*)	X="$X $A" ;;
	*)	echo "$A:" ; (/lib/cpp $O $A | ${L}1 $X >>$T)2>&1
	esac
	done
case $P in
	unix)	cat $LL/llib-lc.ln >>$T ;;
	"")	cat /dev/null >>$T ;;
	esac
case $C in
	"")	${L}2 $T $X ;;
	*)	cp $T $C.ln ;;
	esac
rm -f $T
