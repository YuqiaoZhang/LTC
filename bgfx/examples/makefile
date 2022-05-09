#
# Copyright 2011-2016 Branimir Karadzic. All rights reserved.
# License: http://www.opensource.org/licenses/BSD-2-Clause
#

rebuild:
	@make -s --no-print-directory rebuild -C xx-arealights

build:
	@make -s --no-print-directory TARGET=0 -C xx-arealights
	@make -s --no-print-directory TARGET=1 -C xx-arealights
	@make -s --no-print-directory TARGET=2 -C xx-arealights
	@make -s --no-print-directory TARGET=3 -C xx-arealights
	@make -s --no-print-directory TARGET=4 -C xx-arealights

build-osx:
	@make -s --no-print-directory TARGET=3 -C xx-arealights
	@make -s --no-print-directory TARGET=4 -C xx-arealights
	@make -s --no-print-directory TARGET=5 -C xx-arealights

metal:
	@make -s --no-print-directory TARGET=5 -C xx-arealights
