# $Id: filtergen.spec,v 1.6 2002/11/13 10:01:45 matthew Exp $
Summary: packet filter generator for various firewall systems
Name: filtergen
Version: 0.11
Release: 1
License: GPL
Group: Applications/System
Source: http://hairy.beasts.org/filter/filtergen-%{version}.tar.gz
URL: http://hairy.beasts.org/filter/
Buildroot: %{_tmppath}/%{name}-root
BuildPrereq: rpm-build gcc flex make
Packager: Wil Cooley <wcooley@nakedape.cc>
Vendor: Naked Ape Consulting <http://nakedape.cc>

%description
filtergen is a packet filter generator.  It compiles a fairly high-level
description language into iptables, ipchains, or ipfilter rules (and has
bits of support for Cisco IOS access lists).


%prep
%setup -q -n filtergen-%{version}

%build
make CFLAGS="${RPM_OPT_FLAGS}" LDFLAGS="-lfl"

%install
rm -rf $RPM_BUILD_ROOT
make PREFIX=${RPM_BUILD_ROOT}/usr BINDIR=${RPM_BUILD_ROOT}/sbin install
install -d ${RPM_BUILD_ROOT}/etc/init.d
install -m755 extras/filter.init ${RPM_BUILD_ROOT}/etc/init.d/filtergen
install -d ${RPM_BUILD_ROOT}/etc/filter
install extras/etc-filter-Makefile ${RPM_BUILD_ROOT}/etc/filter/Makefile

%post
/sbin/chkconfig --add filtergen
/sbin/chkconfig filtergen off

%preun
/sbin/service filtergen accept
/sbin/chkconfig --del filtergen

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
/sbin/filtergen
/etc/init.d/filtergen
/etc/filter/
%{_mandir}/*
%doc HISTORY HONESTY README TODO tests/*.filter

%changelog
* Wed Nov 13 2002 Matthew Kirkwood <matthew@hairy.beasts.org> 0.11-1
- Release 0.11, rename init script from "filter" to "filtergen"

* Mon Sep  2 2002 Matthew Kirkwood <matthew@hairy.beasts.org> 0.10-1
- Add new sample and documentation stuff

* Sun Aug 18 2002 Wil Cooley <wcooley@nakedape.cc> 0.8-1
- Initial RPM creation
