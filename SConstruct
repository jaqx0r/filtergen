EnsureSConsVersion(0, 96)

opts = Options()
opts.AddOptions(
	EnumOption('debug', 'debugging compiler options', 'no',
			   allowed_values = ('yes', 'no'))
	)

e = Environment(options = opts)

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
	e.Append(CPPFLAGS='-W%s ' % (w,))

# compile as GNU SOURCE to get strndup
e.Append(CPPFLAGS = '-D_GNU_SOURCE ')

Help(opts.GenerateHelpText(e))

if not e.GetOption("clean"):
	conf = Configure(e)
	if conf.CheckCHeader('getopt.h'):
		conf.env.Append(CPPFLAGS = '-DHAVE_GETOPT_H ')
	if conf.CheckLib('getopt', 'getopt'):
		conf.env.append(LIBS = 'getopt')
	e = conf.Finish()

DESTDIR = ARGUMENTS.get('DESTDIR', '')

sbindir = '/usr/sbin'
mandir = '/usr/share/man'
sysconfdir = '/etc/filtergen'
pkgdocdir = '/usr/share/doc/filtergen'
pkgexdir = pkgdocdir + '/examples'

filtergen  = e.Program('filtergen', ['filtergen.c',
					  'gen.c',
					  'filter.c',
					  'fg-util.c',
					  'fg-iptables.c',
					  'fg-ipchains.c',
					  'fg-ipfilter.c',
					  'fg-cisco.c',
					  'parser.y',
					  'scanner.l',
					  'glue.c',
					  'resolver.c',
					  'icmpent.c',
					  'factoriser.c'])

def sed(target, source, env):
	expandos = {
		'SYSCONFDIR': sysconfdir,
		'PKGEXDIR': pkgexdir,
		'SBINDIR': sbindir
		}
	for (t, s) in zip(target, source):
		o = file(str(t), "w")
		i = file(str(s), "r")
		o.write(i.read() % expandos)
		i.close()
		o.close()
	return None

fgadm = e.Command('fgadm', 'fgadm.in', [sed, Chmod('fgadm', 0755)])

fgadmconf = e.Command('fgadm.conf', 'fgadm.conf.in', sed)

rulesfilter = e.Command('rules.filter', 'rules.filter.in', sed)

e.Install(DESTDIR + sbindir, [filtergen, fgadm])
bin = e.Alias('install-bin', DESTDIR + sbindir)

e.Install(DESTDIR + mandir + '/man5', 'filter_syntax.5')
man5 = e.Alias('install-man5', DESTDIR + mandir + '/man5')
e.Install(DESTDIR + mandir + '/man7', 'filter_backends.7')
man7 = e.Alias('install-man7', DESTDIR + mandir + '/man7')
e.Install(DESTDIR + mandir + '/man8', ['filtergen.8', 'fgadm.8'])
man8 = e.Alias('install-man8', DESTDIR + mandir + '/man8')
man = e.Alias('install-man', [man5, man7, man8])

e.Install(DESTDIR + sysconfdir, ['fgadm.conf', 'rules.filter'])
sysconf = e.Alias('install-sysconf', DESTDIR + sysconfdir)

e.Alias('install', [bin, man, sysconf])
