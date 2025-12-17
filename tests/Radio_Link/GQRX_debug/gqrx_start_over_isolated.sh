#!/usr/bin/env bash
set -euo pipefail

echo "=== GQRX START-OVER: purge SDR stack + reinstall isolated (Ubuntu) ==="

if [[ ${EUID:-999} -ne 0 ]]; then
  echo "ERROR: run with sudo: sudo $0"
  exit 1
fi

REAL_USER="${SUDO_USER:-root}"
USER_HOME="$(getent passwd "$REAL_USER" | cut -d: -f6 || true)"

if [[ -z "${USER_HOME:-}" || ! -d "$USER_HOME" ]]; then
  echo "ERROR: cannot determine user HOME for $REAL_USER"
  exit 1
fi

. /etc/os-release
CODENAME="${VERSION_CODENAME:-}"
if [[ -z "$CODENAME" ]]; then
  echo "ERROR: cannot determine Ubuntu codename from /etc/os-release"
  exit 1
fi

echo "[0/8] Host"
echo "  user     : $REAL_USER"
echo "  home     : $USER_HOME"
echo "  ubuntu   : ${NAME:-?} ${VERSION:-?} (codename: $CODENAME)"

echo "[1/8] Stop running processes (best-effort)"
pkill -x gqrx 2>/dev/null || true

echo "[2/8] Backup + remove GQRX user configs (clean state)"
TS="$(date +%Y%m%d_%H%M%S)"
BK="$USER_HOME/gqrx_purge_backup_$TS"
mkdir -p "$BK"

for p in \
  "$USER_HOME/.config/gqrx" \
  "$USER_HOME/.config/gqrx.conf" \
  "$USER_HOME/.gqrx" \
  "$USER_HOME/.cache/gqrx" \
  "$USER_HOME/.local/share/gqrx"
do
  if [[ -e "$p" ]]; then
    echo "  backup+remove: $p"
    mv "$p" "$BK/" || true
  fi
done
chown -R "$REAL_USER":"$REAL_USER" "$BK" 2>/dev/null || true
echo "  backup dir: $BK"

echo "[3/8] Purge GQRX + SDR dependency stack (APT)"
# Keep the purge scoped to SDR/GQRX-related packages only.
apt-get purge -y \
  gqrx gqrx-sdr \
  gnuradio \
  gr-osmosdr \
  soapysdr-tools \
  'soapysdr-module-*' \
  rtl-sdr 'librtlsdr*' \
  uhd-host 'libuhd*' \
  'libgnuradio*' \
  'gr-*' || true

apt-get autoremove -y || true
apt-get autoclean -y || true

echo "[4/8] Remove common manual-install leftovers that override distro libs (best-effort)"
# These paths are frequent sources of ABI mismatches.
rm -f /usr/local/bin/gqrx /usr/local/bin/gqrx-sdr 2>/dev/null || true
rm -f /usr/local/bin/gnuradio-config-info 2>/dev/null || true

# Remove locally-installed shared libs ONLY for SDR stack, if present.
# (Do not touch unrelated /usr/local content.)
find /usr/local/lib /usr/local/lib64 -maxdepth 2 -type f \( \
  -name 'libgnuradio*.so*' -o \
  -name 'libuhd*.so*'      -o \
  -name 'libosmosdr*.so*'  -o \
  -name 'libSoapySDR*.so*' -o \
  -name 'librtlsdr*.so*'   \
\) -print -delete 2>/dev/null || true

ldconfig

echo "[5/8] Build isolated Ubuntu-only APT sources (ignores ALL your third-party repos)"
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

echo "[6/8] apt-get update (isolated)"
apt-get update "${APT_OPTS[@]}"

echo "[7/8] Install clean, consistent GQRX stack (isolated)"
# This is the “known-good” set for Ubuntu repos.
apt-get install -y --no-install-recommends "${APT_OPTS[@]}" \
  gqrx-sdr \
  gnuradio \
  gr-osmosdr \
  rtl-sdr \
  soapysdr-tools \
  soapysdr-module-rtlsdr

ldconfig

echo "[8/8] Verify"
echo "  which gqrx:"
command -v gqrx || true

echo "  GNU Radio version:"
if command -v gnuradio-config-info >/dev/null 2>&1; then
  gnuradio-config-info --version || true
else
  echo "  (gnuradio-config-info not found)"
fi

echo "  ldd(gqrx) missing libs:"
ldd "$(command -v gqrx)" | awk '/not found/ {print "  " $0}' || true

# Hard fail if missing libs remain
if ldd "$(command -v gqrx)" | grep -q 'not found'; then
  echo "ERROR: gqrx still has unresolved shared libraries."
  exit 1
fi

echo "=== DONE ==="
echo "Now run as normal user (NOT sudo):"
echo "  gqrx"
