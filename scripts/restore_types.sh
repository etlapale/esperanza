#! /bin/sh
# Restore the symlinks and exec perms

ln -s  ../../kicker/include/kicker.h ./kernel/include/kicker.h
ln -s  ../../../kernel/include/l4/kip.h ./servers/include/l4/kip.h
ln -s  ../../../kernel/include/l4/types.h ./servers/include/l4/types.h
ln -s  ../../kernel/include/stdint.h ./servers/include/stdint.h
ln -s  ../../kernel/include/stddef.h ./servers/include/stddef.h
ln -s  ../../kernel/include/io.h ./servers/include/io.h
ln -s  ../../kernel/include/stdarg.h ./servers/include/stdarg.h
ln -s  ../common/Makefile ./servers/console/Makefile
ln -s  ../../../kernel/src/btalloc.c ./servers/sigma0/src/btalloc.c
ln -s  ../../../kernel/src/printf.c ./servers/sigma0/src/printf.c
ln -s  ../common/Makefile ./servers/sigma0/Makefile
ln -s  ../../../kernel/include/btalloc.h ./servers/sigma0/include/btalloc.h
chmod +x  scripts/configure.py
chmod +x  scripts/test.py
chmod +x  scripts/dump_types.sh
