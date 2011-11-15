#! /usr/bin/env python2
# -*- coding: utf-8 -*-
# Create a virtual disk
# Authors: Tuna & Neil for Esperanza


import os, sys, shutil
from optparse import OptionParser
from tempfile import mkstemp, mkdtemp


# Search if a command exists
def searchCommand (cmd) :
  for d in os.environ['PATH'].split(':') :
    if os.path.isfile (d + '/' + cmd) :
      return d
  return None


# Create an empty floppy
def createFloppy (output) :
  os.system ("dd if=/dev/zero of=" + output + " bs=18k count=80 2>/dev/null")


# Format a floppy
def formatFloppy (floppy, tool) :
  
  if tool == 'mtools' :
    checkMtoolsrc (floppy)
    os.system ('mformat a:')

  elif tool == 'genext2fs' :
    os.system ('mke2fs -q -F ' + floppy)

  else :
    sys.exit ('Unknown tool (' + tool + '), use one of: mtools, genext2fs')


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


# Make sure we have an mtoolsrc
def checkMtoolsrc (floppy) :
  if not 'MTOOLSRC' in os.environ :
    (s, os.environ['MTOOLSRC']) = mkstemp ('.rc', 'mtools_')
    os.close (s)

  f = open (os.environ['MTOOLSRC'], 'w')
  f.write ('drive a: file="'  + floppy + '" 1.44M filter\n')
  f.close ()


# Copy a file from real fs to the virtual disk
def copyFile (src, floppy, tool) :
  print 'Copying %s into %s with %s' % (src, floppy, tool)
  
  if tool == 'mtools' :
    checkMtoolsrc (floppy)
    os.system ('mmd -D s a:/esperanza')
    os.system ('mcopy -o "' + src + '" a:/esperanza')
  elif tool == 'genext2fs' :
    os.mkdir ('.gene2fs')
    os.mkdir ('.gene2fs/esperanza')
    os.system ('cp "' + src + '" .gene2fs/esperanza')
    os.system ('genext2fs -x ' + floppy + ' -d .gene2fs ' + floppy)
    shutil.rmtree ('.gene2fs')
  else :
    sys.exit ('Unknown tool, use one of: mtools, genext2fs')

# List the modules
def listModules (list) :
  if not os.path.isfile (list) :
    sys.exit('Modules list not found in: ' + list)
  f = open (list, 'r')
  
  modules = []
  for line in f :
    line = line.strip ()
    if len (line) > 0 :
      modules.append(line)
      
  f.close ()
  return modules

# Install GRUB on a floppy
def installGrubOnFloppy (floppy, tool, grubconf) :
  
  grubdir = lookupGrub ()

  # Prepare GRUB directories
  if tool == "mtools" :
    checkMtoolsrc (floppy)
    os.system ('mmd a:/boot a:/boot/grub')
    if os.path.isdir (grubdir + '/stage1') :
      os.system ('mcopy "' + grubdir + '/stage1/stage1" a:/boot/grub/')
      os.system ('mcopy "' + grubdir + '/stage2/stage2" a:/boot/grub/')
    else :
      os.system ('mcopy "' + grubdir + '/stage1" a:/boot/grub/')
      os.system ('mcopy "' + grubdir + '/stage2" a:/boot/grub/')
    os.system ('mcopy "' + grubconf + '" a:/boot/grub/grub.conf')
    os.system ('mcopy "' + grubconf + '" a:/boot/grub/menu.lst')

  elif tool == 'genext2fs' :
    os.system ('rm -fr .gene2fs')
    os.mkdir ('.gene2fs')
    os.mkdir ('.gene2fs/boot')
    os.mkdir ('.gene2fs/boot/grub')
    if os.path.isdir (grubdir + '/stage1') :
      os.system ('cp "' + grubdir + '/stage1/stage1" .gene2fs/boot/grub/')
      os.system ('cp "' + grubdir + '/stage2/stage2" .gene2fs/boot/grub/')
    else :
      os.system ('cp "' + grubdir + '/stage1" .gene2fs/boot/grub')
      os.system ('cp "' + grubdir + '/stage2" .gene2fs/boot/grub')
    shutil.copy (grubconf, '.gene2fs/boot/grub/grub.conf')
    shutil.copy (grubconf, '.gene2fs/boot/grub/menu.lst')
    os.system ('genext2fs -b 1440 -d .gene2fs ' + floppy)
    os.system ('rm -fr .gene2fs')


  # Install GRUB
  os.system ('''grub --no-floppy --batch >/dev/null 2>&1 <<EOT
device (fd0) ''' + floppy + '''
root (fd0)
setup (fd0)
quit
EOT''')

# Create the GRUB configuration file
def createGrubConf (grubconf) :
  f = open (grubconf, 'w')
  f.write ('# Auto-generated GRUB configuration for Esperanza\n' +
           'default 0\n' +
           'timeout 0\n\n' +
           'title   Esperanza\n' +
           'root    (fd0)\n' +
           'kernel  /esperanza/kicker\n' +
           'module  /esperanza/kernel\n')
  modules = listModules('servers/list')
  for mod in modules :
    f.write ('module  /esperanza/' + mod + '\n')
  f.close ()

