# Main Makefile for Esperanza

PYTHON = python2

.PHONY: all clean config distclean kernel kicker \
	servers symlinks test menuconfig


all: symlinks kicker kernel servers

kernel kicker: config.h
	@$(MAKE) -C $@

servers: config.h
	@for name in `cat servers/list`; do $(MAKE) -C servers/$$name; done

symlinks:
	sh scripts/restore_types.sh 2>/dev/null

dumplinks:
	sh scripts/dump_types.sh > scripts/restore_types.sh

config.h:
	if [ ! -e "$@" ] ; then $(MAKE) menuconfig; fi

clean:
	@$(RM) *~ */*~ bochsout.txt
	@$(MAKE) -C kernel clean
	@$(MAKE) -C kicker clean
	@for name in `cat servers/list`; do \
		if [ -e "servers/$$name/Makefile" ] ; then \
			$(MAKE) -C servers/$$name clean;\
		fi ;\
		done
	@echo "Everything clean"

config menuconfig:
	$(PYTHON) ./scripts/configure.py ./scripts/configurable.xml

test: all
	$(PYTHON) ./scripts/test.py

distclean: clean
	$(RM) -fr config.h .esperanza* .testconf .grubconf *.img \
		 `find . -type l` `find . -name semantic.cache`
