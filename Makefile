TARGET = main.efi
include uefi/Makefile

words.h: answers.txt guesses.txt generateWordlist.py
	python3 generateWordlist.py

main.c: words.h
