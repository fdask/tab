<?php
class Tab {
    private $filename;
    private $data;
    private $tuning;
    private $stringCount;
    private $staffs;
    private $strings;
    private $betweenSlots;
    private $slotCount;

    public function __construct($filename = null) {
        $this->data = null;

        if (!is_null($filename)) {
            if (file_exists($filename)) {
                $this->filename = $filename;
                $this->data = file($filename);
                $this->parseStringCount();
                $this->parseTuning();
                $this->parseStaffs();
                $this->parse();
            }
        }
    }

    public function getBetweenSlots() {
        return $this->betweenSlots;
    }

    public function getStrings() {
        return $this->strings;
    }

    public function getStringCount() {
        return $this->stringCount;
    }

    public function getTuning() {
        return $this->tuning;
    }

    public function getSlotCount() {
        return $this->slotCount;
    }

    private function parseStringCount() {
        // get the number of strings
        for ($s = 0; $s < count($this->data); $s++) {
            if (trim($this->data[$s]) == "") {
                break;
            }
        }

        $this->stringCount = $s;
    }

    private function parseTuning() {
        $tuning = array();

        for ($s = 0; $s < count($this->data); $s++) {
            if (trim($this->data[$s]) == "") {
                break;
            }

            $tuning[] = substr($this->data[$s], 0, 2);
        }

        $this->tuning = array_reverse($tuning);
    }

    private function parseStaffs() {
        $ret = array();
        $stanza = array();

        for ($ln = 0; $ln < count($this->data); $ln++) {
            if (trim($this->data[$ln]) == "") {
                $ret[] = $stanza;
                $stanza = array();
            } else {
                // remove leadin
                $add = substr(trim($this->data[$ln]), 4);

                // remove trailing
                $add = substr($add, 0, strlen($add) - 1);

                $stanza[] = $add;
            }
        }

        if (count($stanza)) {
            $ret[] = $stanza;
        }

        $this->staffs = $ret;
    }

    public function parse() {
        $this->slotCount = 0;

        // initalize the main returns
        $strings = array();
        $betweenSlots = array();

        for ($x = 0; $x < $this->stringCount; $x++) {
            $strings[$x] = array();
            $betweenSlots[$x] = array();
        }

        foreach ($this->staffs as $chunk) {
            $length = strlen($chunk[0]);

            for ($x = 0; $x < $length; $x++) {
                for ($string = 0; $string < $this->stringCount; $string++) {
                    echo "Setting size to 1\n";
                    $size = 1;

                    // check to see what size we have
                    if (preg_match("@[0-9x]@", $chunk[$string][$x])) {
                        // we have a 1.  lets see if the next is also a number
                        if (preg_match("@[0-9x]@", $chunk[$string][$x + 1])) {
                            echo "Setting size to 2\n";
                            $size = 2;

                            break;
                        }
                    }
                }

                if ($size === 1) {
                    for ($string = 0; $string < $this->stringCount; $string++) {
                        if ($chunk[$string][$x] != "-") {
                            $strings[$string][$this->slotCount] = $chunk[$string][$x];
                        } else {
                            $strings[$string][$this->slotCount] = Config::$empty;
                        }

                        $gapChar = "";

                        if (isset($chunk[$string][$x + 1])) {
                            $gapChar = $chunk[$string][$x + 1];

                            if ($gapChar != "" && $gapChar != "-") {
                                $betweenSlots[$string][$this->slotCount] = $gapChar;
                            }
                        }

                        echo "We have a gapchar of '$gapChar'\n";
                    }
                } else if ($size === 2) {
                    for ($string = 0; $string < $this->stringCount; $string++) {
                        $str = $chunk[$string][$x] . $chunk[$string][$x + 1];
                
                        if (preg_match("@[0-9x]+@", $str, $matches)) {
                            $strings[$string][$this->slotCount] = $matches[0];
                        } else {
                            $strings[$string][$this->slotCount] = Config::$empty;
                        }

                        $gapChar = "";

                        if (isset($chunk[$string][$x + 2])) {
                            $gapChar = $chunk[$string][$x + 2];

                            if ($gapChar != "" && $gapChar != "-") {
                                $betweenSlots[$string][$slotCount] = $gapChar;
                            }
                        }
                    }
                }

                $this->slotCount++;

                // advance x ahead to the next spot.
                $x = $x + $size;
            }
        }

        $this->strings = array_reverse($strings);
        $this->betweenSlots = array_reverse($betweenSlots);
    }
}