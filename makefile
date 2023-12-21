ifeq ($(WINDOWS),1)
program := chess-windows-x86_64/chess.exe
archive := chess-windows-x86_64.zip
archive_target = chess-windows-x86_64/

CC := x86_64-w64-mingw32-gcc
AR := zip

LDFLAGS := -L./lib/Windows/ -lallegro_monolith-static -luser32 -lgdi32 -lcomdlg32 -lole32 -lwinmm -lkernel32 -lpsapi -lshlwapi -ld3d9 -ldinput8 -lstdc++ -lopengl32 -lglu32 -lgdiplus -luuid
ARFLAGS := -r

objs := src/main.c.obj src/render.c.obj src/board.c.obj src/move.c.obj src/vector.c.obj src/algebraic_notation.c.obj

deps := $(patsubst %.c.obj,%.d,$(objs))
else
program := chess-x11-x86_64/chess
archive := chess-x11-x86_64.tar.gz
archive_target = chess-x11-x86_64/

CC := gcc
AR := tar

LDFLAGS := -L./lib/Linux/ -lallegro_monolith-static -lm -lSM -lICE -lX11 -lXext -lXcursor -lXpm -lXi -lXinerama -lXrandr -lXss -lOpenGL -lGLU -lGLX -lpng -lz -lgtk-3 -lgdk-3 -lz -lpangocairo-1.0 -lpango-1.0 -lharfbuzz -latk-1.0 -lcairo-gobject -lcairo -lgdk_pixbuf-2.0 -lgio-2.0 -lgobject-2.0 -lglib-2.0 -lgthread-2.0 -lglib-2.0
ARFLAGS := chfvz

objs := src/main.o src/render.o src/board.o src/move.o src/vector.o src/algebraic_notation.o

deps := $(patsubst %.o,%.d,$(objs))
endif

-include $(deps)

CFLAGS := -Wall -Wextra -Werror -MMD -I./include/

ifneq ($(D),1)
CFLAGS += -O3
else
CFLAGS += -g -DDEBUG
endif

all: $(program)

$(program): $(objs)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

%.c.obj: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

archive: $(archive)

$(archive): $(program)
	$(AR) $(ARFLAGS) $@ $(archive_target)

clean:
	rm -f $(program) $(objs) $(deps) $(archive)
