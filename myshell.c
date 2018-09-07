/****************************************************************
 * Name        :                                                *
 * Class       : CSC 415                                        *
 * Date        :                                                *
 * Description :  Writting a simple bash shell program          *
 * 	          	  that will execute simple commands. The main   *
 *                goal of the assignment is working with        *
 *                fork, pipes and exec system calls.            *
 ****************************************************************/
//May add more includes
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>

//Max amount allowed to read from input
#define BUFFERSIZE 256
//Shell prompt
#define PROMPT "myShell "
//sizeof shell prompt
#define PROMPTSIZE sizeof(PROMPT)

#define NOCOMMANDS 7

#define READ  0
#define WRITE 1

static char* args[512];
pid_t child_pid;
static int n = 0;
 
static int pipe_(char* cmd, int input, int first, int last);

int parseInput(char* temp, char** parsed) {
	int k = 0;

	for (int i = 0; i < BUFFERSIZE; i++) {
		parsed[i] = strsep(&temp, " ");
		
		if (parsed[i] == NULL) {
			return k;
			break;
		}
		if (strlen(parsed[i]) == 0) {
            i--;
            k--;
		}
        k++;
	}
	return k;
}

int 
main(int argc, char** argv)
{
	int argType, stat, numArgs, bg = 0, x, out = 0, in = 0, f_desc0, f_desc1, p, first;
	char input[BUFFERSIZE];
	char* outfile, *infile, *cmd, *next, *work_dir;
	char* ListOfCmds[BUFFERSIZE];
	char* parsedInput[600];
	child_pid = 0;

	ListOfCmds[0] = "exit";
	ListOfCmds[1] = "cd";
	ListOfCmds[2] = "pwd";
	ListOfCmds[3] = "grep";
	ListOfCmds[4] = "ls";
	ListOfCmds[5] = "cat";
	ListOfCmds[6] = "wc";

	while (1) {
		cont:   
		bg = 0;
        out = 0;
   		in = 0;
   		
   		getwd(work_dir);
		printf("%s%s >> ", PROMPT, work_dir);
		fflush(NULL);
		wait(NULL);
		fgets(input, BUFFERSIZE, stdin);

		if (strcmp(input, "\n") == 0)
			continue;

		if(strcmp(input, "exit\n")==0) {
        	printf("Bye\n");
        	break;
    	}

    	p = 0;
		first = 1;
 
		cmd = input;
	if ((next = strchr(cmd, '|'))) {
		while (next != NULL) {
			*next = '\0';
			//parseInput(cmd, &cmd);
			p = pipe_(cmd, p, first, 0);
 
			cmd = next + 1;
			next = strchr(cmd, '|');
			first = 0;
		}
		p = pipe_(cmd, p, first, 1);
		for (int i = 0; i < n; ++i) 
			wait(NULL); 
		n = 0;
	}
	else {
		int length = strlen(input);
		if (input[length - 1] == '\n')
    		input[length - 1] = '\0';

    	numArgs = parseInput(input, parsedInput);

    	//printf("%s\n", parsedInput[0]);
    	//printf("%s\n", parsedInput[1]);
    	//printf("%s\n", &input[0]);
    	//printf("%s\n", parsedInput[2]);
    	//printf("%s\n", parsedInput[3]);
    	//printf("%ld\n", sizeof(ListOfCmds));
    	//printf("%d\n", numArgs);
    	//printf("%s\n", parsedInput[numArgs - 1]);

    	for (int i = 0; i < numArgs; i++) {
    		if (strcmp(parsedInput[i], ">") == 0 || strcmp(parsedInput[i], ">>") == 0) {
    			outfile = parsedInput[i + 1];
    			parsedInput[i] = NULL;
    			out = 1;
    			break;
    		}

    		if (strcmp(parsedInput[i], "<") == 0 || strcmp(parsedInput[i], "<<") == 0) {
    			infile = parsedInput[i + 1];
    			parsedInput[i] = NULL;
    			in = 1;
    			break;
    		}
    	}
    			//printf("Test\n");

    	//int k = 0;
    	

    	//printf("%s\n", outfile);
    	//printf("%s\n", infile);
    	//printf("%s\n", parsedInput[0]);
    	//printf("%s\n", parsedInput[1]);
    	//printf("%s\n", parsedInput[2]);
    	//printf("%d\n", STDOUT_FILENO);
    	//printf("%s\n", p[0].input[1]);
    	//printf("%s\n", p[1].input[1]);

    	
		for (int i = 0; i < NOCOMMANDS; i++) {
			if (strcmp(parsedInput[0], ListOfCmds[i]) == 0) {
				argType = i + 1;
				//printf("SUCCESS!!");
				break;
			} else if ((i + 1) == NOCOMMANDS) {
				printf("Not a valid command...\n");
				goto cont;
			}
		}

				//printf("Test\n");


		switch (argType) {
			case 1:
				printf("Bye\n");
				return 0;
			case 2:
			//printf("%s\n", parsedInput[numArgs - 1]);
				if (parsedInput[numArgs] == NULL) {
					chdir(getenv("HOME"));

					break;
				}
				chdir(parsedInput[numArgs - 1]);
				break;
			case 3:
				 getwd(work_dir);
				 printf("%s\n", work_dir);
				 break;
			default:
				if(strcmp(parsedInput[numArgs - 1], "&") == 0)
            	{
              		bg = 1;
              		parsedInput[numArgs - 1] = NULL;
              		//printf("%s\n", parsedInput[numArgs]);
            	}

					child_pid = fork();


				if(child_pid == 0 && out == 1) {
    				f_desc0 = creat(outfile, 0644);
    				out = 0;
    				dup2(f_desc0, STDOUT_FILENO);
    				close (f_desc0);
    				execvp(parsedInput[0], parsedInput);
    				
    				//exit(EXIT_SUCCESS);
       			}

       			if(child_pid == 0 && in == 1) {
    				f_desc1 = open(infile, O_RDONLY);
    				in = 0;
    				dup2(f_desc1, STDIN_FILENO);
    				close (f_desc1);
    				execvp(parsedInput[0], parsedInput);
    				
    				//goto cont;
    				//exit(EXIT_SUCCESS);
       			}

          		if(child_pid == 0 && bg == 1) {
              		execvp(parsedInput[0], parsedInput);
              		return 0;
            	} 
          		else if(child_pid == 0 && bg != 1 && out == 0) {
              		execvp(parsedInput[0], parsedInput);
            	}
          		else if(bg != 1 && out != 1){
              		waitpid(child_pid, &stat, WUNTRACED);
            	}
            	//free(parsedInput[0]);
          		
		}

	}
		
	
    }
return 0;
}
 
