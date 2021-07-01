# Commit message style
When submitting changes via a pull request, or any other means, please format commit messages using ["50/72" guidelines, as suggested by Tim Pope](https://tbaggery.com/2008/04/19/a-note-about-git-commit-messages.html).

## Summary:
- Maximum 50 character first "title" line, written as a capitalized [imperative sentence](http://examples.yourdictionary.com/imperative-sentence-examples.html).
- If necessary, more detailed explanatory paragraphs following a blank line, with lines wrapped at maximum 72 characters.
- If used, bullet points have a "hanging indent".

## Example 1:
```
Make state variables used by pollButtons() public

The variables currentButtonState and previousButtonState used by
pollButtons(), justPressed() and justReleased() have been made public.
This allows them to be manipulated if circumstances require it.

The documentation added for previousButtonState includes an example.
```

## Example 2:
```
Refactor text code and add char size functions

- Functions write() and drawChar() were refactored for smaller code
  size and to make text wrapping operate more like what would normally
  be expected.

- A new flag variable, textRaw, has been added, along with functions
  setTextRawMode() and getTextRawMode() to set and read it.
```