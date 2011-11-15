#! /bin/sh
# Dump symlinks and exec permissions


# Shell script headline
echo '#! /bin/sh'
echo '# Restore the symlinks and exec perms'
echo ""

# Dump the symlinks
for name in `find . -type l` ; do
    echo "ln -s " `readlink $name` $name
done


# Dump the executables
for name in `find scripts -type f -perm '+u+x'`; do
    echo "chmod +x " $name
done
