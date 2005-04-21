import glob

EnsureSConsVersion(0, 95)

VERSION = "0.13"

opts = Options()
opts.AddOptions(
	EnumOption('debug', 'debugging compiler options', 'no',
			   allowed_values=('yes', 'no', 'gcov'),
			   map={})
	)

env = Environment(options = opts)

warnings = ['',
			'all',
			'error',
			'aggregate-return',
			'cast-align',
			'cast-qual',
			'nested-externs',
			'shadow',
			'bad-function-cast',
			'write-strings']
env.AppendUnique(CCFLAGS=['-W%s' % (w,) for w in warnings])

# set the version
env.AppendUnique(CPPFLAGS=['-DVERSION=\\\"%s\\\"' % (VERSION,)])

# compile as GNU SOURCE to get strndup
env.AppendUnique(CPPFLAGS=['-D_GNU_SOURCE'])

Help(opts.GenerateHelpText(env))

if not env.GetOption("clean"):
	conf = Configure(env)
	if conf.CheckCHeader('getopt.h'):
		conf.env.AppendUnique(CPPFLAGS=['-DHAVE_GETOPT_H'])
	conf.CheckLib('getopt', 'getopt')
	env = conf.Finish()

# choose debugging level
if ARGUMENTS.get("debug") in ('yes', 'gcov'):
	env.AppendUnique(CCFLAGS=['-g', '-O0'])
	if ARGUMENTS.get("debug") in ('gcov'):
		env.AppendUnique(CCFLAGS=['-fprofile-arcs', '-ftest-coverage'])
else:
	env.AppendUnique(CCFLAGS=['-O2'])

DESTDIR = ARGUMENTS.get('DESTDIR', '')

sbindir = '/usr/sbin'
mandir = '/usr/share/man'
sysconfdir = '/etc/filtergen'
pkgdocdir = '/usr/share/doc/filtergen'
pkgexdir = pkgdocdir + '/examples'

# Add the top level directory to the include path
env.AppendUnique(CPPPATH=['#'])

filtergen  = env.Program('filtergen', ['filtergen.c',
									   'gen.c',
									   'filter.c',
									   'fg-util.c',
									   'icmpent.c',
									   'factoriser.c'],
						 LIBS=['in_filtergen',
							   'in_iptables_save',
							   'out_iptables',
							   'out_ipchains',
							   'out_ipfilter',
							   'out_cisco',
							   'out_filtergen',
							   ],
						 LIBPATH=['input/filtergen',
								  'input/iptables-save',
								  'output/iptables',
								  'output/ipchains',
								  'output/ipfilter',
								  'output/cisco',
								  'output/filtergen',
								  ]
						 )

def sed(target, source, env):
	expandos = {
		'SYSCONFDIR': sysconfdir,
		'PKGEXDIR': pkgexdir,
		'SBINDIR': sbindir,
		'VERSION': VERSION
		}
	for (t, s) in zip(target, source):
		o = file(str(t), "w")
		i = file(str(s), "r")
		o.write(i.read() % expandos)
		i.close()
		o.close()
	return None

fgadm = env.Command('fgadm', 'fgadm.in', [sed, Chmod('fgadm', 0755)])

env.Command(['fgadm.conf', 'rules.filter'],
		  ['fgadm.conf.in', 'rules.filter.in'],
		  sed)

SConscript([
	'input/filtergen/SConscript',
	'input/iptables-save/SConscript',
	'output/iptables/SConscript',
	'output/ipchains/SConscript',
	'output/ipfilter/SConscript',
	'output/cisco/SConscript',
	'output/filtergen/SConscript',
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

env.Install(DESTDIR + pkgdocdir, ['doc/flow', 'doc/generator.notes', 'doc/notes'])
pkgdoc = env.Alias('install-doc', DESTDIR + pkgdocdir)

env.Alias('install', [bin, man, sysconf, pkgdoc, pkgex])
