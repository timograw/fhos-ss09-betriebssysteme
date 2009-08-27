#!/bin/bash
#TESTFILE=$1
# for TESTFILE in PARAMETER $&
for TESTFILE;
do
	echo "----------------------------"
	echo "Ueberpruefe \"$TESTFILE\""
	echo "----------------------------"
	
	# Existiert
	if [ -e $TESTFILE ]; then
		echo $TESTFILE existiert.
		
		# Ist ein Verzeichnis?
		if [ -d $TESTFILE ]; then
			echo "ist ein Verzeichnis"
		# Kein Verzeichnis
		else
			echo "ist kein Verzeichnis"
		fi
		
		# Ist Datei lesbar?
		if [ -r $TESTFILE ]; then
			echo "ist lesbar"
		# nicht lesbar
		else
			echo "ist nicht lesbar"
		fi
		
		# Ist Symbolischer Link?
		if [ -h $TESTFILE ]; then
			echo "ist symbolischer link"
		# kein symbolischer Link
		else
			echo "ist kein symbolischer link"
		fi
		
		FILEOWNER=$(ls -ld $TESTFILE | awk '{ print $3 }')
		
		# Gehoert aktueller UserID?
		if [ -O $TESTFILE ]; then
			echo "gehoert dem aktuellen Benutzer $FILEOWNER"
		# gehoert nicht aktueller UserID
		else
			# Ausgabe des Besitzers
			echo "gehoert nicht dem aktuellen Benutzer, sondern $FILEOWNER"
		fi
		
	# Existiert nicht
	else
		echo "$TESTFILE existiert nicht"
	fi
	echo
done
