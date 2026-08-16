Name:       harbour-sailreel

Summary:    Video and music streaming and downloading
Version:    0.5
Release:    1
License:    LICENSE
URL:        https://github.com/shano/harbour-sailpipe
Source0:    %{name}-%{version}.tar.bz2
Requires:   sailfishsilica-qt5 >= 0.10.9
BuildRequires:  pkgconfig(sailfishapp) >= 1.0.2
BuildRequires:  pkgconfig(Qt5Core)
BuildRequires:  pkgconfig(Qt5Qml)
BuildRequires:  pkgconfig(Qt5Quick)
BuildRequires:  pkgconfig(sailfishsilica)
BuildRequires:  pkgconfig(nemotransferengine-qt5)
BuildRequires:  pkgconfig(ambermpris)
BuildRequires:  pkgconfig(Qt5Test)
BuildRequires:  desktop-file-utils
BuildRequires:  cmake

%description
Provides a Sailfish user interface for streaming and downloading video and
music from multiple online services. YouTube is handled via yt-dlp;
SoundCloud, Media.ccc.de, PeerTube and Bandcamp are handled via the NewPipe
Extractor.

%global _missing_build_ids_terminate_build 0
%define __requires_exclude ^lib/appwrapper.*$

%prep
%setup -q -n %{name}-%{version}

%build

%cmake

%make_build


%install
%make_install


desktop-file-install --delete-original       \
  --dir %{buildroot}%{_datadir}/applications             \
   %{buildroot}%{_datadir}/applications/*.desktop

%files
%defattr(-,root,root,-)
%{_bindir}/%{name}
%{_datadir}/%{name}
%{_datadir}/%{name}/lib/appwrapper.so
%{_datadir}/applications/%{name}.desktop
%{_datadir}/icons/hicolor/*/apps/%{name}.png
%{_sysconfdir}/sailjail/permissions/harbour-sailreel.profile
