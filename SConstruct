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

#fgadm = e.Command('fgadm', 'fgadm.in')

DESTDIR = ARGUMENTS.get('DESTDIR', '')

sbindir = DESTDIR + '/usr/sbin'
mandir = DESTDIR + '/usr/share/man'
sysconfdir = DESTDIR + '/etc/filtergen'

e.Install(sbindir, filtergen)
bin = e.Alias('install-bin', sbindir)

e.Install(mandir + '/man5', 'filter_syntax.5')
man5 = e.Alias('install-man5', mandir + '/man5')
e.Install(mandir + '/man7', 'filter_backends.7')
man7 = e.Alias('install-man7', mandir + '/man7')
e.Install(mandir + '/man8', ['filtergen.8', 'fgadm.8'])
man8 = e.Alias('install-man8', mandir + '/man8')
man = e.Alias('install-man', [man5, man7, man8])

e.Install(sysconfdir, ['fgadm.conf', 'rules.filter'])
sysconf = e.Alias('install-sysconf', sysconfdir)

e.Alias('install', [bin, man, sysconf])
