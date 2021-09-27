/*
 * A3 Synchronization problem code
 */

#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>
#include <stdbool.h>

/*** Constants that define parameters of the simulation ***/

#define MAX_SEATS 3          	/* Number of seats in TA's office */
#define TA_LIMIT 10   		/* Number of students a TA can help before he needs a break */
#define MAX_STUDENTS 1000       /* Maximum number of students in the simulation */

#define CLASS_OS "OS"
#define CLASS_PFUN "PFUN"

/* Add your synchronization variables here */
sem_t class_limit;
sem_t os;
sem_t pfun;
sem_t TA_break;
sem_t lock;
/* Basic information about simulation.  They are printed/checked at the end 
 * and in assert statements during execution.
 *
 * You are responsible for maintaining the integrity of these variables in the 
 * code that you develop. 
 */

static int students_in_office;   /* Total numbers of students currently in the office */
static int class_os_inoffice;      /* Total numbers of students from OS class currently in the office */
static int class_pfun_inoffice;      /* Total numbers of students from PFUN class in the office */
static int students_since_break = 0;

int pfun_outside = 0;
int os_outside = 0;


typedef struct {
          int arrival_time;  // time between the arrival of this student and the previous student
          int question_time; // time the student needs to spend with the TA
          char student_class [5];
          int student_id;
  } student_info;

/* Called at beginning of simulation.  Create/initialize all synchronization
 * variables and other global variables that you add.
 */
static int
initialize(student_info *si, char *filename) {

	students_in_office = 0;
	class_os_inoffice = 0;
	class_pfun_inoffice = 0;
    students_since_break = 0;

	sem_init(&class_limit,0,MAX_SEATS); //Initialise semaphore with relevant value for 3 max seats
	sem_init(&os,0,0); //Initialise semaphore to simulate an ordering event
	sem_init(&pfun,0,0); //Initialise semaphore to simulate an ordering event
	sem_init(&TA_break,0,TA_LIMIT); //Initialise semaphore with relevant value for 10 studnets till break
	sem_init(&lock,0,1); //Initialise semaphore with 1 to make a lock

	/* Initialize your synchronization variables (and 
         * other variables you might use) here
	 */


        /* Read in the data file and initialize the student array */
        FILE *fp;

        if((fp=fopen(filename, "r")) == NULL) {
          printf("Cannot open input file %s for reading.\n", filename);
          exit(1);
        }
        int i =0;
        while ( (fscanf(fp, "%d%d%s\n", &(si[i].arrival_time), &(si[i].question_time), si[i].student_class)!=EOF) && i < MAX_STUDENTS ) {
             i++;
        }
        fclose(fp);
        return i;
}

/* Code executed by TA to simulate taking a break 
 * You do not need to add anything here.  
 */
static void 
take_break() {
	sleep(5);
        printf("The TA is taking a break now.\n");
}

/* Code for the TA thread. This is fully implemented except for synchronization
 * with the students.  See the comments within the function for details.
 */
void *TAthread(void *junk) {
        printf("The TA arrived and is starting his office hours\n");
	/* Loop while waiting for students to arrive. */
	while (1) {
		/* YOUR CODE HERE. */
		/* Add code here to handle the student's request.      */
                /* Currently the body of the loop is empty. There's    */
                /* no communication between TA and students, i.e. all  */
                /* students are admitted without regard of the number  */ 
                /* of available seats, which class a student is in,    */
                /* and whether the TA needs a break.                   */

		// Checking if students_since_break has reached its limit and if students_in_office is 0 i.e all 10 students have left the office
		if (students_since_break == TA_LIMIT && students_in_office == 0) {
			// TA takes break and then sem_post is called TA_LIMIT times to start another TA session
			students_since_break = 0;
			take_break();
			for (int i = 0;i < TA_LIMIT;i++) {
				sem_post(&TA_break);
			}
		}
		
		
	}

	pthread_exit(NULL);
}


