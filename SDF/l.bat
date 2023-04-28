@echo off
sdcc -mz80 --code-loc 0x8048 --data-loc 0x7000 --no-std-crt0 ../crtcv.rel ../cvlib.lib ../getput.lib bonus.rel fonctions.rel gameOver.rel ingame1.rel main.rel musique.rel nextLevel.rel nextLevel2.rel sprites_charset_sources.rel
objcopy --input-target=ihex --output-target=binary crtcv.ihx result.rom
del *.ihx
del crtcv.lnk
pause
