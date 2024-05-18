
<?php
/* main game class */
class Keys {
    public $key;
    public $selectedString;
    public $selectedSlot;
    public $displaySlots;
    public $maxSlots;
    public $strings;
    public $buffer;
    public $term;
    public $betweenSlots;
    public $mode;
    public $tuning;
    public $stringCount;

    public function __construct() {
        $this->term = `stty -g`;
		system("stty -icanon -echo");

        // assign a random string
        $this->key = Config::$keyArray[array_rand(Config::$keyArray)];

        $this->reset();

        // last thing in the constructor
        $this->gameLoop();
    }

    public function reset($displaySlotCount = null, $stringCount = null) {
        // reset some variables
        $this->mode == Config::$modeNone;
        $this->selectedString = 0;
        $this->selectedSlot = 0;
        $this->buffer = "";
        $this->displaySlots = is_null($displaySlotCount) ? Config::$displaySlotCount : $displaySlotCount;

        // how much actual data we have
        $this->maxSlots = 1;
        
        $this->stringCount = is_null($stringCount) ? Config::$stringCount : $stringCount;

        $this->strings = array();
        $this->betweenSlots = array();
        $this->tuning = array();

        for ($x = 0; $x < $this->stringCount; $x++) {
            $this->strings[] = array();
            $this->betweenSlots[] = array();
            $this->tuning[] = "XX";
        }
    }

    public function validateTuning($tune) {
        if (preg_match("@[A-G][b#]?[0-9]@", $tune)) {
            return true;
        }

        return false;
    }

    public function getTuningLength() {
        // this is the minimum 
        $len = 2;

        for ($x = 0; $x < $this->stringCount; $x++) {
            if (strlen($this->tuning[$x]) > $len) {
                $len = strlen($this->tuning[$x]);
            }
        }

        return $len;
    }

