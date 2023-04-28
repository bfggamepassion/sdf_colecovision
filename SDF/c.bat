@echo off
sdcc -mz80 -c --std-c99 --opt-code-speed bonus.c
del bonus.lst
del bonus.sym
del bonus.asm
sdcc -mz80 -c --std-c99 --opt-code-speed fonctions.c
del fonctions.lst
del fonctions.sym
del fonctions.asm
sdcc -mz80 -c --std-c99 --opt-code-speed gameOver.c
del gameOver.lst
del gameOver.sym
del gameOver.asm
sdcc -mz80 -c --std-c99 --opt-code-speed ingame1.c
del ingame1.lst
del ingame1.sym
del ingame1.asm
sdcc -mz80 -c --std-c99 --opt-code-speed main.c
del main.lst
del main.sym
del main.asm
sdasz80 -o musique.rel musique.s
sdcc -mz80 -c --std-c99 --opt-code-speed nextLevel.c
del nextLevel.lst
del nextLevel.sym
del nextLevel.asm
sdcc -mz80 -c --std-c99 --opt-code-speed nextLevel2.c
del nextLevel2.lst
del nextLevel2.sym
del nextLevel2.asm
sdcc -mz80 -c --std-c99 --opt-code-speed sprites_charset_sources.c
del sprites_charset_sources.lst
del sprites_charset_sources.sym
del sprites_charset_sources.asm
pause
