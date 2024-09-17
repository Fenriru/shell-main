#define  _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <assert.h>

#include "hw2.h"

#define NARGS 1024
#define BMAX 1024

static void Trim(char ** filename) {
	char * start = *filename;
	while (*start == ' ')
		start++;
	*filename = start;
	
	char * end = start;
	while (*end != ' ' && *end != '\0')
		end++;
	*end = '\0';
}

static void echo(char * argv[]) {
	int index = 1;
	while (argv[index] != NULL) {
		printf("%d|%s\n", index, argv[index]);
		index++;
	}
}

static int Open(char * fname, int opts, int permissions) {
	int fd = open(fname, opts, permissions);
	if (fd < 0) {
		perror("open");
		exit(1);
	}
	return fd;
}

static void Dup2(int old, int new)
{
    if (dup2(old, new) < 0) {
        perror("dup2");
        exit(1);
    }
}

int Dup(int old_fd)
{
    int fd = dup(old_fd);
    if (fd < 0) {
        perror("dup");
        exit(1);
    }
    return fd;
}

void execute(char * input)
{
	//added by me if user only hits ENTER
	if (strlen(input) == 0) {
		return;
	}
	//
    char * sep = NULL;

    if((sep=strstr(input, "2>"))) {
        *sep = 0;
        char * fileout = sep + 2;

	Trim(&fileout);
	int fd = Open(fileout, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        Dup2(fd, STDERR_FILENO);
	
	// TODO: stderr redirect
    }

    if((sep=strstr(input,">"))) {
        *sep = 0;
        char * fileout = sep + 1;
	
	Trim(&fileout);
        int fd = Open(fileout, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	Dup2(fd, STDOUT_FILENO);

	// TODO: stdout redirect
    }

    if((sep=strstr(input,"<"))) {
        *sep = 0;
        char * filein = sep + 1;
	
	Trim(&filein);
        int fd = Open(filein, O_RDONLY, 0);
	Dup2(fd, STDIN_FILENO);
	
	// TODO: stdin redirect
    }
    // TODO: parse the command line args
    char * cmd = strtok(input," \n");

    if (cmd == NULL) return;

    char * argv[NARGS];
    argv[0] = cmd;

    int i = 1;
    char * word = NULL;
    while (i < NARGS - 1 && (word = strtok(NULL, " \n")))
	    argv[i++] = word;
    argv[i] = NULL;
    
    if (!strcmp("echo", cmd)) {
	echo(argv);
        // TODO: built-in echo
	// OLD CODE: COMMENTED OUT
	//char * tok = NULL;
	//tok = strtok(NULL, " ");
	//int i = 1;
	//while (tok != NULL) {
		//printf("%d|%s\n", i, tok);
		//i += 1;
		//tok = strtok(NULL, " ");
	//}
	//fflush(stdout);

    } else {
        // TODO: execute the user's command
	//OLD CODE:
	//char * argv[1024];
	//argv[0] = cmd;	
	//execv(cmd, argv);
	//char * paths = getenv("PATH");
	//static char buffer[1024];
	//snprintf(buffer, BMAX, "%s/%s", paths, cmd);
	//execv(buffer, argv);
	
	//pid_t cpid = fork();
	//if (cpid) {
		//waitpid(cpid, NULL, 0);
	//} else {
		//char * args[3];
		//args[0] = cmd;
		//char * tok = strtok(NULL, "\n");
		//args[1] = tok;
		//args[2] = NULL;

		//if (args[0][0] != '/') {
			//char * path = strtok(getenv("PATH"), ":");
			//int NPATHS = sizeof(path)/sizeof(char*);
			//execv(cmd, args);
			//while (path != NULL) {
				//char filename[64];
				//snprintf(filename, 64, "%s/%s", path, cmd);
				//execv(filename, args);
				//path = strtok(NULL, ":");
			//}
		//} else {
			//execv(cmd, args);
		//}
	execv(cmd, argv);

	char * paths = getenv("PATH");
	char * path = strtok(paths, ":");

	static char buffer[BMAX];
	do {
		snprintf(buffer, BMAX, "%s/%s", path, cmd);
		execv(buffer, argv);
		path = strtok(NULL, ":");
	} while (path);
	fprintf(stderr,"cs361sh: command not found: %s\n",cmd);
        fflush(stderr);
	exit(0);
	//}
	
        //fprintf(stderr,"cs361sh: command not found: %s\n",cmd);
        //fflush(stderr);
    }
    return;
}



void pipeline(char * head, char * tail)
{
    // TODO
    int fds[2];
    pipe(fds);

    int readfds = fds[0];
    int writefds = fds[1];

    pid_t cpid = fork();
    if (cpid == 0) {
	    close(readfds);
	    Dup2(writefds, STDOUT_FILENO);
	    execute(head);
	    exit(0);
    } else {
	    waitpid(cpid, NULL, 0);
	    close(writefds);
	    int orig_in = dup(STDIN_FILENO);
	    Dup2(readfds, STDIN_FILENO);
	    run_cmd(tail);
	    Dup2(orig_in, STDIN_FILENO);
    }
}


void sequence(char * head, char * tail)
{
    // TODO
    
    //execute(head);
    //execute(tail);
    run_cmd(head);
    run_cmd(tail);
    
    fflush(stderr);
}


void run_cmd(char * line)
{
    char *sep = 0;

    if((sep=strstr(line,";"))) {
        //OLD CODE:
	*sep = 0;
	sequence(line, sep+1);
	//sep = strstr(sep, ";");

        // TODO: sequence
    }
    else if((sep=strstr(line,"|"))) {
        *sep = 0;
	pipeline(line, sep+1);


	//int filedes[2];
	//pipe(filedes);
	//int pipe_in = filedes[0];
	//int pipe_out = filedes[1];

	//pid_t cpid = fork();
	//if (cpid == 0) {
		//char* argv[5];
		//char * cmd = strtok(line," \n");
		//char * tok = strtok(NULL, " ");
		//argv[0] = cmd;
		//argv[1] = tok;
		//Dup2(filedes[1], STDOUT_FILENO);
		//execv(cmd, argv);
	//} else {
		//waitpid(cpid, NULL, 0);
	//}
	//cpid = fork();
	//if (cpid == 0) {
		//char* argv[2];
		//char * cmd = strtok(line," \n");
		//char * tok = strtok(NULL, " ");
		//argv[0] = cmd;
		//argv[1] = NULL;
		//Dup2(filedes[0], STDIN_FILENO);
		//execv(cmd, argv);
		//exit(1);
	//} else {
		//waitpid(cpid, NULL, 0);
	//}

        // TODO: pipeline
    }
    else {
        // TODO: executing the user's command
        pid_t cpid = fork();
	if (cpid == 0) {
		execute(line);
		exit(1);
	} else {
		waitpid(cpid, NULL, 0);
	}
	
    }
}


int main(int argc, char * argv[])
{
    printf("cs361sh> ");
    fflush(stdout);

    // handy copies of original file descriptors

    int orig_in = dup(0);
    int orig_out = dup(1);
    int orig_err = dup(2);

    char line[BSIZE];

    while(read_line(STDIN_FILENO, line, BSIZE) > 0)  {

        // make sure line ends with null
        line[strlen(line)-1] = '\0';

        run_cmd(line);

        // TODO: possibly fix redirection related changes

        printf("cs361sh> ");
        fflush(stdout);
   }

    printf("\n");
    fflush(stdout);
    fflush(stderr);
    return 0;
}


/*
    read_line will read one line from stdin
    and  store that  line in  buf.  It will
    include the trailing newline in buf.
*/

int read_line(int fd, char buf[], int max)
{
    if (max <= 0)
        return 0;

    int i = 0;
    while (i < max - 1) {
        char c;
        size_t amt = read(fd, &c, 1);
        if (amt != 1)
            break;
        buf[i++] = c;
        if (c == '\n')
            break;
    }
    buf[i] = '\0';

    return i;
}