    public function gameLoop() {
        echo $this;

        while (1) {
            $redraw = false;
            
            $c = Config::waitForInput();

            if ($this->mode == Config::$modeClearing) {
                switch ($c) {
                    case 'y':
                        // clear 
                        $this->reset();
                    default:
                        $this->mode = Config::$modeNone;
                }

                $redraw = true;
            } else if ($this->mode == Config::$modeTuning) {
                // tuning mode
                $bufLen = strlen($this->buffer);

                switch ($c) {
                    case '0':
                    case '1':
                    case '2':
                    case '3':
                    case '4':
                    case '5':
                    case '6':
                    case '7':
                    case '8':
                    case '9':
                        if ($bufLen >= 1 && $bufLen < 3) {
                            $this->buffer .= $c;
                        }

                        if ($this->validateTuning($this->buffer)) {
                            $this->tuning[$this->selectedString] = $this->buffer;
                            $this->buffer = "";
                            $this->mode = Config::$modeNone;
                        }

                        break;
                    case '#':
                        if ($bufLen >= 1 && $bufLen < 2) {
                            $this->buffer .= "#";
                        }

                        break;
                    case 'b':
                        if ($bufLen == 0) {
                            $this->buffer = "B";
                        } else if ($bufLen == 1) {
                            $this->buffer .= "b";
                        }

                        break;
                    case 'a':
                    case 'A':
                    case 'B':
                    case 'c':
                    case 'C':
                    case 'd':
                    case 'D':
                    case 'e':
                    case 'E':
                    case 'f':
                    case 'F':
                    case 'g':
                    case 'G':
                        if ($bufLen == 0) {
                            $this->buffer = strtoupper($c);
                        }

                        break;
                    case Config::keypressEnter():
                        if ($this->validateTuning($this->buffer)) {
                            $this->tuning[$this->selectedString] = $this->buffer;
                            $this->buffer = "";
                            $this->mode = Config::$modeNone;
                        }

                        break;
                    case 't':
                    case Config::keypressEscape():
                        $this->mode = Config::$modeNone;
                        $this->buffer = "";
                    
                        break;
                    case Config::keypressBackspace():
                        // delete the last char from the buffer
                        $count = strlen($this->buffer);

                        if ($count >= 1) {
                            $this->buffer = substr($this->buffer, 0, $count - 1);
                        }

                        break;
                    case 'q':
                        $this->gameOver();

                        break;
                    default:
                }

                $redraw = true;
            } else {
                $redraw = true;

                switch ($c) {
                    case Config::keypressHome():
                        $this->selectedSlot = 0;

                        break;
                    case Config::keypressEnd():
                        $this->selectedSlot = $this->maxSlots - 1;

                        break;
                    case Config::keypressUpArrow():
                        if ($this->selectedString < $this->stringCount - 1) {
                            $this->selectedString++;
                        }

                        break;
                    case Config::keypressDownArrow():
                        if ($this->selectedString > 0) {
                            $this->selectedString--;
                        }

                        break;
                    case Config::keypressLeftArrow():
                        if ($this->selectedSlot > 0) {
                            $this->selectedSlot--;
                        } else {
                            $redraw = false;
                        }

                        break;
                    case Config::keypressRightArrow():
                        if ($this->selectedSlot < $this->maxSlots - 1) {
                            $this->selectedSlot++;
                        } else {
                            $redraw = false;
                        }

                        break;
                    case Config::keypressShiftLeftArrow():
                        $this->selectedSlot = $this->selectedSlot - Config::$shiftWidth;

                        if ($this->selectedSlot < 0) {
                            $this->selectedSlot = 0;
                        }

                        break;
                    case Config::keypressShiftRightArrow():
                        $this->selectedSlot += Config::$shiftWidth;

                        if ($this->selectedSlot >= $this->maxSlots) {
                            $this->selectedSlot = $this->maxSlots - 1;
                        }
                        
                        break;
                    
                    case Config::keypressEnter():
                        // submit the buffer
                        if (!empty($this->buffer)) {
                            if ($this->validateNote($this->buffer)) {
                                $this->addNote($this->buffer);
                            }
                        }

                        // clear the buffer
                        $this->buffer = "";

                        // clear the mode
                        $this->mode = Config::$modeNone;

                        break;
                    case Config::keypressEscape():
                        $this->mode = Config::$modeNone;
                        $this->buffer = "";
                    
                        break;
                    case '0':
                        // if the buffer is empty, go right to adding
                        if ($this->buffer === "") {
                            $this->addNote($c);
                            $this->buffer = "";
                            $this->mode = Config::$modeNone;

                            break;
                        } 
                    case '1':
                    case '2':
                    case '3':
                    case '4':
                    case '5':
                    case '6':
                    case '7':
                    case '8':
                    case '9':
                    case 'x':
                        $this->buffer .= $c;

                        if (strlen($this->buffer) == 2 || in_array($c, array('3', '4', '5', '6', '7', '8', '9')) || $c == 'x') {
                            // skip to adding
                            if ($this->validateNote($this->buffer)) {
                                $this->addNote($this->buffer);
                                $this->buffer = "";
                                $this->mode = Config::$modeNone;
                            } else {
                                $this->buffer = substr($this->buffer, 0, strlen($this->buffer) - 1);
                            }
                        }

                        break;
                    case 'b':
                        // bend mode
                        $this->mode = ($this->mode == Config::$modeBend) ? Config::$modeNone : Config::$modeBend;

                        break;
                    case 'c':
                        // clear mode
                        $this->mode = Config::$modeClearing;
                        
                        break;
                    case 'd':
                        // dump out the object
                        file_put_contents("dump.out", serialize($this));
                        echo "dumped.";

                        $redraw = false;

                        break;
                    case 'h':
                        // hammer on mode (not usable on first slot)
                        if ($this->selectedSlot > 0) {
                            $this->mode = ($this->mode == Config::$modeHammer) ? Config::$modeNone : Config::$modeHammer;
                        }

                        break;
                    case 'l':
                        // load a file
                        $filename = readline("filename? ");

                        if (!$this->load($filename)) {
                            $redraw = false;
                        }

                        break;
                    case 'p':
                        // print!
                        $this->print("out.txt");

                        $redraw = false;

                        break;
                    case 'q':
                        // quit
                        $this->gameOver();

                        break;
                    case 's':
                        // slide mode (not usable on first slot)
                        if ($this->selectedSlot > 0) {
                            $this->mode = ($this->mode == Config::$modeSlide) ? Config::$modeNone : Config::$modeSlide;
                        }

                        break;
                    case 't':
                        // set the tuning!
                        $this->mode = ($this->mode == Config::$modeTuning) ? Config::$modeNone : Config::$modeTuning;

                        break;
                    case Config::keypressSpaceBar():
                        if (isset($this->strings[$this->selectedString][$this->selectedSlot])) {
                            $val = $this->strings[$this->selectedString][$this->selectedSlot];
                        } else {
                            $val = Config::$empty;
                        }

                        if ($val != Config::$empty) {
                            // delete the value on the slot (and any gapChar)
                            $this->strings[$this->selectedString][$this->selectedSlot] = Config::$empty;

                            if (($this->selectedSlot - 1) >= 0) {
                                $this->betweenSlots[$this->selectedString][$this->selectedSlot - 1] = Config::$empty;
                            }
                        }

                        break;
                    case 'z':
                        print_r($this->strings);
                        break;
                    case Config::keypressDelete():
                        $val = $this->strings[$this->selectedString][$this->selectedSlot];

                        if ($val != Config::$empty) {
                            // delete the value on the slot (and any gapChar)
                            $this->strings[$this->selectedString][$this->selectedSlot] = Config::$empty;

                            if (($this->selectedSlot - 1) >= 0) {
                                $this->betweenSlots[$this->selectedString][$this->selectedSlot - 1] = Config::$empty;
                            }
                        } else {
                            // delete the entire slot!
                            $this->nukeSlot($this->selectedSlot);
                        }

                        break;
                    case Config::keypressBackspace():
                        // delete the last char from the buffer
                        $count = strlen($this->buffer);

                        if ($count >= 1) {
                            $this->buffer = substr($this->buffer, 0, $count - 1);
                        } else {
                            $redraw = false;
                        }

                        break;
                    default:
                        echo "unhandled: ";
                        print_r(Config::strToChr($c));
                }
            }

            if ($redraw) {
                echo $this;
            }
        }

        $this->gameOver();
    }

