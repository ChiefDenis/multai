# AI Workspace

A resource-conscious, multi-provider AI desktop application for Fedora Linux with KDE Plasma.

## Overview

AI Workspace is a native Qt 6 application that embeds multiple AI provider websites in a single desktop window with intelligent browser lifecycle management. The application is designed to minimise resource usage while maintaining responsiveness through automatic tab freezing and hibernation.

### Key Features

- **Multi-provider support**: Access 9 major AI providers from one application
- **Multi-account support**: Separate browser profiles for different accounts per provider
- **Resource-conscious**: Automatic lifecycle management to minimise memory and CPU usage
- **Native desktop**: Built with Qt 6 Widgets for native KDE Plasma integration
- **Profile isolation**: Each account uses an isolated browser profile
- **Persistent sessions**: Authentication and site data persist across application restarts

## Supported Providers

| Provider | URL | Multi-Account Support |
|----------|-----|----------------------|
| DeepSeek | https://www.deepseek.com/ | ✅ |
| Qwen | https://qwen.ai/home | ✅ |
| GLM | https://z.ai | ✅ |
| Microsoft Copilot | https://copilot.microsoft.com/ | ✅ |
| Google Gemini | https://gemini.google.com/ | ✅ |
| Anthropic Claude | https://claude.ai/ | ✅ |
| Perplexity | https://www.perplexity.ai/ | ✅ |
| ChatGPT | https://chatgpt.com/ | ✅ |
| Kimi | https://www.kimi.ai/ | ✅ |

## System Requirements

- **Operating System**: Fedora Linux 43 (or compatible)
- **Desktop Environment**: KDE Plasma (recommended)
- **Qt Version**: 6.10.3 or later
- **RAM**: 4 GB minimum, 8 GB recommended
- **Disk Space**: 500 MB for application, additional space for profiles

## Verified Dependencies

### Runtime Dependencies

- Qt 6.10.3 or later:
  - qt6-qtbase
  - qt6-qtbase-gui
  - qt6-qtwebengine
  - qt6-qtwidgets (implicit via qtbase-gui)

### Build Dependencies

- CMake 3.31 or later
- GCC 15.3.1 or later with C++20 support
- Qt 6 development packages:
  - qt6-qtbase-devel
  - qt6-qtdeclarative-devel
  - qt6-qtwayland-devel
  - qt6-qttools-devel
  - qt6-qtwebengine-devel

### Optional Dependencies

- SQLite 3.50.2 or later (for structured workspace persistence)

## Installation

### From Source

#### Prerequisites

Install the required development packages:

```bash
sudo dnf install cmake gcc-c++ qt6-qtbase-devel qt6-qtdeclarative-devel \
    qt6-qtwayland-devel qt6-qttools-devel qt6-qtwebengine-devel
```

#### Building

```bash
# Clone the repository
git clone <repository-url>
cd ai-workspace

# Configure the build
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release

# Build the application
cmake --build build

# Install (optional)
sudo cmake --install build
```

#### Running

```bash
# From build directory
./build/ai-workspace

# Or if installed
ai-workspace
```

### From RPM Package (Future)

Once packaged, installation will be:

```bash
sudo dnf install ai-workspace
```

## Configuration

### Application Settings

Settings are stored in Qt's standard settings location:

```
~/.config/ai-workspace/ai-workspace.conf
```

### Profile Storage

Browser profiles are stored in:

```
~/.config/ai-workspace/profiles/
```

Each profile is stored in a separate subdirectory with user-only permissions (0700).

### Lifecycle Settings

The following lifecycle settings can be configured in the Settings dialog:

- **Freeze Delay**: Time before inactive tabs are frozen (default: 5 minutes)
- **Hibernate Delay**: Time before frozen tabs are discarded (default: 30 minutes)
- **Memory Pressure Threshold**: System memory level that triggers hibernation
- **User Overrides**: Allow manual state transitions with documented side effects

### Appearance Settings

- **Theme**: Light, Dark, or System default
- **Font Size**: UI font scaling

## Usage

### Creating a New Tab

1. Click the "New Tab" button in the tab bar
2. Select a provider from the dialog
3. Select a profile (or create a new one)
4. Click "Create"

### Managing Profiles

