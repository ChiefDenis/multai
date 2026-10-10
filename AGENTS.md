# AI Workspace - Agent Working Protocol

## Project Overview

This is a resource-conscious, multi-provider AI desktop application for Fedora Linux with KDE Plasma. The application embeds provider websites in a native Qt 6 Widgets window with comprehensive browser lifecycle management.

## Technology Stack

- **Language**: C++20
- **GUI Framework**: Qt 6 Widgets
- **Web Engine**: Qt WebEngine and Qt WebEngine Core (Qt 6.10.3)
- **Build System**: CMake 3.31+
- **Testing**: CTest and Qt Test
- **Configuration**: QSettings for simple preferences
- **Persistence**: SQLite (if required for structured workspace persistence)
- **Version Control**: Git

## Environment

- **Platform**: Fedora Linux 43
- **Desktop**: KDE Plasma
- **Qt Version**: 6.10.3 (verified available)
- **CMake**: 3.31.11
- **Compiler**: GCC 15.3.1 with C++20 support
- **SQLite**: 3.50.2

## Available Dependencies

The following Qt 6 packages are installed on the system:
- qt6-qtbase-devel-6.10.3
- qt6-qtdeclarative-devel-6.10.3
- qt6-qtwayland-devel-6.10.3
- qt6-qttools-devel-6.10.3
- qt6-qtwebengine-6.10.3 (runtime, development package available)

**NOTE**: qt6-qtwebengine-devel package is available but not installed. Must be installed before building.

## Working Protocol

### 1. Stage-based Development

1. **Inspection**: Inspect the existing repository and environment
2. **Documentation**: Create or update required documentation
3. **Implementation**: Implement only the stage explicitly authorised by the user
4. **Verification**: Build and test every stage
5. **Diagnosis**: Diagnose failed tests before continuing
6. **Recording**: Record actual results; never invent successful tests
7. **Documentation Update**: Update AI_PROGRESS.md and relevant documentation
8. **Compliance**: Do not silently skip requirements
9. **Scope Control**: Do not expand scope or refactor unrelated working code
10. **Stop and Report**: Stop at the end of each stage and wait for authorisation

### 2. Mandatory Documentation Files

The following files must be created and maintained:

- **AGENTS.md**: This file - project rules and working protocol
- **SPEC.md**: Complete requirements including 9 providers and lifecycle architecture
- **ARCHITECTURE.md**: System design and component relationships
- **AI_PROGRESS.md**: Authoritative progress ledger (MUST be updated after each stage)
- **README.md**: Project overview, dependencies, installation, build, configuration, troubleshooting
- **docs/TESTING.md**: Testing strategy and procedures
- **docs/PERFORMANCE.md**: Performance criteria and measurement approach
- **docs/DECISIONS.md**: Technical decisions and their rationale

### 3. Required Documentation in AI_PROGRESS.md

AI_PROGRESS.md is the authoritative progress ledger. It must record:

- Completed stages
- Actual build/test results (never invented)
- Blockers and resolutions
- Changed files
- Performance measurements
- Next authorised stage

### 4. Build and Test Commands

These commands should be run during development:

```bash
# Install development dependencies (if needed)
sudo dnf install qt6-qtwebengine-devel

# Configure and build
cmake -B build -S . -DCMAKE_BUILD_TYPE=Debug
cmake --build build

# Run tests
ctest --test-dir build --output-on-failure

# Install (optional)
cmake --install build
```

### 5. Code Style

- Use the provided `.clang-format` configuration
- Follow Google C++ style with C++20 standard
- Indent with 2 spaces, no tabs
- Column limit: 100 characters
- Pointer alignment: Left

### 6. Code Conventions

- Do NOT add or remove comments unless asked
- Write compact code – collapse duplicate else branches, avoid unnecessary nesting
- Follow idiomatic C++20 conventions
- Avoid excessive & verbose error handling
- Think about the right error boundaries and follow existing code style

### 7. Dependency Management

- NEVER assume a library is available without verification
- Check existing code patterns before adding dependencies
- Prefer using system packages (dnf) over manual installation
- When adding dependencies, prefer versions published at least 7 days ago
- Avoid floating ranges (latest, *, unbounded >=)

### 8. Security Constraints

- Never circumvent provider authentication, bot detection, or embedding restrictions
- Do not disable browser security to make a provider work
- Do not log credentials, cookies, tokens, or sensitive website contents
- Protect profile data with appropriate user-only filesystem permissions
- Validate external URLs and handle downloads deliberately
- Document provider compatibility limitations

### 9. Resource Constraints

- No Electron or Node.js runtime
- No duplicate browser views for one tab
- No automatic page reload loops
- No unnecessary polling
- No unbounded application logs, histories, queues, retries, or timers
- No hidden duplicate browser instances
- No deletion of persistent authentication data during cache cleanup
- No indiscriminate deletion of IndexedDB, local storage, cookies, or site data
- Cache limits must be verified against actual Qt APIs and behaviour
- Browser profile growth must be measured separately from disposable cache growth
- No unrequested telemetry or background network activity

### 10. Browser Lifecycle Architecture Requirements

Every tab must have an explicit state with valid transitions:

- **Unloaded**: No live page; only metadata and persistent profile remain
- **Loading**: Page creation or navigation is in progress
- **Active**: Interactive page
- **Frozen**: Page retained but background execution suspended
- **Discarded/Hibernated**: Browser context discarded to release resources; restoration reloads page
- **Closing**: Cleanup in progress

Mandatory implementation requirements:
- Lazy page creation
- Automatic freezing of eligible inactive tabs after configurable delay
- Hibernation of eligible inactive tabs after longer configurable delay
- Memory-pressure-triggered hibernation
- Least-recently-used selection among eligible tabs
- Immediate activation when a tab is selected
- Complete cleanup of application-owned page and view objects when a tab closes
- Persistence of tab metadata and browser authentication/site data across application restarts
- Graceful recovery after renderer crashes and failed reloads

