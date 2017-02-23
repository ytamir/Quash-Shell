/**
 * @file execute.c
 *
 * @brief Implements interface functions between Quash and the environment and
 * functions that interpret an execute commands.
 *
 * @note As you add things to this file you may want to change the method signature
 */

#include "execute.h"

#include <stdio.h>

#include "quash.h"
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "job_queue.h"

#define    READ_END     0
#define    WRITE_END    1

/* Create a global tracker for active pipes */
static int environment_pipes[2][2];        	/* Global Variable */


static int next_pipe = 0;                  	/* Global Variable */


static int prev_pipe = -1;



job_queue BG_Jobs;
job_queue FG_Jobs;

pid_queue processes_temp;

bool initialized = 0;

int BG_num;
int FG_num;

// Remove this and all expansion calls to it
/**
 * @brief Note calls to any function that requires implementation
 */
#define IMPLEMENT_ME()                                                  \
  fprintf(stderr, "IMPLEMENT ME: %s(line %d): %s()\n", __FILE__, __LINE__, __FUNCTION__)

/***************************************************************************
 * Interface Functions
 ***************************************************************************/

// Return a string containing the current working directory.
char* get_current_directory(bool* should_free) {
  // TODO: Get the current working directory. This will fix the prompt path.
  // HINT: This should be pretty simple
  //IMPLEMENT_ME();//1

  // Change this to true if necessary
  *should_free = false; // not sure what this should_free is


  return getcwd(NULL, 1024);
}

// Returns the value of an environment variable env_var
const char* lookup_env(const char* env_var) {
  // TODO: Lookup environment variables. This is required for parser to be able
  // to interpret variables from the command line and display the prompt
  // correctly
  // HINT: This should be pretty simple
  //IMPLEMENT_ME();//2
    return (getenv(env_var));

  // TODO: Remove warning silencers
  //(void) env_var; // Silence unused variable warning

 // return "???";
}

// Check the status of background jobs
void check_jobs_bg_status() {
  // Check on the statuses of all processes belonging to all background
  // jobs. This function should remove jobs from the jobs queue once all
  // processes belonging to a job have completed.

  size_t length = length_job_queue(&BG_Jobs);
  for(int i=0;i < length;i++){

      jobtype temp_job = pop_back_job_queue(&BG_Jobs);
      pid_queue processes = temp_job.process_queue;

      pid_t first_process_in_job = pop_front_pid_queue(&processes);
      push_front_pid_queue(&processes,first_process_in_job);

      size_t processes_num = length_pid_queue(&(processes));
      for(int j=0;j<processes_num;j++){

          pid_t process_id = pop_back_pid_queue(&processes);

          if(kill(process_id,0) == 0){
              //the process is still running
              push_front_pid_queue(&processes,process_id);
          }
          else{
              //the process is done
          }

      }
      if(is_empty_pid_queue(&processes)){
          //the job is finished

          //TODO: figure out what the command parameter is
          print_job_bg_complete(temp_job.id, first_process_in_job, "???");
      }
      else{
          //the job is not finished, put it back in the queue
          push_front_job_queue(&BG_Jobs, temp_job);
      }

  }



}

// Prints the job id number, the process id of the first process belonging to
// the Job, and the command string associated with this job
void print_job(int job_id, pid_t pid, const char* cmd) {
  printf("[%d]\t%8d\t%s\n", job_id, pid, cmd);
  fflush(stdout);
}

// Prints a start up message for background processes
void print_job_bg_start(int job_id, pid_t pid, const char* cmd) {
  printf("Background job started: ");
  print_job(job_id, pid, cmd);
}

// Prints a completion message followed by the print job
void print_job_bg_complete(int job_id, pid_t pid, const char* cmd) {
  printf("Completed: \t");
  print_job(job_id, pid, cmd);
}

/***************************************************************************
 * Functions to process commands
 ***************************************************************************/
// Run a program reachable by the path environment variable, relative path, or
// absolute path
void run_generic(GenericCommand cmd) {
  // Execute a program with a list of arguments. The `args` array is a NULL
  // terminated (last string is always NULL) list of strings. The first element
  // in the array is the executable
  char* exec = cmd.args[0];
  char** args = cmd.args;

  // TODO: Remove warning silencers
  //exec; // Silence unused variable warning
  //args; // Silence unused variable warning

  // TODO: Implement run generic
  //IMPLEMENT_ME();//4
  execvp(exec,args);
  perror("ERROR: Failed to execute program");
}

// Print strings
void run_echo(EchoCommand cmd) {
  // Print an array of strings. The args array is a NULL terminated (last
  // string is always NULL) list of strings.
  char** str = cmd.args;


  while (*str != NULL) {
    printf("%s", *str);
    fflush(stdout);

    str =str + 1; // Recurse on the next element in the string array
  }
  printf("\n");
  // TODO: Remove warning silencers
  (void) str; // Silence unused variable warning

  // TODO: Implement echo
  //IMPLEMENT_ME();//5

  // Flush the buffer before returning
  fflush(stdout);
}

