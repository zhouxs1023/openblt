# $Id: //depot/blt/srv/Makefile#3 $

all: namer console ne2000 fish video vfs

console::
	@echo "--- srv/console ---------"
	@cd console ; $(MAKE)

namer::
	@echo "--- srv/namer -----------"
	@cd namer ; $(MAKE)

ne2000::
	@echo "--- srv/ne2000 ----------"
	@cd ne2000 ; $(MAKE)

fish::
	@echo "--- srv/fish ------------"
	@cd fish ; $(MAKE)

video::
	@echo "--- srv/video -----------"
	@cd video ; $(MAKE)

vfs::
	@echo "--- srv/vfs -------------"
	@cd vfs ; $(MAKE)

clean:
	@cd console ; $(MAKE) clean
	@cd namer ; $(MAKE) clean
	@cd ne2000 ; $(MAKE) clean
	@cd video ; $(MAKE) clean
	@cd fish ; $(MAKE) clean
