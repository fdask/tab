A tool for making your own simple tabs.

# Run
./keys.php

# Instructions
Ever seen a guitar tab before?  It's just numbers on lines showing which frets to press
on each string.  This is a tool for entering and saving your own!

Navigate with the arrow keys, add to your tab by typing in numbers.
Enter hammer/slide/bend mode, then pressing a number, adds that modifier.

# Keys
Arrow keys navigate the cursor
Shift left arrow or shift right arrow skip the cursor in that direction
Home key goes to start of measure
End key goes to end of tab
Delete clears the selected value, or column if there is no value
Spacebar clears the selected value
'h': Hammer on mode
's': Slide mode
'b': Bend mode

't': Set tuning on active string
'c': Clear your work

'l': Load a file
'p': Write to out.txt
'x': Don't play!
'z': Debug dump (to screen)
'd': Debug dump (to file)

# Config
In include/config.inc, there are several configurable parameters you may want to change.

