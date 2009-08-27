#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/wait.h>

#include "alloc.h"
#include "shell.h"
#include "memcheck.h"

/*
Name: shell.c
Purpose:
	Hauptdatei fuer eigene Shell.

Autoren:
	Timo Graw
	Marcel Teuber

Version:
	0.5 - Speicherueberwachung mit memcheck.h
	0.4 - Praktikum05 - Pipes, Threadsafe str_tok_r()
	0.3 - Praktikum04 - Aufloesen von '~'
 	0.2 - changedir und Variablenaufloesung funktionsfaehig
	0.1 - start
*/

const char *VERSION = "0.5";

char *user;

/*
Function: main()

Purpose:
	Startfunktion

Parameters:
	int argc: Argumentcount
	char *argv[]: Arguments
	char *env[]: Enviroment Variables

Returns:
	int: Statuscode
*/
int main (int argc, char *argv[], char *env[]) {
	int_fast8_t prompt_errno;
	
	M_SETDEBUG(DL_Warning);
	
	user = getenv_malloc("USER");
	
	printf("\E[36mBeSh %s\E[0m (Betriebssysteme Shell) von \E[36mTimo Graw\E[0m & \E[33mMarcel Teuber\E[0m - Viel Spass!\n", VERSION);
	
	while ((prompt_errno = prompt()) == 0);
	
	M_FREE(user);
	
	if (M_GETUSAGE())
		printf("%u Bytes still in use\n", M_GETUSAGE());
	M_SHOW();
	
	if (prompt_errno != RET_EXIT)
		return prompt_errno;
	else
		return 0;
}

/*
Function: prompt()

Purpose:
	Fragt den Benutzer nach einem Kommando und fuehrt es aus

Returns:
	int: Statuscode
*/
int_fast16_t prompt() {
	/* Eingabezeile */
	char input_string[INPUTLINEBUFFER_LENGTH];
	
	/* Prompt ausgabe (farbig) */
	printf("\E[32m%s:\E[31m%s\E[0m>", user, getcwd(NULL, PATH_BUFFERLENGTH));

	/* Zeile einlesen */
	fgets(input_string, INPUTLINEBUFFER_LENGTH, stdin);

	/* newline character entfernen wenn vorhanden */
	if (input_string[strlen(input_string)-1] == '\n')
		input_string[strlen(input_string)-1] = '\0';

	/* check exit & cd */
	if (strcmp(input_string, "exit") == 0)
		return RET_EXIT;
	else if (strncmp(input_string, "cd", 2) == 0) {
		change_directory(input_string);
		return 0;
	}
	else if (strncmp(input_string, "memory", 6) == 0) {
		M_SHOW();
		return 0;
	}
	else {
		/* Neuen Prozess forken und Eingabe bearbeiten */
		return fork_process(input_string);
	}

}

/*
Function: change_directory()
	
Purpose:
	Parsed die Eingabe und aendert ggf. das Arbeitsverzeichnis.

Parameters:
	char *directory_string: cd befehlszeile
	
Returns:
	int: Statuscode
*/
int change_directory(char *directory_string) {
	int err;
	char *cd_param;
	char *tmp_ptr;
	char *last;
	
	/* Eingabe ueberpruefen und cd abschneiden */
	if (!strtok_r(directory_string, " ", &last) || !(cd_param = strtok_r_malloc(NULL, " ", &last))) {
		return -1;
	}
	
	/* String ersetzen (bei Nichtvorkommen wird NULL zurueckgegeben */
	tmp_ptr = string_replace(cd_param, "~", getenv("HOME"));
	
	/* ~ wurde gefunden */
	if (tmp_ptr) {
		M_FREE(cd_param);
		cd_param = tmp_ptr;
	}
	
	/* Verzeichnis wechseln */
	err = chdir(cd_param);
	
	/* ggf. Fehler ausgeben */
	if (err)
		perror("change_directory()");
		
	return err;
}

