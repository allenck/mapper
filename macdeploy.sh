#! /bin/bash
#create MacOs installation file
#
rm mapper.dmg
rm ReadableAndWritableCopy.dmg
rm mapper2.dmg
rm temp.txt


$QTDIR/bin/macdeployqt mapper.app -dmg always-overwrite

hdiutil convert -format UDRW -o ReadableAndWritableCopy.dmg mapper.dmg

hdiutil attach ReadableAndWritableCopy.dmg > temp.txt

echo "$device"
declare $(awk '$1 ~ /\/[(dev\/disk)[:digit:]]./ && $2 != "GUID_partition_scheme" {print "device="$1}' temp.txt)
echo $device

cp -r Resources /Volumes/mapper

hdiutil detach $device

hdiutil convert -format UDBZ -o mapper2.dmg ReadableAndWritableCopy.dmg

