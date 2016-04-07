"""SCons.Tool.lcov

A Tool for executing lcov against coverage output.
"""

import SCons.Builder
import SCons.Script

def lcov_generator(source, target, env, for_signature):
  cmd = ['$LCOV $LCOVFLAGS']
  cmd += ['--output-file', str(target[0])]
  if 'LCOVDIR' in env:
    cmd += ['--directory', str(SCons.Script.Dir(env['LCOVDIR']))]
  if 'LCOVBASEDIR' in env:
    cmd += ['--base-directory', str(SCons.Script.Dir(env['LCOVBASEDIR']))]
  if env['CC'].startswith('gcc-'):
    version = env['CC'].split('-')[1]
    cmd += ['--gcov-tool', 'gcov-' + version]
  return ' '.join(SCons.Script.Flatten(cmd))
  
def generate(env):
  env['LCOV'] = 'lcov'
  env['LCOVFLAGS'] = ['--capture', '--no-external']
  lcov_builder = SCons.Builder.Builder(
    generator=lcov_generator)
  env.Append(BUILDERS={'LCov': lcov_builder})
  
def exists(env):
  return env.Detect('lcov')
