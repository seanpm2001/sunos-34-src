/* @(#)umount.s 1.1 86/09/24 SMI */

#include "SYS.h"

#define SYS_umount	22
SYSCALL(umount)
	RET