/* Code executed by a OS class student to enter the office.
 * You have to implement this.  Do not delete the assert() statements,
 * but feel free to add your own.
 */
void
class_os_enter() {

	/* Request permission to enter the office.  You might also want to add  */
        /* synchronization for the simulations variables below                  */
        /*  YOUR CODE HERE.                                                     */ 

	// Checking if there are students of pfun in the office and calling sem_wait() if there are
	if (class_pfun_inoffice > 0) {
		os_outside += 1;
		sem_wait(&os);
	}
	sem_wait(&class_limit); //calling wait on that semaphore and decrementing it
	sem_wait(&TA_break); //calling wait on that semaphore and decrementing it
	
	// This is a crticial section so it is locked using the semaphore called lock
	sem_wait(&lock);
	students_in_office += 1;
	students_since_break += 1;
	class_os_inoffice += 1;
	sem_post(&lock);

}

/* Code executed by a PFUN class student to enter the office.
 * You have to implement this.  Do not delete the assert() statements,
 * but feel free to add your own.
 */
void
class_pfun_enter() {

	/* Request permission to enter the office.  You might also want to add  */
        /* synchronization for the simulations variables below                  */
        /*  YOUR CODE HERE.                                                     */ 

	// Checking if there are students of os in the office and calling sem_wait() if there are
	if (class_os_inoffice > 0) {
		pfun_outside += 1;
		sem_wait(&pfun);
	}
	sem_wait(&class_limit); //calling wait on that semaphore and decrementing it
	sem_wait(&TA_break); //calling wait on that semaphore and decrementing it
	
	// This is a crticial section so it is locked using the semaphore called lock
	sem_wait(&lock);
	students_in_office += 1;
	students_since_break += 1;
	class_pfun_inoffice += 1;
	sem_post(&lock);

}

/* Code executed by a student to simulate the time he spends in the office asking questions
 * You do not need to add anything here.  
 */
static void 
ask_questions(int t) {
	sleep(t);
}


/* Code executed by a OS class student when leaving the office.
 * You need to implement this.  Do not delete the assert() statements,
 * but feel free to add as many of your own as you like.
 */
static void 
class_os_leave() {
	/* 
	 *  YOUR CODE HERE. 
	 */

	// This is a crticial section so it is locked using the semaphore called lock
	sem_wait(&lock);
	students_in_office -= 1;
	class_os_inoffice -= 1;
	sem_post(&lock);
	// Checking if all os students have left the office so pfun students can go in
	if (class_os_inoffice == 0) {
		for (int i = 0; i <= pfun_outside ; i++) {
			sem_post(&pfun);
		}
		pfun_outside = 0;
	}
	sem_post(&class_limit); //calling post on that semaphore and incrementing it

}

/* Code executed by a PFUN class student when leaving the office.
 * You need to implement this.  Do not delete the assert() statements,
 * but feel free to add as many of your own as you like.
 */
static void 
class_pfun_leave() {
	/* 
	 *  YOUR CODE HERE. 
	 */

	// This is a crticial section so it is locked using the semaphore called lock
	sem_wait(&lock);
	students_in_office -= 1;
	class_pfun_inoffice -= 1;
	sem_post(&lock);
	// Checking if all pfun students have left the office so os students can go in
	if (class_pfun_inoffice == 0 ) {
		for (int i = 0; i <= os_outside;i++) {
			sem_post(&os);
		}
		os_outside = 0;
	}
	sem_post(&class_limit); //calling post on that semaphore and incrementing it

}

/* Main code for OS class student threads.  
 * You do not need to change anything here, but you can add
 * debug statements to help you during development/debugging.
 */
