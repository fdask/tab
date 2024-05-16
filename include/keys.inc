
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

    public function __construct() {
        $this->term = `stty -g`;
		system("stty -icanon -echo");

        // assign a random string
        $this->key = Config::$keyArray[array_rand(Config::$keyArray)];

        // reset some variables
        $this->mode == Config::$modeNone;
        $this->selectedString = 0;
        $this->selectedSlot = 0;
        $this->buffer = "";
        $this->displaySlots = 20;

        // how much actual data we have
        $this->maxSlots = 0;
        
        $this->strings = array();

        for ($x = 0; $x < Config::$stringCount; $x++) {
            $this->strings[] = array();
            $this->betweenSlots[] = array();
        }

        // last thing in the constructor
        $this->gameLoop();
    }

    public function gameLoop() {
        echo $this;

        while (1) {
            $redraw = false;
            
            $c = Config::waitForInput();

            switch ($c) {
                case Config::keypressUpArrow():
                    if ($this->selectedString < 3) {
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
                    $this->buffer .= $c;

                    $redraw = true;

                    break;
                case 'b':
                    print_r($this->betweenSlots);

                    break;
                case 'h':
                    // hammer on mode
                    if ($this->selectedSlot > 0) {
                        $this->mode = ($this->mode == Config::$modeHammer) ? Config::$modeNone : Config::$modeHammer;
                    
                        $redraw = true;
                    }

                    break;
                case 'q':
                    $this->gameOver();

                    break;
                case 's':
                    if ($this->selectedSlot > 0) {
                        $this->mode = ($this->mode == Config::$modeSlide) ? Config::$modeNone : Config::$modeSlide;

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

        for ($x = 0; $x < Config::$stringCount; $x++) {
            if (isset($this->strings[$x][$col]) && $this->strings[$x][$col] !== Config::$empty && $this->strings[$x][$col] > 9) {
                $size = 2;
            }
        }

        return $size;
    }

    public function addNote($note) {
        // add either the note, or an empty space
        for ($x = 0; $x < Config::$stringCount; $x++) {
            if ($x == $this->selectedString) {
                $this->strings[$x][$this->selectedSlot] = $note;
            } else {
                //$this->strings[$x][$this->selectedSlot] = Config::$empty;
            }
        }

        // add the modifier 
        if ($this->selectedSlot >= 1) {
            echo "in the 1\n";
            if ($this->mode == Config::$modeHammer) {
                echo "in the hammer mode\n";
                $this->betweenSlots[$this->selectedString][$this->selectedSlot - 1] = Config::$modeHammerChar;
                echo "Adding hammer on!\n";
            } else if ($this->mode == Config::$modeSlide) {
                // are we sliding up?  or sliding down
                
                $this->betweenSlots[$this->selectedString][$this->selectedSlot - 1] = Config::$modeSlideCharUp;
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

    public function __toString() {
        $ret = "\n";
        $ret .= "Key: " . $this->key . "\n";
        $ret .= "Current slot: " . $this->selectedSlot . "\n";

        $startX = (($this->selectedSlot - $this->displaySlots) > 0) ? ($this->selectedSlot - $this->displaySlots) : 0;

        $ret .= "startX: $startX\n";

        for ($s = 3; $s >= 0; $s--) {
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
                    echo "Changing the divider between s: $s, $x\n";
                    $divider = $this->betweenSlots[$s][$x];
                } 
                    
                // if this is our note, highlight it
                if ($x == $this->selectedSlot && $s == $this->selectedString) {
                    if (isset($this->strings[$s][$x]) && $this->strings[$s][$x] !== Config::$empty) {
                        $ret .= Colors::fg_color('white', str_pad($this->strings[$s][$x], $colSize, $divider, STR_PAD_LEFT));
                    } else {
                        $ret .= Colors::fg_color('white', str_pad("", $colSize, $divider, STR_PAD_LEFT));
                    }
                } else {
                    if (isset($this->strings[$s][$x]) && $this->strings[$s][$x] !== Config::$empty) {
                        $ret .= str_pad($this->strings[$s][$x], $colSize, $divider, STR_PAD_LEFT);
                    } else {
                        $ret .= str_pad("", $colSize, $divider, STR_PAD_LEFT);
                    }
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
        }

        $ret .= "? " . $this->buffer;

        return $ret;
    }
}