#!/usr/bin/env bash
set -euo pipefail

echo "=== GQRX clean reinstall using isolated Ubuntu-only APT sources ==="

if [[ ${EUID:-999} -ne 0 ]]; then
  echo "ERROR: run with sudo: sudo $0"
  exit 1
fi

REAL_USER="${SUDO_USER:-root}"
USER_HOME="$(getent passwd "$REAL_USER" | cut -d: -f6 || true)"

. /etc/os-release
CODENAME="${VERSION_CODENAME:-}"
if [[ -z "$CODENAME" ]]; then
  echo "ERROR: cannot determine Ubuntu codename from /etc/os-release"
  exit 1
fi

echo "[0/6] Host"
echo "  user     : $REAL_USER"
echo "  ubuntu   : ${NAME:-?} ${VERSION:-?} (codename: $CODENAME)"

echo "[1/6] Stop running GQRX (exact process name)"
pkill -x gqrx 2>/dev/null || true

echo "[2/6] Remove existing GQRX installs (APT/Snap/Flatpak/manual) + user config"
apt-get purge -y gqrx gqrx-sdr 2>/dev/null || true
apt-get autoremove -y || true
apt-get autoclean -y || true

if command -v snap >/dev/null 2>&1; then
  if snap list 2>/dev/null | awk '{print $1}' | grep -qx "gqrx"; then
    snap remove gqrx || true
  fi
fi

if command -v flatpak >/dev/null 2>&1; then
  flatpak uninstall -y --delete-data org.gqrx.Gqrx 2>/dev/null || true
  flatpak uninstall -y --delete-data io.github.csete.gqrx 2>/dev/null || true
  while read -r appid; do
    [[ -n "$appid" ]] && flatpak uninstall -y --delete-data "$appid" || true
  done < <(flatpak list --app 2>/dev/null | awk '{print $1}' | grep -i gqrx || true)
fi

rm -f /usr/local/bin/gqrx /usr/local/bin/gqrx-sdr 2>/dev/null || true

if [[ -n "${USER_HOME:-}" && -d "$USER_HOME" ]]; then
  rm -rf "$USER_HOME/.config/gqrx" "$USER_HOME/.cache/gqrx" "$USER_HOME/.local/share/gqrx" 2>/dev/null || true
fi

echo "[3/6] Build isolated Ubuntu-only APT sources (ignores your broken repos)"
TMPDIR="$(mktemp -d)"
trap 'rm -rf "$TMPDIR"' EXIT

# Use a reliable primary mirror. If you prefer ro.archive.ubuntu.com, change below.
UBU_MIRROR="http://archive.ubuntu.com/ubuntu"
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

echo "[4/6] apt-get update (isolated)"
apt-get update "${APT_OPTS[@]}"

echo "[5/6] Install GQRX + SDR dependencies (isolated APT)"
apt-get install -y "${APT_OPTS[@]}" \
  gqrx-sdr \
  rtl-sdr \
  soapysdr-tools \
  soapysdr-module-rtlsdr \
  soapysdr-module-hackrf \
  soapysdr-module-airspy

echo "[6/6] Verify install"
echo "  dpkg -l gqrx-sdr:"
dpkg -l | awk '/^ii  gqrx-sdr\b/ {print "  " $2 " " $3}' || true
echo "  which gqrx:"
command -v gqrx || true

echo "=== DONE ==="
echo "Run: gqrx"
