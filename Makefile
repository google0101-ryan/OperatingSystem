override MAKEFLAGS += -rR

override KERNEL := kernel.elf

define DEFAULT_VAR =
	ifeq ($(origin $1),default)
		override $(1) := $(2)
	endif
	ifeq ($(origin $1),undefined)
		override $(1) := $(2)
	endif
endef

$(eval $(call DEFAULT_VAR,CC,cc))
$(eval $(call DEFAULT_VAR,CPP,g++))
$(eval $(call DEFAULT_VAR,LD,ld))

CFLAGS ?= -g -O2 -pipe -Wall -Wextra

CPPFLAGS ?=

NASMFLAGS ?= -F dwarf -g

LDFLAGS ?=

override CFLAGS += \
	-ffreestanding \
	-fno-stack-protector \
	-fno-stack-check \
	-fno-lto \
	-fno-pie \
	-fno-pic \
	-m64 \
	-march=x86-64 \
	-mabi=sysv \
	-mno-80387 \
	-mno-mmx \
	-mno-sse \
	-mno-sse2 \
	-mno-red-zone \
	-mcmodel=kernel

override CPPFLAGS := \
	-I. \
	-I./src/kernel \
	$(CPPFLAGS) \
	-MMD

override LDFLAGS += \
	-nostdlib \
	-static \
	-melf_x86_64 \
	-z max-page-size=0x1000 \
	-T linkd/link.ld

# Check if the linker supports -no-pie and enable it if it does.
ifeq ($(shell $(LD) --help 2>&1 | grep 'no-pie' >/dev/null 2>&1; echo $$?),0)
    override LDFLAGS += -no-pie
endif
 
# Internal nasm flags that should not be changed by the user.
override NASMFLAGS += \
    -f elf64
 
# Use find to glob all *.c, *.S, and *.asm files in the directory and extract the object names.
override CFILES := $(shell find . -type f -name '*.cpp' | grep -v 'limine/')
override ASFILES := $(shell find . -type f -name '*.S' | grep -v 'limine/')
override NASMFILES := $(shell find . -type f -name '*.asm' | grep -v 'limine/')
override OBJ := $(CFILES:.cpp=.o) $(ASFILES:.S=.o) $(NASMFILES:.asm=.o)
override HEADER_DEPS := $(CFILES:.cpp=.d) $(ASFILES:.S=.d)
 
# Default target.
.PHONY: all
all: $(KERNEL)
 
# Link rules for the final kernel executable.
$(KERNEL): $(OBJ)
	$(LD) $(OBJ) $(LDFLAGS) -o $@
	rm -rf $(OBJ) $(HEADER_DEPS)

iso:
	cp $(KERNEL) iso_root/boot/$(KERNEL)
	xorriso -as mkisofs -b limine-cd.bin -no-emul-boot -boot-load-size 4 \
	-boot-info-table --efi-boot limine-eltorito-efi.bin \
	-efi-boot-part --efi-boot-image --protective-msdos-label \
	iso_root -o os.iso

run:
	qemu-system-x86_64 -cdrom os.iso -m 2G -M q35 --enable-kvm -d int -no-shutdown -no-reboot -s -S &
 
# Include header dependencies.
-include $(HEADER_DEPS)
 
# Compilation rules for *.c files.
%.o: %.cpp
	$(CPP) $(CFLAGS) $(CPPFLAGS) -c $< -o $@
 
# Compilation rules for *.S files.
%.o: %.S
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@
 
# Compilation rules for *.asm (nasm) files.
%.o: %.asm
	nasm $(NASMFLAGS) $< -o $@
 
# Remove object files and the final executable.
.PHONY: clean
clean:
	rm -rf $(KERNEL) $(OBJ) $(HEADER_DEPS)
