
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
HEADER= $(wildcard $(INC_PATH)/*.h)							#展开匹配此模式的所有文件 用空格分开
OBJ = $(patsubst %.c, $(OBJ_PATH)/%.o, $(notdir ${SRC})) 	# patsubst <pattern>,<replacement>,<text> 
															# 查找<text>中的单词（单词以“空格”、“Tab”或“回车”“换行”分隔）是否符合模式<pattern>，如果匹配的话，则以<replacement>替换
															# use notdir: remove dir info in SRC, eg: './src/fw-update.c' --> 'fw-update.c'
#OBJ = $(SOURCE_LIST:%.c=%.o)

${TARGET} : $(OBJ) 
	$(CC) $(CFLAGS) -o $@ $(OBJ)

${OBJ_PATH}/%.o : ${SRC_PATH}/%.c 
	$(CC) $(CFLAGS) -o $@ -c $<
	
.PHONY:clean
clean:
	find $(OBJ_PATH) -name *.o -exec rm -rf {} \; 
	rm -rf $(TARGET)
    