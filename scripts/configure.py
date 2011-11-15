#! /usr/bin/env python
# -*- coding: utf-8 -*-
# This is the Esperanza configurator
# Usage: ./configure.py [configurable [output [prefix]]]


import os, sys, xml.dom, xml.dom.minidom, curses, traceback, string
from sets import Set


def getText (nodelst) :
  ans = ''
  for n in nodelst :
    if n.nodeType == n.TEXT_NODE:
      ans = ans + n.data
  return ans

def getChildText (node, name) :
  for child in node.childNodes :
    if child.nodeName == name :
      return getText (child.childNodes)
  return None

def getTitle (node) :
  return getChildText (node, 'title')

def paintBackground (win) :
  for l in range (0, win.getmaxyx()[0]) :
    win.hline (l, 0, ' ', win.getmaxyx()[1])

def centered (win, line, str) :
  l = len (str)
  width = win.getmaxyx()[1]
  win.addstr (line, (width - l) / 2, str)

def actions (win, selected) :
  line = win.getmaxyx()[0] - 2
  strs = ["<Select>", "<Exit>", "<Help>"]
  offset = (win.getmaxyx()[1] - 24) / 2
  
  for i in range (len (strs)) :
    if selected == i :
      win.addstr (line, offset, strs[i], curses.A_REVERSE)
    else :
      win.addstr (line, offset, strs[i])
    offset = offset + len (strs[i]) + 2

def configurable (win, conf) :
  paintBackground (win)
  win.box ()
  centered (win, 0, ' ' + getTitle (conf) + ' ')

  # Description
  num = 2
  desc = getChildText (conf, 'description')
  if not desc :
    desc = "Use up/down to select an option, left/right to select an action\nEnter to validate your choice."
  lines = desc.splitlines ()
  for line in lines :
    line = line.strip ()
    # TODO: split long lines
    win.addnstr (num, 2, line, win.getmaxyx()[1] - 4)
    num = num + 1

  actions (win, 0)
  return num

def getChild (node, num) :
  global config
  
  n = 0
  for child in node.childNodes :
    if child.nodeType == child.ELEMENT_NODE :
      if child.nodeName == "configurable" or child.nodeName == "choose" or child.nodeName == "option" :
        # Skip hidden dependencies
        req = child.getAttribute ('require')
        if not req or req in config :
          if n == num :
            return child
          n = n + 1
  return None

def options (win, conf, selected) :
  win.box ()
  line = 2
  atts = curses.color_pair (2) | curses.A_BOLD
  
  for child in conf.childNodes :
    if child.nodeType == child.ELEMENT_NODE :
      
      if line - 2 == selected :
        atts = atts | curses.A_REVERSE
      else :
        atts = atts & (~ curses.A_REVERSE)
        
      if child.nodeName == "configurable" :
        win.addnstr (line, 2, '--> ' + getTitle (child),
                     win.getmaxyx()[1] - 4, atts)
      elif child.nodeName == "choose" :
        win.addnstr (line, 2, '(-) ' + getTitle (child),
                     win.getmaxyx()[1] - 4, atts)
        
      elif child.nodeName == "option" :
        
        # Check if the attribute require another one
        req_attr = child.getAttribute ('require')
        if req_attr and not (req_attr in config) :
          continue
        
        if child.getAttribute  ('id') in config :
          if (conf.nodeName == 'choose') :
            txt = '[x] '
          else :
            txt = '[*] '
        else :
          txt = '[ ] '
        win.addnstr (line, 2, txt + getText(child.childNodes).strip(),
                     win.getmaxyx()[1] - 4, atts)
      else :
        continue
      line = line + 1
      
  return line - 2

def initOptions (doc, opts) :
  doc_opts = doc.getElementsByTagName ('option')
  for o in doc_opts :
    if (o.getAttribute ('default') == 'yes') :
      opts.add (o.getAttribute ('id'))

def loadConfig (config, filename, prefix) :
  f = open (filename, 'r')
  for line in f :
    if line.startswith ('#define ' + prefix) :
      c = (line[len ('#define ' + prefix) :]).strip()
      config.add (c)
  f.close ()

