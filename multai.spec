Name:           multai
Version:        0.5.0
Release:        1%{?dist}
Summary:        Resource-conscious multi-provider AI desktop application

License:        GPL-3.0-or-later
URL:            https://github.com/ChiefDenis/
Source0:        %{name}-%{version}.tar.gz

BuildArch:      x86_64

Obsoletes:      ai-workspace < 0.5.0
Provides:       ai-workspace = %{version}-%{release}

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
Requires:       desktop-file-utils

%description
Multai is a resource-conscious, multi-provider AI desktop application
for Fedora Linux with KDE Plasma. It embeds provider websites in a native
Qt 6 Widgets window with comprehensive browser lifecycle management,
including automatic tab freezing, hibernation, and memory-pressure response.

Supports 9 AI providers: DeepSeek, Qwen, GLM, Microsoft Copilot, Google
Gemini, Anthropic Claude, Perplexity, ChatGPT, and Kimi.

%prep
%autosetup

%build
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=%{_prefix} -DBUILD_TESTS=OFF
cmake --build build --target multai

%install
DESTDIR=%{buildroot} cmake --install build

%files
%license LICENSE
%doc README.md
%{_bindir}/%{name}
%{_datadir}/applications/%{name}.desktop
%{_datadir}/icons/hicolor/*/apps/multai.png
%{_datadir}/icons/hicolor/scalable/apps/multai.svg

%changelog
* Sun Nov 15 2026 Multai <noreply@localhost> - 0.5.0-1
- Rebrand the application from AI Workspace to Multai
- Rename the executable, package, desktop entry, icon and Qt identity to multai
- Move the existing icon artwork to the `multai` icon name; only the identifiers
  change, the artwork itself is unchanged
- Migrate existing QSettings, profiles, WebEngine storage and cache from the legacy
  ai-workspace locations without overwriting existing Multai data
- Keep the legacy data in place for rollback; migration is one-time and idempotent
- Preserve the hyphen-free KDE icon name so Kora cannot redirect the lookup
- Bump version to 0.5.0

* Sat Nov 14 2026 AI Workspace <noreply@localhost> - 0.4.5-1
- Fix KDE icon resolution root cause: KIconLoader strips hyphen-separated suffixes
  inside each theme before consulting hicolor. With the active Kora theme,
  "ai-workspace" resolved to the "ai" Adobe Illustrator mimetype icon instead of
  the installed hicolor application icon, regardless of cache or packaging state.
- Rename the canonical icon to the hyphen-free "ai_workspace" in the desktop entry,
  hicolor installation and Qt resource.
- Install only the canonical icon name; old "ai-workspace" icon aliases are removed.
- Remove redundant icon-cache/desktop-database scriptlets; hicolor-icon-theme and
  desktop-file-utils provide the Fedora transfiletriggers.
- Add regression tests for the hyphen-free icon name, artwork, Qt window icon,
  desktop-entry identity and installed KDE icon lookup.
- Bump version to 0.4.5

* Sat Nov 14 2026 AI Workspace <noreply@localhost> - 0.4.4-1
- Attempted icon fix via hicolor-icon-theme dependency and forced icon-cache updates
- This did not address the actual KIconLoader hyphen-fallback collision and was superseded by 0.4.5

* Sat Nov 14 2026 AI Workspace <noreply@localhost> - 0.4.3-1
- Release version 0.4.3
- Fix duplicate Settings/Expand controls in collapsed sidebar
- Fix RPM build pipeline to ensure clean build
- Fix KDE application icon installation and desktop identity
- Update collapsed sidebar to use single bottom control section
- Bump version to 0.4.3

* Fri Nov 13 2026 AI Workspace <noreply@localhost> - 0.4.2-1
- Release version 0.4.2
- Authoritative version source via generated version.h
- Dynamic collapsed sidebar showing open tabs
- Settings/Expand stacked in fixed bottom section
- Developer link updated to https://github.com/ChiefDenis/

* Fri Nov 13 2026 AI Workspace <noreply@localhost> - 0.4.1-1
- Release version 0.4.1
- Remove sidebar branding header (app icon and name)
- Fix tooltip contrast in both light and dark themes
- Audit and improve settings screen contrast across all states
- Fix application icon rendering in window and KDE taskbar
- Add regression tests for sidebar, tooltips, settings contrast, and application icon

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
