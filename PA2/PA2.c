#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <pthread.h>
#define ARRAY_LENGTH(arr) (sizeof(arr) / sizeof((arr)[0]))
#define MAX_SIZE 20

// Defining a fixed size array to handle push back operations dynamic memory will be cause a lot of problems
typedef struct
{
    int array[MAX_SIZE];
    size_t size;
} ProcessArray;
typedef struct
{
    pthread_t array[MAX_SIZE];
    size_t size;
} ThreadArray;
ProcessArray initFixedArray();
ProcessArray initFixedArray()
{
    ProcessArray fixedArray;
    fixedArray.size = 0;
    return fixedArray;
}
ThreadArray initThreadArray();
ThreadArray initThreadArray()
{
    ThreadArray fixedArray;
    fixedArray.size = 0;
    return fixedArray;
}

// To keep track of processes and threads

// Creating a push back function to handle the processes
void pushBack_f(ProcessArray *fixedArray, int element)
{
    // Check if the array is not full
    if (fixedArray->size < MAX_SIZE)
    {
        // Add the new element to the end
        fixedArray->array[fixedArray->size++] = element;
    }
}
// Creating a push back function to handle the threads

void pushBack_t(ThreadArray *threadArray, pthread_t thread)
{
    // Check if the array is not full
    if (threadArray->size < MAX_SIZE)
    {
        // Add the new element to the end
        threadArray->array[threadArray->size++] = thread;
    }
}

pthread_mutex_t myMutex = PTHREAD_MUTEX_INITIALIZER; // mutex for threads

void *listener(void *arg) // Argument getting the information from pipe
{
    int pipe_fd1 = *((int *)arg);
    int islistening = 1;
    char pipe_line[256];
    FILE *pipestream;

    while (islistening) // try to acquire the lock
    {
        pthread_mutex_lock(&myMutex);
        pipestream = fdopen(pipe_fd1, "r");

        if (pipestream) // check the pipestream
        {
            printf("----  %lu\n", (unsigned long)pthread_self());
            fflush(stdout);

            if (fgets(pipe_line, sizeof(pipe_line), pipestream) != NULL) // check the lines
            {
                printf("%s", pipe_line);
                fflush(stdout);

                while (fgets(pipe_line, sizeof(pipe_line), pipestream) != NULL) // get the lines
                {
                    printf("%s", pipe_line);
                    fflush(stdout);
                }
            }

            printf("----  %lu\n", (unsigned long)pthread_self());
            fflush(stdout);
            islistening = 0;
        }

        pthread_mutex_unlock(&myMutex); // release the lock for other threads
        fclose(pipestream);
    }

    pthread_exit(NULL);

    return NULL;
}

