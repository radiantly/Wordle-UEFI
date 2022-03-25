#include <uefi.h>

#include "words.h"

const int answerCount = sizeof(answers) / sizeof(answers[0]);
const int guessCount = sizeof(guesses) / sizeof(guesses[0]);

// Assumes word length is exactly 5
int wordcmp(const char *word1, const char *word2) {
    for (int i = 0; i < 5; i++)
        if (word1[i] != word2[i])
            return word1[i] - word2[i];

    return 0;
}

int isInWordlist(const char **wordlist, const int n, const char *word) {
    int low = 0;
    int high = n - 1;

    while (low <= high) {
        int mid = low + (high - low) / 2;
        int compareResult = wordcmp(word, wordlist[mid]);
        if (compareResult == 0) return 1;

        if (compareResult < 0)
            high = mid - 1;
        else
            low = mid + 1;
    }

    return 0;
}

int isValid(const char *word) {
    return isInWordlist(answers, answerCount, word) || isInWordlist(guesses, guessCount, word);
}

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
            if ('A' <= Key.UnicodeChar && Key.UnicodeChar <= 'Z') letter = Key.UnicodeChar + 32;
            if ('a' <= Key.UnicodeChar && Key.UnicodeChar <= 'z') letter = Key.UnicodeChar;
            if (letter) {
                if (idx >= 5) continue;
                buffer[idx] = letter;
                printf("%c", letter - 32);
                idx++;
            } else if (idx && (Key.ScanCode == SCAN_DELETE || Key.UnicodeChar == CHAR_BACKSPACE)) {
                printf("\b");
                idx--;
            } else if (idx == 5 && (Key.UnicodeChar == CHAR_CARRIAGE_RETURN || Key.UnicodeChar == CHAR_LINEFEED) && isValid(buffer)) {
                printf("\n");
                idx = 0;
            }
        }
    }

    return 0;
}
