Summary: The Omni Print Driver Framework
Name: Omni
Version: 0.9.2
Release: 1
Provides: Omni = %{version}-%{release}
License: LGPL
URL: http://oss.software.ibm.com/developer/opensource/linux/projects/omni/
Group: Applications/Publishing
Source0: http://prdownloads.sourceforge.net/omniprint/Omni-%{version}.tar.gz

BuildRoot: %{_tmppath}/Omni-%{version}-root

%description
Omni provides support for many printers with a pluggable framework (easy
to add devices). Device data is defined with XML with many different hook
points for code support. Omni also supports many new printing standards.

%package foomatic
Summary: Foomatic data for The Omni Print Driver Framework
Group: System/Libraries
Requires: Omni = %{version}-%{release}, foomatic

%description foomatic
Foomatic driver meta information for the Omni Print Driver framework.

%package cups
Summary: Cups data for The Omni Print Driver Framework
Group: System/Libraries
Requires: Omni = %{version}-%{release}, cups

%description cups
Cups driver meta information for the Omni Print Driver framework.

%prep
%setup -q -n Omni

%build
	# Make the omni project
	./setupOmni
	cd CUPS
	make generateBuildPPDs
	cd ..

%install
	rm -rf $RPM_BUILD_ROOT
	mkdir -p $RPM_BUILD_ROOT

	make DESTDIR=$RPM_BUILD_ROOT \
		install
	cd Foomatic
	make DESTDIR=$RPM_BUILD_ROOT \
		localInstall
	cd ..

%clean
	rm -rf $RPM_BUILD_ROOT

%post
	# add /usr/lib/Omni to /etc/ld.so.conf
	if ! grep /usr/lib/Omni /etc/ld.so.conf >/dev/null 2>&1; then \
		/bin/cp /etc/ld.so.conf /etc/ld.so.conf.preomni; \
		echo /usr/lib/Omni >> /etc/ld.so.conf; \
		ldconfig; \
	fi
	# remove omni.sh from /etc/profile.d
        if [ -f /etc/profile.d/omni.sh ]; then \
		/bin/rm /etc/profile.d/omni.sh; \
	fi
	# remove omni.csh from /etc/profile.d
        if [ -f /etc/profile.d/omni.csh ]; then \
		/bin/rm /etc/profile.d/omni.csh; \
	fi

%postun
	# remove /usr/lib/Omni to /etc/ld.so.conf
	if grep /usr/lib/Omni /etc/ld.so.conf >/dev/null 2>&1; then \
		/bin/cp /etc/ld.so.conf /etc/ld.so.conf.postomni; \
		sed '/\/opt\/Omni\/lib/d' /etc/ld.so.conf > /tmp/$$; \
		/bin/cp /tmp/$$ /etc/ld.so.conf; \
		/bin/rm /tmp/$$; \
		ldconfig; \
	fi
	# remove foomatic
        find /usr/share/foomatic/db/source -name \*omni\* -exec rm {} \;

%post foomatic
	# zap the cache
	rm -rf /var/cache/foomatic/{pcache,compiled}/*

%post cups
	/etc/rc.d/init.d/cups restart

%files
%defattr(-,root,root)
/usr/bin/*
/usr/lib/Omni/*
/usr/share/Omni/*
/usr/share/*.xsd

%files foomatic
%defattr(-,root,root)
/usr/share/foomatic/db/*/*

%files cups
%defattr(-,root,root)
/usr/lib/cups/filter/*
/usr/share/cups/model/omni/*/*

%changelog
* Mon Mar  4 2002 Mark Hamzy <hamzy@us.ibm.com>	0.6.1omni
- added support for Omni-0.6.1
- omni installs in /opt/Omni for LSB
- cache was misspelled

* Thu Oct 25 2001 Crutcher Dunnavant <crutcher@redhat.com> 0.5.0-4
- zap the foomatic cache on install

* Thu Oct 18 2001 Crutcher Dunnavant <crutcher@redhat.com> 0.5.0-2
- patch to make omni seek in /usr/lib/Omni and /opt/Omni/bin

* Mon Oct 15 2001 Crutcher Dunnavant <crutcher@redhat.com> 0.5.0-1
- initial packaging
