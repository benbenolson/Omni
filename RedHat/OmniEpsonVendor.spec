Summary: The Epson modules for the Omni Print Driver Framework
Name: OmniEpsonVendor
Version: 0.9.2
Release: 1
Requires: Omni = %{version}-%{release}
License: Epson
#URL: http://oss.software.ibm.com/developer/opensource/linux/projects/omni/
URL: http://www.epkowa.co.jp/english/linux_e/pips_e.html
Group: System/Libraries
Source0: http://prdownloads.sourceforge.net/omniprint/Omni-%{version}.tar.gz
Source1: http://prdownloads.sourceforge.net/omniprint/OmniEpsonVendor-%{version}.tar.gz

BuildRoot: %{_tmppath}/Omni-%{version}-root

%description
EPSON KOWA Corporation has announced to develop the
"Photo Image Print System for Linux", a omni plugin that supports Linux.
The Photo Image Print System for Linux ensures the photo-quality image
printing with the EPSON printers for the Linux users, and has made the
first achievement on Linux of providing the print quality equivalent to
standard printer drivers for Windows and Macintosh.

%package cups
Summary: Cups data for The Omni Print Driver Framework
Group: System/Libraries
Requires: Omni = %{version}-%{release}, cups

%description cups
Cups driver meta information for the Omni Print Driver framework.

%prep
%setup -q -T -b 0 -n Omni
%setup -q -D -T -a 1 -n Omni -c

%build
	# Make the omni project
	./setupOmni DEVICES="Epson.PDC"
	cd CUPS
	make generateBuildPPDs
	cd ..

%install
	rm -rf $RPM_BUILD_ROOT
	mkdir -p $RPM_BUILD_ROOT

	make DESTDIR=$RPM_BUILD_ROOT \
		install

%clean
	rm -rf $RPM_BUILD_ROOT

%post
	# install foomatic support
	(	if [ -d /usr/share/foomatic/db/source ]; then \
			OmniFoomaticGenerator -b /usr/share/foomatic/db/source; \
		fi;
	)
	# zap the cache
	rm -rf /var/cache/foomatic/{pcache,compiled}/*

%postun
	# remove foomatic
        find /usr/share/foomatic/db/source -name \*omni\* -exec rm {} \;

%post cups
	/etc/rc.d/init.d/cups restart

%files
%defattr(-,root,root)
/usr/bin/*
/usr/lib/Omni/*
/usr/share/Omni/*
/usr/share/*.xsd

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
