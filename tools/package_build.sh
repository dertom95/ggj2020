#!/bin/bash

if [ -z "$1" ]; then
	echo "building packager - packages the builds in the 'builds'-folder"
	echo ""
	echo "package_game.sh [gamename]"
	exit 1
fi

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

cd $DIR/../build


GAME="$1"

rm -Rf $DIR/../packages
mkdir $DIR/../packages

if [ -z "$GAME" ]; then
	GAME="game"
fi

for d in */ ; do
    FOLDERNAME=${d::-1}

    if [ "$FOLDERNAME" != "android" ]; then
	    if [ "$FOLDERNAME" == "native" ]; then
		    GAMENAME="$GAME-linux"
            elif [ "$FOLDERNAME" == "mingw" ]; then
		    GAMENAME="$GAME-windows"
	    else
		    GAMENAME="$GAME-$FOLDERNAME"
            fi

    	    echo "Package: $GAMENAME"
	    echo ""	    
	    cd $FOLDERNAME
	 
	    cp -r -L bin $GAMENAME
	    # copy files that should be packaged with your include
	    cp -r $DIR/../package_include/* $GAMENAME    
	    zip -r $GAMENAME.zip $GAMENAME
	    mv $GAMENAME.zip $DIR/../packages
	    rm -Rf $GAMENAME
	    cd ..
     else
	     echo "android to do"
     fi
done


