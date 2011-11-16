# Main Makefile for Esperanza

GRUBDIR=/boot/grub
VIMG=virtual.img
MTOOLSRC=.mtoolsrc
GRUBCONF=.menu.lst
BOCHSRC=.bochsrc

MMD=MTOOLSRC=$(MTOOLSRC) mmd
MCOPY=MTOOLSRC=$(MTOOLSRC) mcopy
MFORMAT=MTOOLSRC=$(MTOOLSRC) mformat

.PHONY: all clean config distclean kernel kicker \
	servers symlinks test menuconfig install_img


all: symlinks kicker kernel servers

kernel kicker: config.h
	@$(MAKE) -C $@

servers: config.h
	@for name in `cat servers/list`; do $(MAKE) -C servers/$$name; done

config.h:
	if [ ! -e "$@" ] ; then $(MAKE) menuconfig; fi

clean:
	@$(RM) -f *~ */*~ bochsout.txt $(MTOOLSRC) $(BOCHSRC) $(GRUBCONF)
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

test: all install_img $(BOCHSRC)
	bochs -qf $(BOCHSRC)
       
$(BOCHSRC):
	echo 'display_library: x' > $@
	echo 'megs: 64' >> $@
	echo 'floppya: 1_44="$(VIMG)", status=inserted' >> $@
	echo 'boot: floppy' >> $@

install_img: $(VIMG) kernel kicker servers
	-$(MMD) -D s a:/esperanza
	$(MCOPY) -o kicker/kicker a:/esperanza
	$(MCOPY) -o kernel/kernel a:/esperanza
	for name in `cat servers/list`; do \
		$(MCOPY) -o servers/$$name/$$name a:/esperanza; \
	done

$(VIMG): $(MTOOLSRC) $(GRUBCONF)
	dd if=/dev/zero of=$@ bs=18k count=80
	$(MFORMAT) a:
	# Prepare GRUB
	$(MMD) a:/boot a:/boot/grub
	$(MCOPY) $(GRUBDIR)/stage1 a:/boot/grub/
	$(MCOPY) $(GRUBDIR)/stage2 a:/boot/grub/
	$(MCOPY) $(GRUBCONF) a:/boot/grub/menu.lst
	# Install GRUB
	echo -e 'device (fd0) $(VIMG)\nroot (fd0)\nsetup (fd0)\nquit' | grub --no-floppy --batch

$(GRUBCONF):
	echo '# Grub configuration for Esperanza' > $@
	echo 'default 0' >> $@
	echo 'timeout 0' >> $@
	echo 'title Esperanza' >> $@
	echo 'root (fd0)' >> $@
	echo 'kernel /esperanza/kicker' >> $@
	echo 'module /esperanza/kernel' >> $@
	echo 'module /esperanza/console' >> $@

$(MTOOLSRC):
	echo 'drive a: file="$(VIMG)" 1.44M filter' > $@

distclean: clean
	$(RM) -fr config.h .esperanza* .testconf .grubconf *.img \
		 `find . -name semantic.cache`