// Sets an environment variable
void run_export(ExportCommand cmd) {
  // Write an environment variable
  const char* env_var = cmd.env_var;
  const char* val = cmd.val;

  // TODO: Remove warning silencers
  //(void) env_var; // Silence unused variable warning
  //(void) val;     // Silence unused variable warning

  // TODO: Implement export.
  // HINT: This should be quite simple.
  //IMPLEMENT_ME();//6
  setenv(env_var,val,1);

}

// Changes the current working directory
void run_cd(CDCommand cmd) {
  // Get the directory name
  const char* dir = cmd.dir;

  // Check if the directory is valid
  if (dir == NULL) {
    perror("ERROR: Failed to resolve path");
    return;
  }

  // TODO: Change directory
  chdir(dir);
  // TODO: Update the PWD environment variable to be the new current working
  // directory and optionally update OLD_PWD environment variable to be the old
  // working directory.
  //IMPLEMENT_ME();//7
  setenv("PWD",dir,1);
}

// Sends a signal to all processes contained in a job
void run_kill(KillCommand cmd) {
  int signal = cmd.sig;
  int job_id = cmd.job;

  // TODO: Remove warning silencers
  (void) signal; // Silence unused variable warning
  (void) job_id; // Silence unused variable warning

  // TODO: Kill all processes associated with a background job
  //IMPLEMENT_ME();//8
   size_t length = length_job_queue(&BG_Jobs);
   for(int i=0;i < length;i++){

   }

}


// Prints the current working directory to stdout
void run_pwd() {
  // TODO: Print the current working directory
  //IMPLEMENT_ME();//9
  char* wd_40 = getcwd(NULL,0);
  printf("%s\n",wd_40);
  // Flush the buffer before returning
  fflush(stdout);
  free(wd_40);
}

// Prints all background jobs currently in the job list to stdout
void run_jobs() {
  printf("Current Background Jobs: ");

  size_t length = length_job_queue(&BG_Jobs);
  for(int i=0;i < length;i++){

      jobtype temp_job = pop_back_job_queue(&BG_Jobs);
      printf(" %d,",temp_job.id);
      push_front_job_queue(&BG_Jobs, temp_job);
  }
  printf("\n");
  // Flush the buffer before returning
  fflush(stdout);
}

/***************************************************************************
 * Functions for command resolution and process setup
 ***************************************************************************/

/**
 * @brief A dispatch function to resolve the correct @a Command variant
 * function for child processes.
 *
 * This version of the function is tailored to commands that should be run in
 * the child process of a fork.
 *
 * @param cmd The Command to try to run
 *
 * @sa Command
 */
void child_run_command(Command cmd) {
  CommandType type = get_command_type(cmd);

  switch (type) {
  case GENERIC:
    run_generic(cmd.generic);
    break;

  case ECHO:
    run_echo(cmd.echo);
    break;

  case PWD:
    run_pwd();
    break;

  case JOBS:
    run_jobs();
    break;

  case EXPORT:

  case CD:
    //run_cd(cmd.cd);
    break;

  case KILL:
  case EXIT:
  case EOC:
    break;

  default:
    fprintf(stderr, "Unknown command type: %d\n", type);
  }
}

/**
 * @brief A dispatch function to resolve the correct @a Command variant
 * function for the quash process.
 *
 * This version of the function is tailored to commands that should be run in
 * the parent process (quash).
 *
 * @param cmd The Command to try to run
 *
 * @sa Command
 */
void parent_run_command(Command cmd) {
  CommandType type = get_command_type(cmd);

  switch (type) {
  case EXPORT:
    run_export(cmd.export);
    break;

  case CD:
    run_cd(cmd.cd);
    break;

  case KILL:
    run_kill(cmd.kill);
    break;

  case GENERIC:
  case ECHO:
  case PWD:
    //run_pwd(cmd.pwd);
    break;
  case JOBS:
  case EXIT:
  case EOC:
    break;

  default:
    fprintf(stderr, "Unknown command type: %d\n", type);
  }
}

/**
 * @brief Creates one new process centered around the @a Command in the @a
 * CommandHolder setting up redirects and pipes where needed
 *
 * @note Processes are not the same as jobs. A single job can have multiple
 * processes running under it. This function creates a process that is part of a
 * larger job.
 *
 * @note Not all commands should be run in the child process. A few need to
 * change the quash process in some way
 *
 * @param holder The CommandHolder to try to run
 *
 * @sa Command CommandHolder
 */
