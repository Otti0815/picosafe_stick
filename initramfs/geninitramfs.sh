#!/bin/bash

[ $1 ] && PEMFILE="`realpath $1`"

OUTPUTDIR="output"                          # output directory
BUSYBOXDIR="busybox"                        # path to busybox
PACKAGES="packages"                         # path to packages
KERNELMODULES=( g_multi.ko )                # list of kernel modules to install
KERNELSRC="../kernel/linux-3.3.0-lpc313x/"  # path to kernel sources
ROOTDIR="initramfs-root/"                   # directory with files to copy into initramfs
DIRECTORIES=( bin lib dev etc mnt root proc root sbin sys usr/bin usr/sbin var/www/cgi-bin tmp )
IP_PREFIX="172.24.42"

cd "`dirname $0`"
WORKDIR="$PWD"

. "../common/common.sh"

check_root
set_crosscompiler

# delete old output directory
if [ -d "$OUTPUTDIR" ];
then
  echo "Deleting old initramfs directory..."
  rm -rf "$OUTPUTDIR"
fi

# create directory structure
echo "Creating directory structure..."
for i in $(seq 0 $((${#DIRECTORIES[@]} - 1)));
do
  directory=${DIRECTORIES[$i]}
  mkdir -p "$OUTPUTDIR/$directory"
done

# polulate dev/
echo "Creating device nodes..."
mknod -m 666 "$OUTPUTDIR/dev/null"      c 1   3
mknod -m 622 "$OUTPUTDIR/dev/console"   c 5   1
mknod -m 666 "$OUTPUTDIR/dev/zero"      c 1   5
mknod -m 666 "$OUTPUTDIR/dev/ptmx"      c 5   2
mknod -m 666 "$OUTPUTDIR/dev/tty"       c 5   0
mknod -m 444 "$OUTPUTDIR/dev/random"    c 1   8
mknod -m 444 "$OUTPUTDIR/dev/urandom"   c 1   9
mknod -m 444 "$OUTPUTDIR/dev/mmcblk0"   b 179 0
mknod -m 444 "$OUTPUTDIR/dev/mmcblk0p1" b 179 1
mknod -m 444 "$OUTPUTDIR/dev/mmcblk0p2" b 179 2
mknod -m 444 "$OUTPUTDIR/dev/mmcblk0p3" b 179 3
mknod -m 444 "$OUTPUTDIR/dev/mmcblk0p4" b 179 4
mknod -m 444 "$OUTPUTDIR/dev/loop0"     b 7   0
mknod -m 444 "$OUTPUTDIR/dev/loop1"     b 7   1
mknod -m 444 "$OUTPUTDIR/dev/loop2"     b 7   2
mknod -m 444 "$OUTPUTDIR/dev/loop3"     b 7   3
mknod -m 444 "$OUTPUTDIR/dev/loop4"     b 7   4
mknod -m 444 "$OUTPUTDIR/dev/loop5"     b 7   5
mknod -m 444 "$OUTPUTDIR/dev/loop6"     b 7   6
mknod -m 444 "$OUTPUTDIR/dev/loop7"     b 7   7
mknod -m 444 "$OUTPUTDIR/dev/hwrandom"  c 10  183

# compile busybox if needed
if [ ! -f "$BUSYBOXDIR/busybox" ];
then
  echo "Compiling busybox..."
  cd "$BUSYBOXDIR"
  make
  cd -
else
  echo "Compiling busybox not needed (skipped)."
fi

# create symlinks
echo "Creating symlinks..."
ln -s /bin/busybox     "$OUTPUTDIR/bin/sh"
ln -s /bin/busybox     "$OUTPUTDIR/bin/ash"
ln -s /bin/busybox     "$OUTPUTDIR/bin/less"
ln -s /bin/busybox     "$OUTPUTDIR/bin/mount"
ln -s /bin/busybox     "$OUTPUTDIR/bin/grep"
ln -s /bin/busybox     "$OUTPUTDIR/bin/ls"
ln -s /usr/bin/lua5.1  "$OUTPUTDIR/usr/bin/lua"
ln -s /usr/bin/luac5.1 "$OUTPUTDIR/usr/bin/luac"

# copy busybox binary to bin/
echo "Copying busybox to initramfs..."
cp "$BUSYBOXDIR/busybox" "$OUTPUTDIR/bin"

# extracting packages
echo "Installing packages:"
tempdir="`mktemp -d`"
for file in $PACKAGES/*deb;
do
  echo "    `basename $file`..."
  (
    cd "$tempdir"
    ar x "$WORKDIR/$file"
    tar -xzf data.tar.gz -C "$WORKDIR/$OUTPUTDIR"
  )
done
rm -rf "$tempdir"

# install kernel modules; first install every kernel module and then remove all
# kernel modules that are not needed
echo "Installing kernel modules..."
export INSTALL_MOD_PATH="$WORKDIR/output"
( cd $KERNELSRC && make modules && make modules_install)
for module in `find output/lib/modules/ -name *.ko`;
do
  delete=1
  for i in $(seq 0 $((${#KERNELMODULES[@]} - 1)));
  do
    test "`basename $module`" "==" "${KERNELMODULES[$i]}" && delete=0
  done

  test $delete == "1" && rm -f "$module"
done

echo "Creating welcome.img..."
dd if=/dev/zero of="$WORKDIR/welcome.img" bs=1M count="`du -sm welcome/ | awk  '{print $1+1}'`"
mkfs.vfat "$WORKDIR/welcome.img" -n picosafe
mount -o loop "$WORKDIR/welcome.img" /mnt
cp -r welcome/* /mnt
rm -Rf /mnt/welcome/.svn
rm -Rf /mnt/welcome/data/.svn
if [ -e "welcome/start.html" ]; then
  sed "s/###IP_PREFIX###/${IP_PREFIX}/g" welcome/start.html > /mnt/start.html
fi
umount /mnt
cp welcome.img $OUTPUTDIR

# copy files into initramfs and ignore stupid .svn files
echo "Copying files into initramfs..."
(
  cd "$ROOTDIR"

  for file in `find . -type f | grep -v "\.svn"`;
  do
    echo "    $file..."
    mkdir -p "$WORKDIR/$OUTPUTDIR/`dirname $file`"
    cp -a "$file" "$WORKDIR/$OUTPUTDIR/`dirname $file`"
  done

  for file in `find . -type l | grep -v "\.svn"`;
  do
    echo "    $file..."
    cp -a "$file" "$WORKDIR/$OUTPUTDIR/`dirname $file`"
  done
)

echo "Adjusting /etc/udhcpd.conf, /init"
sed "s/###IP_PREFIX###/${IP_PREFIX}/g" "$ROOTDIR/etc/udhcpd.conf" > "$OUTPUTDIR/etc/udhcpd.conf"
sed "s/###IP_PREFIX###/${IP_PREFIX}/g" "$ROOTDIR/init" > "$OUTPUTDIR/init"

if [ "$PEMFILE" == "" ]; then
  echo "Generating PEM file..."
  createpem "$OUTPUTDIR/etc/hiawatha/ssl.pem"
else
  echo "Copying pem file..."
  cp "$PEMFILE" "$OUTPUTDIR/etc/hiawatha/ssl.pem"
fi

chown -R 0:0 "$OUTPUTDIR"

echo "Done."
