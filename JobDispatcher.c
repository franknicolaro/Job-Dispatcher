/*  Program: Job Dispatcher (With Threads)
    Author: Frank J. Nicolaro
    Date: Oct. 26, 2021
    File Name: JobDispatcher.c
    Compile: gcc -lpthread -o dispatcher JobDispatcher.c
    Run: ./dispatcher
    Description: The program is designed to utilize threads to
                 add and run programs as they become available to
                 system time. The first thread (scheduler) handles
                 commands that the program previously had done
                 (adding, printing, and manually deleting jobs in the queue).
                 The second thread (dispatcher) deletes the job as it
                 becomes available and runs that job.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <assert.h>
#include <pthread.h>

//Defining the structure of jobs
struct JOB {
   char command[5][25];  //domain = { [a-zA-Z][a-zA-Z0-9_\.]* }
   long submissionTime;  //positive long integer
   int startTime;        //positive integer
   struct JOB *prev;     //points to the prev job. Null if first job.
   struct JOB *next;     //points to the next job. Null if last job.
};
typedef struct JOB Job;

//Defining the structure of the List
struct LIST {
   int numOfJobs;   //number of jobs in the list
   Job *firstJob;   //The first job, which is null if the list is empty
   Job *lastJob;    //The last job, which is null if the list is empty
};
typedef struct LIST List;

//declaring functions
void appendJob(List *listPtr, Job *jobPtr);
void insertOrdered(List *listPtr, Job *jobPtr);
void insertIntoEmptyList(List *listPtr, Job *jobPtr);
void insertAsLastJob(List *listPtr, Job *jobPtr, Job *currJob);
void insertAsFirstJob(List *listPtr, Job *jobPtr);
void insertBetweenJobs(Job *jobPtr, Job *currJob);
void printForwards(List *listPtr);
void printBackwards(List *listPtr);
Job *createJob();
Job *deleteFirstJob(List *listPtr);
void printJob(Job *jobPtr);
List *createList();
void *schedule(void *arg);
void *dispatch(void *arg);

int main() {
   List *listPtr = createList();
   List myList;
   int ret1, ret2;
   pthread_t scheduler, dispatcher;
   ret1 = pthread_create(&scheduler, NULL, schedule, (void *)listPtr);
   ret2 = pthread_create(&dispatcher, NULL, dispatch, (void *)listPtr);

   //Starts both threads
   pthread_join(scheduler, NULL);
   pthread_join(dispatcher, NULL);
}

// The method that complements the scheduler thread. Handles
// the adding, deleting, and printing of jobs on the queue.
void *schedule(void *arg) {
   List *mylist = (List *) arg;
   char input[2];
   Job *tmpJob;
   printf("What would you like to do?\n");
   printf("- : delete first node\n");
   printf("+ : insert job\n");
   printf("p : print the list\n");
   scanf("%s", &input);
   do {
      if(input[0] == '-'){
         if(mylist->numOfJobs == 0) {
            printf("Invalid Option: No jobs in the list.\n");
         }
         else{
            tmpJob = deleteFirstJob(mylist);
            printf("Job Deleted.\n");
            printJob(tmpJob);
            free(tmpJob);
         }
      }
      else if(input[0] == '+'){
         tmpJob = createJob();
         insertOrdered(mylist, tmpJob);
      }
      else if(input[0] == 'p'){
         printForwards(mylist);
         printf("\n");
         printBackwards(mylist);
      }
      printf("Please enter an option (-,+,p)\n");
      scanf("%s", &input);
   } while(1);
}

// The method that complements the dispatcher thread. Handles
// dispatching and running the jobs as they become available.
void *dispatch(void *arg) {
   List *mylist = (List *) arg;
   //Variables needed for executing every job.
   Job *nextJob = NULL;
   int currentSysTime;
   int childPID;
   int status;
   char *argv[5];
   int i;
   char *cmd;
   while(1) {
      if(mylist->numOfJobs != 0){
         currentSysTime = time(NULL);
         nextJob = mylist->firstJob;
         if(currentSysTime >= nextJob->submissionTime + nextJob->startTime) {
            printf("Current System Time: %d\n", currentSysTime);
            Job *deletedJob = deleteFirstJob(mylist);
            printJob(deletedJob);
            childPID = fork();
            if(childPID == 0) {
               cmd = deletedJob->command[0];
               i = 0;
               //Takes all parts of command and stores into argv
               while(deletedJob->command[i][0] != '\0') {
                  argv[i] = deletedJob->command[i];
                  i++;
               }
               argv[i] = NULL;
               free(deletedJob);
               status = execvp(cmd, argv);
               exit(1);
            }
            else {
               waitpid(childPID, &status, 0);
            }
         }
         else {
            sleep(1);
         }
      }
   }
}

// Creates a new Job, prompting the user to enter in the job details.
Job *createJob(){
   char com[5][25];     // com == command
   int subTime;         // subTime == submissionTime
   int staTime;         // staTime == startTime
   int numOfParts;      // the number of parts to the command

   printf("Enter the number of parts of the command ");
   printf("you would like (NOTE: the number of parts ");
   printf("must be no more than 5): \n");
   scanf("%d", &numOfParts);

   //Takes in all parts of the command
   for(int i = 0; i < numOfParts; i++) {
      printf("Enter one part of the command: \n");
      scanf("%s", com[i]);
   }
   if(numOfParts != 5){
      com[numOfParts][0] = '\0';
   }
   subTime = time(NULL);

   printf("Now, enter a start time:\n");
   scanf("%d", &staTime);

   Job *jobPtr = (Job *) malloc(sizeof(Job));

   //Stores all parts of the com into command
   for(int j = 0; j < 5; j++) {
      *strcpy(jobPtr->command[j], com[j]);
   }
   jobPtr->submissionTime = subTime;
   jobPtr->startTime = staTime;

   return jobPtr;
}

// Creates a new list.
List *createList() {
   List *listPtr = (List*) malloc(sizeof(List));
   listPtr->numOfJobs = 0;
   listPtr->firstJob = NULL;
   listPtr->lastJob = NULL;

   return listPtr;
}

//Appends the new job that was just created
void appendJob(List *listPtr, Job *newPtr) {
   if(listPtr->numOfJobs == 0) {
      //empty list
      newPtr->prev = NULL;
      newPtr->next = NULL;

      listPtr->firstJob = newPtr;
      listPtr->lastJob = newPtr;
   }
   else {
      // at least one job
      newPtr->prev = listPtr->lastJob;
      newPtr->next = NULL;

      listPtr->lastJob->next = newPtr;
      listPtr->lastJob = newPtr;
   }
   listPtr->numOfJobs++;
}

//Inserts the job into the list. It is inserted such that it is in
//non-decreasing order of (submissionTime+startTime)
void insertOrdered(List *listPtr, Job *jobPtr) {
   int timeOfJob = jobPtr->submissionTime + jobPtr->startTime;
   Job *currJob;
   int timeOfCurJob;
   if(listPtr->numOfJobs == 0) {
      insertIntoEmptyList(listPtr, jobPtr);
   }
   else {
      currJob = listPtr->firstJob;
      timeOfCurJob = currJob->submissionTime + currJob->startTime;
      if(timeOfJob <= timeOfCurJob) {//The new job has least time in the list
         insertAsFirstJob(listPtr, jobPtr);
      }
      else {
         if(currJob->next == NULL){//should occur if numJobs is about to be 2
            insertAsLastJob(listPtr, jobPtr, currJob);
         }
         else {
            currJob = currJob->next;
            timeOfCurJob = currJob->submissionTime + currJob->startTime;
            while(currJob->next != NULL && timeOfJob > timeOfCurJob) {
               currJob = currJob->next;
               timeOfCurJob = currJob->submissionTime + currJob->startTime;
            }
            if(currJob->next == NULL && timeOfJob > timeOfCurJob) {
               insertAsLastJob(listPtr, jobPtr, currJob);
            }
            else {
               insertBetweenJobs(jobPtr, currJob);
            }
         }
      }
   }
   listPtr->numOfJobs++;
}

//Inserts the job into an empty list.
void insertIntoEmptyList(List *listPtr, Job *jobPtr) {
   jobPtr->prev = NULL;
   jobPtr->next = NULL;
   listPtr->firstJob = jobPtr;
   listPtr->lastJob = jobPtr;
}

//Inserts the job as the last job in the list.
void insertAsLastJob(List *listPtr, Job *jobPtr, Job *currJob) {
   jobPtr->next = NULL;
   jobPtr->prev = currJob;
   jobPtr->prev->next = jobPtr;
   listPtr->lastJob = jobPtr;
}

//Inserts the job as the first job in the list.
void insertAsFirstJob(List *listPtr, Job *jobPtr) {
   jobPtr->next = listPtr->firstJob;
   jobPtr->next->prev = jobPtr;
   listPtr->firstJob = jobPtr;
}

//Inserts the job in between two nodes given the specified currJob.
void insertBetweenJobs(Job *jobPtr, Job *currJob) {
   jobPtr->next = currJob;
   jobPtr->prev = currJob->prev;
   currJob->prev->next = jobPtr;
   currJob->prev = jobPtr;
}

//Deletes the first job in the list, and returns that first job.
//If the list is empty, then it returns null.
Job *deleteFirstJob(List *listPtr) {
   Job *result;
   if(listPtr->numOfJobs != 0) {
      result = listPtr->firstJob;
      if(listPtr->numOfJobs == 1) {
         listPtr->firstJob == NULL;
         listPtr->lastJob == NULL;
      }
      else {
         listPtr->firstJob = listPtr->firstJob->next;
         listPtr->firstJob->prev = NULL;
      }
      listPtr->numOfJobs--;
   }
   return result;
}

//Prints the list in order from first to last.
void printForwards(List *listPtr) {
   if(listPtr->numOfJobs == 0) {
      printf("Empty list.\n");
   }
   else {
      //First to last.
      Job *ptr = listPtr->firstJob;
      for (int i = 0; i < listPtr->numOfJobs; i++) {
         printf("Job %d:\n", i+1);
         printJob(ptr);
         ptr = ptr->next;
      }
   }
}

//Prints the list in order from last to first.
void printBackwards(List *listPtr) {
   if(listPtr->numOfJobs == 0) {
      printf("Empty list.\n");
   }
   else {
      //Last to first.
      Job *ptr = listPtr->lastJob;
      for (int i = 0; i < listPtr->numOfJobs; i++) {
         printf("Job %d:\n", i+1);
         printJob(ptr);
         ptr = ptr->prev;
      }
   }
}

// Prints the job in the following format:
// command: all parts of the command
// submission time: jobPtr->submissionTime
// start time: jobPtr->startTime
void printJob(Job *jobPtr) {
   printf("command: ");
   int count = 0;
   while(jobPtr->command[count][0] != '\0') {
      printf("%s", jobPtr->command[count]);
      printf(" ");
      count++;
   }
   printf("\n");
   printf("submission time: %d\n", jobPtr->submissionTime);
   printf("start time: %d\n", jobPtr->startTime);
}
