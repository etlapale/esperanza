#! /usr/bin/env python
# -*- coding: utf-8 -*-
# Create virtual disks (Generic version for http://osdev.next-touch.com/)
# Authors: Tuna & Neil Dökkalfar for Esperanza


import os, sys, shutil
from optparse import OptionParser
from tempfile import mkstemp, mkdtemp


# Search if a command exists returning its directory or None
def searchCommand (cmd) :
  for d in os.environ['PATH'].split(':') :
    if os.path.isfile (d + '/' + cmd) :
      return d
  return None

# Create an ext2fs formated image
def createExt2File (output, size) :
  os.system ('genext2fs -q -b ' + str (size / 1024) + ' ' + output)

# Create the GRUB configuration file
def createGrubConf (dev, kernel, modules, label) :
  f = open ('.grubconf', 'w')
  f.write ('''# Auto-generated GRUB configuration for Esperanza
default 0
timeout 0

title   %s
root    %s
kernel  /%s
''' % (label, dev, kernel))
  for mod in modules :
    f.write ('module  /' + mod + '\n')
  f.close ()

# Search for GRUB
def lookupGrub () :
  
  # Search according to the GRUBDIR variable
  if ('GRUBDIR' in os.environ and os.path.isdir (os.environ['GRUBDIR'])) :
    return os.environ['GRUBDIR']
  
  # Search in common dirs
  grubdirs = ['/boot/grub',
              '/lib64/grub/x86_64-pc',
              '/usr/local/share/grub/i386-freebsd',
              '/usr/local/share/grub/i386-pc',
              '/usr/share/grub/i386-pc',
              '/usr/lib/grub/i386-pc',
              '/usr/local/grub',
              '/usr/share/grub/i386-redhat',
              '$HOME/share/grub/i386-pc/']
  for d in grubdirs :
    if os.path.isdir (d) :
      return d
  
  # Search with locate
  if searchCommand ('locate') :
    p = os.popen ('locate stage2', 'r')
    for d in p :
      if d.find ('grub') :
        return os.path.dirname (d)
  
  # Maybe try to download it?
  
  sys.exit ("GRUB was not found on you machine, try to set GRUBDIR")

# Prepend a MBR to the disk image
def prependMBR (image, size) :
  cylinders = size / (512 * 16 * 63)
  os.system ('dd if=/dev/zero of=.mbr bs=512 count=63')
  os.system ('cat %s %s > %s.new' % ('.mbr', image, image))
  shutil.move (image + '.new', image)
  os.unlink ('.mbr')
  os.system ('''fdisk -C %d -H 16 -S 63 %s >/dev/null
n
p
1


w
q
''' % (cylinders, image))

# Install GRUB on a disk
def installGrub (image, dev, size) :
  
  grubdir = lookupGrub ()
  
  # Prepare GRUB directories
  grubconf = '.grubconf'
  gendir = '.gene2fs'
  if os.path.isdir (gendir) :
    shutil.rmtree (gendir)
  os.mkdir (gendir)
  os.mkdir (gendir + '/boot')
  os.mkdir (gendir + '/boot/grub')
  if os.path.isdir (grubdir + '/stage1') :
    shutil.copy (grubdir + '/stage1/stage1', gendir + '/boot/grub/')
    shutil.copy (grubdir + '/stage1/stage2', gendir + '/boot/grub/')
  else :
    shutil.copy (grubdir + '/stage1', gendir + '/boot/grub/')
    shutil.copy (grubdir + '/stage2', gendir + '/boot/grub/')
  shutil.copy (grubconf, gendir + '/boot/grub/grub.conf')
  shutil.copy (grubconf, gendir + '/boot/grub/menu.lst')
  os.system ('genext2fs -x ' + image + ' -d ' + gendir + ' ' + image)
  
  # Install GRUB
  if dev == 'hd' :
    prependMBR (image, size)
    grubdev = '(hd0)'
    grubroot = '(hd0,0)'
  else :
    grubdev = '(fd0)'
    grubroot = '(fd0)'

    print'''grub --no-floppy --batch >/dev/null<<EOT
device %s %s
root %s
setup %s
quit
EOT
''' % (grubdev, image, grubroot, grubdev)
    
  os.system ('''grub --no-floppy --batch <<EOT
device %s %s
root %s
setup %s
quit
EOT
''' % (grubdev, image, grubroot, grubdev))

# Copy a file from real fs to the virtual disk
def copyFile (src, floppy, tool) :
  
#  if tool == 'mtools' :
#    checkMtoolsrc (floppy)
#    os.system ('mcopy -o "' + src + '" a:/esperanza')
  if tool == 'genext2fs' :
    if os.path.isdir ('.gene2fs') :
      shutil.rmtree ('.gene2fs')
    os.mkdir ('.gene2fs')
    os.mkdir ('.gene2fs/esperanza')
    os.system ('cp "' + src + '" .gene2fs/esperanza')
    os.system ('genext2fs -x ' + floppy + ' -d .gene2fs ' + floppy)
    shutil.rmtree ('.gene2fs')

# Entry point
def main () :
  
  parser = OptionParser ("%prog [options] action")
  parser.add_option ('-i', '--image', dest='image', default='virtual.img',
		     help='Specify the disk image')
  parser.add_option ('-k', '--grub-kernel', dest='grub_kernel', default=None,
                     help='Kernel to be booted by GRUB')
  parser.add_option ('-l', '--grub-label', dest='grub_label', default='OSDev',
                     help='GRUB entry label')
  parser.add_option ('-m', '--grub-modules', dest='grub_mods', default=None,
                     help='Coma-separated modules for GRUB')
  parser.add_option ('-q', '--quiet', action='store_false', dest='verbose',
                     help='Print quiet output')
  parser.add_option ('-s', '--size', dest='size', default='1474560',
                     help='Disk size')
  parser.add_option ('-t', '--disk-type', dest='disk_type', default='floppy',
                     help='Define the disk type (floppy, hd)')
  parser.add_option ('-v', '--verbose', action='store_true', dest='verbose',
                     default=True, help='Print verbose output')
  (options, args) = parser.parse_args ()
  
  os.environ['PATH'] = os.environ['PATH'] + ':/sbin:/usr/sbin:/usr/local/sbin'
  
  # Search for the tools (genext2fs)
  if searchCommand ('genext2fs') == None :
    sys.exit ('genext2fs not found, aborting.')
    
  if not options.grub_kernel :
    sys.exit ('You must specify a GRUB kernel')

  modules = []
  if options.grub_mods :
    modules = options.grub_mods.split (',')

  sz = int (options.size)
  rest = sz % (512 * 16 * 63)
  if rest :
    sz = sz + (512 * 16 * 63) - rest;
  
  if len (args) == 1 :
    if args[0] == 'create' :
      if options.disk_type == 'hd' :
        createExt2File (options.image, sz - 512 * 63 + 512)
        createGrubConf ('(hd0,0)', options.grub_kernel, modules,
                        options.grub_label)
      else :
        createExt2File (options.image, sz)
        createGrubConf ('(fd0)', options.grub_kernel, modules,
                        options.grub_label)
      installGrub (options.image, options.disk_type, sz)

if __name__ == "__main__" :
  main ()
