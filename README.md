tsfix
=====

I'm trying to fix [SpaceX's corrupted MPEG-TS stream](http://www.spacex.com/news/2014/04/29/first-stage-landing-video)
by making and using an interactive fixing tool.
As of now, it's just a `curses` hex viewer that always uses 188 characters per
line (the size of any non-corrupt MPEG-TS packet). Vim `hjkl`, Ctrl+U, Ctrl+D, and arrow keys and PgUp/PgDn are implemented.

Next up:

* alignment fixer, finds misaligned packets and helps you to add or remove some bytes in order to realign, OR fix a byte that is supposed to be `0x47` to actually be `0x47`
* field-view mode, displays the MPEG-TS bit-fields separately
* sequence number fixer, finds bad sequence numbers and confirms changing them to the correct sequence number
* fixers for other MPEG-TS fields
* possibly, viewers for I-frames, P-frames, B-frames; fixers if practical
