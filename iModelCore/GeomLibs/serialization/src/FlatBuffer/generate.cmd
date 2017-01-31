@echo Generating primary h file ...
@echo .
@echo .
%srcRoot%libsrc\flatbuffers\bin\beflatc.exe -c allcg.flatbuf
@echo .
@echo .

touch FixedStructs.cpp