void*
class_os_student(void *si) {
	student_info *s_info = (student_info*)si;

	/* enter office */
	class_os_enter();

	 assert(students_in_office <= MAX_SEATS && students_in_office >= 0);
	 assert(class_pfun_inoffice >= 0 && class_pfun_inoffice <= MAX_SEATS);
	 assert(class_os_inoffice >= 0 && class_os_inoffice <= MAX_SEATS);
	
        /* ask questions  --- do not make changes to the 3 lines below*/
        printf("Student %d from OS class starts asking questions for %d minutes\n", s_info->student_id, s_info->question_time);
	ask_questions(s_info->question_time);
        printf("Student %d from OS class finishes asking questions and prepares to leave\n", s_info->student_id);

	/* leave office */
	class_os_leave();  

	assert(students_in_office <= MAX_SEATS && students_in_office >= 0);
	 assert(class_pfun_inoffice >= 0 && class_pfun_inoffice <= MAX_SEATS);
	 assert(class_os_inoffice >= 0 && class_os_inoffice <= MAX_SEATS);
	
	pthread_exit(NULL);
}

/* Main code for PFUN class student threads.
 * You do not need to change anything here, but you can add
 * debug statements to help you during development/debugging.
 */
void*
class_pfun_student(void *si) {
	student_info *s_info = (student_info*)si;

	/* enter office */
	class_pfun_enter();

	 assert(students_in_office <= MAX_SEATS && students_in_office >= 0);
	 assert(class_pfun_inoffice >= 0 && class_pfun_inoffice <= MAX_SEATS);
	assert(class_os_inoffice >= 0 && class_os_inoffice <= MAX_SEATS);

        printf("Student %d from PFUN class starts asking questions for %d minutes\n", s_info->student_id, s_info->question_time);
	ask_questions(s_info->question_time);
        printf("Student %d from PFUN class finishes asking questions and prepares to leave\n", s_info->student_id);

	/* leave office */
	class_pfun_leave();        

	 assert(students_in_office <= MAX_SEATS && students_in_office >= 0);
	 assert(class_pfun_inoffice >= 0 && class_pfun_inoffice <= MAX_SEATS);
	 assert(class_os_inoffice >= 0 && class_os_inoffice <= MAX_SEATS);
	
	pthread_exit(NULL);
}

/* Main function sets up simulation and prints report
 * at the end.
 */
int main(int nargs, char **args) {
	int i;
	int result;
    int student_type;
	int num_students;
    void *status;
    pthread_t ta_tid;
    pthread_t student_tid[MAX_STUDENTS];
    student_info s_info[MAX_STUDENTS];

	if (nargs != 2) {
		printf("Usage: officehour <name of inputfile>\n");
		return EINVAL;
	}

	num_students = initialize(s_info, args[1]);
	if (num_students > MAX_STUDENTS || num_students <= 0) {
		printf("Error:  Bad number of student threads. Maybe there was a problem with your input file?\n");
		return 1;
	}

	printf("Starting officehour simulation with %d students ...\n",
		num_students);
	
	result = pthread_create(&ta_tid, NULL, TAthread, NULL);
	
	if (result) {
		printf("officehour:  pthread_create failed for TA: %s\n", strerror(result));
                exit(1);
	}
	for (i=0; i < num_students; i++) {
        s_info[i].student_id = i;
		sleep(s_info[i].arrival_time);
		student_type = random() % 2;
        

		if (strcmp (s_info[i].student_class, CLASS_OS)==0) {
			result = pthread_create(&student_tid[i], NULL, class_os_student, (void *)&s_info[i]);
			
		}
		else {// student_type == CLASS_PFUN. assuming input is all correct!
			result = pthread_create(&student_tid[i], NULL, class_pfun_student, (void *)&s_info[i]);
			
		}

		if (result) {
			printf("officehour: thread_fork failed for student %d: %s\n", 
			      i, strerror(result));
                        exit(1);
		}
	}

	/* wait for all student threads to finish */
	for (i = 0; i < num_students; i++) 
		pthread_join(student_tid[i], &status);

	/* tell the TA to finish. */
	pthread_cancel(ta_tid);
	printf("Office hour simulation done.\n");

	return 0;
}
