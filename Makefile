# zzsh makefile

CC = gcc
SRC = src/*.c
EXE = zzsh

STD = -std=c89
OPT = -O2 
WFLAGS = -Wall -Wextra -pedantic
INC = -Isrc

CFLAGS = $(STD) $(OPT) $(WFLAGS) $(INC)

$(EXE): $(SRC)
	$(CC) -o $@ $(SRC) $(CFLAGS)

exe: $(SRC)
	$(CC) -o $(EXE) $^ $(CFLAGS)

clean: build.sh
	./$^ $@
