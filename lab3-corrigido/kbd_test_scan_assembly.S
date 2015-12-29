#include "8042.h"

.global _obf
.global _err
.global _testscan
.data
_obf: .byte 0
_err: .byte 0

.text
_testscan:
	xor	 %eax,%eax
	inb  $STATUS_REG,%al
	xor  %ecx,%ecx
	or	 $PARITY_ERROR,%cl
	or	 $TIMEOUT_ERROR, %cl
	or 	 $OBF, %cl
	and  %al,%cl	# err = 1
	cmp  $1,%cl
	jne  next
	movb $0,%cl	# err = 0
	next: inb	$OUT_BUF,%al
	movb %al,_obf
	movb %cl,_err
	fim: ret
