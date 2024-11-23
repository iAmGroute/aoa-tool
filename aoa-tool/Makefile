CC = gcc
CFLAGS = -Wall -Wextra -Wno-unused-function -std=c11 -O2 -flto -D_REENTRANT
TARGET = main.out

INCLUDES = -I../scrcpy-for-aoa/app/src/ -I../scrcpy-for-aoa/builddir/app/ -I../scrcpy-for-aoa/app/src/usb/

# LIB = -static -lusb-1.0 `pkg-config --libs --static sdl2`
# LIB = -static -lusb-1.0 -lSDL2 -Wl,--no-undefined -lm -ldl -lpthread -lX11 -lXext -lXcursor -lXinerama -lXi -lXrandr -lXss -lXxf86vm -lrt
# -lpulse-simple -lpulse -lwayland-egl -lwayland-client -lwayland-cursor -lxkbcommon
LIB = -lusb-1.0 -lSDL2

# detect with:
#   make |& grep -F 'undefined reference' | grep -Po '`[^'\'']+'\' | grep -o '[^`'\'']*' | sort -u | grep '^sc' | xargs -rL1 -d '\n' bash -c 'grep -lr "^$1" ../scrcpy-for-aoa/' -- | sort -u | grep '\.c$'
SRC = \
	../scrcpy-for-aoa/app/src/events.c \
	../scrcpy-for-aoa/app/src/usb/aoa_hid.c \
	../scrcpy-for-aoa/app/src/usb/keyboard_aoa.c \
	../scrcpy-for-aoa/app/src/usb/usb.c \
	../scrcpy-for-aoa/app/src/util/log.c \
	../scrcpy-for-aoa/app/src/util/thread.c \
	../scrcpy-for-aoa/app/src/hid/hid_keyboard.c \
	../scrcpy-for-aoa/app/src/util/acksync.c \
	../scrcpy-for-aoa/app/src/util/memory.c \
	../scrcpy-for-aoa/app/src/util/str.c \
	../scrcpy-for-aoa/app/src/util/tick.c \
	../scrcpy-for-aoa/app/src/util/strbuf.c \
	main.c

OBJ = $(SRC:.c=.o)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $(INCLUDES) -o $(TARGET) $(OBJ) $(LIB)

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

.PHONY: clean
clean:
	rm -f $(OBJ) $(TARGET)
