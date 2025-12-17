#!/usr/bin/env bash
set -euo pipefail

echo "=== Fix GQRX on Ubuntu focal: downgrade GNU Radio 3.10 -> 3.8 (Ubuntu stack) ==="

if [[ ${EUID:-999} -ne 0 ]]; then
  echo "ERROR: run with sudo: sudo $0"
  exit 1
fi

. /etc/os-release
CODENAME="${VERSION_CODENAME:-}"
if [[ "$CODENAME" != "focal" ]]; then
  echo "ERROR: This script is intended for Ubuntu focal. Detected: ${CODENAME:-unknown}"
  exit 1
fi

REAL_USER="${SUDO_USER:-root}"
USER_HOME="$(getent passwd "$REAL_USER" | cut -d: -f6 || true)"

echo "[1/7] Stop running GQRX"
pkill -x gqrx 2>/dev/null || true

echo "[2/7] Backup+reset GQRX user config (safe)"
TS="$(date +%Y%m%d_%H%M%S)"
BK="$USER_HOME/gqrx_backup_$TS"
mkdir -p "$BK"
for p in \
  "$USER_HOME/.config/gqrx" \
  "$USER_HOME/.config/gqrx.conf" \
  "$USER_HOME/.gqrx" \
  "$USER_HOME/.cache/gqrx" \
  "$USER_HOME/.local/share/gqrx"
do
  [[ -e "$p" ]] && mv "$p" "$BK/" || true
done
chown -R "$REAL_USER":"$REAL_USER" "$BK" 2>/dev/null || true
echo "  config backup: $BK"

echo "[3/7] Purge GNU Radio 3.10+ stack (PPA-provided) and related packages"
# This removes the incompatible ABI set that makes gqrx crash on focal.
apt-get purge -y \
  'gnuradio*' \
  'libgnuradio*' \
  'gr-*' \
  'python3-gnuradio*' \
  'soapysdr*' \
  'librtlsdr*' \
  'rtl-sdr' \
  'gqrx*' || true

apt-get autoremove -y || true
apt-get autoclean -y || true

echo "[4/7] Install focal-compatible stack using isolated Ubuntu-only APT (ignores broken repos)"
TMPDIR="$(mktemp -d)"
trap 'rm -rf "$TMPDIR"' EXIT

UBU_MIRROR="http://ro.archive.ubuntu.com/ubuntu"
SEC_MIRROR="http://security.ubuntu.com/ubuntu"

cat > "$TMPDIR/ubuntu.sources.list" <<EOF
deb $UBU_MIRROR $CODENAME main restricted universe multiverse
deb $UBU_MIRROR $CODENAME-updates main restricted universe multiverse
deb $UBU_MIRROR $CODENAME-backports main restricted universe multiverse
deb $SEC_MIRROR $CODENAME-security main restricted universe multiverse
EOF

mkdir -p "$TMPDIR/empty.sourceparts"

APT_OPTS=(
  -o "Dir::Etc::sourcelist=$TMPDIR/ubuntu.sources.list"
  -o "Dir::Etc::sourceparts=$TMPDIR/empty.sourceparts"
  -o "APT::Get::List-Cleanup=0"
)

apt-get update "${APT_OPTS[@]}"

# Focal-native packages (GNU Radio 3.8 series)
apt-get install -y --no-install-recommends "${APT_OPTS[@]}" \
  gnuradio \
  gr-osmosdr \
  rtl-sdr \
  soapysdr-tools \
  soapysdr-module-rtlsdr \
  gqrx-sdr

echo "[5/7] Refresh dynamic linker cache"
ldconfig

echo "[6/7] Verify versions"
if command -v gnuradio-config-info >/dev/null 2>&1; then
  echo -n "  GNU Radio: "
  gnuradio-config-info --version || true
fi

echo "  gqrx path: $(command -v gqrx || true)"

echo "[7/7] Verify gqrx dependencies are satisfied"
MISSING="$(ldd "$(command -v gqrx)" 2>/dev/null | awk '/not found/ {print $1}' || true)"
if [[ -n "$MISSING" ]]; then
  echo "ERROR: still missing shared libraries:"
  echo "$MISSING" | sed 's/^/  /'
  exit 1
fi

echo "=== DONE ==="
echo "Run as normal user (NOT sudo):"
echo "  gqrx"
