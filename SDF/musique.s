	.module musique
	
	.globl _direct_sound
	
	.area _CODE
	
	_direct_sound:
	pop     bc
    pop		de
    push    de
	push    bc
    push	ix
    push    iy
    ld		b,e
    ld		a,#255
    ld		c,a
    out    (c),b
    pop		iy
    pop     ix
	ret