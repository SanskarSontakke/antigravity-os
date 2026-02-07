#!/bin/bash
echo "[Disk] Creating 128MB Disk..."
dd if=/dev/zero of=disk.img bs=1M count=128 status=none

echo "[Disk] Formatting as Ext4..."
/sbin/mkfs.ext4 -F -q disk.img

echo "[Disk] Injecting Files..."
# We use debugfs to write without mounting (no sudo needed)
/sbin/debugfs -w disk.img <<EOF
write /etc/hostname host.txt
mkdir my_folder
cd my_folder
dump_extents
quit
EOF

echo "[Disk] Done. Files injected: host.txt, my_folder"
