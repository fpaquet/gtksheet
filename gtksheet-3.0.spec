
%define name  gtkextra3
%define major 3
%define minor 5
%define micro 1
%define ver   %major.%minor.%micro
%define rel      1

Summary: A library of gtk+ widgets
Name: %name 
Version: %ver
Release: %rel%{?dist}
License: LGPL
Group: System Environment/Libraries
Source: https://github.com/fpaquet/gtksheet/archive/V%version.tar.gz
BuildRoot:	%(mktemp -ud %{_tmppath}/%{name}-%{version}-%{release}-XXXXXX)
URL: https://fpaquet.github.io/gtksheet/
Requires:        gtk2 >= 2.12.0
Requires:        glib2 >= 2.12.0
Requires:        pango
Requires:        atk
BuildRequires:   gtk2-devel
BuildRequires:   glib2-devel
BuildRequires:   pango-devel
BuildRequires:   atk-devel

%description

A library of dynamically linked gtk+ widgets including:
GtkSheet, GtkPlot, and GtkIconList

%package devel
Summary: A library of gtk+ widgets
Group: Development/Libraries
Requires: %name = %{version}
Requires:   gtk2-devel
Requires:   glib2-devel
Requires:   pango-devel
Requires:   atk-devel

%description devel
The gtksheet-devel package includes the static libraries, header files,
and documentation for compiling programs that use gtksheet widgets.

%prep
%setup -q -n gtksheet-%{version}

%build
export CFLAGS="$RPM_OPT_FLAGS" 
%configure --disable-tests
make

%install
rm -rf $RPM_BUILD_ROOT
make DESTDIR=$RPM_BUILD_ROOT install

%clean
rm -rf $RPM_BUILD_ROOT

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

%files
%defattr(-, root, root)

%doc INSTALL README docs/*.ChangeLog
%{_libdir}/libgtksheet*.so.*

%files devel
%defattr(-, root, root)
%{_libdir}/*so
%{_libdir}/*a
%{_libdir}/pkgconfig/*
%{_includedir}/*
%{_datadir}/gtk-doc/html/gtksheet-3/*

%changelog
* Fri May 29 2015 Tom Schoonjans <Tom.Schoonjans@me.com>
- Renamed doc folder

* Wed Apr 17 2013 Tom Schoonjans <Tom.Schoonjans@me.com>
- Modified spec file in order to make it compatible with the latest RPM building recommendations

* Fri Jan 21 2005 Adrian E. Feiguin <afeiguin@uci.edu>
- First version for gtksheet-2

* Fri Dec 20 2002 Toby D. Reeves <toby@solidstatescientific.com>
- Make the rpm name a variable.
- Set rpm name to gtksheet allowing both 0.99.x and 1.1.x to be installed.

* Mon Jul 22 2002 Toby D. Reeves <toby@solidstatescientific.com>
- Corrected for use of pkg-config

* Wed Jul 17 2002 Toby D. Reeves <toby@solidstatescientific.com>
- Corrected -install to use DESTDIR
- Make gtksheet-devel package require gtksheet
- Added *.so to devel files

* Wed Mar 15  2000 Conrad Steenberg <conrad@srl.caltech.edu>
- First spec file

