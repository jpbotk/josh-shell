#include<stdio.h> 
#include<string.h> 
#include<stdlib.h> 
#include<unistd.h> 
#include<sys/types.h> 
#include<sys/wait.h> 
#include<readline/readline.h> 
#include<readline/history.h> 
  
#define MAXCHAR 1000 // max number of letters to be supported 
#define MAXCMD 100 // max number of commands to be supported 
  

void clear() {	// function to clear the screen
	printf("\e[1;1H\e[2J");
}
  
int getInput(char* string) // Function to get user input after prompt "0_0 >"
{ 
    char* in;   
    in = readline(" 0_o >"); //input is after prompt

    if (strlen(in) != 0) { 
        add_history(in); //adds user input to history buffer
        strcpy(string, in); //set string to input val
        return 0; 
    } else { 
        return 1; //no input given
    } 
} 
  

void printDir() // Function to get cwd as string/print
{ 
    char cwd[1024]; 
    getcwd(cwd, sizeof(cwd)); 
    printf("\n%s", cwd); 
} 

void execute(char** parse) // executes non piped commands supported by jsh
{ 

    pid_t proID = fork();  //process ID is that of new child being forked
  
    if (proID == -1) { //process not forked properly
        printf("\nForking issue"); 
        return; 
    } else if (proID == 0) { //process not supported by shell
        if (execvp(parse[0], parse) < 0) { 
            printf("\n%s not a supported command.\n",*parse); 
        } 
        exit(0); 
    } else { 
        wait(NULL);  //wait for child to be terminated
        return; 
    } 
} 
  

void executePiped(char** parsed, char** parsedpipe) 
{ // Function where the piped system commands is executed 
    // 0 is read end, 1 is write end 
    int p[2];  
    pid_t p1, p2; 
  
    if (pipe(p) < 0) { 
        printf("\nPipe could not be initialized"); 
        return; 
    } 
    p1 = fork(); 
    if (p1 < 0) { 
        printf("\nCould not fork"); 
        return; 
    } 
  
    if (p1 == 0) { 
        // Child 1 executing.. 
        // It only needs to write at the write end 
        close(p[0]); //deallocate file number
        dup2(p[1], STDOUT_FILENO); //replace file number at p[1] with 1
//STDERR_FILENO
//    File number of stderr; 2.
//STDIN_FILENO
//    File number of stdin; 0.
//STDOUT_FILENO
//    File number of stdout; 1.
        close(p[1]); 
  
        if (execvp(parsed[0], parsed) < 0) { 
            printf("\nProblem with first command"); 
            exit(0); 
        } 
    } else { 
        p2 = fork(); //execute parent process
  
        if (p2 < 0) { 
            printf("\nCould not fork"); 
            return; 
        } 
  
        if (p2 == 0) { //child2 needs to read at the end
            close(p[1]); 
            dup2(p[0], STDIN_FILENO); //same process described above
            close(p[0]); 
            if (execvp(parsedpipe[0], parsedpipe) < 0) { 
                printf("\nProblem with second command"); 
                exit(0); 
            } 
        } else { //parent is executing and must wait for children
            wait(NULL); 
            wait(NULL); 
        } 
    } 
} 
 
void printCommands() // Help command builtin 
{ 
    printf("\nSupported commands:"
	"\n-hello"
	"\n-help"
        "\n-cd"
        "\n-ls"
        "\n-exit"
        "\n-pipe handling"
        "\n-improper space handling"
	"\n-bash builtins(not a command)"); 
  
    return; 
} 

void start() // starting shell message  
{ 
    clear();
    printf("~~~~~~~~~~\tJosh shell\t~~~~~~~~~~\n\n");
    char* user = getenv("USER"); 
    printf("\nWelcome %s! \n", user); 
    printCommands();
    printf("\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
} 
  
// Josh shell builtins
int builtIns(char** parsed) 
{ 
    int numBI = 4; //# of builtins
    int cmdChoice = 0; //command chosen
    char* bis[numBI]; //available builtins
    char* user = getenv("USER");; 
  
    bis[0] = "exit";
    bis[1] = "cd"; 
    bis[2] = "help"; 
    bis[3] = "hello"; 
  
    for (int i = 0; i < numBI; i++) { 
        if (strcmp(parsed[0], bis[i]) == 0) { //compare user input to all strings in BI, if match then execute that commands function
            cmdChoice = i + 1; 
            break; 
        } 
    } 
  
    switch (cmdChoice) { 
    case 1: 
        printf("\nTerminating Josh Shell :(\n"); 
        exit(0); 
    case 2: 
        chdir(parsed[1]); 
        return 1; 
    case 3: 
        printCommands(); 
        return 1; 
    case 4: 
        printf("\nHello %s! :)\n",user);
        return 1; 
    default: 
        break; 
    } 
  
    return 0; 
} 
int parsePipe(char* str, char** piped) // parsing for pipes
{ 
    for (int i = 0; i < 2; i++) { 
        piped[i] = strsep(&str, "|"); 
        if (piped[i] == NULL) 
            break; 
    } 
  
    if (piped[1] == NULL) 
        return 0; // returns zero if no pipe is found
    else { 
        return 1; 
    } 
} 

void parseSpace(char* string, char** parsed) // parses commands by spaces
{   
    for (int i = 0; i < MAXCMD; i++) { //loops throuch all char vals finding spaces to parse 
        parsed[i] = strsep(&string, " "); 
  
        if (parsed[i] == NULL) 
            break; 
        if (strlen(parsed[i]) == 0) 
            i--; 
    } 
} 
  
int analyzeInput(char* string, char** parsed, char** parsedpipe) //checks input for pipe vals to pick execution route
{ 
  
    char* strpiped[2]; 
    int piped = 0; 
  
    piped = parsePipe(string, strpiped); 
  
    if (piped) { 
        parseSpace(strpiped[0], parsed); 
        parseSpace(strpiped[1], parsedpipe); 
  
    } else { 
  
        parseSpace(string, parsed); 
    } 
  
    if (builtIns(parsed)) 
        return 0; 
    else
        return 1 + piped; 
} 
  
void main() 
{ 
    char input[MAXCHAR]; //holds entire sring input as char array (easier to parse)
    char* parse[MAXCMD]; //holds parse vals for standard commands
    char* parsePipe[MAXCMD]; //holds for piped commands
    int pipe = 0; //c style boolean value

    start(); //print welcome message
  
    while (1) { //while true (infinite loop)
        printDir();  //print cwd
        if (getInput(input)) //check input is not null, set input to val after prompt
            continue; 
        // process 
        pipe = analyzeInput(input, parse, parsePipe); //check input for pipe prescence (1 for no pipe, 2 for pipe)
        if (pipe == 1) //no pipe so do standard execute function
            execute(parse);  
        if (pipe == 2) //pipe present so do special execution
            executePiped(parse, parsePipe); 
    }  
} 