    public function nukeSlot($slot) {
        $newStrings = array();
        $newBetweens = array();

        for ($str = 0; $str < $this->stringCount; $str++) {
            if (!isset($newStrings[$str])) {
                $newStrings[$str] = array();
                $newBetweens[$str] = array();
            }

            for ($x = 0; $x < $this->maxSlots; $x++) {
                if ($x != $slot) {
                    $useX = $x;

                    if ($useX > $slot) {
                        $useX--;
                    }

                    $newStrings[$str][$useX] = $this->strings[$str][$x];

                    if (isset($this->betweenSlots[$str][$x])) {
                        $newBetweens[$str][$useX] = $this->betweenSlots[$str][$x];
                    } else {
                        $newBetweens[$str][$useX] = Config::$empty;
                    }
                }
            }
        }

        $this->strings = $newStrings;
        $this->betweenSlots = $newBetweens;
        $this->maxSlots--;

        if ($this->selectedSlot >= $this->maxSlots) {
            $this->selectedSlot--;
        }
    }

    public function gameOver() {
        system("stty " . $this->term);

        exit;
    }

    public function load($filename) {
        $t = new Tab($filename);

        $stringCount = $t->getStringCount();
        $displayCount = $this->displaySlots;

        // reset everything to the settings from the file
        $this->reset($displayCount, $stringCount);

        // load data into the strings
        $this->tuning = $t->getTuning();
        $this->strings = $t->getStrings();
        $this->betweenSlots = $t->getBetweenSlots();
        $this->maxSlots = $t->getSlotCount() + 1;
        $this->selectedSlot = 0;
    }

    public function getDividedRanges($width) {
        $ret = array();

        $colCount = 0;
        $start = 0;

        // how many chunks will it take to get us to rows no longer than $width chars?
        // what are those chunks?
        echo "Width: $width Maxslots: " . $this->maxSlots . "\n";

        while (1) {
            for ($x = $start; $x < $this->maxSlots; $x++) {
                $colSize = $this->getColSize($x);

                echo "colCount: $colCount Adding a colSize of $colSize at x: $x\n";

                // running tally of character width
                $colCount += $colSize + 1;

                if ($colCount > $width) {
                    echo "stopping at slot ($x)\n";
                    $stop = $x;
                    
                    $ret[] = array($start, $stop, $colCount);

                    // continue with an updated start
                    $start = $x + 1;
                    $colCount = 0;
                    echo "breaking\n";
                    continue;
                }

                echo "bottom of the for\n";
            }

            $ret[] = array($start, $x, $colCount);
            
            break;
        }

        return $ret;
    }

    public function print($filename, $lineWidth = 20) {
        $ret = "";

        // in cases where we exceed the line length, we split into staffs
        $dividedRanges = $this->getDividedRanges($lineWidth);

        // get the maximum width of the rows
        $maxWidth = 0;

        foreach ($dividedRanges as $range) {
            if ($range[2] > $maxWidth) {
                $maxWidth = $range[2];
            }
        }

        print_r($dividedRanges);
        
        foreach ($dividedRanges as $range) {
            $start = $range[0];
            $stop = $range[1];
        
            $tuningLength = $this->getTuningLength();

            for ($s = $this->stringCount - 1; $s >= 0; $s--) {
                $ret .= str_pad($this->tuning[$s], $tuningLength, " ", STR_PAD_LEFT);

                $ret .= "|-";
                
                // for each slot
                $width = 0;

                for ($x = $start; $x <= $stop; $x++) {
                    // figure out if we need a width of one or two
                    $colSize = $this->getColSize($x);

                    // get the given divider character (default -)
                    $divider = "-";

                    if (isset($this->betweenSlots[$s][$x]) && !empty($this->betweenSlots[$s][$x])) {
                        $divider = $this->betweenSlots[$s][$x];
                    } 
                    
                    $val = "";

                    if (isset($this->strings[$s][$x]) && $this->strings[$s][$x] !== Config::$empty) {
                        $val = $this->strings[$s][$x];
                    }
                    
                    $ret .= str_pad($val, $colSize, $divider, STR_PAD_LEFT);
                    $ret .= $divider;

                    $width += $colSize + 1;
                }

                if ($width < $maxWidth) {
                    for ($w = $width; $w < $maxWidth; $w++) {
                        $ret .= "-";
                    }
                }

                $ret .= "|\n";
            }

            $ret .= "\n";
        }

        file_put_contents($filename, $ret);
    }

