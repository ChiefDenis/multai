Name:           ai-workspace
Version:        0.4.0
Release:        1%{?dist}
Summary:        Resource-conscious multi-provider AI desktop application

License:        GPL-3.0-or-later
URL:            https://github.com/Kilo-Org/ai-workspace
Source0:        %{name}-%{version}.tar.gz

BuildArch:      x86_64

BuildRequires:  cmake >= 3.31
BuildRequires:  gcc-c++
BuildRequires:  qt6-qtbase-devel >= 6.10
BuildRequires:  qt6-qtdeclarative-devel >= 6.10
BuildRequires:  qt6-qttools-devel >= 6.10
BuildRequires:  qt6-qtwebengine-devel >= 6.10
BuildRequires:  pkgconfig(Qt6WebEngineWidgets)

Requires:       qt6-qtbase >= 6.10
Requires:       qt6-qtwebengine >= 6.10
Requires:       hicolor-icon-theme

%description
AI Workspace is a resource-conscious, multi-provider AI desktop application
for Fedora Linux with KDE Plasma. It embeds provider websites in a native
Qt 6 Widgets window with comprehensive browser lifecycle management,
including automatic tab freezing, hibernation, and memory-pressure response.

Supports 9 AI providers: DeepSeek, Qwen, GLM, Microsoft Copilot, Google
Gemini, Anthropic Claude, Perplexity, ChatGPT, and Kimi.

%prep
%autosetup

%build
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=%{_prefix}
cmake --build build --target ai-workspace

%install
DESTDIR=%{buildroot} cmake --install build

# Install desktop file
desktop-file-install --dir=%{buildroot}%{_datadir}/applications \
  %{name}.desktop

# Install icons
for size in 16 24 32 48 64 128 256; do
  install -D -m 0644 icons/hicolor/${size}x${size}/apps/%{name}.png \
    %{buildroot}%{_datadir}/icons/hicolor/${size}x${size}/apps/%{name}.png
done

%post
gtk-update-icon-cache %{_datadir}/icons/hicolor &>/dev/null || :
update-desktop-database %{_datadir}/applications &>/dev/null || :

%postun
gtk-update-icon-cache %{_datadir}/icons/hicolor &>/dev/null || :
update-desktop-database %{_datadir}/applications &>/dev/null || :

%files
%license LICENSE
%doc README.md
%{_bindir}/%{name}
%{_datadir}/applications/%{name}.desktop
%{_datadir}/icons/hicolor/*/apps/%{name}.png

%changelog
* Fri Nov 13 2026 AI Workspace <noreply@localhost> - 0.4.0-1
- Release version 0.4.0
- Persist and restore open tabs, profiles, ordering and selected tab across restarts
- Restore inactive tabs lazily and validate restored provider URLs
- Replace the vertical provider list with a horizontal provider icon strip
- Remove the browser navigation toolbar and obsolete icons

* Fri Nov 06 2026 AI Workspace <noreply@localhost> - 0.3.0-1
- Release version 0.3.0
- Migrate to GPL-3.0-or-later license
- UI refinement: coherent visual design, dark mode, spacing
