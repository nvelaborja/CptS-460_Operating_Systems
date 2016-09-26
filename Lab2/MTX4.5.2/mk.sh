VFD=FDimage

as86 -o ts.o ts.s
bcc  -c -ansi main.c
ld86 -d -o mtx ts.o main.o mtxlib /usr/lib/bcc/libc.a

echo mount $VFD on /mnt

mount -o loop $VFD /mnt

rm /mnt/boot/*

cp mtx /mnt/boot
umount /mnt

echo ready to go?
read dummy

qemu-system-i386 -fda FDimage -no-fd-bootchk
