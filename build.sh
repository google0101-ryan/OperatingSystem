g++ -Wall -Wextra -pedantic -ffreestanding -ggdb -O0 -ggdb -I ./src/kernel src/kernel/kernel.cpp -o build/kernel.o -c -m64 -mcmodel=kernel -mno-red-zone -fno-exceptions -fno-pic -fno-pie -std=c++23
g++ -Wall -Wextra -pedantic -ffreestanding -ggdb -O0 -ggdb -I ./src/kernel src/kernel/drivers/vga.cpp -o build/vga.o -c -m64 -mcmodel=kernel -mno-red-zone -fno-exceptions -fno-pic -fno-pie -std=c++23
g++ -Wall -Wextra -pedantic -ffreestanding -ggdb -O0 -ggdb -I ./src/kernel src/kernel/x86/gdt.cpp -o build/gdt.o -c -m64 -mcmodel=kernel -mno-red-zone -fno-exceptions -fno-pic -fno-pie -std=c++23
g++ -Wall -Wextra -pedantic -ffreestanding -ggdb -O0 -ggdb -I ./src/kernel src/kernel/x86/idt.cpp -o build/idt.o -c -m64 -mcmodel=kernel -mno-red-zone -fno-exceptions -fno-pic -fno-pie -std=c++23
g++ -Wall -Wextra -pedantic -ffreestanding -ggdb -O0 -ggdb -I ./src/kernel src/kernel/drivers/pic.cpp -o build/pit.o -c -m64 -mcmodel=kernel -mno-red-zone -fno-exceptions -fno-pic -fno-pie -std=c++23
ld -n -nostdlib -T linkd/link.ld build/*.o -o kernel.elf
