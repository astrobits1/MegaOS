make all
cp build/boot32.elf iso/boot/boot32.elf
cp build/kernel64.elf iso/boot/kernel64.elf
grub-mkrescue -o MegaOS.iso iso
