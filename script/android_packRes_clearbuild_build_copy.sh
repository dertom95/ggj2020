#!/bin/bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

echo "DIR: $DIR"
sudo rm -Rf $DIR/../android/launcher-app/build
sudo rm -Rf $DIR/../android/launcher-app/.cxx
sudo mkdir -p $DIR/../android/launcher-app/src/main/assets
sudo rm -Rf $DIR/../android/launcher-app/src/main/assets/*
sudo $DIR/../tools/package_folder.sh $DIR/../bin $DIR/../android/launcher-app/src/main/assets
cd $DIR/..
script/dockerized.sh android ./gradlew -P ANDROID_ABI=armeabi-v7a build 

mkdir -p $DIR/../build/android
cp -r $DIR/../android/launcher-app/build/outputs/apk $DIR/../build/android
