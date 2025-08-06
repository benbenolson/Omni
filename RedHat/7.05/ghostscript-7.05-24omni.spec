%define gs_ver 7.05
Summary: A PostScript(TM) interpreter and renderer.
Name: ghostscript
Version: %{gs_ver}
BuildRequires: gimp-print-devel
BuildRequires: patchutils >= 0.2.13

Release: 25
%define hpijs 1.3
%define hpijs_release %{release}


License: GPL
URL: http://www.ghostscript.com/doc/gnu/
Group: Applications/Publishing
Source0: ftp://mirror.cs.wisc.edu/pub/mirrors/ghost/gnu/gs705/%{name}-%{gs_ver}.tar.bz2
%define jpeg_ver 6b
Source1: ftp://ftp.uu.net/graphics/jpeg/jpegsrc.v%{jpeg_ver}.tar.gz
%define pcl3_ver 3.3
Source2: http://home.t-online.de/home/Martin.Lottermoser/pcl3dist/pcl3-%{pcl3_ver}.tar.gz
%define md2k_ver 0.2a
Source3: http://plaza26.mbn.or.jp/~higamasa/gdevmd2k/gdevmd2k-%{md2k_ver}.tar.gz
Source4: http://lcewww.et.tudelft.nl/~haver/cgi-bin/download/linux/epson740.tgz
%define lxm_ver 0.4.1-gs5.50
Source6: http://www.geocities.com/dgordini/lxm3200-%{lxm_ver}-src.tar.gz
Source7: http://www.powerup.com.au/~pbwest/lexmark/gdevlx50.c
Source8: ftp://bimbo.fjfi.cvut.cz/users/paluch/lexmarkgs/lexmarkgs990908.tar.gz
# Url for gs-cjk project, the source of Source9:
# http://www.gyve.org/gs-cjk/
%define cjk_ver 6.51-cjk-M2-R3
Source9: ftp://ftp.gyve.org/pub/gs-cjk/M2/gs%{cjk_ver}.tar.gz
# HP Ink Jet Server
Source10: http://prdownloads.sf.net/hpinkjet/hpijs-%{hpijs}.tar.gz
# The CJK patch is for an older version of ghostscript; some patches don't
# apply cleanly to the current one. Fixed versions here...
Source100: lib_gs_cidcm.ps.patch
Source101: lib_pdf_font.ps.patch
%define gsj_ver gs550j1
Source11: %{gsj_ver}.tar.gz
# Source12 is gs6.51 friendly contrib.mak-add for gsj
Source12: contrib.mak-gsj651.add
Source13: ftp://ftp.gyve.org/pub/gs-cjk/adobe-cmaps-200202.tar.gz
%define vflib_ver gs704
Source15: ftp://ftp.u-aizu.ac.jp/pub/tex/ptex-win32/gs/%{vflib_ver}-j-vlib.zip
Source16: ftp://ftp.gyve.org/pub/gs-cjk/acro5-cmaps-2001.tar.gz
Source17: gdevgdi.c
Source18: http://www.epkowa.on.arena.ne.jp/pips/data/2050/eplaseren-1.0.2-550.tgz
Source20: CIDFnmap-cjk-20020627.tar.gz

# Some drivers mistakenly got left out of 7.05.
Source21: gnu-drivers.tar.gz

# CUPS device (from ESP gs 7.05.4).
Source23: gdevcups.c

Patch0: ghostscript-7.05-config.patch
Patch3: ghostscript-6.51-gcc296.patch
Patch5: ghostscript-7.05-gsj.patch
Patch6: ghostscript-7.05-_ds.patch
Patch7: ghostscript-6.51-gx_device.patch
Patch8: ghostscript-6.51-gsj_update.patch
Patch10: ghostscript-7.05-config.kfvflib.patch
Patch12: ghostscript-7.05-vflib.fixup.patch
Patch13: ghostscript-6.51-res_path.patch
Patch14: ghostscript-7.05-ps2epsi.patch
Patch16: ghostscript-6.51-gdevgdi.patch
Patch17: ghostscript-6.51-Epson_eplaseren.patch
Patch18: ghostscript-6.51-gs_path.patch
Patch19: ghostscript-6.51-ia64_jmp_buf.patch
Patch21: Omni-7.05-121002.patch
Patch24: ghostscript-7.05-scripts.patch
Patch25: ghostscript-7.05-gsj-fixups.patch
Patch26: ghostscript-7.05-dx6.patch
Patch27: ghostscript-7.05-fPIC.patch
Patch28: ghostscript-7.05-gb18030.patch
Patch29: hpijs-1.3-rss.1.patch
Patch30: ghostscript-7.05-x11.patch

# I dont think this patch is needed anymore, so it is not applied.
# but I might be wrong, and dont really understand it, so it is still
# included.	-Crutcher
Patch11: gs5.50-rth.patch

Requires: VFlib2, zlib, libpng, glib
Requires: urw-fonts >= 1.1, ghostscript-fonts
Requires: Omni >= 0.6.1
BuildRequires: zlib, zlib-devel, libpng, libpng-devel, unzip, gtk+-devel
BuildRequires: VFlib2-devel, glib-devel, XFree86-devel
# Omni requires libxml
BuildRequires: libxml-devel
BuildRequires: cups-devel >= 1.1.13
BuildPreReq: freetype-devel
BuildRoot: %{_tmppath}/%{name}-%{gs_ver}-root

%description
Ghostscript is a set of software that provides a PostScript(TM)
interpreter, a set of C procedures (the Ghostscript library, which
implements the graphics capabilities in the PostScript language) and
an interpreter for Portable Document Format (PDF) files. Ghostscript
translates PostScript code into many common, bitmapped formats, like
those understood by your printer or screen. Ghostscript is normally
used to display PostScript files and to print PostScript files to
non-PostScript printers.

