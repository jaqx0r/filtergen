import glob

EnsureSConsVersion(0, 96)

VERSION = "0.13"

opts = Options()
opts.AddOptions(
	BoolOption('debug', 'debugging compiler options', 0)
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
for w in warnings:
	env.Append(CPPFLAGS='-W%s ' % (w,))

# set the version
env.Append(CPPFLAGS = '-DVERSION=\\\"%s\\\" ' % (VERSION,))

# compile as GNU SOURCE to get strndup
env.Append(CPPFLAGS = '-D_GNU_SOURCE ')

Help(opts.GenerateHelpText(env))

if not env.GetOption("clean"):
	conf = Configure(env)
	if conf.CheckCHeader('getopt.h'):
		conf.env.Append(CPPFLAGS = '-DHAVE_GETOPT_H ')
	conf.CheckLib('getopt', 'getopt')
	env = conf.Finish()

if ARGUMENTS.get('debug') == 'yes':
	env.Append(CPPFLAGS = '-g -O0 ')
else:
	env.Append(CPPFLAGS = '-O2 ')

DESTDIR = ARGUMENTS.get('DESTDIR', '')

sbindir = '/usr/sbin'
mandir = '/usr/share/man'
sysconfdir = '/etc/filtergen'
pkgdocdir = '/usr/share/doc/filtergen'
pkgexdir = pkgdocdir + '/examples'

env.Append(CPPPATH = 'input/filtergen')

filtergen  = env.Program('filtergen', ['filtergen.c',
									 'gen.c',
									 'filter.c',
									 'fg-util.c',
									 'icmpent.c',
									 'factoriser.c'],
						 LIBS=['in_filtergen',
							   'out_iptables',
							   'out_ipchains',
							   'out_ipfilter',
							   'out_cisco'
							   ],
						 LIBPATH=['input/filtergen',
								  'output/iptables',
								  'output/ipchains',
								  'output/ipfilter',
								  'output/cisco'
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
	'input/iptables/SConscript',
	#'input/ipchains-save/SConscript',
	'output/iptables/SConscript',
	'output/ipchains/SConscript',
	'output/ipfilter/SConscript',
	'output/cisco/SConscript',
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
