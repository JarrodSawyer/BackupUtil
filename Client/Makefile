CC = gcc

DEBUG_FLAGS = -g -DDEBUG

INC = -I../ErrorHandling -I../Messaging
OUT_DIR=./bin
CFLAGS = $(DEBUG_FLAGS) -Wall $(INC)

src = $(wildcard *.c)
obj = $(patsubst %.c, $(OUT_DIR)/%.o, $(src))

$(OUT_DIR)/Client.out : $(obj)
	$(CC) $(CFLAGS) -o $@ $^

$(obj): | $(OUT_DIR)

$(OUT_DIR)/%.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(OUT_DIR):
	Test -d $(OUT_DIR) || mkdir $(OUT_DIR)

.PHONY: clean
clean:
	rm -f $(obj) $(OUT_DIR)/Client.out *.*~