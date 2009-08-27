#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/wait.h>

#include "alloc.h"
#include "shell.h"

/*
Name: shell.c
Purpose:
	Hauptdatei fuer eigene Shell.

Autoren:
	Timo Graw
	Marcel Teuber

Version:
	0.3 - Praktikum04 - Aufloesen von '~' 
 	0.2 - changedir und Variablenaufloesung funktionsfaehig
	0.1 - start
*/

const char *VERSION = "0.3";

char *user;
char *pwd;

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
	
	user = getenv_malloc("USER");
	
	printf("\E[36;40mBeSh %s\E[0m (Betriebssysteme Shell) von \E[36;40mTimo Graw\E[0m & \E[33;40mMarcel Teuber\E[0m - Viel Spass!\n", VERSION);
	
	while ((prompt_errno = prompt()) == 0);
	
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
	
	/* Current Working Directory freigeben und neu ermitteln */
	free(pwd);	
	pwd = getcwd(NULL, PATH_BUFFERLENGTH);
	
	/* Prompt ausgabe (farbig) */
	printf("\E[32;40m%s:\E[31;40m%s\E[0m>", user, pwd);

	/* Zeile einlesen */
	fgets(input_string, INPUTLINEBUFFER_LENGTH, stdin);

	/* newline character entfernen wenn vorhanden */
	if (input_string[strlen(input_string)-1] == '\n')
		input_string[strlen(input_string)-1] = '\0';

	/* check exit & cd */
	if (strcmp(input_string, "exit") == 0)
		return RET_EXIT;
	else if (strncmp(input_string, "cd", 2) == 0) {
		return change_directory(input_string);
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
	
	/* Eingabe ueberpruefen und cd abschneiden */
	if (!strtok(directory_string, " ") || !(cd_param = strtok_malloc(NULL, " "))) {
		return -1;
	}
	
	/* String ersetzen (bei Nichtvorkommen wird NULL zurueckgegeben */
	tmp_ptr = string_replace(cd_param, "~", getenv("HOME"));
	
	/* ~ wurde gefunden */
	if (tmp_ptr) {
		free(cd_param);
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
	
	if (input_string == NULL) {
		printf("fork_process(): Nullpointer uebergeben\n");
		return ERROR_NULLPOINTER;
	}
	
	pid = fork();
	
	if (pid == -1) {
		/* Fork fehlgeschlagen */
		perror("fork_process()");
		return errno;
	}
	else if (pid == 0) {
		/* Child Process 
		Eingabe bearbeiten und dann beenden */
		exit(handle_command(input_string));
	}
	else {
		/* Parent Process
		Auf Kind warten*/
		waitpid(pid, &stat_loc, 0);
		
		if (!WIFEXITED(stat_loc)) {
			printf("Kindprozess wurde nicht normal beendet, Fehlercode: %i\n", WEXITSTATUS(stat_loc));
			return WEXITSTATUS(stat_loc);
		}
	}
	
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
		printf("handle_command(): Nullpointer uebergeben\n");
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
		free(parameters[parameter_count]);
	free(path);
	
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
	
	if (command_string == NULL || parameters == NULL) {
		printf("parse_parameters(): Nullpointer uebergeben\n");
		return ERROR_NULLPOINTER;
	}
	
	n = 0;
	parameters[0] = strtok_malloc(command_string, " ");
	if (parameters[0] == (char*)ERROR_MALLOCFAILED)
		return ERROR_MALLOCFAILED;
	
	/* Kommando ist nicht leer */
	if (parameters[0] != NULL) {
		
		do {
			n++;
			parameters[n] = strtok_malloc(NULL, " ");
			if (parameters[n] == (char*)ERROR_MALLOCFAILED)
				return ERROR_MALLOCFAILED;
		} while (parameters[n] != NULL && n < PARAMETERS_MAX-1);
		
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
		printf("execute_command(): Nullpointer uebergeben\n");
		return ERROR_NULLPOINTER;
	}
	
	/* Programm aufrufen */
	err = execvp(path, parameters);
	
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
	char *tmp_ptr;
	
	if (parameter == NULL || *parameter == NULL) {
		printf("resolve_parameters(): Nullpointer uebergeben\n");
		return ERROR_NULLPOINTER;
	}
	
	/* Erstes Zeichen ist Variablenmakierung */
	if (*parameter[0] == '$') {
		/* Entsprechenden Variablenwert ermitteln */
		tmp_ptr = getenv_malloc(strtok(*parameter, "$"));
		
		free(*parameter);
		*parameter = tmp_ptr;
		return 1;
	}
	/* Sonst auf ~ ueberpruefen */
	else {
		/* String ersetzen (bei Nichtvorkommen wird NULL zurueckgegeben */
		tmp_ptr = string_replace(*parameter, "~", getenv("HOME"));
		
		/* ~ wurde gefunden */
		if (tmp_ptr) {
			free(*parameter);
			*parameter = tmp_ptr;
		}
		return 0;
	}
}


