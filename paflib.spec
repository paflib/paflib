Name: paflib
Version: 0.1
Release: 1%{?dist}
Summary: IBM library to expose Power Arch Facilities to user space via an API
Group: Development/Libraries
License: MIT
URL: https://github.com/paflib/paflib
Source0: %{name}-%{version}.tar.gz
ExclusiveArch: ppc ppc64 ppc64le
BuildRequires: libtool

%package devel
Summary: Header files for paflib
Group: Development/Libraries
Requires: %{name} = %{version}-%{release}
%description devel
Contains header files for building with paflib.

%description
PAFLib is a IBM written library which exposes Power Architecture Facilities to
user space via an API. This includes the Data Stream Control Register Facility
(DSCR) and the Event-Based Branching facility (EBB). Linux kernel 3.9 has
exposed problem-state DSCR usage for ISA 2.06 (POWER7 – emulated) and ISA
2.07 (POWER8– in hardware). Linux 3.10 has exposed the EBB facility.

%prep
%setup -q

%build
cd %{_builddir}
%{name}-%{version}/configure --prefix=%{_usr} --libdir=%{_libdir}
make %{?_smp_mflags}

%install
rm -rf $RPM_BUILD_ROOT
cd %{_builddir}
make install DESTDIR=$RPM_BUILD_ROOT

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

%files
%defattr(-,root,root,-)
%exclude %{_libdir}/libpaf-dsc.la
%{_libdir}/libpaf-ebb.so.0.0.1
%{_libdir}/libpaf-dsc.so.0.0.1
%{_libdir}/libpaf-dsc.so.0
%{_libdir}/libpaf-ebb.so.0
%exclude %{_libdir}/libpaf-ebb.la
%{_mandir}/man3/libpaf-dsc.3.gz
%{_mandir}/man3/libpaf-ebb.3.gz

%doc COPYING README.md

%files devel
%defattr(-,root,root,-)
%{_libdir}/libpaf-dsc.so
%{_libdir}/libpaf-ebb.so
%{_libdir}/libpaf-dsc.a
%{_libdir}/libpaf-ebb.a
%{_includedir}/paf/dsc.h
%{_includedir}/paf/ebb.h



%changelog
* Mon Sep 01 2014 Rajalakshmi S <raji@linux.vnet.ibm.com> 0.1-1
- Initial RPM release
