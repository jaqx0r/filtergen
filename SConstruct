# -*- python -*-
import glob
import os
import SCons.Node.FS

EnsureSConsVersion(0, 95)

VERSION = '0.13'

vars = Variables(None, ARGUMENTS)
vars.AddVariables(
    BoolVariable('debug', 'debugging compiler options', 1),
    BoolVariable('profiler', 'enable support for profiler', 0),
    BoolVariable('gcov', 'enable test coverage with gcov', 0),
)

env = Environment(variables=vars)

# all below thanks to Paul Davis and his ardour build system


def distcopy(target, source, env):
    treedir = str(target[0])

    try:
        os.mkdir(treedir)
    except OSError, (errnum, strerror):
        if errnum != errno.EEXIST:
            print 'mkdir %s:%s' % (treedir, strerror)

    cmd = 'tar cf - '
    #
    # we don't know what characters might be in the file names
    # so quote them all before passing them to the shell
    #
    all_files = ([str(s) for s in source])
    cmd += ' '.join(["'%s'" % quoted for quoted in all_files])
    cmd += ' | (cd ' + treedir + ' && tar xf -)'
    p = os.popen(cmd)
    return p.close()


def tarballer(target, source, env):
    cmd = 'tar -zcf ' + str(target[0]) + ' ' + \
        str(source[0]) + "  --exclude '*~'"
    print 'running ', cmd, ' ... '
    p = os.popen(cmd)
    return p.close()

dist_bld = Builder(action=distcopy,
                   target_factory=SCons.Node.FS.default_fs.Entry,
                   source_factory=SCons.Node.FS.default_fs.Entry,
                   multi=1)

tarball_bld = Builder(action=tarballer,
                      target_factory=SCons.Node.FS.default_fs.Entry,
                      source_factory=SCons.Node.FS.default_fs.Entry)

env.Append(BUILDERS={'Distribute': dist_bld})
env.Append(BUILDERS={'Tarball': tarball_bld})
# end Paul Davis' ardour coolness

Help(vars.GenerateHelpText(env))

if not env.GetOption('clean'):
    conf = Configure(env)
    if conf.CheckCHeader('getopt.h'):
        conf.env.AppendUnique(CPPFLAGS=['-DHAVE_GETOPT_H'])
    conf.CheckLib('getopt', 'getopt')

    if ARGUMENTS.get('profiler', 0):
        conf.CheckLib('profiler', 'ProfilerStart')

    if ARGUMENTS.get('gcov', 0):
        if conf.CheckLib('gcov'):
            conf.env.AppendUnique(
                CCFLAGS=['-fprofile-arcs', '-ftest-coverage'])

    env = conf.Finish()

# choose debugging level
if ARGUMENTS.get('debug', 1):
    env.AppendUnique(CCFLAGS=['-g', '-O0'])
else:
    env.AppendUnique(CCFLAGS=['-O2'])

# set warning flags
warnings = ['',
            'all',
            'error',
            'aggregate-return',
            'cast-align',
            'cast-qual',
            'nested-externs',
            'shadow',
            'bad-function-cast',
            'write-strings',
            'init-self',
            'format=2',
            'uninitialized',
            'pointer-arith',
            'strict-aliasing', ]
env.AppendUnique(CCFLAGS=['-W%s' % (w,) for w in warnings])

# Paired with -Wstrict-aliasing
if 'strict-aliasing' in warnings:
    env.AppendUnique(CCFLAGS=['-fstrict-aliasing'])

# set the version
env.AppendUnique(CPPFLAGS=['-DVERSION=\\\"%s\\\"' % (VERSION,)])

# compile as GNU SOURCE to get strndup
env.AppendUnique(CPPFLAGS=['-D_GNU_SOURCE'])

# tell yacc to create a header file
env.AppendUnique(YACCFLAGS=['-d'])

# set up the disttree and tarball names
env.AppendUnique(DISTTREE='#filtergen-%s' % (VERSION,))
env.AppendUnique(TARBALL='filtergen-%s.tar.gz' % (VERSION,))


DESTDIR = ARGUMENTS.get('DESTDIR', '')

sbindir = '/usr/sbin'
mandir = '/usr/share/man'
sysconfdir = '/etc/filtergen'
pkgdocdir = '/usr/share/doc/filtergen'
pkgexdir = pkgdocdir + '/examples'

# Add the top level directory to the include path
env.AppendUnique(CPPPATH=['#'])


