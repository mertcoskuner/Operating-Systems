
#include <stdio.h> 
#include <stdlib.h>
#include <unistd.h>
#include <string.h> 
#include <fcntl.h> 
#include <sys/wait.h>

int main(){

  printf("I'm SHELL process, with PID: %d Main command is: man ls | grep -e -B -A 2 -m 1  \n",getpid()); 

  int pipe_fd[2]; 
  pipe(pipe_fd); 
  int process_man  = fork(); 
  
  if(process_man<0) {
	  //fork failed
	  fprintf(stderr,"Fork failed") ; 
	  exit(1) ; 
  }
  else if(process_man  == 0 ) {
	  //child process 
	  //sleep(10);
          close(pipe_fd[0]);// Close the read end 
	  printf("I'm MAN  process, with PID: %d My command is: man ls \n",getpid()); 
	  const char *concurrency = "concurrency message" ; 
	  write(pipe_fd[1], concurrency, strlen(concurrency)) ; 
	  dup2(pipe_fd[1],STDOUT_FILENO); 
	  char *args[3];
	  args[0] = strdup("man");
	  args[1] = strdup("ls"); 
	  args[2]  =  NULL ;
          close(pipe_fd[1]);
	  execvp(args[0],args);
  }
  else{
	// parent process 
        int  process_grep  = fork() ; 
        if(process_grep<0){
          //fork failed 
          fprintf(stderr, "Fork failed") ; 
          exit(1); 

        }
        else if(process_grep == 0 ){
        //child process
	    close(pipe_fd[1]);
	    char reading_from_concurrency[1024]; 
	    int read_concur = read(pipe_fd[0],reading_from_concurrency,sizeof(reading_from_concurrency));
            printf("I'm GREP  process, with PID: %d My command is: grep -e -B -A 2 -m 1  \n",getpid()); 
            int my_output = open("output.txt", O_CREAT|O_WRONLY|O_TRUNC,S_IRWXU);

            dup2(pipe_fd[0],STDIN_FILENO);
	    dup2(my_output, STDOUT_FILENO);
            close(pipe_fd[0]);
	    char * args[8];//man ls | grep -e -B -A 2 -m 1 
	    args[0]  = strdup("grep") ; 
	    args[1] = strdup("-e") ;
	    args[2] = strdup("-B") ;
	    args[3] = strdup("-A") ;
	    args[4] = strdup("2") ;
	    args[5] = strdup("-m") ;
    	    args[6] = strdup("1") ; 
	    args[7] = NULL;

	    execvp(args[0],args);
        }
        else{
          close(pipe_fd[0]);
          close(pipe_fd[1]);
	  waitpid(process_man,NULL,0);
	  waitpid(process_grep,NULL,0);
  
	  printf("I'm SHELL process, with PID: %d - execution is completed, you can find the results in output.txt \n",getpid()); 
	}
}


return 0; 
}
