xipade:
	@echo building emulator...
	@gcc ./emulator.c -o xipade
	@echo installing...
	@chown root:root ./xipade
	@chmod u+s ./xipade
	@chmod 4755 ./xipade
	@mv -f ./xipade /home/draxx/.local/bin/
	@echo done.
