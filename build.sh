make
cp build/boot32.elf iso/boot/boot32.elf
grub-mkrescue -o MegaOS.iso iso
