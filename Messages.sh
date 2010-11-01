#! /bin/sh
$EXTRACTRC $(find . -name "*.ui") >> rc.cpp || exit 11
$XGETTEXT `find . -name \*.h -o -name \*.cpp -o -name \*.cc | grep -v '/tests/'` -o $podir/libkdegames.pot
rm -f rc.cpp 
