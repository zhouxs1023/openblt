# $Id: //depot/blt/Makefile#3 $

include make.conf

all: lib kernel srv boot util image

boot:: 
	@echo "--- boot ----------------"
	@cd boot ; $(MAKE)

kernel::
	@echo "--- kernel --------------"
	@cd kernel ; $(MAKE)

lib::
	@echo "--- lib -----------------"
	@cd lib ; $(MAKE)

srv::
	@echo "--- srv -----------------"
	@cd srv ; $(MAKE)

util::
	@echo "--- util ----------------"
	@cd util ; $(MAKE)

netboot::
	@echo "--- netboot -------------"
	@cd netboot ; $(MAKE)

image:
	./util/bootmaker boot/openblt.ini boot/openblt.boot

run: image
	./util/netboot boot/openblt.boot $(IP)


image2:
	./util/bootmaker boot/test.ini boot/test.boot

run2: image2
	./util/netboot boot/test.boot $(IP)

clean:
	@cd lib ; $(MAKE) clean
	@cd kernel ; $(MAKE) clean
	@cd srv ; $(MAKE) clean
	@cd boot ; $(MAKE) clean
	@cd netboot ; $(MAKE) clean
	@rm -f boot/openblt.boot

