A tool for making your own simple tabs.

# Running
Run `keys.php`.

# Instructions
Ever seen a guitar tab before?  It's just numbers on lines showing which frets to press
on each string.  This is a tool for entering and saving your own!

Navigate with the arrow keys, add to your tab by typing in numbers.
Entering hammer/slide/bend mode, then pressing a number, adds that modifier.

'^' represents hammer ons, or pull-offs
'/' or '\\' represent slides
still figuring out the best character for bend

Tuning, shown at the start of the tab to the left of the |'s, is meant
to be set in scientific notation.  <note><number> [A-G][#b]?[1-8]

# Control
| key | desc |
| --- | --- |
|`0-9`|enter values|
|`x`|add an 'X' (indicates not to play)|
|`i`|inserts a blank column at current cursor location|
|`v`|inserts/deletes a vertical column at current cursor location|
|`Delete`|clears the selected value, or column if there is no value|
|`Spacebar`|clears the selected value.|
|modifiers||
|`h`|Hammer on mode|
|`s`|Slide mode|
|`b`|Bend mode|
| navigation ||
|`←↑→↓`|arrow keys navigate the cursor|
|`shift + ←` or `shift + →`|skip the cursor in that direction|
|`Home`|move cursor to start of measure|
|`End`|move cursor to end of tab|
| others ||
|`t`|Set tuning on active string|
|`T`|Set standard tuning (4 string `E1 A1 D2 G2`), (6 string `E2 A2 D3 G3 B3 E4`)|
|`c`|Clear your work|
|`l`|Load a file|
|`p`|Write to out.txt|
|`z`|Debug dump (to screen)|
|`d`|Debug dump (to file)|
|`q`|Quit|

# Config
In include/config.inc, there are several configurable parameters you may want to change.

