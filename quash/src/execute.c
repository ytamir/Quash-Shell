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

  return "???";
}

// Check the status of background jobs
void check_jobs_bg_status() {
  // TODO: Check on the statuses of all processes belonging to all background
  // jobs. This function should remove jobs from the jobs queue once all
  // processes belonging to a job have completed.
  IMPLEMENT_ME();//3

  // TODO: Once jobs are implemented, uncomment and fill the following line
  // print_job_bg_complete(job_id, pid, cmd);
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
  IMPLEMENT_ME();//8
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
  // TODO: Print background jobs
  IMPLEMENT_ME();//10

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

    //PLACE TO FORK
    /*
     *
     * The create_process() function is intended to be the place where you fork processes,
     *  handle pipe creation, and file redirection. You should not call execvp(3) from this
     *  function. Instead you should call derivatives of the example_run_command() function.
     *  Also you can determine whether you should use the boolean variables at the top of this
     *  function to determine if pipes and redirects should be setup. It may be necessary to
     *  keep a global execution state structure so that different calls to create process can
     *  view important information created in previous invocations of create_process() (i.e.
     *  the file descriptors for open pipes of previous processes).
     */

     if(r_out){

         pid_t child1;
         child1 = fork();
         //child2 = fork();
         FILE *fout;

         int pipey[2];
         pipe(pipey);

         if(child1 == 0){
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

            // dup2(pipey[1],1);
             //close(pipey[0]);
             //close(pipey[1]);

             child_run_command(holder.cmd);
             //printf("I am the child process. My PID is %d\n", getpid());
             //printf("    My parent's PID is %d\n", parent);

             exit(0);

        /* else if(child2){

             dup2(pipey[0],0);


             //FILE * file;

             fout = fopen(holder.redirect_out, "w");
             dup2(fileno(fout), 1);
             //dup2(fileno(fout),pipey[0]);
             //dup2(pipey[1],1);

             printf((char*)pipey[1]);
             //printf(holder.redirect_out);
             fclose(fout);
             //dup2(file,1);

             //close(file);
             //close(pipey[0]);
             //close(pipey[1]);
             //exit(0);
         }
         else{ //parent process

             parent_run_command(holder.cmd);
             //printf("I am the parent process. My PID is %d\n", getpid());
         }*/


     }
     else{
          pid_t child;
          child = fork();
          if(child == 0){ //child process

            child_run_command(holder.cmd);
            //printf("I am the child process. My PID is %d\n", getpid());
            //printf("    My parent's PID is %d\n", parent);

            exit(0);
          }
          else{ //parent process

            parent_run_command(holder.cmd);
            //printf("I am the parent process. My PID is %d\n", parent);
          }
     }



    // TODO: Remove warning silencers
    (void) p_in;  // Silence unused variable warning
    (void) p_out; // Silence unused variable warning
    (void) r_in;  // Silence unused variable warning
    (void) r_out; // Silence unused variable warning
    (void) r_app; // Silence unused variable warning

    // TODO: Setup pipes, redirects, and new process
    //IMPLEMENT_ME();//11

    //parent_run_command(holder.cmd); // This should be done in the parent branch of
                                    // a fork
    //child_run_command(holder.cmd); // This should be done in the child branch of a fork



}
}

// Run a list of commands
void run_script(CommandHolder* holders) {
  if (holders == NULL)
    return;

  check_jobs_bg_status();

  if (get_command_holder_type(holders[0]) == EXIT &&
      get_command_holder_type(holders[1]) == EOC) {
    end_main_loop();
    return;
  }

  CommandType type;

  // Run all commands in the `holder` array
  for (int i = 0; (type = get_command_holder_type(holders[i])) != EOC; ++i)
    create_process(holders[i]);

  if (!(holders[0].flags & BACKGROUND)) {
    // Not a background Job
    // TODO: Wait for all processes under the job to complete
    IMPLEMENT_ME();//12
  }
  else {
    // A background job.
    // TODO: Push the new job to the job queue
    IMPLEMENT_ME();//13

    // TODO: Once jobs are implemented, uncomment and fill the following line
    // print_job_bg_start(job_id, pid, cmd);
  }
}
