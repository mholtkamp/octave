#!/usr/bin/env bash
set -euo pipefail
cd /repo

MODE="${1:-serve}"      # serve | build
CLEAN="${CLEAN:-0}"

if [[ "${CLEAN}" == "1" ]]; then
  rm -rf site doxygen_xml Documentation/api Documentation/index.md || true
fi

test -d Documentation || { echo "Missing Documentation/"; exit 1; }
test -f mkdocs.yml || { echo "Missing mkdocs.yml"; exit 1; }
test -f Doxyfile || { echo "Missing Doxyfile (run: doxygen -g Doxyfile)"; exit 1; }

# Homepage
if [[ -f README.md ]]; then
  cp -f README.md Documentation/index.md
fi

# Generate API HTML
doxygen Doxyfile

# Serve / build mkdocs
if [[ "$MODE" == "build" ]]; then
  mkdocs build --strict
  echo "Built site into /repo/site"
else
  mkdocs serve -a 0.0.0.0:8000
fi