#### Creating a Profile

1. Open Settings → Profiles
2. Select a provider
3. Click "Create Profile"
4. Enter a profile name
5. Click "OK"

#### Deleting a Profile

1. Open Settings → Profiles
2. Select a profile
3. Click "Delete Profile"
4. Confirm deletion

**Warning**: Deleting a profile permanently removes all authentication data, cookies, and site data for that profile.

### Tab Lifecycle

Tabs automatically transition through lifecycle states to conserve resources:

- **Unloaded**: Tab exists but no browser page is created
- **Loading**: Page is being created or loaded
- **Active**: Page is fully loaded and interactive
- **Frozen**: Page is suspended to save CPU (background tabs after 5 minutes)
- **Discarded**: Page is discarded to save memory (after 30 minutes of inactivity)
- **Closing**: Tab is being cleaned up

Tabs are automatically activated when selected, restoring them from Frozen or Discarded states.

### Workspace Persistence

The application automatically saves:

- Tab configuration (provider, profile, URL)
- Tab order and position
- Window geometry and state
- User preferences

These are restored when the application is launched.

## Troubleshooting

### Application Won't Start

**Problem**: Application fails to launch

**Solutions**:
1. Verify Qt 6.10.3 or later is installed: `rpm -qa | grep qt6-qtbase`
2. Verify qt6-qtwebengine is installed: `rpm -qa | grep qt6-qtwebengine`
3. Check for missing libraries: `ldd build/ai-workspace`
4. Run from terminal to see error messages

### Provider Won't Load

**Problem**: A provider's website doesn't load or shows an error

**Solutions**:
1. Check your internet connection
2. Try loading the provider in a regular browser
3. The provider may block embedding (see Limitations)
4. Check if the provider URL has changed (update in settings if needed)

### High Memory Usage

**Problem**: Application uses more memory than expected

**Solutions**:
1. Reduce the number of active tabs
2. Close unused tabs
3. Adjust lifecycle settings to freeze/hibernate sooner
4. Clear browser cache in Settings → Advanced
5. Check for memory leaks using system monitoring tools

### Profile Data Not Persisting

**Problem**: Login sessions are lost after restart

**Solutions**:
1. Check profile directory permissions: `ls -la ~/.config/ai-workspace/profiles/`
2. Ensure directory has user-only permissions (0700)
3. Check disk space: `df -h`
4. Check application logs for errors

### Renderer Crashes

**Problem**: Tab shows "Renderer crashed" error

**Solutions**:
1. Click "Retry" to reload the page
2. If it persists, close and recreate the tab
3. Check system memory: may be out of memory
4. Update Qt WebEngine: `sudo dnf update qt6-qtwebengine`

## Testing

### Running Tests

```bash
# Build tests
cmake --build build --target test

# Run all tests
ctest --test-dir build --output-on-failure

# Run specific test
./build/tests/test_lifecycle
```

### Test Coverage

Unit tests cover:
- Tab state machine transitions
- Lifecycle management logic
- Profile management
- Configuration persistence
- Provider configuration

Integration tests cover:
- Qt WebEngine lifecycle API integration
- Profile isolation
- Memory pressure response
- Renderer crash recovery

### Performance Testing

Performance tests measure:
- Memory usage per tab state
- Tab creation and activation times
- Profile and cache growth
- Multi-tab scenarios

See [docs/TESTING.md](docs/TESTING.md) for detailed testing procedures.

## Performance

### Performance Targets

| Metric | Target |
|--------|--------|
| Baseline memory (no tabs) | < 200 MB |
| Per active tab memory | < 300 MB |
| Per frozen tab memory | < 100 MB |
| Per discarded tab memory | < 10 MB |
| Startup time | < 3 seconds |
| First tab creation | < 2 seconds |
| Subsequent tab creation | < 1 second |
| Tab selection (active) | < 100 ms |
| Tab activation (frozen) | < 500 ms |
| Tab activation (discarded) | < 2 seconds |
| Profile growth per week | < 50 MB |
| Cache growth per week | < 100 MB |

See [docs/PERFORMANCE.md](docs/PERFORMANCE.md) for detailed performance criteria and measurement approach.

## Privacy and Security

### Data Protection

