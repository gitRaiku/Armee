#!/bin/sh

wget "https://kaikki.org/dictionary/German/kaikki.org-dictionary-German.jsonl" -O deutsch.json
./parse_dict.py
chmod 666 rdict
SIZE=$(du -b rdict | awk '{print $1}')
echo "#define STRINGS_SIZE $SIZE" > ../Libs/dictsize.h
sudo mv rdict ../Resources/rdict
echo "Copied rdict to ../Resources/rdict. Done!"


