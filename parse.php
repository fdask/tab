#!/usr/bin/php
<?php
include 'include/config.inc';

$t = new Tab("sample2.txt");

print_r($t->getStrings());
print_r($t->getBetweenSlots());