#!/bin/sh

rm -rf platforms/android
rm -rf plugins/*

phonegap build android

cp -R www/img/res/* platforms/android/res

echo "Installing SplashScreen"
cordova plugin add https://git-wip-us.apache.org/repos/asf/cordova-plugin-splashscreen.git

phonegap run android
