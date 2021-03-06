#!/bin/bash

# Konfiguration inkludieren
source /home1/2007/timograw/Documents/betriessysteme/praktikum03/garbage-conf.sh

# HOME Pfad escapen
HOME_ESCAPED=$(echo $HOME|sed -e "s@\/@\\\/@g")

GARBAGE_INDEX_PATHES=$(sed -e "s/^.* //g" -e "s/\/storage3_l0//g" -e "s/^$HOME_ESCAPED/~/g" $GARBAGE_INDEX)

I=1

echo "Bitte waehlen sie die Datei aus die Sie wiederherstellen moechten"

select FILEPATH in $GARBAGE_INDEX_PATHES Beenden; do
	if [ $FILEPATH = "Beenden" ]; then
		break
	fi
	
	# ~ ersetzen und escapen
	FILEPATH_ESCAPED=$(echo $FILEPATH|sed -e "s/^~/$HOME_ESCAPED/g"|sed -e "s@\/@\\\/@g")
	
	# in Index suchen und undeleten
	/home1/2007/timograw/Documents/betriessysteme/praktikum03/undelete.sh $(sed -n "/$FILEPATH_ESCAPED\$/p" $GARBAGE_INDEX|awk '{ print $1 }')
done
