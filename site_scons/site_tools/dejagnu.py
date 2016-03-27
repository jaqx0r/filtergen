"""SCons.Tool.dejagnu.

A Tool for executing runtest against dejagnu testsuites.

The 'target' argument to the DejaGnu builder is the name of a DejaGnu tool.
From that tool we follow the same rules for determining the suite to run:
i.e. passing 'foo' as in the example below will execute all the testsuite
directories that match the glob 'foo.*' in the testsuite directory.

The targets 'foo.log' and 'foo.sum' are placed in the testsuite directory, so
as to not clutter the top level with generated files; this differs from the
behaviour of running 'runtest' manually from the top level.

    env = Environment(tools = ['dejagnu'])

    foo_tests = env.DejaGnu('foo')
    env.Depends(foo_tests, '#foo.exe')
    env.Alias('check', foo_tests)

"""

import SCons.Action
import SCons.Builder
import glob
import os
import os.path
import re


def dejagnuEmitter(target, source, env):
    """Rewrite source and target lists for setting up DejaGnu dependencies."""
    new_t = []
    for t in target:
        # The target argument passed in to the Builder is the name of the tool.
        # DejaGnu will create the sum and log files based on the tool name.
        new_t.extend([env.File(str(t) + '.sum'),
                      env.File(str(t) + '.log')])
    new_s = []
    # The source argument passed in to the Builder is the same as the target.
    # Look in all "${source}.*" paths for *.exp files, they comprise all the
    # tests to execute and thus are dependencies of the target.
    for s in source:
      # The first path must be the tool name, so transfer this without modification.
      new_s.append(s)
      if s.isdir():
        # If this source is a path, we can glob in it
        new_s.extend(s.glob('*.exp'))
      else:
        # Otherwise, check for more test suites that start with the tool name and glob
        # in them too
        for p in env.Glob(str(s) + '.*'):
          #print str(p)
          if p.isdir():
            new_s.extend(p.glob('*.exp'))
    # print 'returning', [x.abspath for x in new_t], [x.abspath for x in new_s]
    # print new_t, new_s
    return new_t, new_s


def dejagnuMessage(target, source, env):
    return 'Running DejaGNU test suite'


def dejagnuActionGenerator(target, source, env, for_signature):
    # for t in target:
    #     print 'action target', t.abspath
    # for s in source:
    #     print 'action source', s.abspath
    # The Emitter above emits the directory as the first source argument; we
    # use the directory name to deter
    tool = os.path.splitext(os.path.basename(str(source[0])))[0]
    # 
    outdir = os.path.dirname(str(source[0]))
    #print 'tool', tool
    #return 'runtest --all --debug -v -v --outdir %s --tool %s' % (outdir, tool)
    return 'runtest --outdir %s --tool %s' % (outdir, tool)


def generate(env):
    dgBuilder = SCons.Builder.Builder(
        chdir=env.Dir('#'),
        generator=dejagnuActionGenerator,
        emitter=dejagnuEmitter,
        source_factory=SCons.Node.FS.Dir)
    env.Append(BUILDERS={'DejaGnu': dgBuilder})


def exists(env):
    return env.Detect('runtest')
