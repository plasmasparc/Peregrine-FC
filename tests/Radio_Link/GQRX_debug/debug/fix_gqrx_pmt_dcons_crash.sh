#!/usr/bin/env bash
set -euo pipefail

echo "=== Fix GQRX crash: pmt_dcons: not a dict ==="

if [[ ${EUID:-999} -ne 0 ]]; then
  echo "ERROR: run with sudo: sudo $0"
  exit 1
fi

REAL_USER="${SUDO_USER:-root}"
USER_HOME="$(getent passwd "$REAL_USER" | cut -d: -f6 || true)"

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

echo "[0/6] Context"
echo "  user      : $REAL_USER"
echo "  home      : ${USER_HOME:-unknown}"
echo "  gqrx bin   : $(command -v gqrx)"

echo "[1/6] Stop running GQRX"
pkill -x gqrx 2>/dev/null || true

echo "[2/6] Hard-reset GQRX user config (this often fixes pmt_dcons crashes)"
if [[ -z "${USER_HOME:-}" || ! -d "$USER_HOME" ]]; then
  echo "ERROR: cannot determine user home directory"
  exit 1
fi

TS="$(date +%Y%m%d_%H%M%S)"
BK="$USER_HOME/gqrx_backup_$TS"
mkdir -p "$BK"

# Backup then remove all likely GQRX config locations (dir + file variants)
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

chown -R "$REAL_USER":"$REAL_USER" "$BK" || true

echo "[3/6] Detect GNU Radio version"
GR_VER="unknown"
if command -v gnuradio-config-info >/dev/null 2>&1; then
  GR_VER="$(gnuradio-config-info --version 2>/dev/null || echo unknown)"
fi
echo "  gnuradio version: $GR_VER"

echo "[4/6] If GNU Radio is not 3.8.x (Ubuntu focal default), standardize stack (isolated Ubuntu-only APT)"
NEED_STANDARDIZE=0
if [[ "$CODENAME" == "focal" ]]; then
  # GQRX in focal is built for GNU Radio 3.8.x. If you installed GNU Radio 3.9/3.10 from PPA, GQRX may crash.
  if [[ "$GR_VER" != 3.8* ]]; then
    NEED_STANDARDIZE=1
  fi
fi

if [[ $NEED_STANDARDIZE -eq 1 ]]; then
  echo "  Detected mismatch for focal: GNU Radio is '$GR_VER' (expected 3.8.x)."
  echo "  Standardizing to Ubuntu focal packages (isolated APT; ignores broken third-party repos)."

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

  # Force consistent versions from Ubuntu repos; allow downgrades if PPA versions are installed.
  apt-get install -y --no-install-recommends --allow-downgrades "${APT_OPTS[@]}" \
    gqrx-sdr \
    gnuradio \
    python3-gnuradio \
    gr-osmosdr

  apt-get install -y --reinstall --no-install-recommends --allow-downgrades "${APT_OPTS[@]}" \
    gqrx-sdr

  ldconfig
else
  echo "  No forced standardization performed."
fi

echo "[5/6] Verify: check missing shared libraries"
echo "  ldd(gqrx) missing libs:"
ldd "$(command -v gqrx)" | awk '/not found/ {print "  " $0}' || true

echo "[6/6] Done"
echo "Backup of removed configs is here:"
echo "  $BK"
echo "Now run as normal user (NOT sudo):"
echo "  gqrx"
