"""SCons.Tool.splint

A Tool for executing splint against C source files.
"""

# TODO ideas from https://lists.nongnu.org/archive/html/gpsd-users/2012-04/txtw13gIUqcgn.txt

# better way than cpp -v to parse include header path?

import re

import subprocess

import SCons.Action
import SCons.Builder

def _commandOutput(command):
  process = subprocess.Popen(command, shell=True, stdout=subprocess.PIPE)
  return process.communicate()[0].strip().split('\n')

_START = re.compile(r'^#include \<\.\.\.\> search starts here:')
_END = re.compile(r'End of search list\.')

def CppIncludePaths():
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

def generate(env):
  env['SPLINT'] = 'splint'
  env['SPLINTPATH'] = CppIncludePaths()
  env['SPLINTOPTS'] = ['+posixlib']
  env['_SPLINTINCPATHS'] = '$( ${_concat(CPPINCPREFIX, SPLINTPATH, CPPINCSUFFIX, __env__, RDirs, TARGET, SOURCE)} $)'
  splint = SCons.Builder.Builder(
    action='$SPLINT $_SPLINTINCPATHS $_CPPDEFFLAGS $SPLINTOPTS $SOURCE',
    )
  env.Append(BUILDERS={'Splint': splint})

def exists(env
):
  return env.Detect('splint')
