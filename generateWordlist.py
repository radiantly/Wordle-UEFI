import string

with open("answers.txt") as f:
    answers = list(filter(None, [line.strip() for line in f]))

with open("guesses.txt") as f:
    guesses = list(filter(None, [line.strip() for line in f]))


def arrayFormat(words):
    # make sure array is sorted
    assert guesses == sorted(guesses)
    # all words must be length 5 and lowercase
    assert all(len(word) == 5 and word.islower() for word in guesses)

    return "{" + ", ".join(f'"{word}"' for word in words) + "}"


with open("words.h", "w") as f:
    f.write(
        f"""const char *answers[] = {arrayFormat(answers)};
const char *guesses[] = {arrayFormat(guesses)};
"""
    )
print(f"Success! Added {len(answers)} answers and {len(guesses)} additional allowed guesses.")
