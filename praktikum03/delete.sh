#!/bin/bash

# Konfiguration inkludieren
source /home1/2007/timograw/Documents/betriessysteme/praktikum03/garbage-conf.sh

# Erste Benutzung, Papierkorb existiert noch nicht?
if [ ! -e $GARBAGE_DIR ]; then
	# Papierkorbverzeichnis anlegen
	echo "erzeuge Papierkorbverzeichnis '$GARBAGE_DIR'"
	mkdir $GARBAGE_DIR
	touch $GARBAGE_INDEX
fi

I=0

for FILENAME;
do
	# Datei existiert und ist regulaer
	#set -xv
	if [ -f $FILENAME ]; then
		# Alten Pfad speichern
		OLD_PATH=$PWD
		
		# Dateinamen ermitteln
		FILENAME_FILEONLY=$(basename $FILENAME)
		
		# In Dateipfad wechseln
		cd $(dirname $FILENAME)
		# Absoluten Dateipfad ermitteln
		FILENAME_FULLPATH=$(pwd)/$FILENAME_FILEONLY
		
		# In altes Verzeichnis wechseln
		cd $OLD_PATH
		
		# Neuen Dateinamen ermitteln
		GARBAGE_FILENAME=$(date '+%y%m%d%H%M%S')_$$_$I.dat
		
		# Zum Index hinzufuegen
		echo "$GARBAGE_FILENAME $FILENAME_FULLPATH">>$GARBAGE_INDEX
		
		# Datei verschieben
		mv $FILENAME_FULLPATH $GARBAGE_DIR/$GARBAGE_FILENAME
		
		# Benutzer benachrichtigen
		echo "Datei $FILENAME wurde in den Papierkorb verschoben, um sie wiederherzustellen benutzen sie folgenden Befehl:"
		echo "undelete $GARBAGE_FILENAME"
		
		I=$(($I+1))
	else
		echo "Datei '$FILENAME' wurde nicht gefunden, nichts wurde geloescht"
	fi
done