If you need to display PostScript files or print them to
non-PostScript printers, you should install ghostscript. If you
install ghostscript, you also need to install the ghostscript-fonts
package.

%package devel
Summary: Files for developing applications that use ghostscript.
Requires: %{name} = %{version}
Group: Development/Libraries

%description devel
The header files for developing applications that use ghostscript.

%package gtk
Summary: A GTK-enabled PostScript(TM) interpreter and renderer.
Requires: %{name} = %{version}
Group: Applications/Publishing

%description gtk
A GTK-enabled version of Ghostscript, called 'gsx'.

%package -n hpijs
Version: %{hpijs}
Release: %{hpijs_release}
Summary: HP Printer Drivers
Requires: %{name}
Group: Applications/Publishing

%description -n hpijs
hpijs is a collection of optimized drivers for HP printers.
hpijs supports the DeskJet 350C, 600C, 600C Photo, 630C, Apollo 2000,
Apollo 2100, Apollo 2560, DeskJet 800C, DeskJet 825, DeskJet 900,
PhotoSmart, DeskJet 990C and PhotoSmart 100 series.

%prep
%setup -q -a 10 -a 21

# Set up the jpeg library
	tar xzf %{SOURCE1}
	ln -s jpeg-%{jpeg_ver} jpeg

# Apply the gs-cjk patches
	tar xzf %{SOURCE9}
	(cd gs%{cjk_ver}; cp -f %{SOURCE100} %{SOURCE101} .
			  rm	src_time_.h.patch \
				src_unix-gcc.mak.patch \
				src_gp_unifs.c.patch \
				lib_gs_res.ps.patch \
				lib_pdf_ops.ps.patch \
				src_zfont42.c.patch
	)
#	a="0"
#	for i in gs%{cjk_ver}/*.patch; do
#		cat $i |patch -p0 -b -V simple -z .cjk$a
#		a=`expr $a + 1`
#	done

	# Create a Resource directory
%patch13 -p1 -b .res_path
	mkdir Resource
	mkdir Resource/Font
	mkdir Resource/CIDFont
	tar xzf %{SOURCE13} -C Resource
	tar xzf %{SOURCE16} -C Resource

	tar xzf gs%{cjk_ver}/install-cid.tar.gz -C Resource

	# Replace CJK CIDFnmap files
        tar xzf %{SOURCE20} -C lib


# Add the VFlib/jpdf patch
# The unix-gcc.mak portions of this patch are in the config patch below
	mkdir vflib-source
	unzip %{SOURCE15} -d vflib-source
	patch vflib-source/gs7.04-j-vlib.diff -b -z .vflib.fixup -i %{PATCH12}

	# Ignore most of the patches to things in lib/ (they are bogus).
        (filterdiff -p1 -x 'lib/*' vflib-source/gs7.04-j-vlib.diff;
	 filterdiff -p1 -i 'lib/gs_init.ps' vflib-source/gs7.04-j-vlib.diff) |\
	patch -p1 -b -V simple -z .vflib


# RPM configuration changes to the makefile
# The config patch sets up the use of a symbol, _XXX_RPM_GS_DEVICES_XXX_ which
# gets replaced by sed with the contents of the variable GS_DEVS. This
# allows devices to be added to the Makefile by adding them to the
# GS_DEVS string in this spec file. New devices in this spec file
# should be added with the line:
#     GS_DEVS=$GS_DEVS'$(DD)foo.dev ' <-- note the space
# Those paying attention will wonder why it works this way, the reason is that
# the DEVICE_DEVS? are NOT treated as real variables by ghostscript's build
# system, and so we have to actually change the file.
#
%patch0 -p1 -b .config
# Includes japanese changes
%patch10 -p1 -b .config.kfvflib

