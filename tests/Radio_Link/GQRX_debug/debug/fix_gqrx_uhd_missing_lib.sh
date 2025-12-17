#!/usr/bin/env bash
set -euo pipefail

echo "=== Fix GQRX missing libuhd.so.4.7.0 via Ettus UHD PPA (isolated APT) ==="

if [[ ${EUID:-999} -ne 0 ]]; then
  echo "ERROR: run with sudo: sudo $0"
  exit 1
fi

if ! command -v gqrx >/dev/null 2>&1; then
  echo "ERROR: gqrx not found in PATH"
  exit 1
fi

. /etc/os-release
CODENAME="${VERSION_CODENAME:-}"
if [[ -z "$CODENAME" ]]; then
  echo "ERROR: cannot determine Ubuntu codename from /etc/os-release"
  exit 1
fi

echo "[1/7] Check missing libs for gqrx"
GQRX_BIN="$(command -v gqrx)"
MISSING="$(ldd "$GQRX_BIN" 2>/dev/null | awk '/not found/ {print $1}' || true)"
echo "$MISSING" | sed 's/^/  /' || true

if ! echo "$MISSING" | grep -qx 'libuhd.so.4.7.0'; then
  echo "ERROR: This script targets missing libuhd.so.4.7.0 specifically."
  exit 1
fi

echo "[2/7] Build isolated APT sources: Ubuntu official + Ettus UHD PPA only"
TMPDIR="$(mktemp -d)"
trap 'rm -rf "$TMPDIR"' EXIT

UBU_MIRROR="http://ro.archive.ubuntu.com/ubuntu"
SEC_MIRROR="http://security.ubuntu.com/ubuntu"
ETTUS_PPA="http://ppa.launchpad.net/ettusresearch/uhd/ubuntu"

cat > "$TMPDIR/isolated.sources.list" <<EOF
deb $UBU_MIRROR $CODENAME main restricted universe multiverse
deb $UBU_MIRROR $CODENAME-updates main restricted universe multiverse
deb $UBU_MIRROR $CODENAME-backports main restricted universe multiverse
deb $SEC_MIRROR $CODENAME-security main restricted universe multiverse

# Ettus UHD PPA (already present on your system; we isolate it here)
deb $ETTUS_PPA $CODENAME main
EOF

mkdir -p "$TMPDIR/empty.sourceparts"

APT_OPTS=(
  -o "Dir::Etc::sourcelist=$TMPDIR/isolated.sources.list"
  -o "Dir::Etc::sourceparts=$TMPDIR/empty.sourceparts"
  -o "APT::Get::List-Cleanup=0"
)

echo "[3/7] apt-get update (isolated)"
apt-get update "${APT_OPTS[@]}"

echo "[4/7] Install UHD runtime (try common package names)"
# Try several common package name variants; only one needs to succeed.
set +e
apt-get install -y --no-install-recommends "${APT_OPTS[@]}" uhd-host libuhd4.7.0
RC1=$?
apt-get install -y --no-install-recommends "${APT_OPTS[@]}" uhd-host libuhd4
RC2=$?
apt-get install -y --no-install-recommends "${APT_OPTS[@]}" uhd-host libuhd-dev
RC3=$?
set -e

if [[ $RC1 -ne 0 && $RC2 -ne 0 && $RC3 -ne 0 ]]; then
  echo "ERROR: Failed to install UHD packages from isolated sources."
  echo "Diagnostics (isolated):"
  apt-cache "${APT_OPTS[@]}" policy uhd-host libuhd4.7.0 libuhd4 libuhd-dev | sed -n '1,220p' || true
  exit 1
fi

echo "[5/7] Refresh dynamic linker cache"
ldconfig

echo "[6/7] Verify libuhd.so.4.7.0 resolution"
FOUND="$(ldconfig -p | awk '$1=="libuhd.so.4.7.0"{print $NF}' | head -n 1 || true)"
if [[ -z "$FOUND" ]]; then
  echo "ERROR: libuhd.so.4.7.0 still not in ldconfig cache."
  echo "Search on disk (limited paths):"
  find /usr /lib /opt -type f -name 'libuhd.so.4.7.0' 2>/dev/null | sed 's/^/  /' || true
  echo
  echo "ldconfig -p | grep libuhd:"
  ldconfig -p | grep -E 'libuhd\.so' || true
  exit 1
fi
echo "  OK: $FOUND"

echo "[7/7] Verify gqrx has no missing libs now"
if ldd "$GQRX_BIN" | grep -q 'not found'; then
  echo "ERROR: Still missing libraries:"
  ldd "$GQRX_BIN" | awk '/not found/ {print "  " $0}'
  exit 1
fi

echo "=== DONE ==="
echo "Run: gqrx"
