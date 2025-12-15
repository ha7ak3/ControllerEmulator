CC     = gcc
CFLAGS = $(shell pkg-config --cflags gtk+-3.0)
LIBS   = $(shell pkg-config --libs gtk+-3.0 x11)

xipade: emulator.c
	@echo Building Emulator...
	@${CC} ${CFLAGS} -o $@ $< ${LIBS}
	@echo Installing...
	@mv -f ./xipade ${HOME}/.local/bin/
	@echo Done
