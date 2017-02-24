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

job_queue BG_Jobs; // Backgorund Jobs Queue

pid_queue processes_temp; // Pid Queue

bool initialized = 0;

int BG_num;

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
  *should_free = false; // not sure what this should_free is
  return getcwd(NULL, 1024);//return current directory
}

// Returns the value of an environment variable env_var
const char* lookup_env(const char* env_var) {
    return (getenv(env_var)); //return the current enviornment
}

// Check the status of background jobs
void check_jobs_bg_status() {
  // Check on the statuses of all processes belonging to all background
  // jobs. This function should remove jobs from the jobs queue once all
  // processes belonging to a job have completed.

  size_t length = length_job_queue(&BG_Jobs); // create a length variable for iteration purposes
  for(int i=0;i < length;i++){ // loop through background

      jobtype temp_job = pop_back_job_queue(&BG_Jobs); // create a temp for each job
      pid_queue processes = temp_job.process_queue; // create a queue of the process within the Jobs object

      size_t processes_num = length_pid_queue(&(processes)); // length of pid_queue
      for(int j=0;j<processes_num;j++){ //iterate through pid_queue

          pid_t process_id = pop_back_pid_queue(&processes); // pop from the back of the queue
          int status;
          if(waitpid(process_id, &status, WNOHANG) == 0){
              //the process is still running
              push_front_pid_queue(&processes,process_id);
          }
          else{
              //the process is done
          }

      }
      if(is_empty_pid_queue(&processes)){
          //the job is finished
          print_job_bg_complete(temp_job.id, temp_job.pid, temp_job.cmd);
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

  execvp(exec,args);//run the command
  perror("ERROR: Failed to execute program");
}

// Print strings
void run_echo(EchoCommand cmd) {
  // Print an array of strings. The args array is a NULL terminated (last
  // string is always NULL) list of strings.
  char** str = cmd.args;


  while (*str != NULL) { // recurse through the string to print it out
    printf("%s", *str);//print the string
    fflush(stdout);

    str =str + 1; // Recurse on the next element in the string array
  }
  printf("\n");
  // TODO: Remove warning silencers
  (void) str; // Silence unused variable warning

  // Flush the buffer before returning
  fflush(stdout);
}

// Sets an environment variable
void run_export(ExportCommand cmd) {
  // Write an environment variable
  const char* env_var = cmd.env_var;
  const char* val = cmd.val;
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
  chdir(dir); // change the directory

  setenv("PWD",dir,1);//change the enviornment
}

// Sends a signal to all processes contained in a job
void run_kill(KillCommand cmd) {
  int signal = cmd.sig;
  int job_id = cmd.job;

  jobtype tempJob; // create a temp job to hold for execution
  size_t length = length_job_queue(&BG_Jobs); // temp size for iteration purposes
  for(int i=0;i < length;i++){ // iterate through the background jobs
      tempJob = pop_back_job_queue(&BG_Jobs);//pop a job from the queue
      if(tempJob.id == job_id){
          break;
      }
      else if(i == length-1){// if the job doesnt exist
          printf("This Job ID does not exist: %d\n",job_id);
          return;
      }
      else{// push the jobs
          push_front_job_queue(&BG_Jobs,tempJob);
      }
  }

  pid_queue SeventySixers = tempJob.process_queue; // create a pid_queue from the job

  while(!(is_empty_pid_queue(&SeventySixers))){// loop through the queue
      pid_t JoelEmbiid = pop_back_pid_queue(&SeventySixers);//get the processes within the queue
      kill(JoelEmbiid,signal);//KILL THE PROCESS
  }

  push_front_job_queue(&BG_Jobs, tempJob); //put the job back in the queue, even though it's empty. We'll let check_bg_status handle it from here

}


// Prints the current working directory to stdout
void run_pwd() {

  char* wd_40 = getcwd(NULL,0); // run pwd
  printf("%s\n",wd_40);
  // Flush the buffer before returning
  fflush(stdout);
  free(wd_40);
}

// Prints all background jobs currently in the job list to stdout
void run_jobs() {

  size_t length = length_job_queue(&BG_Jobs);// get the length of the jobs queue
  for(int i=0;i < length;i++){//loop through jobs queue

      jobtype temp_job = pop_back_job_queue(&BG_Jobs);// temp job for each job in queue
      print_job(temp_job.id, temp_job.pid, temp_job.cmd);//print the job
      push_front_job_queue(&BG_Jobs, temp_job);// push it to the front
  }
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
void child_run_command(Command cmd) { // child commands
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
void parent_run_command(Command cmd) { // parent commands
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
     if (p_out) { // CODE taken from PIAZA from TA (largely, ours added within the proivided framework)
         /* This is the only condition under which a new pipe creation is required.
            You should be able to understand why this is the case */
         pipe (environment_pipes[next_pipe]); // create a pipe for piping
     }

    // ...
     pid_t pid; //create a pid

     pid = fork (); // creat the child thorugh forking
     push_front_pid_queue(&processes_temp,pid); // push the child thruogh the jobs queue
     if (0 == pid) {
         /* Check if this process needs to receive from previous process */
         if (p_in) { //pin
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

         FILE *fout; // create a file out for riderection purposes
         if(r_in == 1)
         {
             fout = fopen(holder.redirect_in, "r"); // read in and riderect in
             dup2(fileno(fout),0); // dup2 with the redirect in

         }
         if (r_out == 1) //readout
         {

             if (r_app == 1)//append
             {
                 fout = fopen(holder.redirect_out, "a");//append with a
                 dup2(fileno(fout),1);//print the output

             }
             else
             {
                 fout = fopen(holder.redirect_out, "w");//write to a new file
                 dup2(fileno(fout),1); // print to the new file
             }
         }

         child_run_command (holder.cmd);//run the child command

         /* Adios child process */
         exit (EXIT_SUCCESS);
     } else {
         /* Close the hanging pipes in parent */
         if (p_out) {
             close (environment_pipes[next_pipe][WRITE_END]);
         }

         /* Update the pipe trackers for next iteration */
         next_pipe = (next_pipe + 1) % 2;
         prev_pipe = (prev_pipe + 1) % 2;

         parent_run_command(holder.cmd);//run parent process
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

      while(!(is_empty_pid_queue(&processes_temp))){//loop through proccess queue

          pid_t JoelEmbiid = pop_back_pid_queue(&processes_temp);//creaete temp
          waitpid(JoelEmbiid,NULL,0);//waitpid command on the temp
      }

      destroy_pid_queue(&processes_temp); // destroy the pid queue
  }
  else {
    //create a job from the process queue and add it to the global job queue.
    struct Job tempJob;
    tempJob.id = BG_num; // set the id no.
    tempJob.process_queue = processes_temp;// set the process queue
    tempJob.cmd = get_command_string(); // set the cmd string
    tempJob.pid = peek_back_pid_queue(&processes_temp);// set the pidqueue

    push_front_job_queue(&BG_Jobs,tempJob);//push the new job to the job queue
    BG_num += 1;//length incrementing

    print_job_bg_start(tempJob.id, tempJob.pid, tempJob.cmd);//print the job
  }
}

void Initialize(){
    BG_Jobs = new_job_queue(1);

    BG_num = 1;
    initialized = 1;

}
