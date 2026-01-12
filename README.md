# Armee
A tool to turn german subtitles into anki flashcards.

It is based off of a rip of wiktionary from [here](https://kaikki.org/dictionary/German/words.html) and it uses ncurses for the tui.

![Armee](https://github.com/gitRaiku/Armee/blob/master/Resources/Armee.png?raw=true)

# Installation
Running 
```
sudo make install
```
should build it and copy the client (armeec) and server (sarmale) to ``/usr/local/bin/``

# Running
To start the server (necessary as reading the dictionary on every client open would slow it down by 5 seconds every time) run
```
sarmale
```

And then to interact with it run
```
armee <display text> [path/to/audio/file]
```

For the anki integration to work you need to have the [anki-connect](https://git.sr.ht/~foosoft/anki-connect/) plugin open and running.

# Usage 
```
Movement
   hjkl   Let you move inside squares
   HJKL   Let you move between squares
   Interacting with text
       c      Adds currently selected item in full to the final string
       <space>
              Adds currently selected item without the begining parentheses to the final
              string, or if the word on the right is selected, it searches it 
              (useful for conjugated words)

       n      Queries anki for the current selected text
       v      Enters visual mode to select words
       w      Goes to the end of the current word (only works in visual mode)
       0      Goes to the start of the string
       $      Goes to the end of the string
       f      Takes the currently selected text but prompts the user as to what to search in
              the dictionary
   Exit
       q      Exits without doing anything
       O      Bolds selected text, pushes entire string, no prompt
       I      Bolds selected text, pushes entire string, gui prompt
       u      Bolds selected text, pushes string without word names, no prompt
       U      Bolds selected text, pushes string without word names, gui prompt
       p      Doesn't bold selected text, pushes string without word names, no prompt
       p      Doesn't bold selected text, pushes string without word names, gui prompt
```

# Integration with mpv
Below is a very hacky integration which i strongly suggest you try to reimplement

The ``Resources/german_sub_to_anki.lua`` mpv script which takes the current mpv subtitle, prints it out to ``/tmp/armeect``, then uses ffmpeg to extract the audio for the current subtitle putting it in ``/tmp/armeecp``. These can then be accessed by a newly created terminal that calls armee with them as parameters.

By adding 
```
if [ "$$ARMEEC" = "1" ]
    sleep 0.1'
    exec armee "$$(cat /tmp/armeect)" "$$(cat /tmp/armeecp)"
end
```
to your ``config.fish`` or the equivalent for your shell, and modifying ``Resources/german_sub_to_anki.lua`` to be configured for your terminal+shell combo.

The mpv lua script should go in ``~/.config/mpv/scripts/`` and will be called upon pressing ``y``.

# Ankee
[Ankee](https://github.com/gitRaiku/Ankee) is a tool that does the same thing but for japanese subtitles
