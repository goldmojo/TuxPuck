# Makefile for TuxPuck , Copyright Jacob Kroon 2001-2002
NAME		= tuxpuck
VERSION		= 0.9.0-prealpha1

TOOLCHAIN	= /opt/gcw0-toolchain/usr
CC			= $(TOOLCHAIN)/bin/mipsel-linux-gcc
SDL_CONFIG  = $(TOOLCHAIN)/mipsel-gcw0-linux-uclibc/sysroot/usr/bin/sdl-config

CFLAGS		+= -D_VERSION=\"$(VERSION)\" -g -Wall -ansi -O3 -march=mips32r2 -mtune=mips32r2

CSOURCES	= tuxpuck.c video.c audio.c menu.c sprite.c font.c timer.c \
		  board.c entity.c glass.c scoreboard.c player.c zoom.c png.c \
		  jpg.c intro.c tux.c arcana.c buff.c a1d2.c smasher.c lex.c \
		  morth.c sam.c coler.c tin.c 

INCLUDES	= tuxpuck.h video.h audio.h font.h

#############################################################

OBJS=$(CSOURCES:.c=.o)

%.o	: %.c
	$(CC) $(CFLAGS) `$(SDL_CONFIG) --cflags` -c -o $@ $<

$(NAME) : $(OBJS)
	cd data; $(MAKE)
	$(CC) $(CFLAGS) $(OBJS) data/libdata.a `$(SDL_CONFIG) --libs` -lm -lpng \
	-ljpeg -lz -lvorbisfile -lvorbis -logg -o $(NAME)

clean :
	cd utils; $(MAKE) clean;
	cd data; $(MAKE) clean;
	rm -f *~ $(OBJS) $(NAME)
	rm -f opkdata/$(NAME)
	rm -f TuxPuck.opk

opk : $(NAME)
	cp -f $(NAME) opkdata
	mksquashfs opkdata TuxPuck.opk
