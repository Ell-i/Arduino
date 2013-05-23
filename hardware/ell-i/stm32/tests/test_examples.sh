#!/bin/sh
#
# Hacky version.  XXX Enhance.
#

set -e

TMPDIR=tmp
PREAMBLE=preamble.cpp

find ../../../../build/shared/examples/ -name "*.ino" -print |
while read EXAMPLE; do
    mkdir -p "$TMPDIR"
    
    DIR="`dirname $EXAMPLE`"
    FILE="`basename $EXAMPLE .ino`"
    cp "$DIR"/* "$TMPDIR"
    mv "$TMPDIR"/"$FILE".ino "$TMPDIR"/"$FILE".cpp
    cat "$PREAMBLE"   > "$TMPDIR"/test.cpp
    # Heuristics to convert function definitions to declarations. Fails with some examples
    gcc -fpreprocessed -dD -E "$TMPDIR"/"$FILE".cpp |
    sed  >> "$TMPDIR"/test.cpp \
        -n \
        -e '/^[a-zA-Z].*[	 ][a-zA-Z][a-zA-Z0-9_]*(.*)/s/{/;/p' 
    cat "$EXAMPLE"   >> "$TMPDIR"/test.cpp

    cd ..
    echo '=================>' `basename $EXAMPLE` 
    make tests/"$TMPDIR"/test > /dev/null
    cd tests

    rm -rf "$TMPDIR"
done


