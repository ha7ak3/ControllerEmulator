xipade: emulator.c
	@echo building emulator...
	@${CC} `pkg-config --cflags gtk+-3.0` -o $@ $< `pkg-config --libs gtk+-3.0` `pkg-config --cflags --libs x11`
	@echo installing...
	@mv -f ./xipade /home/draxx/.local/bin/
	@echo done.