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

Help(opts.GenerateHelpText(e))

conf = Configure(e)
if conf.CheckCHeader('getopt.h'):
	conf.env.Append(CPPFLAGS = '-DHAVE_GETOPT_H ')
if conf.CheckLib('getopt', 'getopt'):
	conf.env.append(LIBS = 'getopt')
e = conf.Finish()

e.Program('filtergen', ['filtergen.c',
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
