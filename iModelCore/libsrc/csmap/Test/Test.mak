C_FLG = -c -DGCC_3 -D__CPP__ -Wall -O3 -I../Include

CPP_FLG = -c -DGCC_3 -D__CPP__ -Wall -O3 -I../Include

.cpp.o:
	gcc $(CPP_FLG) $<

.c.o:
	gcc $(C_FLG) $<

TEST_SRC = \
CS_test.c \
CStest1.c \
CStest2.c \
CStest3.c \
CStest4.c \
CStest5.c \
CStest6.c \
CStest7.c \
CStest8.c \
CStest9.c \
CStestA.c \
CStestB.c \
CStestC.c \
CStestD.c \
CStestE.c \
CStestF.c \
CStestG.c \
CStestH.c \
CStestI.c \
CStestJ.c \
CStestK.c \
CStestL.c \
CStestS.c \
CStestSupport.c \
CStestT.c

TEST_OBJ = \
CS_test.o \
CStest1.o \
CStest2.o \
CStest3.o \
CStest4.o \
CStest5.o \
CStest6.o \
CStest7.o \
CStest8.o \
CStest9.o \
CStestA.o \
CStestB.o \
CStestC.o \
CStestD.o \
CStestE.o \
CStestF.o \
CStestG.o \
CStestH.o \
CStestI.o \
CStestJ.o \
CStestK.o \
CStestL.o \
CStestS.o \
CStestSupport.o \
CStestT.o

CS_Test : $(TEST_OBJ)
	gcc -v -o CS_Test $(TEST_OBJ) ../Source/CsMap.a -lm -lc -lgcc -lstdc++


