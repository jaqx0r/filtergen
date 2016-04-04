"""SCons.Tool.lcov

A Tool for executing lcov against coverage output.
"""

import SCons.Builder
import SCons.Script

def lcov_generator(source, target, env, for_signature):
  env['LCOV'] = 'lcov'
  env['LCOVFLAGS'] = '--capture'
  cmd = ['$LCOV $LCOVFLAGS']
  cmd += ['--output-file', str(target[0])]
  if 'LCOVDIR' in env:
    cmd += ['--directory', str(SCons.Script.Dir(env['LCOVDIR']))]
  if 'LCOVBASEDIR' in env:
    cmd += ['--base-directory', str(SCons.Script.Dir(env['LCOVBASEDIR']))]
  return ' '.join(SCons.Script.Flatten(cmd))
  
def generate(env):
  lcov_builder = SCons.Builder.Builder(
    generator=lcov_generator)
  env.Append(BUILDERS={'LCov': lcov_builder})
  
def exists(env):
  return env.Detect('lcov')
