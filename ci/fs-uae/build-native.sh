#!/usr/bin/env bash
set -euo pipefail
IMAGE="${AMBOT_BEBBO_IMAGE:-amigadev/m68k-amigaos-gcc@sha256:b18080e6ffca8f793e0f539536a9138e9d2a548ca1a301c7483f43ee15fedfed}"
OUT_DIR="${1:-build/fs-uae/native}"
mkdir -p "$OUT_DIR"
docker pull "$IMAGE"
docker image inspect "$IMAGE" --format '{{join .RepoDigests "\n"}}' | tee "$OUT_DIR/toolchain-image.txt"
docker run --rm -v "$PWD:/work" -w /work "$IMAGE" m68k-amigaos-gcc \
  -Iinclude -Os -Wall -Wextra -Werror -m68000 -mcrt=nix20 \
  -o AmBot \
  src/main.c src/net.c src/irc.c src/events.c src/commands.c src/operations.c \
  src/rexx.c src/hooks.c src/config.c src/modules.c src/networks.c src/multinet.c src/session.c
cp AmBot "$OUT_DIR/AmBot"
file "$OUT_DIR/AmBot" | tee "$OUT_DIR/file.txt"
sha256sum "$OUT_DIR/AmBot" | tee "$OUT_DIR/AmBot.sha256"
if ! grep -Eiq 'AmigaOS|Amiga.*executable|loadseg' "$OUT_DIR/file.txt"; then
  echo 'ERROR: native output is not recognized as an Amiga executable' >&2
  exit 1
fi
printf 'STATUS=PASS\nGATE=M9_1_NATIVE_BEBBO_BUILD\nIMAGE=%s\nBINARY=%s\n' "$IMAGE" "$OUT_DIR/AmBot" | tee "$OUT_DIR/result.txt"
