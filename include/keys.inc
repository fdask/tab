
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
        $this->maxSlots = 0;
        
        $this->stringCount = is_null($stringCount) ? Config::$stringCount : $stringCount;

        $this->strings = array();
        $this->betweenSlots = array();

        for ($x = 0; $x < $stringCount; $x++) {
            $this->strings[] = array();
            $this->betweenSlots[] = array();
        }
    }

    public function gameLoop() {
        echo $this;

        while (1) {
            $redraw = false;
            
            $c = Config::waitForInput();

            switch ($c) {
                case Config::keypressUpArrow():
                    if ($this->selectedString < $this->stringCount - 1) {
                        $this->selectedString++;
                        $redraw = true;
                    }

                    break;
                case Config::keypressDownArrow():
                    if ($this->selectedString > 0) {
                        $this->selectedString--;
                        $redraw = true;
                    }

                    break;
                case Config::keypressLeftArrow():
                    if ($this->selectedSlot > 0) {
                        $this->selectedSlot--;
                        $redraw = true;
                    }

                    break;
                case Config::keypressRightArrow():
                    if ($this->selectedSlot < $this->maxSlots) {
                        $this->selectedSlot++;
                        $redraw = true;
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

                    $redraw = true;

                    break;
                case '0':
                    // if the buffer is empty, go right to adding
                    if ($this->buffer === "") {
                        $this->addNote($c);

                        $redraw = true;

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

                    $redraw = true;

                    break;
                case 'b':
                    // bend mode
                    $this->mode = ($this->mode == Config::$modeBend) ? Config::$modeNone : Config::$modeBend;

                    $redraw = true;

                    break;
                case 'h':
                    // hammer on mode (not usable on first slot)
                    if ($this->selectedSlot > 0) {
                        $this->mode = ($this->mode == Config::$modeHammer) ? Config::$modeNone : Config::$modeHammer;
                    
                        $redraw = true;
                    }

                    break;
                case 'l':
                    // load a file
                    $filename = readline("filename? ");

                    if ($this->load($filename)) {
                        $this->redraw = true;
                    }

                    break;
                case 'p':
                    // print!
                    $this->print("out.txt");

                    break;
                case 'q':
                    // quit
                    $this->gameOver();

                    break;
                case 's':
                    // slide mode (not usable on first slot)
                    if ($this->selectedSlot > 0) {
                        $this->mode = ($this->mode == Config::$modeSlide) ? Config::$modeNone : Config::$modeSlide;

                        $redraw = true;
                    }

                    break;
                case Config::keypressBackspace():
                    // delete the last char from the buffer
                    $count = strlen($this->buffer);

                    if ($count >= 1) {
                        $this->buffer = substr($this->buffer, 0, $count - 1);
                        
                        $redraw = true;
                    }

                    break;
                default:
                    echo "unhandled: ";
                    print_r(Config::strToChr($c));
            }

            if ($redraw) {
                echo $this;
            }
        }

        $this->gameOver();
    }

    public function gameOver() {
        system("stty " . $this->term);

        exit;
    }

    public function load($filename) {
        if (file_exists($filename)) {
            $data = file($filename);

            print_r($data);

            $tuning = array();

            // get the number of strings
            for ($s = 0; $s < count($data); $s++) {
                if ($data[$s] == "") {
                    $stringCount = $s + 1;

                    echo "Got a stringcount of $stringCount\n";
                }

                $tuning[] = substr($data[$s], 0, 2);
            }

            // get the number of slots per line
            $length = strlen($data[0]) - 4;

            for ($x = 4; $x < strlen($data[0]) - 1; $x++) {
                // look for a character on any of the strings
                for ($s = 0; $s < $stringCount; $s++) {
                    if ($data[$s] != "-") {
                        // we have data on string $s

                    }
                }
            }
            

            // get the tuning
            print_r($tuning);

            // reset everything to the settings from the file
            $this->reset($displayCount, $stringCount);

            // load data into the strings


            return true;
        }

        return false;
    }

    public function getDividedRanges($width) {
        $ret = array();

        $start = 0;

        // how many chunks will it take to get us to rows no longer than $width chars?
        // what are those chunks?
        while (1) {
            for ($x = $start; $x < $this->maxSlots; $x++) {
                $colSize = $this->getColSize($x);

                $colCount += $colSize + 1;

                if ($colCount >= $width) {
                    $stop = $x;
                    
                    $ret[] = array($start, $stop);

                    $start = $x + 1;

                    break;
                }
            }

            $ret[] = array($start, $x);
            
            break;
        }

        return $ret;
    }

    public function print($filename, $lineWidth = 40) {
        $ret = "";

        $dividedRanges = $this->getDividedRanges($lineWidth);

        foreach ($dividedRanges as $range) {
            $start = $range[0];
            $stop = $range[1];
        
            for ($s = $this->stringCount - 1; $s >= 0; $s--) {
                $ret .= "|-";
                
                // for each slot
                for ($x = $start; $x < $stop; $x++) {
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
                }

                $ret .= "|\n";
            }
        }

        file_put_contents($filename, $ret);
    }

    public function validateNote($note) {
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
        $this->selectedSlot++;
        $this->maxSlots++;

        print_r($this->strings);
        print_r($this->betweenSlots);
    }

    function getStringCount($data) {
    // get the number of strings
    for ($s = 0; $s < count($data); $s++) {
        if (trim($data[$s]) == "") {
            $stringCount = $s;

            break;
        }
    }

    $stringCount = $s;

    return $stringCount;
}

function getTuning($data) {
    $tuning = array();

    for ($s = 0; $s < count($data); $s++) {
        if (trim($data[$s]) == "") {
            break;
        }

        $tuning[] = substr($data[$s], 0, 2);
    }

    return array_reverse($tuning);
}

parse($data);

function breakIntoStaffs($data) {
    $ret = array();
    $stanza = array();

    for ($ln = 0; $ln < count($data); $ln++) {
        if (trim($data[$ln]) == "") {
            $ret[] = $stanza;
            $stanza = array();
        } else {
            // remove leadin
            $add = substr(trim($data[$ln]), 4);

            // remove trailing
            $add = substr($add, 0, strlen($add) - 1);

            $stanza[] = $add;
        }
    }

    if (count($stanza)) {
        $ret[] = $stanza;
    }

    return $ret;
}

function parse($data) {
    $stringCount = getStringCount($data);

    echo "String count $stringCount\n";

    $tuning = getTuning($data);

    print_r($tuning);

    // break the things into chunks
    $chunks = breakIntoStaffs($data);

    print_r($chunks);

    $slotCount = 0;

    // initalize the main returns
    $strings = array();
    $betweenSlots = array();

    for ($x = 0; $x < $stringCount; $x++) {
        $strings[$x] = array();
        $betweenSlots[$x] = array();
    }

    foreach ($chunks as $chunk) {
        $length = strlen($chunk[0]);

        for ($x = 0; $x < $length; $x++) {
            for ($string = 0; $string < $stringCount; $string++) {
                echo "Setting size to 1\n";
                $size = 1;

                // check to see what size we have
                if (preg_match("@[0-9x]@", $chunk[$string][$x])) {
                    // we have a 1.  lets see if the next is also a number
                    if (preg_match("@[0-9x]@", $chunk[$string][$x + 1])) {
                        echo "Setting size to 2\n";
                        $size = 2;
                    }
                }
            }

            if ($size === 1) {
                for ($string = 0; $string < $stringCount; $string++) {
                    if ($chunk[$string][$x] != "-") {
                        $strings[$string][$slotCount] = $chunk[$string][$x];
                    } else {
                        $strings[$string][$slotCount] = Config::$empty;
                    }

                    $gapChar = "";

                    if (isset($chunk[$string][$x + 1])) {
                        $gapChar = $chunk[$string][$x + 1];

                        if ($gapChar != "" && $gapChar != "-") {
                            $betweenSlots[$string][$slotCount] = $gapChar;
                        }
                    }

                    echo "We have a gapchar of '$gapChar'\n";
                }
            } else if ($size === 2) {
                for ($string = 0; $string < $stringCount; $string++) {
                    $str = $chunk[$string][$x] . $chunk[$string][$x + 1];
            
                    if (preg_match("@[0-9x]+@", $str, $matches)) {
                        $strings[$string][$slotCount] = $matches[0];
                    } else {
                        $strings[$string][$slotCount] = Config::$empty;
                    }

                    $gapChar = "";

                    if (isset($chunk[$string][$x + 2])) {
                        $gapChar = $chunk[$string][$x + 2];

                        if ($gapChar != "" && $gapChar != "-") {
                            $betweenSlots[$string][$slotCount] = $gapChar;
                        }
                    }

                    echo "We have a gapcar of '$gapChar'\n";
                }
            }

            $slotCount++;

            // advance x ahead to the next spot.
            $x = $x + $size;
        }
    }

    print_r(array_reverse($strings));
    print_r(array_reverse($betweenSlots));
}
    public function __toString() {
        $ret = "\n";
        $ret .= "Key: " . $this->key . "\n";
        $ret .= "Current slot: " . $this->selectedSlot . "\n";

        $startX = (($this->selectedSlot - $this->displaySlots) > 0) ? ($this->selectedSlot - $this->displaySlots) : 0;

        $ret .= "startX: $startX\n";

        for ($s = $this->stringCount - 1; $s >= 0; $s--) {
            if ($s == $this->selectedString) {
                $ret .= Colors::fg_color('white', $s + 1);
            } else {
                $ret .= $s + 1;
            }
            
            $ret .= "|-";
            
            // for each slot
            for ($x = $startX; $x < $startX + $this->displaySlots; $x++) {
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
                    $ret .= Colors::fg_color($color, str_pad($this->strings[$s][$x], $colSize, $divider, STR_PAD_LEFT));
                } else {
                    $ret .= Colors::fg_color($color, str_pad("", $colSize, $divider, STR_PAD_LEFT));
                }

                $ret .= $divider;
            }

            $ret .= "|\n";
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
        }

        $ret .= "? " . $this->buffer;

        return $ret;
    }
}