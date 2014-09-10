Name: paflib
Version: 0.1.3
Release: 3%{?dist}
Summary: Library for accessing Power Architecture Facilities
Group: Development/Libraries
License: MIT
URL: https://github.com/paflib/paflib
Source0: https://github.com/paflib/paflib/archive/%{version}.tar.gz
ExclusiveArch: ppc %{power64}
BuildRequires: libtool

%package devel
Summary: Header files for paflib
Group: Development/Libraries
Requires: %{name}%{?_isa} = %{version}-%{release}
%description devel
Contains header files for building with paflib.

%description
PAFLib is a IBM written library which exposes Power Architecture Facilities to
user space via an API. This includes the Data Stream Control Register Facility
(DSCR) and the Event-Based Branching facility (EBB). Linux kernel 3.9 has
exposed problem-state DSCR usage for ISA 2.06 (POWER7 – emulated) and ISA
2.07 (POWER8– in hardware). Linux 3.10 has exposed the EBB facility.

%prep
%setup -q -n %{name}-%{version}

%build
%configure --disable-static
make %{?_smp_mflags}

%install
make install DESTDIR=$RPM_BUILD_ROOT

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

%files
%exclude %{_libdir}/libpaf-dsc.la
%{_libdir}/libpaf-ebb.so.0.0.1
%{_libdir}/libpaf-dsc.so.0.0.1
%{_libdir}/libpaf-dsc.so.0
%{_libdir}/libpaf-ebb.so.0
%exclude %{_libdir}/libpaf-ebb.la
%{_mandir}/man3/libpaf-dsc.3*
%{_mandir}/man3/libpaf-ebb.3*

%doc COPYING README.md

%files devel
%{_libdir}/libpaf-dsc.so
%{_libdir}/libpaf-ebb.so
%dir %{_includedir}/paf
%{_includedir}/paf/dsc.h
%{_includedir}/paf/ebb.h

%changelog
* Wed Sep 10 2014 Rajalakshmi S <raji@linux.vnet.ibm.com> 0.1-3
- Various spec file fixes

* Tue Sep 09 2014 Rajalakshmi S <raji@linux.vnet.ibm.com> 0.1-2
- Remove passing libdir and prefix to configure in spec file
- Remove RPM_BUILD_ROOT cleanup in install in spec file

* Tue Sep 09 2014 Rajalakshmi S <raji@linux.vnet.ibm.com> 0.1-1
- Fix builddir in spec-file

* Mon Sep 01 2014 Rajalakshmi S <raji@linux.vnet.ibm.com> 0.1-1
- Initial RPM release
