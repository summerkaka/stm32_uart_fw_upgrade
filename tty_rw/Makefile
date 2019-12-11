
#$@--目标文件，$^--所有的依赖文件，$<--第一个依赖文件。
#! before make, need 'mkdir ./obj' manually

CC = gcc
TARGET = fwupdate
SRC_PATH = ./src
INC_PATH = ./inc
OBJ_PATH = ./obj
CFLAGS = -g -Wall -O0 -I${INC_PATH} 

SRC = $(wildcard ${SRC_PATH}/*.c)
# SRC = $(shell find ./ -name "*.c")
HEADER= $(wildcard $(INC_PATH)/*.h)
OBJ = $(patsubst %.c, $(OBJ_PATH)/%.o, $(notdir ${SRC})) # use notdir: remove dir info in SRC, eg: './src/fw-update.c' --> 'fw-update.c'
#OBJ = $(SOURCE_LIST:%.c=%.o)

${TARGET} : $(OBJ) 
	$(CC) $(CFLAGS) -o $@ $(OBJ)

${OBJ_PATH}/%.o : ${SRC_PATH}/%.c 
	$(CC) $(CFLAGS) -o $@ -c $<
	
.PHONY:clean
clean:
	find $(OBJ_PATH) -name *.o -exec rm -rf {} \; 
	rm -rf $(TARGET)
    