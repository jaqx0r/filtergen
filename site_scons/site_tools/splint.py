"""SCons.Tool.splint

A Tool for executing splint against C source files.
"""

# TODO ideas from https://lists.nongnu.org/archive/html/gpsd-users/2012-04/txtw13gIUqcgn.txt

# better way than cpp -v to parse include header path?

import re
import subprocess

import SCons.Action
import SCons.Builder

# These regular expression match gcc's cpp output.
_START = re.compile(r'^#include \<\.\.\.\> search starts here:')
_END = re.compile(r'End of search list\.')


def _commandOutput(command):
  process = subprocess.Popen(command, shell=True, stdout=subprocess.PIPE, encoding='utf8')
  return process.communicate()[0].strip().split('\n')

def _cppIncludePaths():
  paths = []
  copy = False
  for line in _commandOutput('echo | cpp -v 2>&1'):
    if not copy:
      if _START.match(line):
        copy = True
      continue
    if _END.match(line):
      break
    paths.append(line.strip())
  return paths

def _findTargetCSources(env, target):
  def _find_sources(target, sources, found):
    for item in target:
      if SCons.Util.is_List(item):
        _find_sources(item, sources, found)
      else:
        if item.abspath in found:
          continue
        found[item.abspath] = True
        if item.path.endswith('.c'):
          if not item.exists():
            item = item.srcnode()
          sources.append(item)
        else:
          l = item.children(scan=1)
          _find_sources(l, sources, found)
  sources = []
  _find_sources(target, sources, {})
  return sources


def SplintAll(env, target):
  sources = _findTargetCSources(env, target)
  return env.Splint('splint-'+str(target), sources)
  

def generate(env):
  env['SPLINT'] = 'splint'
  env['SPLINTPATH'] = _cppIncludePaths()
  env['SPLINTOPTS'] = ['+posixlib', '-weak']
  env['_SPLINTINCFLAGS'] = '$( ${_concat(INCPREFIX, SPLINTPATH, INCSUFFIX, __env__)} $)'
  splint = SCons.Builder.Builder(
    action='$SPLINT $SPLINTOPTS $_CPPDEFFLAGS $_SPLINTINCFLAGS $_CPPINCFLAGS $SOURCES'
    )
  env.Append(BUILDERS={'Splint': splint})
  env.AddMethod(SplintAll, 'SplintAll')

def exists(env):
  return env.Detect('splint')
