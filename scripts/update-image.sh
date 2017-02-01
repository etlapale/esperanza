#! /bin/sh

# Default values
output=disk.img
srvlst=servers/list
mnt=`mktemp -d -p .`

# Check usage
if [ $# -ge 1 ] ; then
    output=$1
elif [ $# -ge 2 ] ; then
    srvlst=$2
fi

# Make sure the file does exists
if [ ! -e $output ] ; then
    echo "error: image $output does not exist"
    exit 1
elif [ ! -e $srvlst ] ; then
    echo "error: server list $srvlst does not exist"
    exit 1
fi

# Search for a loop mount points
loop1=""
for x in /dev/loop? ; do
    grep -q $x /proc/mounts
    if [ $? -ne 0 ] ; then
	loop1=$x
	break
    fi
done

# Set a loopback for the partition
losetup $loop1 $output -o 1048576

# Mount the partition
mount $loop1 $mnt

# Update GRUB config
mkdir -p $mnt/boot/grub
grubcfg=$mnt/boot/grub/grub.cfg
cat > $grubcfg <<EOF
set timeout_style=menu
set timeout=3
set default=0 # Set the default menu entry
 
menuentry "Esperanza" {
   multiboot /esperanza/kicker
   module /esperanza/kernel /esperanza/kernel
   module /esperanza/console /esperanza/console
   module /esperanza/storage /esperanza/storage
   boot
}
EOF
#for server in `cat $srvlst` ; do
#    echo "  module (hd0,msdos1)/esperanza/$server /esperanza/$server" >> $grubcfg
#done
#echo -e "  boot\n}" >> $grubcfg
cat $grubcfg

# Update the OS
mkdir -p $mnt/esperanza
cp -a kernel/kernel kicker/kicker $mnt/esperanza
for server in `cat $srvlst` ; do
    cp -a servers/$server/$server $mnt/esperanza
done
tree $mnt

# Cleanup
umount $mnt
rmdir $mnt
losetup -d $loop1
