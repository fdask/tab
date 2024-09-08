
<?php
/* main game class */
class Keys {
    public $key;
    public $scale;
    public $scaleMode;
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

        // initialize everything
        $this->reset();

        // last thing in the constructor
        $this->gameLoop();
    }

    public function reset($stringCount = null) {
        // reset some variables
        $this->mode == Config::$modeNone;
        $this->selectedString = 0;
        $this->selectedSlot = 0;
        $this->buffer = "";
        $this->displaySlots = Config::$displaySlotCount;

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

    /**
     * given a current note of $node, add $interval steps
     */
    public function addInterval($note, $interval) { 
        // position of the current note in the keyArray
        $letterPos = array_search($note, Config::$keyArray);

        if ($letterPos === false) {
            return false;
        }
      
        for ($x = 0; $x < $interval; $x++) {
            $letterPos++;

            if ($letterPos >= count(Config::$keyArray) - 1) {
                $letterPos = 0;
            }
        } 

        return Config::$keyArray[$letterPos];
    }

    // the currently selected note
    public function getNote() {
        // get note if any
        if (isset($this->strings[$this->selectedString][$this->selectedSlot])) {
            $n = intval($this->strings[$this->selectedString][$this->selectedSlot]);

            // we have a good integer.  get tuning
            $tuning = $this->tuning[$this->selectedString];

            // if no tuning is set, we can't get a key
            if ($this->tuning != "XX") {
                // add n to start, and convert back to scientific
                if (preg_match("@([A-G][#b]?)[0-9]@", $tuning, $matches)) {
                    $startNote = $matches[1];

                    return $this->addInterval($startNote, $n);
                }
            } 
        } 

        return false;
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
            } else if ($this->mode == Config::$modeKey) {
                $redraw = true;

                switch ($c) {
                    case 'A':
                    case 'a':
                    case 'B':
                    case 'b':
                    case 'C':
                    case 'c':
                    case 'D':
                    case 'd':
                    case 'E':
                    case 'e':
                    case 'F':
                    case 'f':
                    case 'G':
                    case 'g':
                        if (strlen($this->buffer) == 0) {
                            $this->buffer = strtoupper($c);
                        } else {
                            $redraw = false;
                        }

                        break;
                    case 'b':
                    case '#':
                        if (strlen($this->buffer) == 1) {
                            $this->key = $this->buffer . $c;
                        } else {
                            $redraw = false;
                        }
                    
                        break;
                    case 'k':
                        $this->mode = Config::$modeNone;
                        $this->buffer = "";

                        break;
                    case 'q':
                        $this->gameOver();

                        break;
                    case Config::keypressEnter():
                        $this->key = $this->buffer;
                        $this->mode = Config::$modeNone;
                        $this->buffer = "";

                        break;
                    case Config::keypressEscape():
                        // back out of whatever mode we are in
                        $this->mode = Config::$modeNone;
                        $this->buffer = "";
                        
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
                        // unhandled keypress
                }
            } else if ($this->mode == Config::$modePrint) {
                $redraw = true;

                // print mode.
                switch ($c) {
                    case Config::keypressEscape():
                        // back out of whatever mode we are in
                        $this->mode = Config::$modeNone;
                        $this->buffer = "";
                        
                        break;
                    case Config::keypressBackspace():
                        $count = strlen($this->buffer);

                        if ($count >= 1) {
                            $this->buffer = substr($this->buffer, 0, $count - 1);
                        } else {
                            $redraw = false;
                        }

                        break;
                    case Config::keypressEnter():
                        if (strlen($this->buffer) > 0) {
                            $this->print($this->buffer);
                            $this->mode = Config::$modeNone;
                            $this->buffer = "";
                        }

                        break;
                    default:
                        $this->buffer .= $c;
                }
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
                        // back out of whatever mode we are in
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
                    case 'i':
                        // insert a blank column at present cursor position
                        $this->insertBlankColumn($this->selectedSlot);

                        break;
                    case 'k':
                        // set the key
                        $this->mode = Config::$modeKey;

                        break;
                    case 'l':
                        // load a file
                        $filename = readline("filename? ");

                        if (strlen($filename) > 0) {
                            if (file_exists($filename)) {
                                if (!$this->load($filename)) {
                                    echo "load failed.";
                                    $redraw = false;
                                }
                            } else {
                                echo "error fnf.";
                                $redraw = false;
                            }
                        }

                        break;
                    case 'p':
                        // print mode
                        $this->mode = Config::$modePrint;
                        
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
                    case 'v':
                        // if we already have a vert line on the cursor, remove it
                        if ($this->strings[$this->selectedString][$this->selectedSlot] == Config::$vertLine) {
                            $this->removeVert($this->selectedSlot);
                        } else { 
                            // add a vertical line at the current cursor position
                            $this->addVert($this->selectedSlot);
                        }

                        break;
                    case 'z':
                        // DEBUG
                        print_r($this->strings);
                        print_r($this->betweenSlots);

                        break;    
                    case Config::keypressSpaceBar():
                        // clears the value on the slot
                        $this->nukeVal($this->selectedString, $this->selectedSlot);
                        
                        break;
                    case Config::keypressDelete():
                        // clears the value, or the column if no value present
                        if (isset($this->strings[$this->selectedString][$this->selectedSlot])) {
                            $val = $this->strings[$this->selectedString][$this->selectedSlot];
                        } else {
                            $val = Config::$empty;
                        }

                        if ($val != Config::$empty) {
                            $this->nukeVal($this->selectedString, $this->selectedSlot);
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

    public function addVert($slot) {
        $this->insertBlankColumn($slot);

        for ($string = 0; $string < $this->stringCount; $string++) {
            $this->strings[$string][$slot] = Config::$vertLine;
        }
    }

    public function removeVert($slot) {
        $this->nukeSlot($slot);
    }

    public function insertBlankColumn($slot) {
        // split the string arrays into two parts.
        // before the split, and after the split.
        // data in the present column are included in the after split
        
        // increase maxSlots
        $this->maxSlots++;

        $beforeSlots = array();

        for ($string = 0; $string < $this->stringCount; $string++) {
            $beforeSlots[$string] = array();

            for ($x = 0; $x < $slot; $x++) {
                if (isset($this->strings[$string][$x])) {
                    $beforeSlots[$string][$x] = $this->strings[$string][$x];
                } else {
                    $beforeSlots[$string][$x] = Config::$empty;
                }
            }
        }
 
        $afterSlots = array();

        for ($string = 0; $string < $this->stringCount; $string++) {
            $afterSlots[$string] = array();

            for ($x = $slot; $x < count($this->strings[$string]); $x++) {
                if (isset($this->strings[$string][$x])) {
                    $afterSlots[$string][$x] = $this->strings[$string][$x];
                } else {
                    $afterSlots[$string][$x] = Config::$empty;
                }
            }
        }

        for ($string = 0; $string < $this->stringCount; $string++) {
            $this->strings[$string] = array_merge($beforeSlots[$string], array(Config::$empty), $afterSlots[$string]);
        }

        // now we do the same with the between slots, only excluding any left bordering us
        $betweenLeft = array();
        $betweenRight = array();

        for ($string = 0; $string < $this->stringCount; $string++) {
            $betweenLeft[$string] = array();

            for ($x = 0; $x < $slot - 1; $x++) {
                if (isset($this->betweenSlots[$string][$x])) {
                    $betweenLeft[$string][$x] = $this->betweenSlots[$string][$x];
                } else {
                    $betweenLeft[$string][$x] = Config::$empty;
                }
            }

            $betweenLeft[$string][$slot] = Config::$empty;
        }

        for ($string = 0; $string < $this->stringCount; $string++) {
            $betweenRight[$string] = array();

            for ($x = $slot; $x < count($this->betweenSlots[$string]); $x++) {
                if (isset($this->betweenSlots[$string][$x])) {
                    $betweenRight[$string][$x] = $this->betweenSlots[$string][$x];
                } else {
                    $betweenRight[$string][$x] = Config::$empty;
                }
            }
        }

        for ($string = 0; $string < $this->stringCount; $string++) {
            $this->betweenSlots[$string] = array_merge($betweenLeft[$string], array(Config::$empty), $betweenRight[$string]);
        }

        // move the cursor one the right
        $this->selectedSlot++;
    }

    public function nukeVal($string, $slot) {
        // delete the value on the slot (and any gapChar)
        $this->strings[$string][$slot] = Config::$empty;

        // delete the between slot before
        if (($slot - 1) >= 0) {
            $this->betweenSlots[$this->selectedString][$this->selectedSlot - 1] = Config::$empty;
        }
            
        // delete the slot after
        $this->betweenSlots[$string][$slot] = Config::$empty;
    }

    public function nukeSlot($slot) {
        if ($this->maxSlots == 1) {
            return false;
        }

        $newStrings = array();
        $newBetweens = array();

        for ($str = 0; $str < $this->stringCount; $str++) {
            if (!isset($newStrings[$str])) {
                $newStrings[$str] = array();
                $newBetweens[$str] = array();
            }

            // delete the betweens before and after the nukedSlot
            if (isset($this->betweenSlots[$str][$slot + 1])) {
                $this->betweenSlots[$str][$slot + 1] = Config::$empty;
            }

            if (isset($this->betweenSlots[$str][$slot])) {
                $this->betweenSlots[$str][$slot] = Config::$empty;
            }

            for ($x = 0; $x < $this->maxSlots; $x++) {
                if ($x != $slot) {
                    $useX = $x;

                    if ($useX > $slot) {
                        $useX--;
                    }

                    // if we have a value, add it to the new strings array
                    if (isset($this->strings[$str][$x]) && $this->strings[$str][$x] != Config::$empty) {
                        $newStrings[$str][$useX] = $this->strings[$str][$x];
                    } else {
                        $newStrings[$str][$useX] = Config::$empty;
                    }

                    // if we have a between slot (ahead), move it to new array
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

    /**
     * quit!
     */
    public function gameOver() {
        system("stty " . $this->term);

        exit;
    }

    public function load($filename) {
        // the tab class parses the written files
        $t = new Tab($filename);

        $stringCount = $t->getStringCount();
        
        // reset everything to the settings from the file
        $this->reset($stringCount);

        // load data into the strings
        $this->tuning = $t->getTuning();
        $this->strings = $t->getStrings();
        $this->betweenSlots = $t->getBetweenSlots();
        $this->maxSlots = $t->getSlotCount() + 1;
        $this->selectedSlot = 0;

        return true;
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

    /**
     * print out the current tab to a file
     */
    public function print($filename, $lineWidth = null) {
        $ret = "";

        if (is_null($lineWidth)) {
            $lineWidth = Config::$printWidth;
        }

        // in cases where we exceed the line length, we split into staffs
        $dividedRanges = $this->getDividedRanges($lineWidth);

        // get the maximum width of the rows
        $maxWidth = 0;

        foreach ($dividedRanges as $range) {
            if ($range[2] > $maxWidth) {
                $maxWidth = $range[2];
            }
        }
      
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
                    
                    $ret .= str_pad($val, $colSize, $divider, STR_PAD_RIGHT);
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

    /**
     * a valid note is either an 'x' (for don't play), or a number from 0-24.
     */
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

    /**
     * figure out if a numbers column is one, or two wide.
     */
    public function getColSize($col) {
        $size = 1;

        for ($x = 0; $x < $this->stringCount; $x++) {
            if (isset($this->strings[$x][$col]) && $this->strings[$x][$col] !== Config::$empty && $this->strings[$x][$col] > 9) {
                $size = 2;
            }
        }

        return $size;
    }

    /**
     * add the given note to the currently selected string/slot
     */
    public function addNote($note) {
        // if the current slot is filled with vertical bars, remove them!
        if (isset($this->strings[$this->selectedString][$this->selectedSlot]) && $this->strings[$this->selectedString][$this->selectedSlot] == Config::$vertLine) {
            for ($x = 0; $x < $this->stringCount; $x++) {
                $this->strings[$x][$this->selectedSlot] = Config::$empty;
            }
        }
        
        // add the note
        $this->strings[$this->selectedString][$this->selectedSlot] = $note;
        
        // add the modifier 
        if ($this->selectedSlot >= 1) {
            if ($this->mode == Config::$modeHammer) {
                // hammer on
                $this->betweenSlots[$this->selectedString][$this->selectedSlot - 1] = Config::$modeHammerChar;
            } else if ($this->mode == Config::$modeSlide) {
                // are we sliding up?  or sliding down
                $preVal = $this->strings[$this->selectedString][$this->selectedSlot - 1];

                if ($preVal > $note) {
                    $char = Config::$modeSlideCharDown;
                } else {
                    $char = Config::$modeSlideCharUp;
                }

                $this->betweenSlots[$this->selectedString][$this->selectedSlot - 1] = $char;
            } else if ($this->mode == Config::$modeBend) {
                // bend
                $this->betweenSlots[$this->selectedString][$this->selectedSlot - 1] = Config::modeBendChar();
            } else if ($this->mode == Config::$modeNone) {
                // nothing
                $this->betweenSlots[$this->selectedString][$this->selectedSlot - 1] = "";
            }
        }

        // go to the next slot
        if ($this->selectedSlot == $this->maxSlots - 1) {
            $this->maxSlots++;
        }

        if (Config::$autoAdvance) {
            $this->selectedSlot++;
        }
    } 

    public function __toString() {
        $ret = "\n";
        $ret .= "key: " . $this->key . "\n";
        $ret .= "note: ";
        
        if (isset($this->selectedString) && isset($this->selectedSlot) && isset($this->strings[$this->selectedString][$this->selectedSlot])) {
            if ($this->strings[$this->selectedString][$this->selectedSlot] != Config::$empty) {
                $ret .= $this->getNote();
            } 
        }

        $ret .= "\n";
            
        $startX = (($this->selectedSlot - $this->displaySlots) > 0) ? ($this->selectedSlot - $this->displaySlots) : 0;
        $tuningLength = $this->getTuningLength();

        for ($s = $this->stringCount - 1; $s >= 0; $s--) {
            $str = str_pad($this->tuning[$s], $tuningLength, " ", STR_PAD_LEFT);

            $color = 'bold_gray';

            if ($s == $this->selectedString) {
                 $color = Config::$colorSelectedString;
            }

            $ret .= Colors::fg_color($color, $str);
            
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

                if (isset($this->betweenSlots[$s][$x]) && $this->betweenSlots[$s][$x] != Config::$empty) {
                    $divider = $this->betweenSlots[$s][$x];
                } 
                
                // default color
                $color = 'bold_gray';

                // if this is our note, highlight it
                if ($x == $this->selectedSlot && $s == $this->selectedString) {
                    $color = Config::$colorSelectedSlot;
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
            case Config::$modeKey:
                $ret .= "key";

                break;
            case Config::$modePrint:
                $ret .= "filename";

                break;
        }

        $ret .= "? " . $this->buffer;

        return $ret;
    }
}