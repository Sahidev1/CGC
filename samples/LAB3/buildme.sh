#!/bin/bash
set -e 

gcc lab3.c structures.c -o "lab3.out"
echo "Build successful!"
echo $'Usage:\n./lab3.out [PATH TO VIRTUAL ADDRESS FILE]'


