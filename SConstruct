e = Environment()


conf = Configure(e)
if conf.CheckCHeader('getopt.h'):
	conf.env.Append(CCFLAGS = '-DHAVE_GETOPT_H')
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
