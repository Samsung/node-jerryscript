Name:          node-jerry
Summary:       -
Version:       1.0.0
Release:       1
Group:         System/Servers
License:       MIT
Source:        %{name}-%{version}.tar.gz
Source1001:    x.manifest
BuildRequires: cmake, make, python, ninja, libgcc
BuildRequires: glibc-devel, openssl-devel, libstdc++-devel, zlib-devel
BuildRequires: pkgconfig(dlog), pkgconfig(aul), pkgconfig(capi-appfw-app-common)
BuildRequires: pkgconfig(icu-i18n), pkgconfig(icu-uc)
BuildRequires: pkgconfig(glib-2.0)

%description
Node-JerryScript

%prep
%setup -q

%build

%if "%{build_mode}" == "debug"
%define extra_option "--debug"
%define build_mode_pascal "Debug"
%else
%define build_mode_pascal "Release"
%endif

%define output_path "./out/%{build_mode_pascal}"

gcc --version

rm -rf out

./configure \
  %{?extra_option} \
  --tizen \
  --without-dtrace --without-etw --without-perfctr --without-ssl \
  --without-inspector --without-intl --without-snapshot \
  --dest-cpu arm \
  --with-arm-float-abi=hard --with-arm-fpu=neon \
  --without-bundled-v8 \
  --ninja

ninja -C %{output_path}
strip -v %{output_path}/node

%install
rm -rf %{buildroot}
mkdir -p %{buildroot}%{_bindir}
mkdir -p %{buildroot}%{_libdir}
cp -fr %{output_path}/node %{buildroot}%{_bindir}/node

%clean
rm ./*.list

%post
/sbin/ldconfig

%postun
/sbin/ldconfig

%files
%manifest packaging/x.manifest
%defattr(-,root,root,-)
%if "%{build_shared_lib}" == "true"
%{_libdir}/*
%else
%{_bindir}/*
%endif
