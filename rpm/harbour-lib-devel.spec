Name:       harbour-lib-devel

Summary:    Utility library for Sailfish harbour apps
Version:    1.0
Release:    1
Group:      System/Libraries
License:    BSD
URL:        https://github.com/monich/harbour-lib
Source0:    %{name}-%{version}.tar.bz2

BuildRequires:  pkgconfig(libglibutil)
BuildRequires:  pkgconfig(libqrencode)
BuildRequires:  pkgconfig(Qt5Core)
BuildRequires:  pkgconfig(Qt5DBus)
BuildRequires:  pkgconfig(Qt5Qml)
BuildRequires:  pkgconfig(Qt5Quick)

%{!?qtc_qmake5:%define qtc_qmake5 %qmake5}
%{!?qtc_make:%define qtc_make make}

%description
This package contains utility library for Sailfish harbour apps development

%prep
%setup -q -n %{name}-%{version}

%build
%qtc_qmake5
%qtc_make %{?_smp_mflags}

%install
rm -rf %{buildroot}
%qmake5_install

%files
%defattr(-,root,root,-)
%{_libdir}/libharbour-lib.a
%{_libdir}/pkgconfig/harbour-lib.pc
%{_includedir}/harbour-lib/*.h