# Turn on every possibly pertinent builtin device
	GS_DEVS=$GS_DEVS'$(DD)dmprt.dev $(DD)cdj880.dev '
	GS_DEVS=$GS_DEVS'$(DD)ap3250.dev $(DD)appledmp.dev $(DD)atx23.dev '
	GS_DEVS=$GS_DEVS'$(DD)atx24.dev $(DD)atx38.dev $(DD)bmpa16.dev '
	GS_DEVS=$GS_DEVS'$(DD)bmpa16m.dev $(DD)bmpa256.dev $(DD)bmpa32b.dev '
	GS_DEVS=$GS_DEVS'$(DD)bmpamono.dev $(DD)bmpasep1.dev $(DD)bmpasep8.dev '
	GS_DEVS=$GS_DEVS'$(DD)ccr.dev $(DD)cdj1600.dev $(DD)cdj500.dev '
	GS_DEVS=$GS_DEVS'$(DD)cdj670.dev $(DD)cdj850.dev '
	GS_DEVS=$GS_DEVS'$(DD)cdj890.dev $(DD)cdj970.dev $(DD)cfax.dev '
	GS_DEVS=$GS_DEVS'$(DD)cgm24.dev $(DD)cgm8.dev $(DD)cgmmono.dev '
	GS_DEVS=$GS_DEVS'$(DD)cljet5pr.dev $(DD)coslw2p.dev $(DD)coslwxl.dev '
	GS_DEVS=$GS_DEVS'$(DD)cp50.dev $(DD)declj250.dev $(DD)dfaxlow.dev '
	GS_DEVS=$GS_DEVS'$(DD)dfaxhigh.dev $(DD)djet500c.dev $(DD)dl2100.dev '
	GS_DEVS=$GS_DEVS'$(DD)dnj650c.dev $(DD)eps9high.dev '
	GS_DEVS=$GS_DEVS'$(DD)eps9mid.dev $(DD)epson.dev $(DD)epsonc.dev '
	GS_DEVS=$GS_DEVS'$(DD)escp.dev $(DD)fax.dev $(DD)fs600.dev '
	GS_DEVS=$GS_DEVS'$(DD)hl1250.dev $(DD)hl7x0.dev $(DD)ibmpro.dev '
	GS_DEVS=$GS_DEVS'$(DD)imagen.dev $(DD)inferno.dev $(DD)iwhi.dev '
	GS_DEVS=$GS_DEVS'$(DD)iwlo.dev $(DD)iwlq.dev $(DD)jetp3852.dev '
	GS_DEVS=$GS_DEVS'$(DD)la50.dev $(DD)la70.dev $(DD)la75.dev '
	GS_DEVS=$GS_DEVS'$(DD)la75plus.dev $(DD)lbp8.dev $(DD)lj250.dev '
	GS_DEVS=$GS_DEVS'$(DD)lj3100sw.dev $(DD)lj4dith.dev $(DD)ln03.dev '
	GS_DEVS=$GS_DEVS'$(DD)lp2563.dev $(DD)lp8000.dev $(DD)lq850.dev '
	GS_DEVS=$GS_DEVS'$(DD)lxm5700m.dev $(DD)m8510.dev $(DD)mgr4.dev '
	GS_DEVS=$GS_DEVS'$(DD)mgr8.dev $(DD)mgrgray2.dev $(DD)mgrgray4.dev '
	GS_DEVS=$GS_DEVS'$(DD)mgrgray8.dev $(DD)mgrmono.dev $(DD)miff24.dev '
	GS_DEVS=$GS_DEVS'$(DD)necp6.dev $(DD)oce9050.dev '
	GS_DEVS=$GS_DEVS'$(DD)oki182.dev $(DD)okiibm.dev $(DD)paintjet.dev '
	GS_DEVS=$GS_DEVS'$(DD)photoex.dev $(DD)pjetxl.dev $(DD)plan9bm.dev '
	GS_DEVS=$GS_DEVS'$(DD)psdf.dev $(DD)sgirgb.dev $(DD)r4081.dev '
	GS_DEVS=$GS_DEVS'$(DD)sj48.dev $(DD)st800.dev '
	GS_DEVS=$GS_DEVS'$(DD)stcolor.dev $(DD)sunhmono.dev $(DD)t4693d2.dev '
	GS_DEVS=$GS_DEVS'$(DD)t4693d4.dev $(DD)t4693d8.dev $(DD)tek4696.dev '
	GS_DEVS=$GS_DEVS'$(DD)tfax.dev $(DD)tiffs.dev $(DD)xes.dev '
	GS_DEVS=$GS_DEVS'$(DD)x11_.dev $(DD)x11alt_.dev $(DD)x11cmyk2.dev '
	GS_DEVS=$GS_DEVS'$(DD)x11cmyk4.dev $(DD)x11cmyk8.dev $(DD)x11rg16x.dev '
	GS_DEVS=$GS_DEVS'$(DD)x11rg32x.dev '


