import re
import os
import os.path
import glob


def dejagnuAction(target, source, env):
    if env['with_runtest'] == None:
        print "Testing disabled because 'runtest' not found"
        return 0

    context = os.path.basename(env.File(target[0]).path)
    context = re.sub('(.*?)\..*', '\\1', context)
    os.system('cd ' + os.path.join(env['BuildDir'], 'test') +
              '; DEJAGNU="' +
              os.path.join(env['AbsObjRoot'], 'test', 'site.exp') + '" ' +
              env['with_runtest'] + ' --tool ' + context)
    return 0


def dejagnuMessage(target, source, env):
    return 'Running DejaGNU Test Suite'

def generate(env):
  dgAction = env.Action(dejagnuAction, dejagnuMessage)
  dgBuilder = env.Builder(action=dgAction, suffix='results')
  env.Append(BUILDERS={'DejaGnu': dgBuilder})


def exists(env):
  env.Detect("runtest")
