#include <efi.h>
#include <efilib.h>

#include "words.h"

EFI_STATUS
EFIAPI
efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
    InitializeLib(ImageHandle, SystemTable);

    EFI_TIME time;
    SystemTable->RuntimeServices->GetTime(&time, NULL);

    unsigned int seed = time.Year;
    seed = seed * 100 + time.Day;

    Print(L"Welcome to Wordle on UEFI! Guess today's word:\n\n");
    Print(L"_____\n");
    Print(L"_____\n");
    Print(L"_____\n");
    Print(L"_____\n");
    Print(L"_____\n");
    Print(L"_____\n");

    EFI_INPUT_KEY Key;

    SystemTable->ConOut->SetCursorPosition(SystemTable->ConOut, 0, SystemTable->ConOut->Mode->CursorRow - 6);

    UINTN KeyEvent = 0;

    char buffer[5];
    int idx = 0;

    SystemTable->ConOut->SetAttribute(SystemTable->ConOut, EFI_WHITE);
    while (1) {
        if (SystemTable->ConIn->ReadKeyStroke(SystemTable->ConIn, &Key) == EFI_SUCCESS) {
            int letter = 0;
            if (L'A' <= Key.UnicodeChar && Key.UnicodeChar <= L'Z') letter = Key.UnicodeChar + 32;
            if (L'a' <= Key.UnicodeChar && Key.UnicodeChar <= L'z') letter = Key.UnicodeChar;
            if (letter) {
                if (idx >= 5) continue;
                Print(L"%c", letter - 32);
                idx++;
            } else if (idx && (Key.ScanCode == SCAN_DELETE || Key.UnicodeChar == CHAR_BACKSPACE)) {
                Print(L"\b");
                idx--;
            } else if (idx == 5 && (Key.UnicodeChar == CHAR_CARRIAGE_RETURN || Key.UnicodeChar == CHAR_LINEFEED)) {
                Print(L"\n");
                idx = 0;
            }
        }

    }

    return EFI_SUCCESS;
}