static int pipe_(char* cmd, int input, int first, int last) {
	while (isspace(*cmd)) ++cmd;
	char* next = strchr(cmd, ' ');
	int i = 0;
 
	while(next != NULL) {
		next[0] = '\0';
		args[i] = cmd;
		++i;
		//cmd = skipwhite(next + 1);
		next++;
		while (isspace(*next)) ++next;
		cmd = next;
		next = strchr(cmd, ' ');
	}
 
	if (cmd[0] != '\0') {
		args[i] = cmd;
		next = strchr(cmd, '\n');
		next[0] = '\0';
		++i; 
	}
 
	args[i] = NULL;

	if (args[0] != NULL) {
		n += 1;
		int pipes[2];
 
		pipe(pipes);	
		child_pid = fork();
 
		if (child_pid == 0) {
			if (first == 1 && last == 0 && input == 0) {
				dup2(pipes[WRITE], STDOUT_FILENO);
			} else if (first == 0 && last == 0 && input != 0) {
				dup2(input, STDIN_FILENO);
				dup2(pipes[WRITE], STDOUT_FILENO);
			} else {
				dup2(input, STDIN_FILENO);
			}
 
			if (execvp( args[0], args) == -1)
				_exit(EXIT_FAILURE); 
		}
 		
    	//printf("%s\n", outfile);
    	//printf("%s\n", infile);
    	//printf("%s\n", parsedInput[0]);
    	//printf("%s\n", parsedInput[1]);
    	//printf("%s\n", parsedInput[2]);
    	//printf("%d\n", STDOUT_FILENO);
    	//printf("%s\n", p[0].input[1]);
    	//printf("%s\n", p[1].input[1]);
    	
		if (input != 0) 
			close(input);
 
		close(pipes[WRITE]);
 
		if (last == 1)
			close(pipes[READ]);
 
		return pipes[READ];
	}
	return 0;
}
