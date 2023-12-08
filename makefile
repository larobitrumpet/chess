program := chess

CC := gcc

CFLAGS := -Wall -Wextra -Werror -MMD -I./include/
LDFLAGS := $$(pkg-config --libs allegro-5 allegro_image-5 allegro_dialog-5)

ifneq ($(D),1)
CFLAGS += -O3
else
CFLAGS += -g -DDEBUG
endif

all: $(program)

objs := main.o render.o board.o move.o vector.o

deps := $(patsubst %.o,%.d,$(objs))
-include $(deps)

chess: $(objs)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

clean:
	rm -f $(program) $(objs) $(deps)