void create_process(CommandHolder holder) {
    // Read the flags field from the parser
    bool p_in  = holder.flags & PIPE_IN;
    bool p_out = holder.flags & PIPE_OUT;

    bool r_in  = holder.flags & REDIRECT_IN;
    bool r_out = holder.flags & REDIRECT_OUT;
    bool r_app = holder.flags & REDIRECT_APPEND; // This can only be true if r_out
                                                 // is true

    /*
     int pipey[2];
     pipe(pipey);

     pid_t child;
     child = fork();

     //add to the queue as soon as it is forked
     push_front_pid_queue(&processes_temp,child);

     if(child == 0){
         FILE *fout;
         if(r_in == 1)
         {
             fout = fopen(holder.redirect_in, "r");
             dup2(fileno(fout),1);
             close(fout);

         }
         if (r_out == 1)
         {

             if (r_app == 1)
             {
                 fout = fopen(holder.redirect_out, "a");
                 dup2(fileno(fout),1);

             }
             else
             {
                 fout = fopen(holder.redirect_out, "w");
                 dup2(fileno(fout),1);
             }
         }
         if(p_out){
             dup2(pipey[1],1);
             close(pipey[1]);
             close(pipey[0]);
         }
         if(p_in){
             dup2(pipey[0],0);
             close(pipey[0]);
             close(pipey[1]);
         }

         child_run_command(holder.cmd);

         //printf("I am the child process. My PID is %d\n", getpid());
         //printf("    My parent's PID is %d\n", parent);
         close(pipey[1]);
         close(pipey[0]);

         exit(0);

    }
    else{
        parent_run_command(holder.cmd);

        close(pipey[1]);
        close(pipey[0]);
        //printf("I am the parent process. My PID is %d\n", parent);
    }

    */







     /********************************************/

     if (p_out) {
         /* This is the only condition under which a new pipe creation is required.
            You should be able to understand why this is the case */
         pipe (environment_pipes[next_pipe]);
     }

    // ...
     pid_t pid;

     pid = fork ();
     push_front_pid_queue(&processes_temp,pid);
     if (0 == pid) {
         /* Check if this process needs to receive from previous process */
         if (p_in) {
             dup2 (environment_pipes[prev_pipe][READ_END], STDIN_FILENO);


             /* We are never going to write to the previous pipe so we can safely close it */
             close (environment_pipes[prev_pipe][WRITE_END]);
         }

         if (p_out) {
             dup2 (environment_pipes[next_pipe][WRITE_END], STDOUT_FILENO);

             /* We are never going to read from our own pipe so we can safely close it */
             close (environment_pipes[next_pipe][READ_END]);
         }

         /* Execute what-ever needs to be run in the child process */

         FILE *fout;
         if(r_in == 1)
         {
             fout = fopen(holder.redirect_in, "r");
             dup2(fileno(fout),1);
             close(fout);

         }
         if (r_out == 1)
         {

             if (r_app == 1)
             {
                 fout = fopen(holder.redirect_out, "a");
                 dup2(fileno(fout),1);

             }
             else
             {
                 fout = fopen(holder.redirect_out, "w");
                 dup2(fileno(fout),1);
             }
         }

         child_run_command (holder.cmd);

         /* Adios child process */
         exit (EXIT_SUCCESS);
     } else {
         /* Close the hanging pipes in parent */
         if (p_in) {
             close (environment_pipes[next_pipe][WRITE_END]);
         }

         /* Update the pipe trackers for next iteration */
         next_pipe = (next_pipe + 1) % 2;
         prev_pipe = (prev_pipe + 1) % 2;
     }

     /* This function can safely kick  the bucket now */
     return;


}

// Run a list of commands
void run_script(CommandHolder* holders) {

// If the global Job queues are not initialized, then initialize them
  if(!initialized) Initialize();

  if (holders == NULL)
    return;

  check_jobs_bg_status();

  if (get_command_holder_type(holders[0]) == EXIT &&
      get_command_holder_type(holders[1]) == EOC) {
    end_main_loop();
    return;
  }

  CommandType type;

  // create a new pid queue and a new job for every command
  processes_temp = new_pid_queue(1);

  // Run all commands in the `holder` array
  for (int i = 0; (type = get_command_holder_type(holders[i])) != EOC; ++i){
      create_process(holders[i]);
  }

  if (!(holders[0].flags & BACKGROUND)) { //Not a Background job
      //create a job from the process queue and add it to the global job queue.

      while(!(is_empty_pid_queue(&processes_temp))){

          pid_t JoelEmbiid = pop_back_pid_queue(&processes_temp);
          waitpid(JoelEmbiid,NULL,0);
      }

      destroy_pid_queue(&processes_temp);
  }
  else {
    //create a job from the process queue and add it to the global job queue.
    struct Job tempJob;
    tempJob.id = BG_num;
    tempJob.process_queue = processes_temp;

    push_front_job_queue(&BG_Jobs,tempJob);
    BG_num += 1;

    print_job_bg_start(tempJob.id, 0, "???");
  }
}

void Initialize(){
    FG_Jobs = new_job_queue(1);
    BG_Jobs = new_job_queue(1);

    BG_num = 0;
    FG_num = 0;
    initialized = 1;

}
