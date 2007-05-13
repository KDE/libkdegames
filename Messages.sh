#! /bin/sh
$EXTRACTRC $(find . -name "*.ui") >> rc.cpp || exit 11
$XGETTEXT `find . -name \*.h -o -name \*.cpp -o -name \*.cc` -o $podir/libkdegames.pot 
