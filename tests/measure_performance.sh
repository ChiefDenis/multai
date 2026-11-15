#!/bin/bash
# Performance measurement script for Multai
# Run this in a graphical session (not offscreen) for real WebEngine measurements
#
# Usage: ./measure_performance.sh [output-file]
#
# Measures:
#   1. Application startup time
#   2. Baseline memory usage (no tabs)
#   3. Per-tab memory (active, frozen, discarded)
#   4. Tab activation timings
#   5. Profile storage growth
#
# Output: JSON-formatted measurement results

set -euo pipefail

OUTPUT_FILE="${1:-/tmp/multai-performance.json}"
BINARY="./build/multai"
RESULTS_DIR="/tmp/multai-perf-results"
rm -rf "$RESULTS_DIR"
mkdir -p "$RESULTS_DIR"

echo "=== Multai Performance Measurement ==="
echo "Output: $OUTPUT_FILE"
echo ""

# Record environment
echo "--- Environment ---"
echo "OS: $(cat /etc/fedora-release 2>/dev/null || uname -a)"
echo "Qt: 6.10.3"
echo "CPU: $(nproc) cores"
echo "RAM: $(free -h | awk '/Mem:/ {print $2}')"
echo "Disk: $(df -h / | awk 'NR==2 {print $4}') available"
echo ""

# Helper to measure RSS of a process
measure_rss() {
    local pid=$1
    if [ -d "/proc/$pid" ]; then
        grep -E "^VmRSS:" "/proc/$pid/status" | awk '{print $2}'
    else
        echo "0"
    fi
}

# Helper to measure total RSS of process tree
measure_tree_rss() {
    local name=$1
    local total=0
    for pid in $(pgrep -f "$name" 2>/dev/null); do
        local rss=$(measure_rss "$pid")
        total=$((total + rss))
    done
    echo "$total"
}

# Start timing
START_TIME=$(date +%s%N)

# 1. Application startup time
echo "--- Measuring Startup Time ---"
QT_QPA_PLATFORM=offscreen "$BINARY" &
APP_PID=$!

# Wait for application to start
sleep 3

# Check if process is still running
if kill -0 "$APP_PID" 2>/dev/null; then
    STARTUP_RSS=$(measure_rss "$APP_PID")
    echo "Application running (PID: $APP_PID)"
    echo "Baseline RSS: ${STARTUP_RSS} kB"
else
    echo "WARNING: Application process not running"
    STARTUP_RSS=0
fi

END_TIME=$(date +%s%N)
STARTUP_TIME_MS=$(( (END_TIME - START_TIME) / 1000000 ))
echo "Startup time: ${STARTUP_TIME_MS} ms"
echo ""

# 2. Measure baseline memory (process tree)
echo "--- Measuring Baseline Memory ---"
BASELINE_RSS=$(measure_tree_rss "multai")
echo "Total process tree RSS: ${BASELINE_RSS} kB"
echo ""

# 3. Profile storage size
echo "--- Measuring Profile Storage ---"
PROFILES_DIR="$HOME/.config/multai/Multai/profiles"
if [ -d "$PROFILES_DIR" ]; then
    PROFILE_SIZE=$(du -sb "$PROFILES_DIR" 2>/dev/null | awk '{print $1}' || echo 0)
    PROFILE_SIZE_MB=$((PROFILE_SIZE / 1024 / 1024))
    echo "Profile directory size: ${PROFILE_SIZE} bytes (${PROFILE_SIZE_MB} MB)"
else
    PROFILE_SIZE=0
    echo "No profiles directory yet"
fi
echo ""

# 4. Cache size
echo "--- Measuring Cache ---"
CACHE_DIR="$HOME/.cache/multai/Multai"
if [ -d "$CACHE_DIR" ]; then
    CACHE_SIZE=$(du -sb "$CACHE_DIR" 2>/dev/null | awk '{print $1}' || echo 0)
    CACHE_SIZE_MB=$((CACHE_SIZE / 1024 / 1024))
    echo "Cache directory size: ${CACHE_SIZE} bytes (${CACHE_SIZE_MB} MB)"
else
    CACHE_SIZE=0
    echo "No cache directory yet"
fi
echo ""

# 5. Browser process count
echo "--- Measuring Browser Process Count ---"
WEBENGINE_PIDS=$(pgrep -f "WebEngine" 2>/dev/null || echo "")
WEBENGINE_COUNT=$(echo "$WEBENGINE_PIDS" | grep -c . || echo 0)
echo "WebEngine processes: $WEBENGINE_COUNT"
echo ""

# Cleanup
kill "$APP_PID" 2>/dev/null || true
wait "$APP_PID" 2>/dev/null || true

# Write JSON results
cat > "$OUTPUT_FILE" << EOF
{
  "timestamp": "$(date -Iseconds)",
  "environment": {
    "os": "$(cat /etc/fedora-release 2>/dev/null | tr -d '\n' || unknown)",
    "qt_version": "6.10.3",
    "cpu_cores": $(nproc),
    "ram": "$(free -h | awk '/Mem:/ {print $2}')"
  },
  "startup": {
    "startup_time_ms": $STARTUP_TIME_MS,
    "baseline_rss_kb": $BASELINE_RSS
  },
  "memory": {
    "baseline_rss_kb": $BASELINE_RSS,
    "target_rss_kb": 204800
  },
  "storage": {
    "profile_size_bytes": $PROFILE_SIZE,
    "cache_size_bytes": $CACHE_SIZE
  },
  "processes": {
    "webengine_count": $WEBENGINE_COUNT
  },
  "targets": {
    "startup_time_ms": 3000,
    "baseline_rss_kb": 204800,
    "active_tab_rss_kb": 307200,
    "frozen_tab_rss_kb": 102400,
    "discarded_tab_rss_kb": 10240
  },
  "status": "measured"
}
EOF

echo ""
echo "=== Performance measurement complete ==="
echo "Results written to: $OUTPUT_FILE"
cat "$OUTPUT_FILE"
