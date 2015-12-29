.intel_syntax

.global _enable_irq
.global _disable_irq

.text
_disable_irq:
	cli
	jmp fim

_enable_irq:
	sti
fim: ret
