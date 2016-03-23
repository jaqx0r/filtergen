"""SCons.Tool.dejagnu.

A Tool for executing runtest against dejagnu testsuites.

foo = env.DejaGnu('test')
env.Alias('check', foo)
"""

import re
import os
import os.path
import glob
import SCons.Action
import SCons.Builder


def dejagnuEmitter(target, source, env):
    """Rewrite source and targets."""
    new_t = []
    for t in target:
        new_t.extend([env.File(t.dirname[:-1] + '.sum'),
                      env.File(t.dirname[:-1] + '.log')])
    new_s = []
    for s in source:
        new_s.extend(s.glob('*.exp'))
    return new_t, new_s


def dejagnuMessage(target, source, env):
    return 'Running DejaGNU Test Suite'


def generate(env):
    dgAction = 'cd # && runtest --tool ${SOURCE.dir}'
    dgBuilder = SCons.Builder.Builder(
        action=dgAction, emitter=dejagnuEmitter, source_factory=env.fs.Dir, target_factory=env.fs.Entry)
    env.Append(BUILDERS={'DejaGnu': dgBuilder})


def exists(env):
    return env.Detect('runtest')
