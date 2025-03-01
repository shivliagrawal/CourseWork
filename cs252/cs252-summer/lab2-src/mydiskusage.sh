#!/bin/bash

#Do not insert code here

#DO NOT REMOVE THE FOLLOWING TWO LINES
git add $0 >> .local.git.out
git commit -a -m "Lab 2 commit" >> .local.git.out
git push >> .local.git.out || echo



bytes_to_mb() {
  echo "$(( $1 / 1024 / 1024 ))"
}

USER=$(whoami)

HOME_DIR="/homes/$USER"

DIR=${1:-$HOME_DIR}

DISK_QUOTA=$((5 * 1024 * 1024 * 1024))

TOTAL_USED=$(du -sb $HOME_DIR | cut -f1)

PERCENTAGE_USED=$((100 * $TOTAL_USED / $DISK_QUOTA))

TOTAL_USED_MB=$(bytes_to_mb $TOTAL_USED)

echo "User:                 $USER"
echo "Disk Quota:           5 GB"
echo "Percentage Used:      $PERCENTAGE_USED%"
echo "Total Disk Used       ${TOTAL_USED_MB} MB"

echo
du -sh $DIR/* | while read SIZE NAME; do
  SIZE_BYTES=$(du -sb $NAME | cut -f1)
  SIZE_MB=$(bytes_to_mb $SIZE_BYTES)
  PERCENTAGE=$((100 * $SIZE_BYTES / $DISK_QUOTA))
  echo "$NAME    ${SIZE_MB} MB    ${PERCENTAGE}%"
done
