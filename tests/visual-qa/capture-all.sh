#!/usr/bin/env bash
# Capture the twelve visual QA states (six states x light/dark) and build a
# labeled contact sheet. Requires a real Wayland/X11 session: Qt WebEngine
# cannot paint into an offscreen surface.
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "${SCRIPT_DIR}/../.." && pwd)"
BUILD_DIR="${BUILD_DIR:-${REPO_ROOT}/build}"
LAUNCHER="${BUILD_DIR}/tests/visual-qa/state-launcher"
OUT_DIR="${REPO_ROOT}/artifacts/visual-qa"

if [[ ! -x "${LAUNCHER}" ]]; then
  echo "state-launcher not found: ${LAUNCHER}" >&2
  echo "Build the project first: cmake --build build --target state-launcher" >&2
  exit 1
fi

mkdir -p "${OUT_DIR}"
rm -f "${OUT_DIR}"/01-initial-*.png "${OUT_DIR}"/02-active-workspace-*.png \
      "${OUT_DIR}"/03-multiple-tabs-*.png "${OUT_DIR}"/04-sidebar-collapsed-*.png \
      "${OUT_DIR}"/05-empty-workspace-*.png "${OUT_DIR}"/06-account-management-*.png \
      "${OUT_DIR}"/contact-sheet.png

states=(
  "0:01-initial"
  "1:02-active-workspace"
  "2:03-multiple-tabs"
  "3:04-sidebar-collapsed"
  "4:05-empty-workspace"
  "5:06-account-management"
)

for entry in "${states[@]}"; do
  state="${entry%%:*}"
  name="${entry##*:}"
  for theme in light dark; do
    echo "Capturing ${name} (${theme})"
    QT_ASSUME_STDERR_HAS_CONSOLE=1 \
      timeout 180 "${LAUNCHER}" "${state}" "${theme}" "${OUT_DIR}" \
      >"${OUT_DIR}/${name}-${theme}.log" 2>&1 \
      || { echo "Capture failed for ${name} ${theme}; see ${OUT_DIR}/${name}-${theme}.log" >&2; exit 1; }
  done
done

# Account dialog captures are dialog-sized; create centred sheet variants on a
# 1366x768 canvas so the contact sheet stays readable. Raw captures are kept.
SHEET_DIR="${OUT_DIR}/.sheet"
rm -rf "${SHEET_DIR}"
mkdir -p "${SHEET_DIR}"
python3 - "${OUT_DIR}" "${SHEET_DIR}" <<'PY'
import sys
from pathlib import Path
from PIL import Image

out_dir = Path(sys.argv[1])
sheet_dir = Path(sys.argv[2])
canvas_size = (1366, 768)
states = [
    "01-initial",
    "02-active-workspace",
    "03-multiple-tabs",
    "04-sidebar-collapsed",
    "05-empty-workspace",
    "06-account-management",
]

for theme in ("light", "dark"):
    for name in states:
        path = out_dir / f"{name}-{theme}.png"
        if not path.exists():
            continue
        image = Image.open(path).convert("RGB")
        if image.size != canvas_size:
            canvas = Image.new("RGB", canvas_size, "#202020" if theme == "dark" else "#f0f0f0")
            x = (canvas_size[0] - image.width) // 2
            y = (canvas_size[1] - image.height) // 2
            canvas.paste(image, (x, y))
            image = canvas
        image.save(sheet_dir / f"{name}-{theme}.png")
PY

# Build a two-column (light/dark) contact sheet with labels.
if command -v montage >/dev/null 2>&1; then
  args=()
  for entry in "${states[@]}"; do
    name="${entry##*:}"
    args+=(-label "${name} (light)" "${SHEET_DIR}/${name}-light.png")
    args+=(-label "${name} (dark)" "${SHEET_DIR}/${name}-dark.png")
  done
  montage "${args[@]}" \
    -tile 2x6 -geometry 640x360+8+8 -background "#202020" -fill "#f0f0f0" \
    -pointsize 18 "${OUT_DIR}/contact-sheet.png"
  echo "Wrote ${OUT_DIR}/contact-sheet.png"
else
  echo "ImageMagick montage not found; skipping contact sheet" >&2
fi
rm -rf "${SHEET_DIR}"
