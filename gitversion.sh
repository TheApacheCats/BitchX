#!/bin/sh

test $# -gt 0 || exit

tag="$1"

if test $# -gt 1; then
	cd $2 || exit
fi

test -d .git || exit

desc=`git describe --match "$tag" 2>/dev/null` || exit

echo "$desc" | sed -e "s@^$tag@@"

