#ifndef LIST_H
#define LIST_H

int list_content(char *directory, int show_systemfiles, int show_details);
int print_dir_entry(struct dirent *dir_entry_ptr, int show_systemfiles, int show_details);
int is_systemfile(char *filename);
void print_dir_entry_normal(struct dirent *dir_entry_ptr);
int print_dir_entry_long(struct dirent *dir_entry_ptr);

#endif