/*
Function: fork_proces()
	
Purpose:
	Forked Prozess um Eingabe zu bearbeiten.

Parameters:
	char *input_string: Eingabezeile
	
Returns:
	int: Statuscode
*/
int fork_process(char *input_string) {
	int pid;
	int stat_loc;
	char* pipe_commands[2];
	char *last;
	int pipe_fd[2];
	
	if (input_string == NULL) {
		fprintf(stderr, "fork_process(): Nullpointer uebergeben\n");
		return ERROR_NULLPOINTER;
	}
	
	/* Auf Pipe pruefen (character |) */
	pipe_commands[0] = strtok_r_malloc(input_string, "|", &last);
	if (pipe_commands[0]) {
		pipe_commands[1] = strtok_r_malloc(NULL, "|", &last);
		/* Pipe erstellen */
		if (pipe_commands[1])
			pipe(pipe_fd);
	}
	else
		pipe_commands[1] = NULL;
	

	/* Prozess forken */
	pid = fork();
	
	if (pid == -1) {
		/* Fork fehlgeschlagen */
		perror("fork_process()[1]");
		return errno;
	}
	else if (pid == 0) {
		/* Child Process 
		Eingabe bearbeiten und dann beenden */
		if (pipe_commands[1]) {
			/* Wenn Pipe benoetigt
			stdout an pipeeingang binden */
			if (dup2(pipe_fd[1], STDOUT_FILENO) < 0) {
				perror("fork_process()[1] dup2()");
				return errno;
			}
			
			/* unnoetige filedescriptors schliessen */
			close(pipe_fd[0]);
			close(pipe_fd[1]);
		}
		exit(handle_command(pipe_commands[0]));
	}
	else {
		/* Parent Process
		Auf Kind warten*/
		
		if (!pipe_commands[1]) {
				if (waitpid(pid, &stat_loc, 0) == -1) {
					perror("fork_process()[1] waitpid()");
					return errno;
				}
	
			if (!WIFEXITED(stat_loc)) {
				fprintf(stderr, "Kindprozess 1 wurde nicht normal beendet, Fehlercode: %i\n", WEXITSTATUS(stat_loc));
				return WEXITSTATUS(stat_loc);
			}
		}

		/* Wenn Pipe benoetigt */
		if (pipe_commands[1]) {
			/* Zweites Fork fuer zweiten Prozess */
			pid = fork();
			
			if (pid == -1) {
				/* Fork fehlgeschlagen */
				perror("fork_process()[2]");
				return errno;
			}
			else if (pid == 0) {
				/* Child Process 
				Eingabe bearbeiten und dann beenden
				stdin an Pipeeingang binden */
				if (dup2(pipe_fd[0], STDIN_FILENO) < 0) {
					perror("fork_process()[2] dup2()");
					return errno;
				}
				close(pipe_fd[1]);
				exit(handle_command(pipe_commands[1]));
			}
			else {
				/* Filedescriptoren schliessen */
				close(pipe_fd[0]);
				close(pipe_fd[1]);
				
				/* Auf Prozess warten */
				if (waitpid(pid, &stat_loc, 0) == -1) {
					perror("fork_process()[2] waitpid()");
					return errno;
				}
				
				if (!WIFEXITED(stat_loc)) {
					fprintf(stderr, "Kindprozess 2 wurde nicht normal beendet, Fehlercode: %i\n", WEXITSTATUS(stat_loc));
					return WEXITSTATUS(stat_loc);
				}
				
				M_FREE(pipe_commands[1]);
			}
		}
	}

	M_FREE(pipe_commands[0]);
	
	
	return 0;
}

/*
Function: handle_command()
	
Purpose:
	Kommandostring parsen und ausfuehren.

Parameters:
	char *command_string: Kommandostring
	
Returns:
	int: Statuscode
*/
int handle_command(char *command_string) {
	char *path;
	char *parameters[PARAMETERS_MAX];
	int32_t parameter_count;
	int status;
	
	if (command_string == NULL) {
		//printf("handle_command(): Nullpointer uebergeben\n");
		return ERROR_NULLPOINTER;
	}
	
	parameter_count = parse_parameters(command_string, parameters);
	
	if (parameter_count < 0)
		return parameter_count;
	
	path = parameters[0];
	
	if (path)
		status = execute_command(path, parameters);
	
	/* Speicher freigeben */
	for (; parameter_count < 0; parameter_count--)
		M_FREE(parameters[parameter_count]);
	//M_FREE(path);
	
	if (status)
		return status;
	
	return 0;
}

