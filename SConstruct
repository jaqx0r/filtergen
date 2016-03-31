# -*- python -*-
import glob
import os
import SCons.Node.FS

EnsureSConsVersion(2, 3)
Decider('MD5-timestamp')

VERSION = '0.13'

vars = Variables(None, ARGUMENTS)
vars.AddVariables(
    BoolVariable('debug', 'debugging compiler options', 1),
    BoolVariable('profiler', 'enable support for profiler', 0),
    BoolVariable('gcov', 'enable test coverage with gcov', 0),
    BoolVariable('asan', 'enable compilation with AddressSanitizer', 0),
)

env = Environment(variables=vars, tools=['default', 'dejagnu', 'textfile'])

if os.environ.get('CC', None):
    env['CC'] = os.environ['CC']


Help(vars.GenerateHelpText(env))

if not env.GetOption('clean'):
    confOK = True
    conf = Configure(env)
    if not conf.CheckCC():
        print os.environ['CC'] + ' is not executable.'
        confOK = False
    if conf.CheckCHeader('getopt.h'):
        conf.env.AppendUnique(CPPFLAGS=['-DHAVE_GETOPT_H'])
        conf.CheckLib('getopt', 'getopt')

    if ARGUMENTS.get('profiler', 0):
        conf.CheckLib('profiler', 'ProfilerStart')

    if ARGUMENTS.get('gcov', 0):
        if conf.CheckLib('gcov'):
            conf.env.AppendUnique(
                CCFLAGS=['-fprofile-arcs', '-ftest-coverage'])
            conf.env.AppendUnique(
                LINKFLAGS=['--coverage'])

    if ARGUMENTS.get('asan', 0):
        conf.env.AppendUnique(
            CCFLAGS=['-fsanitize=address', '-fno-omit-frame-pointer'])
        conf.env.AppendUnique(LIBS=['asan'])

    if not confOK:
        Exit(1)

    env = conf.Finish()

# choose debugging level
if ARGUMENTS.get('debug', 1):
    env.AppendUnique(CCFLAGS=['-g', '-O0'])
    env.AppendUnique(LINKFLAGS=['-g'])
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
# tell lex to create a header file and set the name in #line directives
# correctly
env.AppendUnique(LEXFLAGS=['--header-file=${TARGET.dir}/scanner.h',
                           '-o$TARGET'])

# Compress, zip, and transform the tar contents when creating, so that we
# create a distribution tarball that is prefixed by filtergen-VERSION, to match
# common practice.
env.Replace(TARNAME='filtergen-%s' % (VERSION,))
env.Replace(TARBALL='#filtergen-%s' % (VERSION,))
env.Replace(TARSUFFIX='.tar.gz')
env.AppendUnique(TARFLAGS=['-c',
                           '-z',
                           '--xform=s,^,$TARNAME/,'])


DESTDIR = ARGUMENTS.get('DESTDIR', '')
# Individual paths can be overridden
sbindir = ARGUMENTS.get('SBINDIR', '/usr/sbin')
mandir = ARGUMENTS.get('MANDIR', '/usr/share/man')
sysconfdir = ARGUMENTS.get('SYSCONFDIR', '/etc/filtergen')
pkgdocdir = ARGUMENTS.get('PKGDOCDIR', '/usr/share/doc/filtergen')
pkgexdir = ARGUMENTS.get('PKGEXDIR', pkgdocdir + '/examples')

# Add the top level directory to the include path
env.AppendUnique(CPPPATH=['#'],

                 LIBPATH=['#input',
                          '#input/filtergen',
                          '#input/iptables-save',
                          '#output/iptables',
                          '#output/iptablesrestore',
                          '#output/ipchains',
                          '#output/ipfilter',
                          '#output/cisco',
                          '#output/filtergen'])

fg_env = env.Clone()
fg_env.AppendUnique(
    LIBS=['in_filtergen',
          'in_iptables_save',
          'sourcepos',
          'out_iptables',
          'out_iptablesrestore',
          'out_ipchains',
          'out_ipfilter',
          'out_cisco',
          'out_filtergen'])
filtergen_sources = ['filtergen.c',
                     'gen.c',
                     'filter.c',
                     'fg-util.c',
                     'fg-iptrestore.c',
                     'icmpent.c']
filtergen = fg_env.Program('filtergen', filtergen_sources)

tar = env.Tar(env['TARBALL'], filtergen_sources + ['filter.h',
                                                   'icmpent.h',
                                                   'util.h'])

print str(tar)

subst_dict = {
    '@sysconfdir@': sysconfdir,
    '@pkgexdir@': pkgexdir,
    '@sbindir@': sbindir,
    '@VERSION@': VERSION,
}

fgadm = env.Substfile('fgadm.in', SUBST_DICT=subst_dict)
env.AddPostAction(fgadm, Chmod('fgadm', 0755))
Default(fgadm)

env.Substfile('fgadm.conf.in', SUBST_DICT=subst_dict)
env.Substfile('rules.filter.in', SUBST_DICT=subst_dict)

env.Tar(
    env['TARBALL'], ['fgadm.in', 'rules.filter.in', 'fgadm.conf.in'])

env.Substfile('filtergen.spec.in', SUBST_DICT=subst_dict)

env.Tar(env['TARBALL'], ['filtergen.spec', 'filtergen.spec.in'])

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
    'testsuite/SConscript',
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

env.Install(DESTDIR + pkgdocdir, glob.glob('doc/*'))
pkgdoc = env.Alias('install-doc', DESTDIR + pkgdocdir)

env.Alias('install', [bin, man, sysconf, pkgdoc, pkgex])

env.Tar(
    env['TARBALL'],
    ['SConstruct', 'Doxyfile', 'AUTHORS', 'THANKS', 'README.md',
     'HISTORY', 'HONESTY', 'TODO', 'filtergen.8', 'fgadm.8',
     'filter_syntax.5', 'filter_backends.7', 'filtergen.spec.in'])

env.Alias('all', [filtergen, 'test-binaries'])
Default('all')

# Pack up the build system extensions as well.
env.Tar(env['TARBALL'],
        env.Glob('site_scons/site_tools/*.py') + ['site.exp'])

env.Alias('dist', tar)

distcheck = env.Command('#distcheck',
                        tar,
                        'tar zxf $SOURCE && scons -C $TARNAME check --debug=explain --tree=prune')
env.Alias('distcheck', distcheck)
print env.Dump()