## Add a monkey-load of Japanese Printers
	mkdir gsj
	tar xzf %{SOURCE11} -C gsj

	# Zap the older version of the okidata driver
	rm gsj/gdevop4w.*

	cp gsj/*.[ch] src/
	cp gsj/*.{ps,src} lib/
%patch5 -p1 -b .gsj

	# Repair the gsj code
%patch7 -p1 -b .gx_device
%patch8 -p1 -b .gsj_update

	# update the make files
	GS_DEVS=$GS_DEVS'$(DD)ljet4pjl.dev $(DD)lj4dithp.dev $(DD)dj505j.dev '
	GS_DEVS=$GS_DEVS'$(DD)picty180.dev $(DD)pr201.dev $(DD)pr150.dev '
	GS_DEVS=$GS_DEVS'$(DD)pr1000.dev $(DD)pr1000_4.dev $(DD)jj100.dev '
	GS_DEVS=$GS_DEVS'$(DD)bj10v.dev $(DD)bj10vh.dev $(DD)mag16.dev '
	GS_DEVS=$GS_DEVS'$(DD)mag256.dev $(DD)mj700v2c.dev $(DD)mj500c.dev '
	GS_DEVS=$GS_DEVS'$(DD)mj6000c.dev $(DD)mj8000c.dev $(DD)fmpr.dev '
	GS_DEVS=$GS_DEVS'$(DD)fmlbp.dev $(DD)ml600.dev '
	GS_DEVS=$GS_DEVS'$(DD)lbp310.dev $(DD)lbp320.dev $(DD)lips2p.dev '
	GS_DEVS=$GS_DEVS'$(DD)bjc880j.dev $(DD)lips4.dev '
	GS_DEVS=$GS_DEVS'$(DD)lips4v.dev $(DD)escpage.dev $(DD)lp2000.dev '
	GS_DEVS=$GS_DEVS'$(DD)npdl.dev $(DD)md50Mono.dev $(DD)md50Eco.dev '
	GS_DEVS=$GS_DEVS'$(DD)md1xMono.dev '
	cat %{SOURCE12} >> src/contrib.mak


# Turn on IBM's Omni print driver interface
	GS_DEVS=$GS_DEVS'$(DD)omni.dev '
	
	# Apply the 121002 patch.
%patch21 -p0 -b .Omni.121002

	# Fix some shell scripts
%patch24 -p1 -b .scripts


# Turn on HP's hpijs print driver interface
	GS_DEVS=$GS_DEVS'$(DD)ijs.dev $(DD)DJ630.dev '
	GS_DEVS=$GS_DEVS'$(DD)DJ6xx.dev $(DD)DJ6xxP.dev $(DD)DJ8xx.dev '
	GS_DEVS=$GS_DEVS'$(DD)DJ9xx.dev $(DD)DJ9xxVIP.dev $(DD)AP21xx.dev '

# Add pcl3 driver support for some other HP printers
	tar xzfO %{SOURCE2} pcl3-%{pcl3_ver}/pcl3.tar | tar xf -
	mv doc/*.1 man

	# update the make files
	GS_DEVS=$GS_DEVS'$(DD)pcl3.dev '
	ln -s . pcl3
	cat src/contrib.mak-6.51.add >> src/contrib.mak


# Add support for ALPS printers
	tar xzf %{SOURCE3}
	mv gdevmd2k-%{md2k_ver}/*.[ch] src/
	
	# update the make files
	GS_DEVS=$GS_DEVS'$(DD)md2k.dev $(DD)md5k.dev '
	cat gdevmd2k-%{md2k_ver}/gdevmd2k.mak-5.8x >> src/contrib.mak


# Add *.upp files for Epson 740
	tar xzf %{SOURCE4}
	mv epson740/*.upp lib/


# Add the gimp-print stp driver
	# update the make files
	GS_DEVS=$GS_DEVS'$(DD)stp.dev '


# Add Daniel Gordini's lxm3200 driver
	tar xzf %{SOURCE6}
	mv lxm3200-%{lxm_ver}-src/*.[ch] src/

	# update the make files
	GS_DEVS=$GS_DEVS'$(DD)lxm3200.dev '
	echo '
### ---------------- Lexmark 3200 device ----------------- ###
lxm3200_=$(GLOBJ)gdevlx32.$(OBJ)

$(GLOBJ)gdevlx32.$(OBJ): $(GLSRC)gdevlx32.c $(PDEVH) $(gsparam_h)
	$(GLCC) $(GLO_)gdevlx32.$(OBJ) $(C_) $(GLSRC)gdevlx32.c

$(DD)lxm3200.dev: $(lxm3200_) $(DD)page.dev
	$(SETPDEV) $(DD)lxm3200 $(lxm3200_)

' >> src/contrib.mak


# Add the lx5000 driver
	cp %{SOURCE7} src/

	GS_DEVS=$GS_DEVS'$(DD)lx5000.dev '
	echo '
### ----------------- Lexmark 5000 printer ----------------------------- ###
### Note: this driver was contributed by users.  Please contact:         ###
###   Peter B. West <pbwest@netscape.net>                                ###
###   Reported to work with Z51.  May work with 5700 & 7000              ###
###   Provides colour and black-only, unidirectional 300/600x600dpi.     ###

lx5000_=$(GLOBJ)gdevlx50.$(OBJ)

$(GLOBJ)gdevlx50.$(OBJ) : $(GLSRC)gdevlx50.c $(PDEVH)
	$(GLCC) $(GLO_)gdevlx50.$(OBJ) $(C_) $(GLSRC)gdevlx50.c

$(DD)lx5000.dev: $(lx5000_) $(DD)page.dev
	$(SETPDEV) $(DD)lx5000 $(lx5000_)

' >> src/contrib.mak


# Add Henryk Paluch's additional lexmark drivers
	tar xzf %{SOURCE8}
	cp lexmarkgs/gdevlx7.c src/

	# update the make files
	GS_DEVS=$GS_DEVS'$(DD)lex7000.dev $(DD)lex5700.dev '
	GS_DEVS=$GS_DEVS'$(DD)lex3200.dev $(DD)lex2050.dev '
echo '
### ------ Lexmark 2050, 3200, 5700 and 7000 devices ------- ###

lex7000_=$(GLOBJ)gdevlx7.$(OBJ)

$(GLOBJ)gdevlx7.$(OBJ): $(GLSRC)gdevlx7.c $(PDEVH) $(gsparam_h)
	$(GLCC) $(GLO_)gdevlx7.$(OBJ) $(C_) $(GLSRC)gdevlx7.c

$(DD)lex7000.dev: $(lex7000_) $(DD)page.dev
	$(SETPDEV) $(DD)lex7000 $(lex7000_)

$(DD)lex5700.dev: $(lex7000_) $(DD)page.dev
	$(SETPDEV) $(DD)lex5700 $(lex7000_)

$(DD)lex3200.dev: $(lex7000_) $(DD)page.dev
	$(SETPDEV) $(DD)lex3200 $(lex7000_)

$(DD)lex2050.dev: $(lex7000_) $(DD)page.dev
	$(SETPDEV) $(DD)lex2050 $(lex7000_)

' >> src/contrib.mak


# Add Samsung GDI driver for ML-4500
	cp %{SOURCE17} src/
%patch16 -p1 -b .gdevgdi

       # update the make files
       GS_DEVS=$GS_DEVS'$(DD)gdi.dev '
echo '
### ---------------- The Samsung SmartGDI laser printer devices --------- ###
### Note : this driver is used by Samsung SmartGDI compatible printers.   ###
###

GDIMONO=$(GLOBJ)gdevgdi.$(OBJ) $(HPPCL)

$(GLOBJ)gdevgdi.$(OBJ): $(GLSRC)gdevgdi.c $(PDEVH) $(gsparam_h)
	$(GLCC) $(GLO_)gdevgdi.$(OBJ) $(C_) $(GLSRC)gdevgdi.c

$(DD)gdi.dev: $(GDIMONO) $(DD)page.dev
	$(SETPDEV) $(DD)gdi $(GDIMONO)

' >> src/contrib.mak


# Add Epson's old eplaseren drivers
	tar xzf %{SOURCE18} -C src
%patch17 -p1 -b .Epson_eplaseren

	GS_DEVS=$GS_DEVS'$(DD)epl5800.dev $(DD)epl2050.dev $(DD)epl2050p.dev '
	GS_DEVS=$GS_DEVS'$(DD)alc8500.dev $(DD)alc2000.dev '
	cat src/eplaser/*.mak >> src/contrib.mak

	# Repair the gsj code
%patch6 -p1 -b ._ds

# Add ESP gs CUPS device
	cp %{SOURCE23} src
	GS_DEVS=$GS_DEVS'$(DD)cups.dev '
echo '
### ----------------- CUPS Ghostscript Driver ---------------------- ###


cups_=  $(GLOBJ)gdevcups.$(OBJ)

$(DD)cups.dev:  $(cups_) $(GLD)page.dev
	$(SETPDEV2) $(DD)cups $(cups_)
	$(ADDMOD) $(DD)cups -lib cupsimage -lib cups

$(GLOBJ)gdevcups.$(OBJ): $(GLSRC)gdevcups.c $(PDEVH)
	$(GLCC) $(GLO_)gdevcups.$(OBJ) $(C_) $(GLSRC)gdevcups.c

' >> src/contrib.mak

# Patch to make gcc296 happier
%patch3 -p1 -b .gcc296

# Patch to make ps2espi better/work
%patch14 -p1 -b .ps2epsi

# Fix the gs path in lprsetup.sh
%patch18 -p1 -b .gs_path

# Stupid ia64 jmp_buf alignment patch
%patch19 -p1 -b .ia64_jmp_buf

# Fix-up gdevdmpr.c
%patch25 -p1 -b .gsj-fixups

# Fix gtk+ initial window size
%patch26 -p0 -b .dx6

# Fix compilation with newer binutils
%patch27 -p1 -b .fPIC

# Add CIDnmap for GB18030 font (bug #71135)
%patch28 -p1 -b .gb18030

# Patch hpijs with Richard Spencer-Smith's enhancements.
cd hpijs-%{hpijs}
%patch29 -p1 -b .rss
cd ..

# Fix XLIBDIR
%patch30 -p1 -b .x11

# Set up for a unix os, gcc compiler build, and replace our insertion string
sed -e "s/_XXX_RPM_GS_DEVICES_XXX_/$GS_DEVS/;" src/unix-gcc.mak > Makefile

# Link with gimp-print
cat << EOF >> Makefile

STPLIB=gimpprint
EOF

# Turn on IJS support
cat << EOF >> Makefile

IJSDIR=ijs
IJSDEVS='\$(DD)ijs.dev'
IJSEXECTYPE=unix
EOF

%build
# Build IJS
cd ijs
%configure
make
cd ..

make RPM_OPT_FLAGS="$RPM_OPT_FLAGS" prefix=%{_prefix}
make so RPM_OPT_FLAGS="$RPM_OPT_FLAGS" prefix=%{_prefix}

# Build the HP IJS server
cd hpijs-%{hpijs}
%configure
make
cd ..

%install
rm -rf $RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT/{%{_mandir},%{_bindir},%{_datadir},%{_docdir}}
mkdir -p $RPM_BUILD_ROOT/{%{_libdir},%{_includedir}/ijs}

make install soinstall	prefix=$RPM_BUILD_ROOT%{_prefix} \
			mandir=$RPM_BUILD_ROOT%{_mandir} \
			datadir=$RPM_BUILD_ROOT%{_datadir} \
			bindir=$RPM_BUILD_ROOT%{_bindir} \
			libdir=$RPM_BUILD_ROOT%{_libdir} \
			docdir=$RPM_BUILD_ROOT%{_docdir}/%{name}-%{gs_ver}
cd hpijs-%{hpijs}
make install DESTDIR=$RPM_BUILD_ROOT
cd ..

cd ijs
%makeinstall
cd ..

ln -sf gs.1.gz $RPM_BUILD_ROOT/%{_mandir}/man1/ghostscript.1.gz
ln -sf gs $RPM_BUILD_ROOT/usr/bin/ghostscript

cp -a Resource $RPM_BUILD_ROOT/usr/share/ghostscript/

# Don't ship sysvlp.sh.
rm -f $RPM_BUILD_ROOT/usr/bin/sysvlp.sh

# Header files.
mkdir -p $RPM_BUILD_ROOT%{_includedir}/ghostscript
install -m0644 src/iapi.h $RPM_BUILD_ROOT%{_includedir}/ghostscript
install -m0644 src/errors.h $RPM_BUILD_ROOT%{_includedir}/ghostscript
install -m0644 src/gdevdsp.h $RPM_BUILD_ROOT%{_includedir}/ghostscript

# Don't ship ijs_client_example.
rm -f $RPM_BUILD_ROOT%{_bindir}/ijs_client_example

# The man/de/man1 symlinks are broken (bug #66238).
find $RPM_BUILD_ROOT%{_mandir}/de/man1 -type l | xargs rm -f

MAIN_PWD=`pwd`
(cd $RPM_BUILD_ROOT; find ./usr/share/ghostscript/%{gs_ver}/lib/* | \
		sed -e 's/\.//;' | grep -v lib/Fontmap* | grep -v gs_init.ps > $MAIN_PWD/rpm.sharelist
 find .%{_bindir}/ | sed -e 's/\.//;' | \
		grep -v '/$\|/hpijs$\|/gsx$\|/ijs-config$' \
		>> $MAIN_PWD/rpm.sharelist)

%clean
rm -rf $RPM_BUILD_ROOT

%files -f rpm.sharelist
%defattr(-,root,root)
%dir %{_datadir}/ghostscript
%dir %{_datadir}/ghostscript/%{gs_ver}
%dir %{_datadir}/ghostscript/%{gs_ver}/lib
%config %{_datadir}/ghostscript/%{gs_ver}/lib/gs_init.ps
%config %{_datadir}/ghostscript/%{gs_ver}/lib/Fontmap*
%{_datadir}/ghostscript/Resource/
%{_datadir}/ghostscript/%{gs_ver}/vflib
%{_datadir}/ghostscript/%{gs_ver}/examples
%{_mandir}/*/*
%doc %{_docdir}/%{name}-%{gs_ver}
%{_libdir}/libgs.so.*
%{_libdir}/libgs.so
%{_libdir}/libijs.so*

