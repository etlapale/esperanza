#! /bin/sh

# Default values
output=disk.img
size=10M
mnt=`mktemp -d -p .`

# Check usage
if [ $# -ge 1 ] ; then
    output=$1
elif [ $# -ge 2 ] ; then
    size=$2
fi

# Make sure the file does not exists
if [ -e $output ] ; then
    echo "error: image $output already exists"
    exit 1
fi

# Create a zeroed image
# We prefer truncate to avoid really allocating the zeroes
truncate -s $size $output
chmod 666 $output

# Search for a loop mount points
loop1=""
loop2=""
for x in /dev/loop? ; do
    grep -q $x /proc/mounts
    if [ $? -ne 0 ] ; then
	if [ -z $loop1 ] ; then
	    loop1=$x
	elif [ -z $loop2 ] ; then
	    loop2=$x
	fi
    fi
done

# Set a loopback for the full disk
losetup $loop1 $output

# Create a BIOS partition
echo -e 'n\np\n1\n\n\na\nw\n' | fdisk $output

# Set a loopback for the partition
losetup $loop2 $output -o 1048576

# Format the partition
mke2fs $loop2

# Mount the partition
mount $loop2 $mnt

# Install GRUB
grub-install \
    --target=i386-pc \
    --root-directory=$mnt \
    --no-floppy \
    --modules="normal part_msdos ext2 multiboot biosdisk help configfile" \
    $loop1
tree $mnt

# Cleanup
umount $mnt
rmdir $mnt
losetup -d $loop2
losetup -d $loop1