Qt WebEngine lifecycle API usage:
- Use QWebEnginePage lifecycleState and recommendedState properties
- Respect recommendedState by default
- Provide user-configurable overrides only when side effects are clearly documented
- Never suspend a visible page
- Never aggressively discard an actively loading page, active stream, or page where activity makes suspension unsafe

### 11. Provider Configuration

Support these nine providers from initial release:

1. **DeepSeek**: https://www.deepseek.com/
2. **Qwen**: https://qwen.ai/home
3. **GLM**: https://z.ai (Zhipu AI platform)
4. **Microsoft Copilot**: https://copilot.microsoft.com/
5. **Google Gemini**: https://gemini.google.com/
6. **Anthropic Claude**: https://claude.ai/
7. **Perplexity**: https://www.perplexity.ai/
8. **ChatGPT**: https://chatgpt.com/
9. **Kimi**: https://www.kimi.ai/

Each provider must have:
- Configurable definition
- Display name
- Icon
- Official URL (verified, not invented)
- Profile assignment

Support multiple accounts for a provider through separate persistent browser profiles.

### 12. UI Direction

Use a Libadwaita-inspired visual language:
- Clean typography
- Rounded surfaces
- Carefully spaced controls
- Subtle borders
- Attractive icons
- Restrained colours
- Polished light/dark themes
- Adaptive layouts

Implement using Qt Widgets. Do NOT introduce GTK/libadwaita, Electron, a JavaScript UI framework, or another GUI framework.

### 13. Verification Before Completion

Before considering a task complete:

- Check for project-specific verification instructions in AGENTS.md or similar files
- Run relevant verification steps based on scope of changes (lint, typecheck, build, tests)
- For isolated functionality, consider a temporary test file to verify behavior, then delete it
- Self-critique: review changes for edge cases and refine as needed
- If verification commands cannot be found, ask the user and suggest saving them to project config

### 14. Saving Learned Information

If useful project information is discovered (build commands, test commands, verification steps, user preferences, etc.) that isn't already documented:

- If AGENTS.md exists, append to it
- Otherwise, create AGENTS.md in the current directory with the learned information

### 15. Error Recovery

When encountering errors (failed commands, build failures, test failures):

- Keep trying different approaches to resolve the issue
- Search for similar issues in the codebase or documentation
- Only ask the user for help as a last resort after exhausting reasonable options
- Exception: Always ask for help with authentication issues, project configuration changes, or permission problems

### 16. Git Usage

Creating commits:
1. Run in parallel: `git status`, `git diff`, `git log` (to match commit style)
2. Draft a concise commit message focusing on "why" not "what". Check for sensitive info.
3. Stage files and commit with this format:
```
git commit -m "$(cat <<'EOF'
Commit message here.

Generated with [Devin](https://devin.ai)

Co-Authored-By: Devin <158243242+devin-ai-integration[bot]@users.noreply.github.com>
EOF
)"
```
4. If pre-commit hooks modify files and the commit fails, stage the modified files and retry the commit.

Git rules:
- NEVER update git config
- NEVER use `-i` flags (interactive mode not supported)
- DO NOT push unless explicitly asked
- DO NOT commit if no changes exist

### 17. Destructive Operations

NEVER perform irreversible destructive operations without explicit user confirmation for that specific action:
- Deleting or truncating database tables, dropping schemas, bulk-deleting rows
- `rm -rf`, deleting directories, or removing files you did not just create
- Force-pushing, rewriting git history, deleting branches, checking out over uncommitted changes, or bypassing commit hooks
- Sending emails, making payments, or calling APIs with real-world side effects

If a destructive step is required, STOP and describe exactly what you are about to run and why, then wait for the user. Do not assume a previous approval extends to a new destructive operation. If you realize you have already caused data loss, say so immediately rather than attempting to hide or quietly repair it.

## Technical Uncertainties

The following areas require investigation and measurement:

1. **Profile Ownership**: How Qt WebEngine handles profile ownership and cleanup
2. **Page Lifecycle**: Exact behaviour of Qt 6.10.3 lifecycleState and recommendedState API
3. **Renderer Cleanup**: Process cleanup when pages are discarded
4. **Disk Growth**: Actual profile growth vs cache growth over time
5. **Memory Pressure**: How to detect and respond to system memory pressure on Linux
6. **Provider Compatibility**: Which providers actively block embedding and how to detect this

## Project Directory Structure

```
ai-workspace/
├── AGENTS.md              # This file
├── SPEC.md                # Complete requirements
├── ARCHITECTURE.md        # System design
├── AI_PROGRESS.md         # Progress ledger (authoritative)
├── README.md              # Project overview
├── CMakeLists.txt         # CMake build configuration
├── .gitignore             # Git ignore rules
├── .clang-format          # Code style configuration
├── src/                   # Source code
│   ├── core/              # Core application logic
│   ├── ui/                # UI components
│   ├── providers/         # Provider definitions
│   ├── lifecycle/         # Browser lifecycle management
│   └── profiles/          # Profile management
├── tests/                 # Test code
│   ├── unit/              # Unit tests
│   └── integration/       # Integration tests
├── docs/                  # Documentation
│   ├── TESTING.md         # Testing strategy
│   ├── PERFORMANCE.md     # Performance criteria
│   └── DECISIONS.md       # Technical decisions
└── profiles/              # Browser profiles (runtime only, gitignored)
```

## Current Status

**Initialisation Phase**: Environment inspected, Qt 6.10.3 verified available, project structure created.

**Next Stage**: Create SPEC.md with complete requirements including 9 providers and browser lifecycle architecture.