# Create a floppy and install GRUB on it
def createGrubFloppy (options) :

  # Check for the grub.conf file
  if not os.path.isfile (options.grubconf) :
    options.grubconf = 'grub.conf'
    if not os.path.isfile (options.grubconf) :
      options.grubconf = '.grubconf'
      createGrubConf (options.grubconf)
  
  if options.verbose :
    print "Creating an empty disk on " + options.image
  createFloppy (options.image)
  
  if options.verbose :
    print 'Formatting the disk'
  formatFloppy (options.image, options.type)

  if options.verbose :
    print 'Installing GRUB'
  installGrubOnFloppy (options.image, options.type, options.grubconf)


# Ask the user to choose an option on the console
def askUser (question, choices, default) :
  print question
  while True :
    print 'Please choose one of:',
    for c in choices :
      print c,
    print '[' + default + ']', 
    ans = raw_input ().strip ()
    if len (ans) == 0 :
      return default
    for c in choices :
      if c == ans :
        return ans
    print 'Unacceptable answer'
    

# Configure the test
def configureTest (options) :

  f = open (options.config, 'w')
  
  # TODO: search for emulators before displaying options
  emul = askUser ('Which emulator do you plan to use?',
                  ['bochs', 'qemu-system-x86_64', 'qemu', 'none'],
                  'bochs')
  
  if emul == 'bochs' :
    if searchCommand (emul) == None :
      sys.exit ('Could not found the bochs emulator')
    b = open ('.esperanza_bochsrc', 'w')
    b.write ('''config_interface: textconfig
display_library: x
megs: 64
floppya: 1_44="''' + options.image + '''", status=inserted
boot: floppy
cpu: ips=10000000
log: -
''')
    b.close ()
    f.write ('bochs -qf .esperanza_bochsrc\n')
  
  elif emul == 'qemu' :
    if searchCommand (emul) == None :
      sys.exit ('Could not found the qemu emulator')
    f.write ('qemu -fda ' + options.image + '\n')

  elif emul == 'qemu-system-x86_64' :
    if searchCommand (emul) == None :
      sys.exit ('Could not found the qemu-system-x86_64 emulator')
    f.write ('qemu-system-x86_64 -fda ' + options.image + '\n')

  f.close ()


# Run the test
def runTest (options) :
  copyFile ('kicker/kicker', options.image, options.type)
  copyFile ('kernel/kernel', options.image, options.type)
  modules = listModules('servers/list')
  for mod in modules :
    copyFile ('servers/' + mod + '/' + mod, options.image, options.type)
  os.system ("bash " + options.config)


# Entry point
def main () :
  
  parser = OptionParser ("%prog [options] action")
  parser.add_option ('-c', '--config', dest='config', default='.testconf',
                     help='Define the test configuration file')
  parser.add_option ('-g', '--grub-conf', dest='grubconf',
                     default='./scripts/grub.conf',
                     help='Indicate a GRUB configuration file for the disk')
  parser.add_option ('-i', '--image', dest='image', default='virtual.img',
		     help='Specify the disk image')
  parser.add_option ('-q', '--quiet', action='store_false', dest='verbose',
                     help='Print quiet output')
  parser.add_option ('-t', '--tool', dest='type', default='mtools',
		     help='Indicate the tool to be used (genext2fs, mtools)')
  parser.add_option ('-v', '--verbose', action='store_true', dest='verbose',
                     default=True, help='Print verbose output')
  (options, args) = parser.parse_args ()
  
  if 'MTOOLSRC' in os.environ :
    del os.environ['MTOOLSRC']
  os.environ['PATH'] = os.environ['PATH'] + ':/sbin:/usr/sbin:/usr/local/sbin'
  
  # Search for the tools (either genext2fs or mtools)
  if options.type == 'genext2fs' and searchCommand ('genext2fs') == None :
    if options.verbose :
      print 'genext2fs not found, trying mtools'
    options.type = 'mtools'
    if searchCommand ('mformat') == None :
      sys.exit ('Neither genext2fs nor mtools where found')
  if options.type == 'mtools' and searchCommand ('mformat') == None :
    if options.verbose :
      print 'mtools not found, trying genext2fs'
    options.type = 'genext2fs'
    if searchCommand ('genext2fs') == None :
      sys.exit ('Neither genext2fs nor mtools where found')

  # Check the action
  if len (args) == 0 or args[0] == 'run':
    if not os.path.isfile (options.image) :
      createGrubFloppy (options)
    if not os.path.isfile (options.config) :
      configureTest (options)
    runTest (options)
  elif args[0] == 'floppy' :
    createGrubFloppy (options)
  elif args[0] == 'configure' :
    configureTest (options)
  
  if 'MTOOLSRC' in os.environ :
    os.unlink (os.environ['MTOOLSRC'])

if __name__ == "__main__" :
  main ()
