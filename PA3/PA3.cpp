#include <iostream>
#include <pthread.h>
#include <vector>
#include <semaphore.h>
#include <fcntl.h>
using namespace std;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;

pthread_barrier_t barrier;  // looking
pthread_barrier_t barrier1;  // looking

sem_t fanSemaphoreA;
sem_t fanSemaphoreB;
sem_t general_semaphore;
int spin = 0; 
int ID=0;
int captain = 0;
struct FAN
{
    pthread_t fan_thread;
    string team;
};
vector<FAN> Thread_FAN;


void *fan_thread(void *arg)
{
    string* team_ptr = static_cast<string*>(arg);
    string team = *team_ptr;
    int x ;
    int y; 
    int z;
    string signal="0"; 
    pthread_mutex_lock(&mutex);
    spin++; 
    pthread_mutex_unlock(&mutex);

    /*while(signal != "0" && spin >=3){//basic spin based lock structure
        if(team==signal){
            signal = "0" ;  
        }
    }*/


    pthread_mutex_lock(&mutex);
    cout << "Thread ID: " << (unsigned long)pthread_self() << ", Team: "<< team << ", I am looking for a car." << endl;

    //
    if(team=="A"){
        sem_getvalue(&fanSemaphoreA,&x);
        if(x==0){
            pthread_mutex_unlock(&mutex); 
        }
        sem_wait(&fanSemaphoreA); 
    }
    else{

        sem_getvalue(&fanSemaphoreB,&y);
        if(y==0){
            pthread_mutex_unlock(&mutex); 
        }
        sem_wait(&fanSemaphoreB);
    }

 



    if(sem_wait(&general_semaphore)==0){
        sem_getvalue(&fanSemaphoreA,&x); 
        sem_getvalue(&fanSemaphoreB,&y); 
        sem_getvalue(&general_semaphore,&z); 

        if(x==3 && y==0){ 

            pthread_mutex_unlock(&mutex);
            for(int i = 0 ; i< 3 ; i++){
                sem_wait(&fanSemaphoreA);
            }
            signal="B";
            sem_post(&fanSemaphoreB); 
        }
        else if(x==0 && y==3){ 

            pthread_mutex_unlock(&mutex);
            for(int i = 0 ; i< 3 ; i++){
                sem_wait(&fanSemaphoreB);
            }
            signal="A";

            sem_post(&fanSemaphoreA); 
        }
        else if(x==2 && y==1){

            pthread_mutex_unlock(&mutex); 
            sem_wait(&fanSemaphoreA);

            sem_wait(&fanSemaphoreB);

        }
        else if(x==1 && y==2){

            pthread_mutex_unlock(&mutex); 
            sem_wait(&fanSemaphoreB);
            sem_wait(&fanSemaphoreA);
        }
   
        else{
            pthread_mutex_unlock(&mutex);

        }
        
    }

    pthread_barrier_wait(&barrier);


    pthread_mutex_lock(&mutex);
    cout << "Thread ID: " << (unsigned long)pthread_self() << ", Team: "<<team<< ", I have found a spot in a car." << endl;
    captain++;
    pthread_mutex_unlock(&mutex);
    pthread_barrier_wait(&barrier1);


    pthread_mutex_lock(&mutex);

    if(captain==4){
        cout << "Thread ID: " << (unsigned long)pthread_self() << ", Team: "<<team<< ", I am the captain and driving the car with ID " << ID << endl;
        ID++;
        captain =0; 
        for(int i =0; i< 4; i++){
            sem_post(&general_semaphore); 
        }

        for(int i=0 ; i< 3 ; i++){
            sem_post(&fanSemaphoreA);
        }
        for(int i=0 ; i< 3 ; i++){
            sem_post(&fanSemaphoreB);
        }
    }

  
    pthread_mutex_unlock(&mutex);

    return NULL; 

}



int main(int argc, const char *argv[])
{
    int argument1 = stoi(argv[1]);
    int argument2 = stoi(argv[2]);
    sem_init(&fanSemaphoreA, 0, 3);
    sem_init(&fanSemaphoreB, 0, 3);
    sem_init(&general_semaphore, 0, 4);


    pthread_barrier_init(&barrier, nullptr, 4);
    pthread_barrier_init(&barrier1, nullptr, 4);


  


    // Implementing the check details for supporters
    if (!(argument1 % 2 == 0) || !(argument2 % 2 == 0))
    {
        cout << "The main terminates"<<endl;
        return 0;
    }
    if (!((argument1 + argument2) % 4 == 0))
    {
        cout << "The main terminates"<<endl;
        return 0;
    }
    // for team A
    for (int i = 0; i < argument1; i++)
    { // creating threads for team A
        pthread_t thread;
        FAN new_fan;
        new_fan.fan_thread = thread;
        new_fan.team = "A";
        Thread_FAN.push_back(new_fan);
    }

    // for team B
    for (int i = 0; i < argument2; i++)
    { // creating threads for team B
        pthread_t thread;
        FAN new_fan;
        new_fan.fan_thread = thread;
        new_fan.team = "B";
        Thread_FAN.push_back(new_fan);
    }
    for(int i=0; i<Thread_FAN.size();i++){
        pthread_t thread;
        pthread_create(&Thread_FAN[i].fan_thread, NULL, fan_thread, (void *)& Thread_FAN[i].team);

    }

    for (int i = 0; i < Thread_FAN.size(); i++)
    {
        pthread_join(Thread_FAN[i].fan_thread, NULL);
    }


    cout << "The main terminates"<<endl;
    return 0;
}
