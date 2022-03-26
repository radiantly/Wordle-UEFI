#include <uefi.h>

#include "words.h"

const int answerCount = sizeof(answers) / sizeof(answers[0]);
const int guessCount = sizeof(guesses) / sizeof(guesses[0]);

int wordcmp(const char *word1, const char *word2) {
    // Assumes word length is exactly 5
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

int checkGuess(const char *buffer, const char *word_today) {
    // each bit represents whether the letter at that position is the right one
    unsigned int correct = 0;

    int letters[26] = {};

    // We have a bit of logic below to handle repeated letters correctly
    for (int i = 0; i < 5; i++) {
        // if at correct position, mark it correct
        if (buffer[i] == word_today[i]) {
            correct |= 1 << i;
        } else {  // else add it to the letters left
            letters[word_today[i] - 'a']++;
        }
    }

    ST->ConOut->SetCursorPosition(ST->ConOut, 0, ST->ConOut->Mode->CursorRow);

    // Color each letter:
    for (int i = 0; i < 5; i++) {
        int color = EFI_LIGHTGRAY;
        if (correct & (1 << i)) {
            color = EFI_LIGHTGREEN;
        } else if (letters[buffer[i] - 'a']) {
            letters[buffer[i] - 'a']--;
            color = EFI_YELLOW;
        }
        ST->ConOut->SetAttribute(ST->ConOut, color);
        putchar(buffer[i] - 32);
    }
    ST->ConOut->SetAttribute(ST->ConOut, EFI_WHITE);

    return correct == 0x1f;
}

inline int positive_modulo(int a, int b) {
    return (a % b + b) % b;
}

int main(int argc, char **argv) {
    ST->BootServices->SetWatchdogTimer(0, 0, 0, NULL);

    struct tm *local_time = localtime(NULL);
    unsigned int seed = local_time->tm_year * 10000 + local_time->tm_mon * 100 + local_time->tm_mday;
    srand(seed);
    const char *word_today = answers[positive_modulo(rand(), answerCount)];

    ST->ConOut->ClearScreen(ST->ConOut);

    ST->ConOut->SetAttribute(ST->ConOut, EFI_WHITE);
    printf("Welcome to Wordle on UEFI!\n\n");
    ST->ConOut->SetAttribute(ST->ConOut, EFI_LIGHTGRAY);
#ifdef DEBUG
    printf("Seed: %d\n", seed);
    printf("Word chosen: %s\n", word_today);
#endif
    printf("Guess today's word:\n\n");
    ST->ConOut->SetAttribute(ST->ConOut, EFI_WHITE);
    printf("_____\n");
    printf("_____\n");
    printf("_____\n");
    printf("_____\n");
    printf("_____\n");
    printf("_____\n\n");

    int statusRow = ST->ConOut->Mode->CursorRow;

    efi_input_key_t Key;
    ST->ConOut->SetCursorPosition(ST->ConOut, 0, ST->ConOut->Mode->CursorRow - 7);

    char buffer[5];
    int idx = 0;
    int guess_idx = 0;
    const int guess_count = 6;

    ST->ConOut->EnableCursor(ST->ConOut, 1);

    while (guess_idx < guess_count) {
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
                ST->ConOut->SetCursorPosition(ST->ConOut, ST->ConOut->Mode->CursorColumn - 1, ST->ConOut->Mode->CursorRow);
                printf("_");
                ST->ConOut->SetCursorPosition(ST->ConOut, ST->ConOut->Mode->CursorColumn - 1, ST->ConOut->Mode->CursorRow);
                idx--;
            } else if (idx == 5 && (Key.UnicodeChar == CHAR_CARRIAGE_RETURN || Key.UnicodeChar == CHAR_LINEFEED)) {
                if (!isValid(buffer)) {
                    ST->ConOut->SetCursorPosition(ST->ConOut, 0, statusRow);
                    ST->ConOut->SetAttribute(ST->ConOut, EFI_LIGHTRED);
                    printf("Invalid guess - not in wordlist - try again!");
                    ST->ConOut->SetAttribute(ST->ConOut, EFI_WHITE);
                    ST->ConOut->SetCursorPosition(ST->ConOut, 5, statusRow - 7 + guess_idx);
                } else if (checkGuess(buffer, word_today)) {
                    ST->ConOut->SetCursorPosition(ST->ConOut, 0, statusRow);
                    ST->ConOut->SetAttribute(ST->ConOut, EFI_LIGHTGREEN);
                    printf("Yay! You win! The word was %s!              \n", word_today);
                    break;
                } else {
                    ST->ConOut->SetCursorPosition(ST->ConOut, 0, statusRow);
                    printf("                                            ");
                    ST->ConOut->SetCursorPosition(ST->ConOut, 0, statusRow - 6 + guess_idx);
                    idx = 0;
                    guess_idx++;
                }
            }
        }

        if (guess_idx == guess_count) {
            ST->ConOut->SetCursorPosition(ST->ConOut, 0, statusRow);
            ST->ConOut->SetAttribute(ST->ConOut, EFI_YELLOW);
            printf("Uh oh! you didn't guess the word. The word was %s.\n", word_today);
        }
    }
    ST->ConOut->SetAttribute(ST->ConOut, EFI_LIGHTGRAY);
    printf("\nCome back tomorrow! Press any key to exit...\n");
    while (ST->ConIn->ReadKeyStroke(ST->ConIn, &Key) != EFI_SUCCESS)
        ;

    return 0;
}