%files gtk
%defattr(-,root,root)
%{_bindir}/gsx

%files -n hpijs
%defattr(-,root,root)
%{_bindir}/hpijs
%{_docdir}/hpijs-%{hpijs}

%files devel
%defattr(-,root,root)
%dir %{_includedir}/ghostscript
%{_includedir}/ghostscript/*.h
%dir %{_includedir}/ijs
%{_includedir}/ijs/*
%{_bindir}/ijs-config
%{_libdir}/libijs.a

%changelog
* Mon Nov 11 2002 Tim Waugh <twaugh@redhat.com> 7.05-24
- Omni 071902 patch.

* Mon Nov 11 2002 Tim Waugh <twaugh@redhat.com> 7.05-23
- hpijs-1.3, with updated rss patch.
- Fix XLIBDIRS.

* Fri Oct 25 2002 Tim Waugh <twaugh@redhat.com> 7.05-22
- hpijs-rss 1.2.2.

* Mon Oct 14 2002 Tim Waugh <twaugh@redhat.com> 7.05-21
- Set libdir when installing.

* Thu Aug 15 2002 Tim Waugh <twaugh@redhat.com> 7.05-20
- Add cups device (bug #69573).

* Mon Aug 12 2002 Tim Waugh <twaugh@redhat.com> 7.05-19
- Fix the gb18030 patch (bug #71135, bug #71303).

* Sat Aug 10 2002 Elliot Lee <sopwith@redhat.com> 7.05-18
- rebuilt with gcc-3.2 (we hope)

* Fri Aug  9 2002 Tim Waugh <twaugh@redhat.com> 7.05-17
- Add CIDnmap for GB18030 font (bug #71135).
- Fix URL (bug #70734).

* Tue Jul 23 2002 Tim Waugh <twaugh@redhat.com> 7.05-16
- Rebuild in new environment.

* Tue Jul  9 2002 Tim Waugh <twaugh@redhat.com> 7.05-15
- Remove the chp2200 driver again, to fix cdj890 (bug #67578).

* Fri Jul  5 2002 Tim Waugh <twaugh@redhat.com> 7.05-14
- For CJK font support, use CIDFnmap instead of CIDFont
  resources (bug #68009).

* Wed Jul  3 2002 Tim Waugh <twaugh@redhat.com> 7.05-13
- Build requires unzip and gtk+-devel (bug #67799).

* Wed Jun 26 2002 Tim Waugh <twaugh@redhat.com> 7.05-12
- File list tweaking.
- More file list tweaking.

* Tue Jun 25 2002 Tim Waugh <twaugh@redhat.com> 7.05-10
- Rebuild for bootstrap.

* Wed Jun 19 2002 Tim Waugh <twaugh@redhat.com> 7.05-9
- Omni 052902 patch.

* Mon Jun 10 2002 Tim Waugh <twaugh@redhat.com> 7.05-8
- Requires recent version of patchutils (bug #65947).
- Don't ship broken man page symlinks (bug #66238).

* Wed May 29 2002 Tim Waugh <twaugh@redhat.com> 7.05-7
- Put gsx in its own package.

* Tue May 28 2002 Tim Waugh <twaugh@redhat.com> 7.05-6
- New gomni.c from IBM to fix an A4 media size problem.
- Use new Adobe CMaps (bug #65362).

* Sun May 26 2002 Tim Powers <timp@redhat.com> 7.05-5
- automated rebuild

* Wed May 22 2002 Tim Waugh <twaugh@redhat.com> 7.05-4
- New gomni.c from IBM to fix bug #65269 (again).

* Tue May 21 2002 Tim Waugh <twaugh@redhat.com> 7.05-2
- Don't apply bogus parts of vflib patch (bug #65268).
- Work around Omni -sPAPERSIZE=a4 problem (bug #65269).

* Mon May 20 2002 Tim Waugh <twaugh@redhat.com> 7.05-1
- 7.05.
- No longer need mkstemp, vflib.fixup, quoting, or PARANOIDSAFER
  patches.
- Don't apply CJK patches any more (no longer needed).
- Updated Source15, Patch0, Patch10, Patch5, Patch24, Patch14, Patch12.
- Made gdevdmpr.c compile again.
- Move gimp-print to a separate package.
- Ship the shared object too (and a .so file that is dlopened).
- Update Omni patch.  No longer need Omni_path, Omni_quiet, Omni_glib patches.
- Require Omni >= 0.6.1.
- Add patch to fix gtk+ initial window size.
- Add devel package with header files.
- Turn on IJS support.
- Update hpijs to 1.1.
- Don't ship the hpijs binary in the ghostscript package.
- Use -fPIC when building ijs.

* Wed Apr  3 2002 Tim Waugh <twaugh@redhat.com> 6.52-8
- New CIDFonts (bug #61015).

* Wed Apr  3 2002 Tim Waugh <twaugh@redhat.com> 6.52-7
- Fix release numbers of sub packages.
- Handle info files, use ldconfig (bug #62574).

* Tue Mar 19 2002 Tim Waugh <twaugh@redhat.com> 6.52-6
- Fix config patch so that gs --help displays the right thing.
- Don't ship sysvlp.sh.
- Fix some shell scripts.
- Ship escputil man page (bug #58919).

* Mon Feb 11 2002 Tim Waugh <twaugh@redhat.com> 6.52-5
- Add CHP2200 driver (bug #57516).
- Fix gimp-print-4.2.0 so that it builds without cups-config.

* Sat Feb  2 2002 Bill Nottingham <notting@redhat.com> 6.52-4
- do condrestart in %postun, not %post

* Fri Feb  1 2002 Bernhard Rosenkraenzer <bero@redhat.com> 6.52-3
- Restart service cups after installing gimp-print-cups

* Sun Jan 27 2002 Bernhard Rosenkraenzer <bero@redhat.com> 6.52-2
- hpijs is finally free - support it.
- Add extra package for CUPS support

* Mon Jan 21 2002 Bernhard Rosenkraenzer <bero@redhat.com> 6.52-1
- Updates:
  - ghostscript 6.52
  - hpdj 2.6 -> pcl3 3.3
  - CJK Patchlevel 3, adobe-cmaps 200109
  - gimp-print 4.2.0
- Adapt patches
- Fix various URLs
- Begin cleaning up spec file
- Fix bugs #21879 and #50923

* Wed Jan 09 2002 Tim Powers <timp@redhat.com>
- automated rebuild

* Thu Oct 18 2001 Crutcher Dunnavant <crutcher@redhat.com> 6.51-16
- update the Omni driver, and patch it to seek in /usr/lib/Omni/ first
- require Omni

* Mon Oct 01 2001 Crutcher Dunnavant <crutcher@redhat.com> 6.51-15
- change -dPARANOIDSAFER to punch a hole for OutputFile

* Mon Sep 17 2001 Crutcher Dunnavant <crutcher@redhat.com> 6.51-14
- add -dPARANOIDSAFER to let us breathe a little easier in the print spooler.

* Thu Sep 13 2001 Crutcher Dunnavant <crutcher@redhat.com> 6.51-13
- apply jakub's fix to ghostscript's jmp_buf problems; #49591

* Wed Sep  5 2001 Crutcher Dunnavant <crutcher@redhat.com> 6.51-12
- fix lprsetup.sh; #50925

* Fri Aug 24 2001 Crutcher Dunnavant <crutcher@redhat.com> 6.51-11
- added Epson's old eplaseren drivers,
- pointed out by Till Kamppeter <till.kamppeter@gmx.net>

* Tue Aug 21 2001 Paul Howarth <paul@city-fan.org> 6.51-10
- included Samsung GDI driver for ML-4500 printer support.

* Sun Aug 19 2001 Crutcher Dunnavant <crutcher@redhat.com> 6.51-9
- applied IBM's glib patches for Omni, which now works.
- BE AWARE: we now link against libstdc++ and glib for this, and use a c++
- link stage to do the dirty.
- added glib-devel buildreq and glib req, I don't think we require everything
- yet, I could pull in sasl.

* Sun Aug 19 2001 David Suffield <david_suffield@hp.com> 6.51-8
- Added gs device hpijs and updated gdevhpij.c to hpijs 0.97

* Wed Aug 15 2001 Crutcher Dunnavant <crutcher@redhat.com> 6.51-7
- pull in ynakai's update to the cjk resources.

* Thu Aug  9 2001 Crutcher Dunnavant <crutcher@redhat.com> 6.51-6
- turn dmprt and cdj880 back on. for some reason, they work now.
- voodoo, who knows.

* Thu Aug  9 2001 Yukihiro Nakai <ynakai@redhat.com> 6.51-5
- Add cjk resources

* Thu Aug  1 2001 Crutcher Dunnavant <crutcher@redhat.com> 6.51-4
- applied drepper@redhat.com's patch for #50300
- fixed build deps on zlib-devel and libpng-devel, #49853
- made gs_init.ps a config file; #25096
- O\^/nZ the daTa directorieZ now; #50693

* Tue Jul 24 2001 Crutcher Dunnavant <crutcher@redhat.com> 6.51-3
- wired up the Resource dir and the Font and CIDFont maps.

* Mon Jul 23 2001 Crutcher Dunnavant <crutcher@redhat.com> 6.51-2
- luckily, I had a spare chicken. Thanks to some work by Nakai, and one last
- desperate search through google, everything /seems/ to be working. I know
- that there are going to be problems in the japanese code, and I need to turn
- on the cjk font map from adobe, but it /works/ at the moment.

* Thu Jun 21 2001 Crutcher Dunnavant <crutcher@redhat.com>
- upgraded to 6.51, a major version upgrade
- rewrote spec file, threw out some patches
- turned on IBM's Omni print drivers interface
- turned on HP's hpijs print drivers interface
- turned on every driver that looked usable from linux
- sacrificed a chicken to integrate the old Japanese drivers
- - This didn't work. The japanese patches are turned off, pending review.
- - I can do loops with C, but the bugs are in Postscript init files

* Wed Apr 11 2001 Crutcher Dunnavant <crutcher@redhat.com>
- added P. B. West's lx5000 driver

* Tue Feb 27 2001 Crutcher Dunnavant <crutcher@redhat.com>
- added xtt-fonts requirement (for VFlib)

* Fri Feb  9 2001 Adrian Havill <havill@redhat.com>
- cmpskit removed as a build prereq

* Thu Feb  8 2001 Crutcher Dunnavant <crutcher@redhat.com>
- merged in some patches that got away:
-	* Fri Sep  1 2000 Mitsuo Hamada <mhamada@redhat.com>
	- add support JIS B size
	- fix the problem of reconverting GNUPLOT output

* Thu Feb  8 2001 Crutcher Dunnavant <crutcher@redhat.com>
- switched to japanese for everybody

* Thu Feb  8 2001 Crutcher Dunnavant <crutcher@redhat.com>
- tweaked time_.h to test for linux, and include the right
- header

* Wed Feb  7 2001 Crutcher Dunnavnat <crutcher@redhat.com>
- added the lxm3200 driver

* Mon Dec 11 2000 Crutcher Dunnavant <crutcher@redhat.com>
- merged in the (accendental) branch that contained the mktemp
- and LD_RUN_PATH bug fixes.

* Tue Oct 17 2000 Jeff Johnson <jbj@redhat.com>
- tetex using xdvi with ghostscript patch (#19212).

* Tue Sep 12 2000 Michael Stefaniuc <mstefani@redhat.com>
- expanded the gcc296 patch to fix a compilation issue with the new stp
  driver

* Mon Sep 11 2000 Michael Stefaniuc <mstefani@redhat.com>
- added the stp driver from the gimp-print project.
  It supports high quality printing especialy with Epson Stylus Photo.

* Wed Aug  2 2000 Matt Wilson <msw@redhat.com>
- rebuilt against new libpng

* Wed Aug  2 2000 Bernhard Rosenkraenzer <bero@redhat.com>
- Fix up the cdj880 patch (Bug #14978)
- Fix build with gcc 2.96

* Fri Jul 21 2000 Bill Nottingham <notting@redhat.com>
- turn off japanese support

* Thu Jul 13 2000 Prospector <bugzilla@redhat.com>
- automatic rebuild

* Fri Jul 07 2000 Trond Eivind Glomsrød <teg@redhat.com>
- fixed the broken inclusion of files in /usr/doc
- Build requires freetype-devel

* Fri Jun 16 2000 Matt Wilson <msw@redhat.com>
- build japanese support in main distribution
- FHS manpage paths

* Sun Mar 26 2000 Chris Ding <cding@redhat.com>
- enabled bmp16m driver

* Thu Mar 23 2000 Matt Wilson <msw@redhat.com>
- added a boatload of Japanese printers

* Thu Mar 16 2000 Matt Wilson <msw@redhat.com>
- add japanese support, enable_japanese macro

* Mon Feb 14 2000 Bernhard Rosenkraenzer <bero@redhat.com>
- 5.50 at last...
- hpdj 2.6
- Added 3rd party drivers:
  - Lexmark 5700 (lxm5700m)
  - Alps MD-* (md2k, md5k)
  - Lexmark 2050, 3200, 5700 and 7000 (lex2050, lex3200, lex5700, lex7000)

* Fri Feb  4 2000 Bernhard Rosenkraenzer <bero@redhat.com>
- rebuild to compress man page
- fix gs.1 symlink

* Wed Jan 26 2000 Bill Nottingham <notting@redhat.com>
- add stylus 740 uniprint files

* Thu Jan 13 2000 Preston Brown <pbrown@redhat.com>
- add lq850 dot matrix driver (#6357)

* Thu Oct 28 1999 Bill Nottingham <notting@redhat.com>
- oops, include oki182 driver.

* Tue Aug 24 1999 Bill Nottingham <notting@redhat.com>
- don't optimize on Alpha. This way it works.

* Thu Jul 29 1999 Michael K. Johnson <johnsonm@redhat.com>
- added hpdj driver
- changed build to use tar_cat so adding new drivers is sane

* Thu Jul  1 1999 Bill Nottingham <notting@redhat.com>
- add OkiPage 4w+, HP 8xx drivers
* Mon Apr  5 1999 Bill Nottingham <notting@redhat.com>
- fix typo in config patch.

* Sun Mar 21 1999 Cristian Gafton <gafton@redhat.com>
- auto rebuild in the new build environment (release 6)

* Mon Mar 15 1999 Cristian Gafton <gafton@redhat.com>
- added patch from rth to fix alignement problems on the alpha.

* Wed Feb 24 1999 Preston Brown <pbrown@redhat.com>
- Injected new description and group.

* Mon Feb 08 1999 Bill Nottingham <notting@redhat.com>
- add uniprint .upp files

* Sat Feb 06 1999 Preston Brown <pbrown@redhat.com>
- fontpath update.

* Wed Dec 23 1998 Preston Brown <pbrown@redhat.com>
- updates for ghostscript 5.10

* Fri Nov 13 1998 Preston Brown <pbrown@redhat.com>
- updated to use shared urw-fonts package.
* Mon Nov 09 1998 Preston Brown <pbrown@redhat.com>
- turned on truetype (ttf) font support.

* Thu Jul  2 1998 Jeff Johnson <jbj@redhat.com>
- updated to 4.03.

* Tue May 05 1998 Cristian Gafton <gafton@redhat.com>
- enabled more printer drivers
- buildroot

* Mon Apr 27 1998 Prospector System <bugs@redhat.com>
- translations modified for de, fr, tr

* Mon Mar 03 1997 Erik Troan <ewt@redhat.com>
- Made /usr/share/ghostscript/3.33/Fontmap a config file.
