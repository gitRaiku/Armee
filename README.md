# Ankee
A tool to turn german subtitles into anki flashcards.

It is based off of a rip of wiktionary from [here](https://kaikki.org/dictionary/German/words.html) and it uses ncurses for the tui.

![Armee](https://github.com/gitRaiku/Armee/blob/master/Resources/Armee.png?raw=true)

# Installation
Running 
```
sudo make install
```
should build it and copy the client (armeec) and server (sarmale) to ``/usr/local/bin/``

Adding 
```
if [ "$$ARMEEC" = "1" ]
    sleep 0.1'
    exec armee "$$(cat /tmp/armeect)" "$$(cat /tmp/armeecp)"
end
```
to your ``config.fish`` or the equivalent for your shell, and modifying the ``sankee`` script for your terminal+shell combo should let the ``resources/new_sub_to_anki.lua`` mpv script work.

# Usage
You can start the server by just running ``sarmale``

Now you can use the tool by running ``armee <display text> [path/to/audio/file]``

Movement and interaction is done using vim keybindings explained in the man page

Or by using the ``new_german_sub_to_anki.lua`` mpv script which takes the current mpv subtitle and sends that to ``ankeec``

# Armee
[Ankee](github.com/gitRaiku/ankee) is a tool that does the same thing but for japanese subtitles
