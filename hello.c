//la compilare scriem gcc-o hello hello.c -lreadline

#include<stdio.h> 
#include<string.h> 
#include<stdlib.h> 
#include<unistd.h> 
#include<sys/types.h> 
#include<sys/wait.h> 
#include<readline/readline.h> 
#include<readline/history.h> 

#define MAXCOM 1000 // numarul maxim de litere ale comenzii
#define MAXLIST 100 // numarul maxim de comenzi 

// Curatarea shell ului
#define clear() printf("\033[H\033[J") 

void shell() 
{ 
	clear(); 
	printf("Shell: \n");  
	sleep(2); 
	clear(); 
} 
int citire(char* str) 
{ 
	char* buf; 

	buf=readline("\n$ "); 
	if (strlen(buf) != 0) { 
		add_history(buf); 
		strcpy(str, buf); 
		return 0; 
	} else { 
		return 1; 
	} 
} 
//current directory 
void cd() 
{ 
	char cwd[1024]; 
	getcwd(cwd, sizeof(cwd)); 
	printf("\nDir: %s\n", cwd); 
} 
//functia in care sistemul de comanda este excutat 
void pid_arg1(char** v) 
{ 
	// creaza un nou proces 
	pid_t pid = fork(); 

	if (pid == -1) { 
		printf("\nFailed forking child.."); 
		return; 
	} else if (pid == 0) { 
		if (execvp(v[0], v) < 0) { 
			printf("\nCould not execute command"); 
		} 
		exit(0); 
	} else { 
		// asteapta copilul sa termine 
		wait(NULL); 
		return; 
	} 
} 

// executia functiilor care il contin pe pipe
void pid_arg2(char** v, char** vpipe) 
{ 
	// 0 e citirea sfarsitului, 1 scrierea sfarsitului
	int pipefd[2]; 
	pid_t p1, p2; 

	if (pipe(pipefd) < 0) { 
		printf("\nPipe could not be initialized"); 
		return; 
	} 
	p1 = fork(); 
	if (p1 < 0) { 
		printf("\nCould not fork"); 
		return; 
	} 

	if (p1 == 0) { 
		// Primul copil aka scrierea la sfarsit
		close(pipefd[0]); 
		dup2(pipefd[1], STDOUT_FILENO); 
		close(pipefd[1]); 

		if (execvp(v[0], v) < 0) { 
			printf("\nCould not execute command 1"); 
			exit(0); 
		} 
	} else { 
		// Parintele 
		p2 = fork(); 

		if (p2 < 0) { 
			printf("\nCould not fork"); 
			return; 
		} 

		// al doilea copil aka citirea de la sfarsit

		if (p2 == 0) { 
			close(pipefd[1]); 
			dup2(pipefd[0], STDIN_FILENO); 
			close(pipefd[0]); 
			if (execvp(vpipe[0], vpipe) < 0) { 
				printf("\nCould not execute command 2"); 
				exit(0); 
			} 
		} else { 
			// parintele asteapta cei doi copii
			wait(NULL); 
			wait(NULL); 
		} 
	} 
} 

// HEEELP
void Help() 
{ 
	printf("\n~~~HELP~~~"
		"\nList of Commands supported:"
		"\n~cd"
		"\n~ls"
		"\n~exit"
		"\n~history"
		"\n~cd"
		"\n~echo\n"); 

	return; 
} 
//istoricul terminalului
void history(){
system("cat ~/.bash_history");
}

// functia prin care apelez comenzile 
int cmnds(char** v) 
{ 
	int nr = 5, i, arg = 0; 
	char* list_cmnds[nr];  

	list_cmnds[1] = "exit"; 
	list_cmnds[2] = "cd"; 
	list_cmnds[3] = "help"; 
	list_cmnds[4] = "echo"; 
        list_cmnds[5] = "history";
	char* s;
	for (i = 1; i <= nr; i++) { 
		if (strcmp(v[0], list_cmnds[i]) == 0) 
			break;
	} 

	switch (i) { 
	case 1: 
		sleep(1); 
		exit(0); 
	case 2: 
 
		cd();
		return 1; 
	case 3: 
		Help(); 
		return 1; 
	case 4: 
		s=readline("\n$ ");
		printf("%s",s);
 
		return 1; 
	case 5: 
		history(); 
		return 1;
	default: 
		break; 
	} 

	return 0; 
} 


//procesarea stringului dat 
int procesare(char* string,char** v)
{
	
	int i=0;
	v[i]=strtok(string,"|");
	while(v[i]!=NULL)
	{	
		if (cmnds(v)) 
		{
			i++;
			v[i]=strtok(NULL,"|");
			
		}
	else
		return 1; 
	}
return 0;
}

int main() 
{ 
	char string[MAXCOM], * args[MAXLIST]; 
	char* argspipe[MAXLIST]; 	
	int exec = 0; 
	shell(); 

	while (1) { 
		if (citire(string)) 
			continue; 
 
		exec = procesare(string, args); //returneaza 0 daca nu exista nicio comanda implementata

		// pentru o singura comanda 
		if (exec == 1) 
			pid_arg1(args); 

		if (exec == 0) //daca este folosit un pipe
			pid_arg2(args, argspipe); 
	} 
	return 0; 
} 