- **No credential logging**: The application never logs passwords, API keys, or tokens
- **No cookie logging**: Cookie contents are never logged
- **No content logging**: Chat messages and personal data are never logged
- **Profile isolation**: Each profile is isolated with user-only permissions
- **No telemetry**: No background network activity or analytics

### Profile Security

- Profile directories have user-only permissions (0700)
- Profile files have user-only permissions (0600)
- No cross-profile data access
- Authentication data persists only in user-controlled profiles

### Network Security

- External URLs are validated before navigation
- Downloads require user confirmation
- No modification of browser security settings
- Provider authentication and bot detection are respected

### Data Storage

All data is stored locally on your system:
- Configuration: `~/.config/ai-workspace/`
- Profiles: `~/.config/ai-workspace/profiles/`
- Database (if used): `~/.config/ai-workspace/ai-workspace.db`

No data is transmitted to external servers except by the provider websites themselves.

## Limitations

### Provider Limitations

Some providers may block embedding through:
- X-Frame-Options headers
- Content Security Policy (CSP)
- User-Agent detection
- Bot detection

The application does not circumvent these restrictions. If a provider blocks embedding, the website may not load correctly. This is documented in the provider compatibility section of the settings.

### Technical Limitations

- **No API client**: The application embeds web interfaces only; it does not implement AI API clients
- **No browser extensions**: Extensions are not supported
- **No DevTools**: DevTools are not exposed to users (available for debugging only)
- **No cross-platform**: Designed for Fedora Linux with KDE Plasma only
- **No mobile support**: Desktop application only

### Resource Limitations

- **Memory limits**: Extremely high tab counts may exceed system memory
- **Profile growth**: Profiles grow over time with typical browser usage
- **Cache limits**: Cache grows over time and must be manually cleaned
- **Renderer processes**: Each profile may spawn renderer processes

## Documentation

- [AGENTS.md](AGENTS.md) - Project rules and working protocol
- [SPEC.md](SPEC.md) - Complete technical specification
- [ARCHITECTURE.md](ARCHITECTURE.md) - System architecture and design
- [AI_PROGRESS.md](AI_PROGRESS.md) - Progress ledger and implementation status
- [docs/TESTING.md](docs/TESTING.md) - Testing strategy and procedures
- [docs/PERFORMANCE.md](docs/PERFORMANCE.md) - Performance criteria and measurement
- [docs/DECISIONS.md](docs/DECISIONS.md) - Technical decisions and rationale

## Development

### Build System

The project uses CMake with the following structure:

```
ai-workspace/
├── CMakeLists.txt
├── src/
│   ├── core/
│   ├── ui/
│   ├── managers/
│   ├── providers/
│   ├── lifecycle/
│   ├── profiles/
│   └── persistence/
└── tests/
    ├── unit/
    └── integration/
```

### Code Style

The project uses the provided `.clang-format` configuration:
- Based on Google C++ style
- C++20 standard
- 2-space indentation
- 100-character column limit
- Left-aligned pointers

### Contributing

This is currently a solo project. Contributions are not accepted at this time.

## License

[To be determined]

## Acknowledgments

- Qt Framework and Qt WebEngine
- Fedora Project
- KDE Plasma Desktop
- All AI providers whose services are embedded

## Support

For issues, questions, or contributions:

1. Check the troubleshooting section in this README
2. Check the documentation in the `docs/` directory
3. Review [AGENTS.md](AGENTS.md) for working protocol
4. Review [AI_PROGRESS.md](AI_PROGRESS.md) for known issues

## Version History

### 0.1.0 (In Development)

- Initial project structure
- Documentation complete
- Awaiting implementation

## Future Plans

- [ ] Stage 1: Project Foundation
- [ ] Stage 2: Provider System
- [ ] Stage 3: Profile Management
- [ ] Stage 4: Basic Browser Integration
- [ ] Stage 5: Lifecycle State Machine
- [ ] Stage 6: Lifecycle Automation
- [ ] Stage 7: Recovery and Error Handling
- [ ] Stage 8: UI Polish
- [ ] Stage 9: Testing and Performance
- [ ] Stage 10: Documentation and Packaging

See [AI_PROGRESS.md](AI_PROGRESS.md) for detailed implementation status.
