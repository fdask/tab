#!/usr/bin/php
<?php
include 'include/config.inc';

if (file_exists("dump.out")) {
    echo "restoring dump.out\n";
    $k = unserialize(file_get_contents("dump.out"));
    $k->gameLoop();
} else {
    $k = new Keys();
}
