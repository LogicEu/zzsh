#!/bin/bash

exe=zzsh
cc=gcc
src=src/*.c
inc=-Isrc

flags=(
    -std=c89
    -Wall 
    -Wextra
    -pedantic
    -O2
)

cmd() {
   echo "$@" && $@
}

comp() {
    cmd $cc ${flags[*]} $inc $src $@ -o $exe
}
    
clean() {
    [ -f $exe ] && rm $exe && echo "Deleted '$exe'."
    return 0
}

case "$1" in
    "build")
        shift
        comp $@;;
    "clean")
        clean;;
    *)
        echo "Use with 'build' or 'clean'.";;
esac
