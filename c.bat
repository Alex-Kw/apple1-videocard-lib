@echo ======================== APPLE 1 =================================================
call kickc -t asm6502 -D=APPLE1 test.c -o test_apple1.prg -e
copy test.prg test_apple1.prg
@echo ======================== VIC20 ===================================================
call kickc -t VIC20 -D=VIC20 test.c -o test_vic20.prg -e
rem call kickc -vasmoptimize -vasmout -vcreate -vfragment -vliverange -vloop -vnonoptimize -voptimize -vparse -vsequence -vsizeinfo -vunroll -vuplift -t VIC20 -D=VIC20 test.c -o test_vic20.prg -e
copy test.prg test_vic20.prg
