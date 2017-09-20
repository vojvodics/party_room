#include <stdio.h>
#include <pthread.h>

// macros
#define MAX_NUM_OF_STUDENTS 50
#define DEAN_IN_THE_ROOM 1
#define DEAN_WAITING 0
#define DEAN_NOT_HERE -1
#define DEAN_LEFT 2

// global variables
int num_of_students;
int dean_status;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t wait_for_full_room, wait_for_students_to_leave;

// prototypes
void* dean_code(void*);
void* students_code(void*);

int main(void)
{
    // definitions of variables
    pthread_t dean, students;
    num_of_students = 0;
    dean_status = DEAN_NOT_HERE;
    
    // initialize mutex and conds
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&wait_for_full_room, NULL);
    pthread_cond_init(&wait_for_students_to_leave, NULL);
    
    // create threads
    pthread_create(&students, NULL, students_code, NULL);
    pthread_create(&dean, NULL, dean_code, NULL);
    
    // join threads
    pthread_join(dean, NULL);
    pthread_join(students, NULL);
    
    // free memory
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&wait_for_students_to_leave);
    pthread_cond_destroy(&wait_for_full_room);
    
    return 0;
}

void* dean_code(void* param)
{
    pthread_mutex_lock(&mutex);
    // check if students came first
    if (num_of_students > 0 && num_of_students < MAX_NUM_OF_STUDENTS)
    {
        // dean is waiting for students to fill the room
        dean_status = DEAN_WAITING;
        printf("Dean is waiting\n");
        // wait and unlock mutex
        pthread_cond_wait(&wait_for_full_room, &mutex);
    }
    // wait for students to leave the room
    if (num_of_students >= MAX_NUM_OF_STUDENTS)
    {
        // dean enters the room
        dean_status = DEAN_IN_THE_ROOM;
        printf("Dean breaks the party\n");
        pthread_cond_wait(&wait_for_students_to_leave, &mutex);
    }
    // else search the room
    else
    {
        dean_status = DEAN_IN_THE_ROOM;
        printf("Dean is searching the room\n");
    }
    dean_status = DEAN_LEFT;
    printf("Dean left\n");
    pthread_mutex_unlock(&mutex);
}

void* students_code(void* param)
{
    while(1)
    {
        pthread_mutex_lock(&mutex);
        // check if dean is in the room
        if (dean_status == DEAN_IN_THE_ROOM)
        {
            // leave the room
            if (num_of_students > 0)
            {
                printf("Students are leaving: %d\n", num_of_students);
                num_of_students--;
            }
            else
            {
                // signal the dean that all students left
                pthread_cond_signal(&wait_for_students_to_leave);
                // unlock the mutex
                pthread_mutex_unlock(&mutex);
                // and end the party
                break;
            }
        }
        // else students can enter the room and party, because dean already left or he is not in the room
        else
        {
            // not allowing students to enter if dean did not start
            if (num_of_students < MAX_NUM_OF_STUDENTS)
            {
                num_of_students++;
                printf("Students are partying: %d\n", num_of_students);
            }
            // if dean is waiting and students maxed out
            if (dean_status == DEAN_WAITING && num_of_students >= MAX_NUM_OF_STUDENTS)
            {
                // wake up dean thread from waiting for full room
                pthread_cond_signal(&wait_for_full_room);
            }
            // if dean left and students maxed out end the program
            if (dean_status == DEAN_LEFT && num_of_students >= MAX_NUM_OF_STUDENTS)
                break;
        }
        pthread_mutex_unlock(&mutex);
    }
}
