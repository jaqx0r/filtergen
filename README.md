#	PACKET FILTER GENERATOR

![ci](https://github.com/jaqx0r/filtergen/actions/workflows/ci.yml/badge.svg)
[![codecov](https://codecov.io/gh/jaqx0r/filtergen/branch/master/graph/badge.svg)](https://codecov.io/gh/jaqx0r/filtergen)

## GENERAL

This tool is for generating packet filtering rules from a fairly high-
level description language.  It doesn't support all of the whizz-bang
features of the latest and greatest packet filters, but supports a
decent subset which is sufficient for me.

## FILTER SUPPORT

`filtergen` currently supports generating filters for:

 * Linux iptables, ipchains direct command execution
 * Linux iptables-restore, ipchains-restore input files.
 * Cisco IOS (experimental, incomplete)
 * ipfliter (experimental, incomplete)

Filters can be generated from `filtergen`s internal language (see USAGE below)
or from existing `ipchains-save` or `iptables-save` output.

## WARNING!

This package isn't perfect.  Nevertheless, I don't believe that the
ipchains or iptables drivers leak packets.


## LICENCE

It was written, and copyrighted by <matthew@hairy.beasts.org>
and made available you you under the terms of the GNU General Public
Licence.

The current maintainer is Jamie Wilkinson.

## WHY?

Not many large softare projects are written entirely in assembly
language these days.  It's not difficult to be, and the results can
often be more efficient, but it does take longer to implement and
bugfix, and is not portable to other systems.  It is also harder for
others to understand, update and audit.

Similarly, it seems odd that people continue to write packet filters
in the equivalent of assembler, or in inflexible macro languages.

Hence this package -- my intent is that "filter" will be to iptables
and shell what a C compiler (but not a 4GL) is to assembly.


## USAGE

The filter generator is called "filtergen".  It can be invoked as:

    $ filtergen source.file > output.rules

or as a filter itself:

    $ cat source.file | filtergen > output.rules

The default output mode is for iptables, but you can override this
by giving an extra argument on the command-line:

    $ filtergen oldbox.filter -t iptables > oldbox.rules

and using "-" to imply standard input:

    $ cat oldbox.filter | filtergen -t iptables - > oldbox.rules

Some of the backends (notably iptables and iptables) generate a "skeleton"
around the rules that they output.  This can be disabled with "-n":

    $ filtergen -c examples/example.filter -t iptables

The rulesets generated with this option may not be complete, but the
flag is useful to allow one to see what sort of output is generated.

Futher details can be found in the [`filtergen.8`](filtergen.8),
[`filter_syntax.5`](filter_syntax.5), and
[`filter_backends.7`](filter_backends.7) manpages.

## FGADM

A helpful admin tool called `fgadm` is also included which manages compilation
and application of the generated filter.

Further details can be found in the [`fgadm.8`](fgadm.8) manpage.

## SYNTAX

The syntax looks not entirely unlike that of "fk"s ACL facility ("fk"
is another project of mine which lives at http://hairy.beasts.org/fk/),
and it might be worth familiarising yourself with that first (not least
because it is rather less experimental).

An example will probably show most of the basics.  Here's a simple one
for a single-interface host (perhaps a mail server):

	# Unfortunately, we don't have time to audit the
	# communications which go on locally
	{input lo; output lo} accept;

	# But we want to be a bit more careful when speaking
	# to the outside world
	input eth0 {
		proto tcp {
			dport { smtp pop-3 } accept;
			dport ssh source ournet/24 accept;
			# We don't answer this, but don't want to
			# cause timeouts by blocking it
			dport auth reject;
			log drop;
		};
		# We don't run any UDP (or other non-TCP)
		# services
		log drop;
	};
	output eth0 {
		proto tcp {
			dport { smtp auth } accept;
			log drop;
		};
		# Outbound DNS is OK
		proto udp dport domain dest { ns0 ns1 } accept;
		log drop;
	};


At first sight, this might not look like much more than a shorthand
for raw packet filtering rules, and indeed it isn't.  However, the
abbreviation is better than one might imagine because it also (again,
as far as the underlying packet filter allows it) outputs appropriate
rules for returning packets on a connection, and applies stateful
rules where possible.

So a simple rule like:

	output eth0 proto tcp dport http accept;

might generate ipchains rules like:

	ipchains -A output -i eth0 -p tcp --dport=http -j ACCEPT
	ipchains -A input -i eth0 -p tcp ! --syn --sport=http -j ACCEPT

to give simple "state" matching -- the second rule will block any
packets which could initiate a connection.  Otherwise, an attacker
could connect to any port on your machine merely by binding the
attack program to port 80 on his machine.

The same rule might generate iptables rule resembling:

	iptables -A OUTPUT -o eth0 -p tcp -m conntrack --ctstate=NEW,ESTABLISHED \
		--dport=http -j ACCEPT
	iptables -A INPUT -i eth0 -p tcp -m conntrack --ctstate=ESTABLISHED ! --syn \
		--sport=http -j ACCEPT

Note the explicit state checking here (which, in theory, makes the
`! --syn` bit unnecessary, but a bit of defence in depth never hurt).


## SIGNIFICANT DIFFERENCES FROM ACL SYNTAX

There are a number of places where the filter language differs from
the ACL language.  They are not (well, not all) as gratuitous as one
might think at first.

 * ACL has "allow" and "deny", filter has "accept", "drop"
   and "reject".

 * ACL makes no distinction between the actual inbound and
   outbound connections which make up a proxied logical
   connection.

These are essentially because a packet filter isn't really in a position
to permit or refuse a connection -- it acts only on single packets,
and is not really able to reason about connections (whatever the blurb
claims).  It also matches the terminology used by Linux's iptables.

INSTALLING filtergen
====================

Installing filtergen is easy.  If you don't find it easy, that's a bug.  Send
bug reports here in GitHub.

filtergen's build system uses [`bazel`](http://bazel.build), rather than the GNU
autotools or a custom Makefile.  To build and install, ensure you have
[`bazelisk`](https://bazel.build/install/bazelisk) installed, and type `bazel
run //:install --DESTDIR=path-to-install`.

The test suite used [`DejaGNU`](https://www.gnu.org/software/dejagnu/) which is
installed by `bazel`, but the system needs TCL `expect` installed.

```
sudo apt install expect
```

should be sufficient in order to run `bazel test //...`
