#!/usr/bin/env bash

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
SOURCE_DIR="$ROOT_DIR/vtol_pedal_refactored"
TARGET_DIRS=(
  "$ROOT_DIR/vtol_pedal_esp32c3"
  "$ROOT_DIR/vtol_pedal_rp2040"
)
FILES=(
  "SerialCommand.h"
  "SerialCommand.cpp"
  "SerialCommandSharedImpl.h"
)

for target_dir in "${TARGET_DIRS[@]}"; do
  for file in "${FILES[@]}"; do
    cp "$SOURCE_DIR/$file" "$target_dir/$file"
  done
done

echo "Synchronized SerialCommand files from $SOURCE_DIR"
