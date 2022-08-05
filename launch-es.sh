#!/bin/bash
# Created by Sam Gleske
# DESCRIPTION
#     This is a launcher script designed to run from Linux.  It checks for
#     updates to Sam Gleske's fork with plugins and if an update is required,
#     then automatically fetches the latest version.  This is useful for steam
#     deck.
#
# USAGE:
#     Launch the game normally from steam launch options.
#
#         launch-es.sh %command%
#
#     Force checking for updates every time.
#
#         launch-es.sh forceupdate %command%

set -eo pipefail

APPIMAGE_URL="https://github.com/samrocketman/endless-sky/releases/download/continuous-with-plugins/endless-sky-x86_64-continuous-with-plugins.AppImage"
CHECKSUM_URL="${APPIMAGE_URL}.sha256sum"

# persist timestamp for successful update check
function save_last_update_timestamp() {
  date +%s > ~/.cache/endless-sky/lastUpdate
}

# returns true if an update check is required.  Returns false if not elapsed.
function should_check_for_update() {
  if [ ! -f ~/.cache/endless-sky/lastUpdate ]; then
    return 0
  fi
  local last_update="$(< ~/.cache/endless-sky/lastUpdate)"
  local now_timestamp="$(date +%s)"
  # 3600*12 is 12 hrs
  local twelve_hours=43200
  [ "$(( now_timestamp-last_update ))" -gt "${twelve_hours}" ]
}

# Compare currently downloaded game with a remote checksum and download update
# if mismatched.
function update_game() (
  if ! should_check_for_update; then
    return 0
  fi
  if ! timeout 5 nc -vz github.com 443 &> /dev/null; then
    # no internet so continue with playing the game
    return 0
  fi
  echo 'Check for updates' >&2
  if ! curl -sSfLO "${CHECKSUM_URL}"; then
    return 1
  fi
  cd ~/.cache/endless-sky
  if sha256sum -c - < "${TMP_DIR%/}/${CHECKSUM_URL##*/}"; then
    save_last_update_timestamp
    return 0
  fi
  echo 'Downloading new update' >&2
  cd "${TMP_DIR}"
  if ! curl -sSfLO "${APPIMAGE_URL}"; then
    return 1
  fi
  if sha256sum -c - < "${CHECKSUM_URL##*/}"; then
    chmod 755 "${APPIMAGE_URL##*/}"
    mv -f "${APPIMAGE_URL##*/}" ~/.cache/endless-sky/
    save_last_update_timestamp
    return 0
  else
    return 1
  fi
)

#
# MAIN program
#

if [ ! -d ~/.cache/endless-sky ]; then
  mkdir -p ~/.cache/endless-sky
fi

TMP_DIR="$(mktemp -d)"
trap 'cd;rm -rf "${TMP_DIR}"' EXIT


if [ "$1" = forceupdate ]; then
  rm -f ~/.cache/endless-sky/lastUpdate
fi

cd "${TMP_DIR}"
tries=1
until update_game; do
  if [ "${tries}" -gt 3 ]; then
    break
  else
    (( tries=tries+1 ))
  fi
  sleep 3
done
EXE_FILE=~/.cache/endless-sky/"${APPIMAGE_URL##*/}"
if [ ! -x "${EXE_FILE}" ]; then
  echo 'No executable found,  Check your internet connection and try again.' 2>&1
  exit 1
else
  exec "${EXE_FILE}"
fi