int main(void)
{
    // Tried a dynamic version here
    //  int arraysize = 10;
    //  int thread_arraysize = 10;
    /*
    int *myArray = malloc(sizeof(int) * arraysize);
    pthread_t *thread_Array = malloc(sizeof(pthread_t) * arraysize);*/
    // int myArray[15];
    // pthread_t thread_Array[15]={0};

    // Transition to fixed size arrays
    int process_counter = 0;
    ProcessArray processarr = initFixedArray();
    ThreadArray threadarr = initThreadArray();

    char line[1024] = {0};
    char *filename = "commands.txt";
    FILE *file_ptr = fopen(filename, "r");
    int parse_txt = open("parse.txt", O_CREAT | O_WRONLY | O_TRUNC); // parse commands

    if (file_ptr == NULL)
    {
        printf("File cannot be opened \n");
    }
    // Parsing time
    while (fgets(line, sizeof(line), file_ptr) != NULL)
    {
        char *Command = "";
        char *Inputs = "";
        char *Options = "";
        char *Redirection = "-";
        char *Background_Job = "n";
        char *new = strtok(line, " \t\n");
        // we can always assume that the first word will be our cmd-name
        Command = new;
        int counter = 0;
        char *cmd_array[4] = {0}; // command,option,input,redirection,if exists the filename, background info, NULL
        char *redirected_input_name = "";
        char *redirected_output_name = "";
        cmd_array[0] = strdup(Command);
        new = strtok(NULL, " \t\n");

        while (new != NULL)
        {
            int check_contraint = 0; // to understand wheter it is a option or input

            /*for (i = 0; i < strlen(new); i++) // if there is dot in the word it must be an option
            {
                if (new[i] == '.')
                {
                    check_contraint2++;//
                }
            }*/
            int i;
            for (i = 0; i < strlen(new); i++) // if there is dash in the word it must be an option
            {
                if (new[i] == '-')
                {
                    check_contraint++;
                }
            }
            if (check_contraint) // there is a dash so it's an option
            {
                Options = new;
            }
            else if ((*new != '>') && (*new != '<') && (*new != '&')) // There isnt any dash and selected special chars so its input
            {
                Inputs = new;
            }
            else // checking and setting the special characters
            {
                if (*new == '>')
                {
                    Redirection = ">";
                    new = strtok(NULL, " \t\n");
                    redirected_output_name = new;
                }
                else if (*new == '<')
                {
                    Redirection = "<";
                    new = strtok(NULL, " \t\n");
                    redirected_input_name = new;
                }
                else if (*new == '&')
                {
                    Background_Job = "y";
                }
                else
                {
                    redirected_input_name = new;
                }
            }

            counter++;
            new = strtok(NULL, " \t\n");
        }

        // Setup the command array

        if (!strcmp(Inputs, ""))
        {
            cmd_array[1] = NULL;
        }
        else
        {
            cmd_array[1] = strdup(Inputs);
        }
        if (!strcmp(Options, ""))
        {
            cmd_array[2] = NULL;
        }
        else
        {
            cmd_array[2] = strdup(Options);
        }
        cmd_array[3] = NULL;

        /*
                if (!strcmp(Redirection, "-"))
                {
                    cmd_array[3] = NULL;
                    cmd_array[4] = redirected_input_name;
                }
                else if (!strcmp(Redirection, "<"))
                {
                    cmd_array[3] = strdup(Redirection);
                    cmd_array[4] = redirected_input_name;
                }
                else if (!strcmp(Redirection, ">"))
                {
                    cmd_array[3] = strdup(Redirection);
                    cmd_array[4] = redirected_output_name;
                }
                if (!strcmp(Background_Job, "y"))
                {
                    cmd_array[5] = strdup("&");
                }
                else
                {
                    cmd_array[5] = NULL;
                }
                cmd_array[6] = NULL;*/
        int saved_stdout = dup(STDOUT_FILENO);
        dup2(parse_txt, STDOUT_FILENO);
        // printing out the command to the parse.txt file
        // dup2(parse_txt, STDOUT_FILENO);
        printf("----------\n");
        printf("Command : %s\n", Command);
        printf("Inputs : %s\n", Inputs);
        printf("Options: %s\n", Options);
        printf("Redirection: %s\n", Redirection);
        printf("Background Job: %s\n", Background_Job);
        printf("----------\n");
        fflush(stdout);
        dup2(saved_stdout, STDOUT_FILENO);
        close(saved_stdout);

        // There is a need to consider if it is a wait command or not

        if (!strcmp(Command, "wait"))
        { // if wait then the process should wait for all the existing processes
            int i;
            for (i = 0; i < processarr.size; i++)
            {
                waitpid(processarr.array[i], NULL, 0);
            }

            for (i = 0; i < threadarr.size; i++)
            {
                pthread_join(threadarr.array[i], NULL);
            }
        }
        else
        {

            if (!strcmp(Redirection, "-")) // need of threads
            {
                pthread_t thread1;
                int pipe_fd[2];
                pipe(pipe_fd);
                process_counter++;

                int command_process = fork();
                if (command_process < 0)
                { // fork failed
                    fprintf(stderr, "Fork failed");
                    exit(1);
                }
                else if (command_process == 0)
                {                                    // child process
                    close(pipe_fd[0]);               // Close Read-end
                    dup2(pipe_fd[1], STDOUT_FILENO); // STDOUT to write end
                    close(pipe_fd[1]);               // Close write end
                    execvp(cmd_array[0], cmd_array);
                }
                else
                {
                    close(pipe_fd[1]);
                    /*thread_arraysize++;
                    thread_Array = realloc(thread_Array, thread_arraysize * sizeof(pthread_t));*/
                    pthread_create(&thread1, NULL, listener, (void *)&pipe_fd[0]);
                    if (!strcmp(Background_Job, "n"))
                    {
                        waitpid(command_process, NULL, 0);
                        pthread_join(thread1, NULL);
                    }
                    else
                    {
                        pushBack_t(&threadarr, thread1);
                        pushBack_f(&processarr, command_process);

                        // thread_Array[thread_arraysize - 1] = thread1;

                        // arraysize++;
                        /*myArray = realloc(myArray, arraysize * sizeof(int));
                        myArray[arraysize - 1] = command_process;*/
                    }
                }
            }

            else if (!strcmp(Redirection, ">")) // this will be our exception we do not need a threads for this
            {
                process_counter++;
                int command_process = fork();

                if (command_process < 0)
                { // fork failed
                    fprintf(stderr, "Fork failed");
                    exit(1);
                }
                else if (command_process == 0)
                { // child process
                    int file = open(redirected_output_name, O_CREAT | O_WRONLY | O_TRUNC);
                    dup2(file, STDOUT_FILENO);
                    close(file);
                    execvp(cmd_array[0], cmd_array);
                }
                else
                {
                    if (!strcmp(Background_Job, "n"))
                    {
                        waitpid(command_process, NULL, 0);
                    }
                    else
                    {
                        pushBack_f(&processarr, command_process);

                        // arraysize++;
                        /*myArray = realloc(myArray, arraysize * sizeof(int));
                        myArray[arraysize - 1] = command_process;*/
                    }
                }
            }
            else if (!strcmp(Redirection, "<")) // need of threads
            {
                pthread_t thread1;
                int pipe_fd[2];
                pipe(pipe_fd);

                process_counter++;

                int command_process = fork();
                if (command_process < 0)
                { // fork failed
                    fprintf(stderr, "Fork failed");
                    exit(1);
                }
                else if (command_process == 0)
                { // child process
                    int file2 = open(redirected_input_name, O_RDONLY);
                    dup2(file2, STDIN_FILENO);
                    close(file2);

                    close(pipe_fd[0]);
                    dup2(pipe_fd[1], STDOUT_FILENO);
                    close(pipe_fd[1]);

                    execvp(cmd_array[0], cmd_array);
                }
                else
                {

                    close(pipe_fd[1]);

                    /*thread_arraysize++;
                    thread_Array = realloc(thread_Array, thread_arraysize * sizeof(pthread_t));*/

                    pthread_create(&thread1, NULL, listener, (void *)&pipe_fd[0]);
                    if (!strcmp(Background_Job, "n"))
                    {
                        waitpid(command_process, NULL, 0);
                        pthread_join(thread1, NULL);
                    }
                    else
                    {
                        pushBack_f(&processarr, command_process);

                        pushBack_t(&threadarr, thread1);

                        /*
                        thread_Array[thread_arraysize - 1] = thread1;
                        arraysize++;*/
                        /* myArray = realloc(myArray, arraysize * sizeof(int));
                         myArray[arraysize - 1] = command_process;*/
                    }
                }
            }
        }
    }
    // Close the files
    close(parse_txt);
    fclose(file_ptr);

    int i;

    // waiting operations for both processes and threads
    for (i = 0; i < processarr.size; i++)
    {

        int result = waitpid(processarr.array[i], NULL, 0);
    }
    processarr.size = 0;

    for (i = 0; i < threadarr.size; i++)
    {
        int result = pthread_join(threadarr.array[i], NULL);
    }
    threadarr.size = 0;

    // do not need these
    /*
        free(myArray);
        free(thread_Array);*/

    return 0;
}