fg_env = env.Clone()
fg_env.AppendUnique(LIBPATH=['input',
                             'input/filtergen',
                             'input/iptables-save',
                             'output/iptables',
                             'output/iptablesrestore',
                             'output/ipchains',
                             'output/ipfilter',
                             'output/cisco',
                             'output/filtergen', ])
fg_env.AppendUnique(
    LIBS=['in_filtergen',
          'in_iptables_save',
          'sourcepos',
          'out_iptables',
          'out_iptablesrestore',
          'out_ipchains',
          'out_ipfilter',
          'out_cisco',
          'out_filtergen', ])
filtergen_sources = ['filtergen.c',
                     'gen.c',
                     'filter.c',
                     'fg-util.c',
                     'fg-iptrestore.c',
                     'icmpent.c', ]
filtergen = fg_env.Program('filtergen', filtergen_sources)

Default(filtergen)
env.Distribute(env['DISTTREE'], filtergen_sources + ['filter.h',
                                                     'icmpent.h',
                                                     'util.h'])


def sed(target, source, env):
    expandos = {
        'SYSCONFDIR': sysconfdir,
        'PKGEXDIR': pkgexdir,
        'SBINDIR': sbindir,
        'VERSION': VERSION
    }
    for (t, s) in zip(target, source):
        o = file(str(t), 'w')
        i = file(str(s), 'r')
        o.write(i.read() % expandos)
        i.close()
        o.close()
    return None

fgadm = env.Command('fgadm', 'fgadm.in', [sed, Chmod('fgadm', 0755)])

env.Command(['fgadm.conf', 'rules.filter'],
            ['fgadm.conf.in', 'rules.filter.in'],
            sed)
Default(fgadm)
env.Distribute(
    env['DISTTREE'], ['fgadm.in', 'rules.filter.in', 'fgadm.conf.in'])

SConscript([
    'input/SConscript',
    'ir/SConscript',
    'output/iptables/SConscript',
    'output/iptablesrestore/SConscript',
    'output/ipchains/SConscript',
    'output/ipfilter/SConscript',
    'output/cisco/SConscript',
    'output/filtergen/SConscript',
    'examples/SConscript',
    'doc/SConscript',
], 'env')

env.Install(DESTDIR + sbindir, [filtergen, fgadm])
bin = env.Alias('install-bin', DESTDIR + sbindir)

env.Install(DESTDIR + mandir + '/man5', 'filter_syntax.5')
man5 = env.Alias('install-man5', DESTDIR + mandir + '/man5')
env.Install(DESTDIR + mandir + '/man7', 'filter_backends.7')
man7 = env.Alias('install-man7', DESTDIR + mandir + '/man7')
env.Install(DESTDIR + mandir + '/man8', ['filtergen.8', 'fgadm.8'])
man8 = env.Alias('install-man8', DESTDIR + mandir + '/man8')
man = env.Alias('install-man', [man5, man7, man8])

env.Install(DESTDIR + sysconfdir, ['fgadm.conf', 'rules.filter'])
sysconf = env.Alias('install-sysconf', DESTDIR + sysconfdir)

env.Install(DESTDIR + pkgexdir, glob.glob('examples/*.filter'))
pkgex = env.Alias('install-examples', DESTDIR + pkgexdir)

env.Install(
    DESTDIR + pkgdocdir, ['doc/flow', 'doc/generator.notes', 'doc/notes'])
pkgdoc = env.Alias('install-doc', DESTDIR + pkgdocdir)

env.Install(DESTDIR + pkgdocdir, glob.glob('doc/*'))
pkgdoc = env.Alias('install-doc', DESTDIR + pkgdocdir)

env.Alias('install', [bin, man, sysconf, pkgdoc, pkgex])

Precious(env['DISTTREE'])

env.Distribute(
    env['DISTTREE'],
    ['SConstruct', 'Doxyfile', 'AUTHORS', 'THANKS', 'README.md',
     'HISTORY', 'HONESTY', 'TODO', 'filtergen.8', 'fgadm.8',
     'filter_syntax.5', 'filter_backends.7', 'filtergen.spec.in', ])

srcdist = env.Tarball(env['TARBALL'], env['DISTTREE'])
env.Alias('dist', srcdist)
# don't leave the disttree around
env.AddPreAction(
    env['DISTTREE'], Action('rm -rf ' + str(File(env['DISTTREE']))))
env.AddPostAction(srcdist, Action('rm -rf ' + str(File(env['DISTTREE']))))
