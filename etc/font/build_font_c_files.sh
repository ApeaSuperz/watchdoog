#!/bin/bash

function gen_map_file() {
  cat "$1.txt" >build/temp_text.txt
  cat universal.txt >>build/temp_text.txt

  echo '32-128,' >"build/$1.map"
  cat build/temp_text.txt | iconv -f utf-8 -t c99 | sed 's/\\u\([0-9a-f]\{4\}\)/\$\1,\'$'\n/g' | sort | uniq | sed '/^$/d' | tr '/a-f/' '/A-F/' >>"build/$1.map"
}

gen_map_file size12
./bdfconv bdf/wenquanyi_9pt.bdf -b 0 -f 1 -M build/size12.map -n u8g2_font_watchdoog12_t -o build/u8g2_font_watchdoog12_t.c
gen_map_file size13
./bdfconv bdf/wenquanyi_10pt.bdf -b 0 -f 1 -M build/size13.map -n u8g2_font_watchdoog13_t -o build/u8g2_font_watchdoog13_t.c
gen_map_file size14
./bdfconv bdf/wenquanyi_13px.bdf -b 0 -f 1 -M build/size14.map -n u8g2_font_watchdoog14_t -o build/u8g2_font_watchdoog14_t.c
gen_map_file size15
./bdfconv bdf/wenquanyi_11pt.bdf -b 0 -f 1 -M build/size15.map -n u8g2_font_watchdoog15_t -o build/u8g2_font_watchdoog15_t.c
gen_map_file size16
./bdfconv bdf/wenquanyi_12pt.bdf -b 0 -f 1 -M build/size16.map -n u8g2_font_watchdoog16_t -o build/u8g2_font_watchdoog16_t.c

cat c_file_header.txt build/u8g2_font_watchdoog*.c >../../src/display/watchdoog_fonts.cpp
