Job Dispatcher by Frank Nicolaro

How the program works:

The program starts by creating the list of jobs and then running both threads created in the main.
These threads have differing functions:

Scheduler:

The scheduler simply has the job of scheduling jobs (Unix commands such as "ls", "nano filename", etc).
It begins by asking the user for input of either '+', '-', or 'p':

  If the user pressed '+':
  Takes the user to createJob(), which asks for the number of parts the command has (no more than 5).
  Once a number has been entered, the user will then be prompted to enter in each part of the command
  they would like executed. When the user finishes going through this loop, the program automatically gets
  the submission time using time(NULL) as the first of the two necessary time constraints. The user is finally prompted
  to enter a time at which they would like the command to start. When this is done, the Job is then inserted within the
  list by the total value of the time at which the job would be executed (submission time + start time). 
  
  If the user pressed '-':
  The first job inserted gets deleted from the list of Jobs. When the job is deleted, it gets printed out and then frees
  that piece of memory. If there are no jobs in the list, then an error message will appear and the user will be prompted
  to enter in another option. 
  
  If the user pressed 'p':
  The list of jobs that have not been executed yet gets printed forwards and backwards,
  in order of (submission time + start time).
  
  Otherwise:
  Nothing happens and the loop continues through to the input prompt again.
  
Dispatcher:

The dispatcher checks if the first job is ready to be executed based on (submission time + start time).
If the system time is equivalent to this, then the job will execute. It first prints the system time,
and takes the first job off of the list. This job is then printed and then a child process is made to
execute the commmand. The actual command is stored in char* cmd, while any arguments are stored in *argv[].
The system call execvp is then utilized to execute the command after the job was freed. Note that the thread will
wait until the command is executed before continuing on, and will sleep if the job is not ready. 

This is helped with the structs defined in the beginning of the program:

JOB:

Contains the details necessary to execute a Unix command (char command[][], submissionTime, and startTime)
as well as the details of the job which will execute before and after it (if it exists). 

LIST:

Contains the number of jobs within the list as well as the first and last job of the list.
  