/*
Function: parse_parameters()
	
Purpose:
	Parsed einen Befehlsstring, loest Variable auf 
	und speichert die einzelnen Parameter in einem Array.

Parameters:
	char *command_string: Befehlsstring
	char *parameters[PARAMETERS_MAX]: Array zum speichern der Parameter
	
Returns:
	int32_t: Anzahl der Parameter/Fehlercode (negativ)
*/
int32_t parse_parameters(char *command_string, char *parameters[PARAMETERS_MAX]) {
	int n, i;
	int status;
	char *last;
	
	if (command_string == NULL || parameters == NULL) {
		fprintf(stderr, "parse_parameters(): Nullpointer uebergeben\n");
		return ERROR_NULLPOINTER;
	}
	
	n = 0;
	parameters[0] = strtok_r_malloc(command_string, " ", &last);
	if (parameters[0] == (char*)ERROR_MALLOCFAILED)
		return ERROR_MALLOCFAILED;
	
	/* Kommando ist nicht leer */
	if (parameters[0] != NULL) {
		
		do {
			n++;
			parameters[n] = strtok_r_malloc(NULL, " ", &last);
			if (parameters[n] == (char*)ERROR_MALLOCFAILED)
				return ERROR_MALLOCFAILED;
		} while (parameters[n] != NULL && n < PARAMETERS_MAX-2);
		/* Falls die Schleife durch Maximalanzahl beendet wurde fehl noch ein
			Nullzeiger */
		parameters[n] = NULL;
		
		/* Variablen aufloesen */
		for (i = 0; i < n; i++) {
			status = resolve_variable(&parameters[i]);
			if (status == ERROR_MALLOCFAILED)
				return ERROR_MALLOCFAILED;
		}
		
	}
	
	return n;
}

/*
Function: execute_command()

Purpose:
	Startet ein Program.

Parameters:
	char *path: Pfad zum Program
	char *parameters[PARAMETERS_MAX]: Parameterarray
	
Returns
	int: Statuscode
*/
int execute_command(char *path, char *parameters[PARAMETERS_MAX]) {
	int err;
	
	if (path == NULL || parameters == NULL) {
		fprintf(stderr, "execute_command(): Nullpointer uebergeben\n");
		return ERROR_NULLPOINTER;
	}
	
	/* Programm aufrufen */
	err = execvp(parameters[0], parameters);
	
	/* Bei Fehler ausgeben */
	if (err)
		perror("execute_command()");
	
	return err;
}

/*
Function: resolve_variable()

Purpose:
	Ersetzt, falls vorhanden, einen Variablenmarker oder ~ ($HOME)
	
Parameters:
	char **parameter: Parameter, der bearbeitet werden soll.
	
Returns:
	int: Statuscode
*/
int resolve_variable(char **parameter) {
	char *var_ptr;
	char *tmp_ptr;
	
	if (parameter == NULL || *parameter == NULL) {
		fprintf(stderr, "resolve_parameters(): Nullpointer uebergeben\n");
		return ERROR_NULLPOINTER;
	}
	
	/* Erstes Zeichen ist Variablenmakierung */
	if (*parameter[0] == '$') {
		/* Entsprechenden Variablenwert ermitteln */
		var_ptr = strtok_malloc(*parameter, "$");
		tmp_ptr = getenv_malloc(var_ptr);
		M_FREE(var_ptr);
		
		M_FREE(*parameter);
		*parameter = tmp_ptr;
		return 1;
	}
	/* Sonst auf ~ ueberpruefen */
	else {
		/* String ersetzen (bei Nichtvorkommen wird NULL zurueckgegeben */
		tmp_ptr = string_replace(*parameter, "~", getenv("HOME"));
		
		/* ~ wurde gefunden und ersetzt*/
		if (tmp_ptr) {
			M_FREE(*parameter);
			*parameter = tmp_ptr;
		}
		return 0;
	}
}


