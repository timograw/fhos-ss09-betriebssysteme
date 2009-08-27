#!/bin/bash

# Konfiguration inkludieren
source /home1/2007/timograw/Documents/betriessysteme/praktikum03/garbage-conf.sh

GARBAGE_INDEX_TMP=$GARBAGE_DIR/.dir.tmp

GARBAGE_STRING=$(sed -n "/^$1/p" $GARBAGE_INDEX)

if [ -n "$GARBAGE_STRING" ]; then
	# Alten Dateinamen und Pfad ermitteln
	ORIGINAL_FILENAME=$(echo $GARBAGE_STRING|awk '{ print $2 }')
	
	if [ ! -e $ORIGINAL_FILENAME ]; then
	
		# Dateieintrag loeschen
		sed "/^$1/d" $GARBAGE_INDEX>$GARBAGE_INDEX_TMP
		# und Index aktualisieren
		mv $GARBAGE_INDEX_TMP $GARBAGE_INDEX
		
		echo "Datei $1 wird wiederhergestellt zu:"
		echo "$ORIGINAL_FILENAME"
		
		if [ ! -d $(dirname $ORIGINAL_FILENAME) ]; then
			# Verzeichnisstruktur wiederherstellen
			mkdir -p $(dirname $ORIGINAL_FILENAME)
		fi
		
		# Datei wiederherstellen
		mv $GARBAGE_DIR/$1 $ORIGINAL_FILENAME
	else
		echo "Originaldatei $ORIGINAL_FILENAME existiert bereits und kann nicht wiederhergestellt werden."
	fi
	
else
	echo "Datei $1 nicht gefunden, konnte nicht wiederhergestellt werden."
fi
