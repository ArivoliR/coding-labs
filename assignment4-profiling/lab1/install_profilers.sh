#!/usr/bin/env bash
set -euo pipefail

FLAMEGRAPH_DIR="${HOME}/FlameGraph"
SYSCTL_CONF="/etc/sysctl.d/99-perf-lab.conf"

echo "Updating package database..."
sudo pacman -Syu --needed --noconfirm

echo "Installing compiler and profiling tools..."
sudo pacman -S --needed --noconfirm \
  base-devel \
  binutils \
  git \
  kcachegrind \
  perf \
  perl \
  valgrind

echo "Installing FlameGraph into ${FLAMEGRAPH_DIR}..."
if [ -d "${FLAMEGRAPH_DIR}/.git" ]; then
  git -C "${FLAMEGRAPH_DIR}" pull --ff-only
elif [ -e "${FLAMEGRAPH_DIR}" ]; then
  echo "ERROR: ${FLAMEGRAPH_DIR} already exists but is not a git repository."
  echo "Move it aside and rerun this script, or install FlameGraph there manually."
  exit 1
else
  git clone https://github.com/brendangregg/FlameGraph.git "${FLAMEGRAPH_DIR}"
fi

echo "Configuring perf permissions..."
sudo tee "${SYSCTL_CONF}" >/dev/null <<'EOF'
# Settings for the educational profiling lab.
kernel.perf_event_paranoid = -1
kernel.kptr_restrict = 0
EOF

sudo sysctl -p "${SYSCTL_CONF}" >/dev/null

echo "Checking installed tools..."
gprof --version | head -n 1
perf --version
valgrind --version
command -v kcachegrind
test -x "${FLAMEGRAPH_DIR}/flamegraph.pl"
test -x "${FLAMEGRAPH_DIR}/stackcollapse-perf.pl"

echo "Done."
echo "FlameGraph is available at: ${FLAMEGRAPH_DIR}"
echo "Use it as: ${FLAMEGRAPH_DIR}/stackcollapse-perf.pl perf.script > perf.folded"
echo "Then:      ${FLAMEGRAPH_DIR}/flamegraph.pl perf.folded > flamegraph.svg"
