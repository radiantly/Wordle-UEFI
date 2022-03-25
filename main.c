#include <uefi.h>

#include "words.h"
int main(int argc, char **argv) {
    printf("Welcome to Wordle on UEFI! Guess today's word:\n\n");
    printf("_____\n");
    printf("_____\n");
    printf("_____\n");
    printf("_____\n");
    printf("_____\n");
    printf("_____\n");

    efi_input_key_t Key;
    ST->ConOut->SetCursorPosition(ST->ConOut, 0, ST->ConOut->Mode->CursorRow - 6);

    char buffer[5];
    int idx = 0;

    ST->ConOut->SetAttribute(ST->ConOut, EFI_WHITE);
    while (1) {
        if (ST->ConIn->ReadKeyStroke(ST->ConIn, &Key) == EFI_SUCCESS) {
            int letter = 0;
            if ('A' <= Key.UnicodeChar && Key.UnicodeChar <= L'Z') letter = Key.UnicodeChar + 32;
            if (L'a' <= Key.UnicodeChar && Key.UnicodeChar <= L'z') letter = Key.UnicodeChar;
            if (letter) {
                if (idx >= 5) continue;
                printf("%c", letter - 32);
                idx++;
            } else if (idx && (Key.ScanCode == SCAN_DELETE || Key.UnicodeChar == CHAR_BACKSPACE)) {
                printf("\b");
                idx--;
            } else if (idx == 5 && (Key.UnicodeChar == CHAR_CARRIAGE_RETURN || Key.UnicodeChar == CHAR_LINEFEED)) {
                printf("\n");
                idx = 0;
            }
        }
    }

    return 0;
}
