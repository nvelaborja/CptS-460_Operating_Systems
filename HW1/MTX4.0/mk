VFD=FDimage

as86 -o ts.o ts.s
bcc  -c -ansi t.c
ld86 -d -o mtx ts.o t.o mtxlib /usr/lib/bcc/libc.a

echo cp mtx to $VFD/boot/
mount -o loop $VFD /mnt

cp mtx /mnt/boot
umount /mnt

echo ready to go?
read dummy

qemu -fda FDimage -nofd-bootchk

