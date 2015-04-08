#!/bin/bash


testArchiveFile() {
	echo "Test with $1"

	./archiver -c $1
if [ $? -ne 0 ]
	then
		echo "[FAILED] compression"
		tail test.log
		return
	fi

	./archiver -x "$1.comp"
	if [ $? -ne 0 ]
	then
		echo "[FAILED] extracting"
		tail test.log
		return
	fi

	diff -q "$1" "$1.decomp"
	if [ $? ]
	then 
		echo "[PASSED]"
	else 
		echo "[FAILED] diff"
	fi
}

# test archiver with it sources and binary
# you could add here any files to test more.
testArchiveFile "./archiver.c"
testArchiveFile "./lib/lz4.c"
testArchiveFile "./archiver"

