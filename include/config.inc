<?php
class Config {
    public static $debug = true;
    public static $keyArray = array('Ab', 'A', 'Bb', 'B', 'C', 'Db', 'D', 'Eb', 'E', 'F', 'Gb', 'G');
    public static $empty = '';
    public static $stringCount = 6;
    public static $displaySlotCount = 20;
    
    public static $modeNone = "";
    public static $modeHammer = "HAMMERON";
    public static $modeSlide = "SLIDE";
    public static $modeBend = "BEND";
    public static $modeHammerChar = "^";
    public static $modeSlideCharUp = "/";
    public static $modeSlideCharDown = "\\";

    public static function modeBendChar() {
        return json_decode('"\u2518"');
    }

    public static function keypressLeftArrow() {
        return chr(27) . chr(91) . chr(68);
    }

    public static function keypressRightArrow() {
        return chr(27) . chr(91) . chr(67);
    }

    public static function keypressUpArrow() {
        return chr(27) . chr(91) . chr(65);
    }

    public static function keypressDownArrow() {
        return chr(27) . chr(91) . chr(66);
    }

    public static function keypressEnter() {
        return chr(10);
    }

    public static function keypressBackspace() {
        return chr(127);
    }

    public static function waitForInput() {
        $input = '';

        $read = [STDIN];
        $write = null;
        $except = null;

        readline_callback_handler_install('', function() {});

        // Read characters from the command line one at a time until there aren't any more to read
        do {
            $input .= fgetc(STDIN);
        } while (stream_select($read, $write, $except, 0, 1));

        readline_callback_handler_remove();

        return $input;
    }

    public static function strToChr($str) {
        $ret = array();

        $bits = str_split($str);

        for ($x = 0; $x < count($bits); $x++) {
            $ret[] = ord($bits[$x]);
        }

        return $ret;
    }

    public static function autoload($class) {
        $file = __DIR__ . "/" . str_replace('\\', '/', strtolower($class)) . ".inc";
        
        if (file_exists($file)) {
            require $file;
        } else {
            if (Config::$debug) {
                debug_print_backtrace();
            }
            
            die("Couldnt find file: $file\n");
        }
    }
}

spl_autoload_register("Config::autoload");