%define P_NAME position-finder-server
%define APP_LABEL "Position Finder Server"
%define ORG_PREFIX org.tizen

Name:       %{ORG_PREFIX}.%{P_NAME}
%define alias %{name}
Summary:    IoTivity Application
Group:      Applications/Core Applications
Version:    0.0.1
Release:    1
License:    Flora-1.1
Provides:   %{name} = %{version}-%{release}
Source0:    %{name}-%{version}.tar.gz

BuildRequires:  cmake
BuildRequires:  hash-signer
BuildRequires:  pkgconfig(capi-appfw-application)
BuildRequires:  pkgconfig(dlog)
BuildRequires:  pkgconfig(libtzplatform-config)
BuildRequires:  pkgconfig(capi-appfw-service-application)
BuildRequires:  pkgconfig(capi-system-peripheral-io)
BuildRequires:  pkgconfig(ecore)
BuildRequires:  pkgconfig(eina)
BuildRequires:  pkgconfig(iotcon)
BuildRequires:  pkgconfig(gio-2.0)
BuildRequires:  pkgconfig(libcurl)
BuildRequires:  pkgconfig(glib-2.0)
BuildRequires:  pkgconfig(json-glib-1.0)
BuildRequires:  pkgconfig(capi-system-info)
BuildRequires:  pkgconfig(capi-network-connection)
BuildRequires:  pkgconfig(capi-media-camera)

%description
Server for Position Finder

%prep
%setup -q

%build

%define _pkg_dir %{TZ_SYS_RO_APP}/%{alias}
%define _pkg_shared_dir %{_pkg_dir}/shared
%define _pkg_data_dir %{_pkg_dir}/data
%define _pkg_rw_data_dir /home/owner/apps_rw/%{alias}/data
%define _pkg_res_dir %{_pkg_dir}/res
%define _sys_icons_dir %{_pkg_shared_dir}/res
%define _sys_packages_dir %{TZ_SYS_RO_PACKAGES}
%define _sys_license_dir %{TZ_SYS_SHARE}/license
%define _cbor_file iotcon-test-svr-db-server.dat
%define _conf_file pi.conf

%ifarch %{arm}
export CFLAGS="$CFLAGS -DTIZEN_BUILD_TARGET"
export CXXFLAGS="$CXXFLAGS -DTIZEN_BUILD_TARGET"
export FFLAGS="$FFLAGS -DTIZEN_BUILD_TARGET"
%else
export CFLAGS="$CFLAGS -DTIZEN_BUILD_EMULATOR"
export CXXFLAGS="$CXXFLAGS -DTIZEN_BUILD_EMULATOR"
export FFLAGS="$FFLAGS -DTIZEN_BUILD_EMULATOR"
%endif

cmake . -DP_NAME=%{P_NAME} \
	-DORG_PREFIX=%{ORG_PREFIX} \
	-DAPP_LABEL=%{APP_LABEL} \
	-DINSTALL_PREFIX=%{_pkg_dir} \
	-DINSTALL_OWNER_DATADIR=%{_pkg_rw_data_dir} \
	-DINSTALL_RESDIR=%{_pkg_res_dir} \
	-DSYS_ICONS_DIR=%{_sys_icons_dir} \
	-DSYS_PACKAGES_DIR=%{_sys_packages_dir} \
	-DCBOR_FILE=%{_cbor_file} \
	-DCONF_FILE=%{_conf_file}
make %{?jobs:-j%jobs}

%install
%make_install

%define tizen_sign 1
%define tizen_sign_base %{_pkg_dir}
%define tizen_sign_level platform
%define tizen_author_sign 1
%define tizen_dist_sign 1
#%find_lang position-finder-server

%post
/sbin/ldconfig
chsmack -a "User::Pkg::%{alias}" %{_pkg_res_dir}/*.dat
chmod 444 %{_pkg_res_dir}/*.dat

%postun -p /sbin/ldconfig

%files
%{_pkg_res_dir}/*.dat
%{_pkg_res_dir}/*.conf
%manifest %{alias}.manifest
%defattr(-,root,root,-)
%{_pkg_dir}/bin/%{P_NAME}
%{_sys_packages_dir}/%{alias}.xml
%{_sys_icons_dir}/*.png
%{_pkg_dir}/author-signature.xml
%{_pkg_dir}/signature1.xml