def main (win) :
  
  curses.noecho ()
  curses.cbreak ()
  win.keypad (1)
  
  # Check the arguments
  if len (sys.argv) < 2 :
     filename = 'configurable.xml'
  else :
     filename = sys.argv[1]
  if len (sys.argv) < 3 :
    output = "config.h"
  else :
    output = sys.argv[2]
  if len (sys.argv) < 4 :
    prefix = 'CONFIG_'
  else :
    prefix = sys.argv[3]
       
  # Load the configurable
  doc = xml.dom.minidom.parse (filename)

  # Main window colors
  if curses.has_colors () :
    curses.start_color ()
    curses.init_pair (1, curses.COLOR_MAGENTA, curses.COLOR_BLACK)
    win.attron (curses.color_pair (1) | curses.A_BOLD)
    paintBackground (win)

  # Title
  win.addstr (0, 1, getTitle (doc.documentElement))
  win.hline (1, 1, curses.ACS_HLINE, win.getmaxyx()[1] - 2)

  # Current category
  sub = win.subwin (win.getmaxyx()[0] - 3, win.getmaxyx()[1] - 4, 2, 2)
  if curses.has_colors () :
    curses.init_pair (2, curses.COLOR_WHITE, curses.COLOR_BLUE)
    sub.attron (curses.color_pair (2) | curses.A_BOLD)
    sub.attrset (curses.color_pair (2) | curses.A_BOLD)
    
  win.refresh ()

  global current, act, opt, num, numopts, opts, config
  opts = None
  config = Set ()

  # Load the existing config
  if os.path.isfile (output) :
    loadConfig (config, output, prefix)
  # Load the default config from the configurable
  else :
    initOptions (doc, config)
  
  def setConfigurable (conf) :
    global current, act, opt, num, numopts, opts
    current = conf
    num = configurable (sub, current)
    if opts != None :
      del opts
    opts = sub.subwin (sub.getmaxyx()[0] - num - 3, 
                       sub.getmaxyx()[1] - 4,
                       num + 2, 4)
    numopts = options (opts, current, 0)
    act = 0
    opt = 0
    sub.refresh ()
    opts.refresh ()

  setConfigurable (doc.documentElement)

  # Event loop
  while True :
    c = win.getch ()
    
    # Choose an action
    if c == curses.KEY_LEFT :
      act = (act - 1) % 3
      actions (sub, act)
      sub.refresh ()
    elif c == curses.KEY_RIGHT :
      act = (act + 1) % 3
      actions (sub, act)
      sub.refresh ()
    
    # Choose an option
    elif c == curses.KEY_UP :
      opt = (opt - 1) % numopts
      options (opts, current, opt)
      opts.refresh ()
    elif c == curses.KEY_DOWN :
      opt = (opt + 1) % numopts
      options (opts, current, opt)
      opts.refresh ()

    # Select
    elif act == 0 and c == ord ('\n') :
      child = getChild (current, opt)
      if child.nodeName == "configurable" :
        setConfigurable (child)
      elif child.nodeName == "choose" :
        setConfigurable (child)
      elif child.nodeName == "option" :
        id_attr = child.getAttribute ('id')
        if current.nodeName == 'choose' :
          for n in current.childNodes :
            if n.nodeName == 'option' :
              config.discard (n.getAttribute ('id'))
          config.add (id_attr)
        else :
          if id_attr in config :
            config.discard (id_attr)
          else :
            config.add (id_attr)
        options (opts, current, opt)
        opts.refresh ()
    
    # Exit
    elif c == 27 or (act == 1 and c == ord('\n')) :
      if current == doc.documentElement :
        break;
      else :
        setConfigurable (current.parentNode)

  # Remove hidden options
  for node in doc.getElementsByTagName ('option') :
    id_attr = node.getAttribute ('id')
    req_attr = node.getAttribute ('require')
    if req_attr and (id_attr in config) and not (req_attr in config) :
      config.discard (id_attr)

  print 'Hello World!'

  # Save the configuration
  f = open (output, 'w')
  f.write ('/* File generated by the Esperanza configurator */\n')
  f.write ('#ifndef __CONFIG_H\n#define __CONFIG_H\n\n')
  for c in config :
    f.write ('#define ' + prefix + c + '\n')
  f.write ('\n#endif\n')
  f.close ()

# Entry point
curses.wrapper (main)
