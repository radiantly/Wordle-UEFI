all: main.efi

words.h: answers.txt guesses.txt generateWordlist.py
	python3 generateWordlist.py

main.o: main.c words.h
	gcc main.c -c -fno-stack-protector -fpic -fshort-wchar -mno-red-zone -I /usr/include/efi -I /usr/include/efi/x86_64 -DEFI_FUNCTION_WRAPPER -DGNU_EFI_USE_MS_ABI -o main.o

main.so: main.o
	ld main.o /usr/lib/crt0-efi-x86_64.o -nostdlib -znocombreloc -T /usr/lib/elf_x86_64_efi.lds -shared -Bsymbolic -L /usr/lib -l:libgnuefi.a -l:libefi.a -o main.so

main.efi: main.so
	objcopy -j .text -j .sdata -j .data -j .dynamic -j .dynsym -j .rel -j .rela -j .reloc --target=efi-app-x86_64 --subsystem=10 main.so main.efi
