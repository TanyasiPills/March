ifeq ($(origin HOST),undefined)
MAYOS:=0
CFLAGS?=-O2 -g
ifeq ($(OS),Windows_NT)
EXE:=.exe
endif
else
MAYOS:=1
include ../Makefile.config
endif

CFLAGS:=$(CFLAGS) -Wall -Wextra -std=gnu11 -Isrc/headers
CPPFLAGS:=$(CPPFLAGS) 

OBJS=$(patsubst %.c,%.o,$(wildcard src/source/*.c))

ifeq ($(MAYOS),1)
CFLAGS:=$(CFLAGS) -ffreestanding -fno-stack-protector -fno-pic -fno-pie -m64 -march=x86-64
CPPFLAGS:=$(CPPFLAGS) -DMAYOS -I$(DESTDIR)$(INCLUDEDIR)
LDFLAGS:=$(LDFLAGS) -nostdlib -static -T os/link.ld -z max-page-size=0x1000
LDLIBS:=$(LDLIBS) -L$(DESTDIR)$(LIBDIR) -lc $(shell $(CC) -print-libgcc-file-name)
OBJS:=os/crt0.o $(OBJS)
BINARY=march.elf
else
BINARY=March$(EXE)
endif

.PHONY: all clean install
.SUFFIXES: .o .c

all: $(BINARY)

$(BINARY): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS) $(LDLIBS)

.c.o:
	$(CC) -MD -c $< -o $@ $(CFLAGS) $(CPPFLAGS)

clean:
	rm -f march.elf March March.exe $(OBJS)
	rm -f $(OBJS:.o=.d)

run: all
	./March test.mh
	$(MAKE) clean

ifeq ($(MAYOS),1)
install: $(BINARY)
	mkdir -p $(DESTDIR)$(BOOTDIR)
	cp $(BINARY) $(DESTDIR)$(BOOTDIR)
else
install: $(BINARY)
	@echo "nothing to install for a native build"
endif

-include $(OBJS:.o=.d)
