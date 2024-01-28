//
//  HeapManager
//
//  Created by Mert Coskuner on 3.01.2024.
//

#include <iostream>
#include <pthread.h>
using namespace std;
//inside of a Linked list node there are ID, size, index fields
//ID is the thread id // if the chunk is free this field is -1
//SIZE and INDEX fields represent the size and the starting address of the chunk

class LinkedList{
private:
    struct node{
        int threadID ;
        int size;
        int index;
        node* next;
        node*prev;
        node( int ID, int S, int I) : threadID(ID), size(S), index(I), next(nullptr), prev(nullptr) {}
    
    };
    node* head;
    int memory;
    pthread_mutex_t mutex;
    
public:
    LinkedList(){
        pthread_mutex_init(&mutex, nullptr);
    }
    LinkedList(int s){
        head = new node(-1, s, 0);
        memory = s;
    }

    
    int Insert(int threadID, int s){
        int index=0;
        node* allocator = head;

        if(allocator->size <s){
            return -1;
        }
        
        while(allocator!= NULL){
            if(allocator->size >= s && allocator->threadID == -1){
                break;
            }
            else{
                allocator= allocator->next;
            }
            if(allocator == NULL){
                return -1;
            }
            
        }

        allocator->size = allocator->size - s ;
        node* new_node = new node(threadID,s,allocator->index);
        
        
        new_node -> next = allocator;
        new_node->prev = allocator->prev;
        
        if(allocator->prev != NULL){
            allocator->prev->next= new_node;
        }
        
        allocator->prev = new_node;//now new_node is ahead of allocator node so update prev
        //also set the new_node's next node
        if(new_node->prev == NULL){
            head = new_node;

        }
        allocator->index += new_node->size;
        new_node->index =   allocator->index - new_node->size;
        index= new_node->index;
        return index;

     
    }
    
    
    int Delete(int threadID, int index){
        node* allocator = head;

        while(allocator!= NULL){
            if(allocator->threadID == threadID && allocator->index == index){

                break;
            }
            else{

                allocator= allocator->next;
            }
            if(allocator == NULL){
                return -1;
            }
            
        }
        if(allocator-> next == NULL && allocator -> prev == NULL ){
            return 1;
        }
        else {

            if(allocator->prev == NULL &&allocator->next->threadID == -1){
                allocator->next->size += allocator->size;
                allocator->next->index -= allocator->size;
                head = allocator->next;
                head->prev=NULL;
                head->next=NULL;
                delete allocator;
            }
            else if(allocator->next->threadID == -1 && allocator->prev->threadID == -1){
                node* next_to_allocator = allocator->next;
                node* prev_to_allocator = allocator->prev;
                allocator->next->size += allocator->size;
                allocator->next->index -= allocator->size;
                delete allocator;
                next_to_allocator->prev = prev_to_allocator;
                prev_to_allocator->next = next_to_allocator;
            }
            return 1;

        }
    }

    
    void print_list(){
        node* printer = head;
        
        while(printer != NULL ){
            cout<<"["<< printer->threadID <<"]"<<" "<<"["<< printer->size <<"]" << " "<<"["<< printer->index <<"]";
            if(printer->next!= NULL){
                cout<<"- - -";
            }
            printer= printer->next;// iterating through the linked list and checking for an empty space
        }

        cout<<endl;
    }
};


class HeapManager {
public:
    
    HeapManager()  { //initialize the lock
        mutex = PTHREAD_MUTEX_INITIALIZER;
    }
    //takes the size of the heap in terms of bytes as input and initializes the heap with the requested size
    //Do not assume that we are implementing an actual heap You just initialize the linked-list that represents your heap layout. Therefore, we can safely assume that initHeap always succeeds for any positive size input and returns 1 after initializing the heap. Lastly, it prints the initial heap layout (see sample runs at the end).
    int initHeap(int size){
        heap_list = LinkedList(size);
        cout<<"Memory initialized"<<endl;
        print();
        return 1;
    }
    //takes ID of the thread issuing the operation and the requested size in bytes as input and tries to allocate heap space of requested size.
    //If there is a free chunk in the heap that can accommodate the request (that has enough space), this operation must succeed and return the beginning address of the newly allocated space.
    //Otherwise, it fails and returns -1. In both cases, it prints whether the operation succeeds or fails and the final state of the heap
    int myMalloc(int ID, int size){
        pthread_mutex_lock(&mutex);
        int index =heap_list.Insert(ID, size);
        if(index!=-1){
            cout<< "Allocated for thread "<<ID<<endl;
            print();
            pthread_mutex_unlock(&mutex);
            return index;
        }
        else{
            cout<<"Can not allocate, requested size "<<size<<" for thread "<<ID << " is bigger than remaining size"<<endl ;
            print();
            pthread_mutex_unlock(&mutex);
            return -1;
        }

        
    }
    // This operation takes a thread ID and the starting address of the chunk to be freed as input.
    //If this thread has really allocated this chunk, it frees the chunk and returns 1. Otherwise, it returns -1. In both cases, it prints whether the operation succeeds or fails and the final state of the heap
    int myFree(int ID, int index){
        pthread_mutex_lock(&mutex);
        if(heap_list.Delete(ID,index)){
            cout<<"Freed for thread "<<ID<<endl;
            print();
            pthread_mutex_unlock(&mutex);
            return 1;
        }
        else{
            cout <<"Can not free the memory for thread "<<ID<<endl ;
            print();
            pthread_mutex_unlock(&mutex);
            return -1;
        }

    }
    //Prints the memory layout
    void print(){

        heap_list.print_list();
        pthread_mutex_unlock(&mutex);

    }

private:
    LinkedList heap_list;
    pthread_mutex_t mutex;


};
