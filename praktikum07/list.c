/*
Name: ls.c
	
Purpose:
	Hauptdatei Betriebssysteme Praktikumsaufgabe 7

Autoren:
	Timo Graw
	Marcel Teuber

Version:
	0.1 - start
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
/* getopt */
#include <string.h>
#include <ctype.h>
#include <dirent.h>
#include <sys/stat.h>


#include "list.h"


int main (int argc, char **argv)
{
	int show_systemfiles = 0;
	int show_details = 0;
	int getopt_return;
	char *directory;

	while ((getopt_return = getopt (argc, argv, "al")) != -1) {
		switch (getopt_return) {
			case 'a':
				show_systemfiles = 1;
				break;
			case 'l':
				show_details = 1;
				break;
			case '?':
				if (isprint (optopt))
					fprintf (stderr, "Unbekannter Schalter: `-%c'.\n", optopt);
				else
					fprintf (stderr,
                        "Unbekannter Schaltercharakter `\\x%x'.\n",
                        optopt);
             return 1;
			 break;
		}
	}
		
	if (argc <= optind) {
		directory = ".";
	}
	else {
		directory = argv[optind];
	}
	
	list_content(directory, show_systemfiles, show_details);
	
	return 0;
}

int list_content(char *directory, int show_systemfiles, int show_details) {
	DIR *dir_ptr;
	struct dirent *dir_entry_ptr;
	
	if (directory == NULL) {
		fprintf(stderr, "list_content(): Nullpointer uebergenen\n");
		return 1;
	}
	
	dir_ptr = opendir(directory);
	
	if (dir_ptr == NULL) {
		fprintf(stderr, "list_content(): Konnte Verzeichnis nicht oeffnen\n");
		//perror();
		return 2;
	}
	
	while ((dir_entry_ptr = readdir(dir_ptr)) != NULL) {
		print_dir_entry(dir_entry_ptr, show_systemfiles, show_details);
	}
	
	closedir(dir_ptr);
	
	return 0;
}

int print_dir_entry(struct dirent *dir_entry_ptr, int show_systemfiles, int show_details) {
	
	if (dir_entry_ptr == NULL) {
		fprintf(stderr, "print_dir_entry(): Nullpointer uebergeben\n");
		return 1;
	}
	
	if (!show_systemfiles) {
		if (is_systemfile(dir_entry_ptr->d_name)) {
			return 0;
		}
	}
	
	if (show_details) {
		print_dir_entry_long(dir_entry_ptr);
	}
	else {
		print_dir_entry_normal(dir_entry_ptr);
	}
	
	return 0;
}

int is_systemfile(char *filename) {
	return filename[0] == '.';
}

void print_dir_entry_normal(struct dirent *dir_entry_ptr) {
	printf("%-10s", dir_entry_ptr->d_name);
}

int print_dir_entry_long(struct dirent *dir_entry_ptr) {
	struct stat stat_buf;
	mode_t mode;
	int i;
	char flags[] = "----------";
	char rwx[] = "rwx";
	int bits[] = {
		S_IRUSR,S_IWUSR,S_IXUSR,   /* Zugriffsrechte User    */
		S_IRGRP,S_IWGRP,S_IXGRP,   /* Zugriffsrechte Gruppe  */
		S_IROTH,S_IWOTH,S_IXOTH    /* Zugriffrechte der Rest */
	};
	struct tm  *ts;
	char creation_time[20];
	
	if (lstat(dir_entry_ptr->d_name, &stat_buf)) {
		return 1;
	}
	
	mode = stat_buf.st_mode;
	
	if (S_ISDIR(mode))
		flags[0] = 'd';
	
	for (i = 0; i < 9; i++)
		if (mode & bits[i])
			flags[i+1] = rwx[i % 3];

	ts = localtime(&(stat_buf.st_ctime));
	strftime(creation_time, 20, "%b %d %H:%M:%S", ts);
	
	printf("%s %3ld %10ld %s %s\n", flags, stat_buf.st_nlink, stat_buf.st_size, creation_time, dir_entry_ptr->d_name);
	
	return 0;
}
