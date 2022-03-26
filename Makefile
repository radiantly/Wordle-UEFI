TARGET := main.efi
# CFLAGS := -D DEBUG
IMG_FILE := wordle-uefi.img
IMG_SIZE_MiB := 48
include uefi/Makefile

words.h: answers.txt guesses.txt generateWordlist.py
	python3 generateWordlist.py

main.c: words.h

IMG_DD_COUNT := $(shell expr 1024 \* 1024 \* $(IMG_SIZE_MiB) \/ 512)
IMG_PART_DD_COUNT := $(shell expr $(IMG_DD_COUNT) - 2048 + 1)

disk-image: all
# We need to create a disk image file that contains the GPT partition table
# and an EFI partition.
# The EFI partition needs to contain a FAT32 filesystem (this is the most
# supported EFI partition type).
# The minimum size of a FAT32 fs is (sector size) 512 * (clusters) 65525 = ~32MiB
# Here we create a disk file of 48MiB to account for that and extras
# 1024 * 1024 * 48 / 512 = 98304
	dd if=/dev/zero of=$(IMG_FILE) bs=512 count=$(IMG_DD_COUNT)

# Clear and create gpt partition table
	sgdisk -o $(IMG_FILE)

# Create 1st partition 
	sgdisk -n 1:2048:$$(sgdisk -E $(IMG_FILE)) $(IMG_FILE)

# Switch partition type to EFI
	sgdisk -t 1:ef00 $(IMG_FILE)

# Create a temporary image file that serves as our FAT32 partition
	$(eval IMG_PART_FILE := $(shell mktemp XXXXXXXX.img))

# Partition size = Total size - first sector + 1
	dd if=/dev/zero of=$(IMG_PART_FILE) bs=512 count=$(IMG_PART_DD_COUNT)

# Create the FAT32 partition
	mkfs.fat -F 32 $(IMG_PART_FILE)

# Create our generated efi file to the correct location on the filesystem
	mmd -i $(IMG_PART_FILE) ::/EFI
	mmd -i $(IMG_PART_FILE) ::/EFI/BOOT
	mcopy -i $(IMG_PART_FILE) $(TARGET) ::/EFI/BOOT/BOOTX64.efi

# finally, merge this back to the original image
	dd if=$(IMG_PART_FILE) of=$(IMG_FILE) bs=512 count=$(IMG_PART_DD_COUNT) seek=2048 conv=notrunc

# Delete the temporary image file
	rm $(IMG_PART_FILE)

.PHONY: disk-image
