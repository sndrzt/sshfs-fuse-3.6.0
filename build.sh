#!/usr/bin/env bash

sudo apt-get build-dep -y sshfs
sudo apt-get install -y libfuse3-dev
sudo chmod 644 /usr/lib/libusb-1.0.so
pushd sshfs-fuse-3.6.0+repack+really2.10
dh_clean
DEB_BUILD_OPTIONS=nocheck dpkg-buildpackage -b -uc -us
popd

