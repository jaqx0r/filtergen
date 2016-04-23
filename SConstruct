# -*- python -*-
import glob
import os
import SCons.Node.FS

EnsureSConsVersion(2, 3)
Decider('MD5-timestamp')

VERSION = '0.13'

vars = Variables(None, ARGUMENTS)
vars.AddVariables(
    BoolVariable('debug', 'debugging compiler options', True),
    BoolVariable('profiler', 'enable support for profiler', False),
    BoolVariable('coverage', 'enable test coverage reporting (implies debug)', False),
    BoolVariable('asan', 'enable compilation with AddressSanitizer', False),
)

env = Environment(variables=vars, tools=[
                  'default', 'dejagnu', 'textfile', 'lcov', 'splint'])

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
        conf.env.AppendUnique(CPPDEFINES=['HAVE_GETOPT_H'])
        conf.CheckLib('getopt', 'getopt')

    if ARGUMENTS.get('profiler'):
        conf.CheckLib('profiler', 'ProfilerStart')

    if ARGUMENTS.get('coverage'):
        # Always set debug for coverage.
        ARGUMENTS['debug'] = True
        if conf.CheckLib('gcov'):
            conf.env.AppendUnique(
                CCFLAGS=['-fprofile-arcs', '-ftest-coverage'])
            conf.env.AppendUnique(
                LINKFLAGS=['--coverage'])

    if ARGUMENTS.get('asan'):
        conf.env.AppendUnique(
            CCFLAGS=['-fsanitize=address', '-fno-omit-frame-pointer'])
        conf.env.AppendUnique(
            LINKFLAGS=['-fsanitize=address', '-fno-omit-frame-pointer'])

    if not confOK:
        Exit(1)

    env = conf.Finish()

# choose debugging level
if ARGUMENTS.get('debug'):
    env.AppendUnique(CCFLAGS=['-ggdb', '-O0'])
    env.AppendUnique(LINKFLAGS=['-ggdb'])
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
env.AppendUnique(CPPDEFINES=[('VERSION', '\\"%s\\"' % VERSION)])

# compile as GNU SOURCE to get strndup
env.AppendUnique(CPPDEFINES=['_GNU_SOURCE'])

# tell yacc to create a header file
env.AppendUnique(YACCFLAGS=['-d'])
# tell lex to create a header file and set the name in #line directives
# correctly
env.AppendUnique(LEXFLAGS=['--header-file=${TARGET.dir}/scanner.h',
                           '-CF',
                           '-o$TARGET'])

# Compress, zip, and transform the tar contents when creating, so that we
# create a distribution tarball that unpacks to a directory named
# filtergen-VERSION, to match common practice.
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
                 LIBPATH=['#',
                          '#input',
                          '#input/filtergen',
                          '#input/iptables-save',
                          '#output/iptables',
                          '#output/iptablesrestore',
                          '#output/ipchains',
                          '#output/ipfilter',
                          '#output/cisco',
                          '#output/filtergen'])

env.Library('core', ['error.c', 'filter.c', 'icmpent.c', 'fg-util.c'])

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
          'out_filtergen',
          'core'])
filtergen_sources = ['filtergen.c',
                     'gen.c',
                     'fg-iptrestore.c']
filtergen = fg_env.Program('filtergen', filtergen_sources)

tar = env.Tar(env['TARBALL'], filtergen_sources + ['error.c', 'error.h',
                                                   'filter.c', 'filter.h',
                                                   'icmpent.c', 'icmpent.h',
                                                   'fg-util.c',
                                                   'util.h'])

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
                        'scons -C $TARNAME check dist')
env.AddPreAction(distcheck, 'tar zxf $SOURCE')
env.AddPostAction(distcheck, 'rm -rf $TARNAME')
env.Alias('distcheck', distcheck)

splint = env.SplintAll(filtergen)
env.Alias('splint', splint)
