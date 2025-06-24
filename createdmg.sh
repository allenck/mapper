#! /bin/bash
#create MacOs installation file
#
cd $PWD
rm mapper.dmg

$QTDIR/bin/macdeployqt mapper.app -dmg always-overwrite