    public function validateNote($note) {
        // x is valid
        if ($note == "x") {
            return true;
        }

        // 0 to 24
        $i = intval($note);

        if ($i <= 24) {
            return true;
        }

        return false;
    }

    public function getColSize($col) {
        $size = 1;

        for ($x = 0; $x < $this->stringCount; $x++) {
            if (isset($this->strings[$x][$col]) && $this->strings[$x][$col] !== Config::$empty && $this->strings[$x][$col] > 9) {
                $size = 2;
            }
        }

        return $size;
    }

    public function addNote($note) {
        // add either the note, or an empty space
        for ($x = 0; $x < $this->stringCount; $x++) {
            if ($x == $this->selectedString) {
                $this->strings[$x][$this->selectedSlot] = $note;
            } else {
                //$this->strings[$x][$this->selectedSlot] = Config::$empty;
            }
        }

        // add the modifier 
        if ($this->selectedSlot >= 1) {
            if ($this->mode == Config::$modeHammer) {
                $this->betweenSlots[$this->selectedString][$this->selectedSlot - 1] = Config::$modeHammerChar;
            } else if ($this->mode == Config::$modeSlide) {
                // are we sliding up?  or sliding down
                $this->betweenSlots[$this->selectedString][$this->selectedSlot - 1] = Config::$modeSlideCharUp;
            } else if ($this->mode == Config::$modeBend) {
                $this->betweenSlots[$this->selectedString][$this->selectedSlot - 1] = Config::modeBendChar();
            } else if ($this->mode == Config::$modeNone) {
                $this->betweenSlots[$this->selectedString][$this->selectedSlot - 1] = "";
            }
        }

        // go to the next slot
        if ($this->selectedSlot == $this->maxSlots - 1) {
            echo "Expanding maxslots!\n";
            $this->maxSlots++;
        }

        if (Config::$autoAdvance) {
            $this->selectedSlot++;
        }
    } 

    public function __toString() {
        $ret = "\n";
        $ret .= "Key: " . $this->key . "\n";
        $ret .= "maxSlots: " . $this->maxSlots . " selectedSlot: " . $this->selectedSlot . "\n";

        $startX = (($this->selectedSlot - $this->displaySlots) > 0) ? ($this->selectedSlot - $this->displaySlots) : 0;
        $tuningLength = $this->getTuningLength();

        for ($s = $this->stringCount - 1; $s >= 0; $s--) {
            $str = str_pad($this->tuning[$s], $tuningLength, " ", STR_PAD_LEFT);

            if ($s == $this->selectedString) {
                 $ret .= Colors::fg_color('white', $str);
            } else {
                $ret .= $str;
            }
            
            if ($startX === 0) {
                $ret .= ":";
            }

            $ret .= "|-";
            
            // for each slot
            for ($x = $startX; $x <= $startX + $this->displaySlots; $x++) {
                // figure out if we need a width of one or two
                $colSize = $this->getColSize($x);

                // get the given divider character (default -)
                $divider = "-";

                if (isset($this->betweenSlots[$s][$x]) && !empty($this->betweenSlots[$s][$x])) {
                    $divider = $this->betweenSlots[$s][$x];
                } 
                
                $color = 'bold_gray';

                // if this is our note, highlight it
                if ($x == $this->selectedSlot && $s == $this->selectedString) {
                    $color = 'white';
                }

                if (isset($this->strings[$s][$x]) && $this->strings[$s][$x] !== Config::$empty) {
                    $ret .= Colors::fg_color($color, str_pad($this->strings[$s][$x], $colSize, $divider, STR_PAD_RIGHT));
                } else {
                    $ret .= Colors::fg_color($color, str_pad("", $colSize, $divider, STR_PAD_RIGHT));
                }

                $ret .= $divider;
            }

            $ret .= "|";
            
            if ($x == $this->maxSlots) {
                $ret .= ":";
            }

            $ret .= "\n";
        }

        switch ($this->mode) {
            case Config::$modeNone:
                break;
            case Config::$modeHammer:
                $ret .= "h";

                break;
            case Config::$modeSlide:
                $ret .= "s";

                break;
            case Config::$modeBend:
                $ret .= "b";

                break;
            case Config::$modeTuning:
                $ret .= "t";

                break;
            case Config::$modeClearing:
                $ret .= "clear? y/n ";

                break;
        }

        $ret .= "? " . $this->buffer;

        return $ret;
    }
}