#include <iostream>
#include <iostream>
#include <cstdlib>
#include <pthread.h>
#include <vector>
#include <unistd.h>
#include <fstream>
using namespace std;

#define NUM_THREADS 400 //This is actually the maximum number of clients for my program. It is larger than the value specified in the description.
pthread_mutex_t firstMutex = PTHREAD_MUTEX_INITIALIZER; //This is a mutex lock. It  is used by client threads.
pthread_mutex_t secondMutex = PTHREAD_MUTEX_INITIALIZER;    //This is the second mutex lock.  It is used by system threads.
vector<int> seats(NUM_THREADS); //This vector symbolizes seats. As a seat is reserved by system thread, an element of this vector is erased.
pthread_t systemThreads[NUM_THREADS];   //These are the system threads.
vector<int> whichSeatYouGot(NUM_THREADS);   //This vector  shows which client managed to choose which seat.
vector<int> whoOwnsTheSeat(NUM_THREADS);    //This vector shows which seat is owned by which  client.
vector<bool> selecedSeats(NUM_THREADS); //These are the seats (if true) that are reserved by the client thread.
vector<string> output;  //This is the output string.


void*SystemThreadFunc(void *threadid){
    long tid; //Thread id.
    tid = (long)threadid;
    string x; //This string will be pushed into the output vector.
    //This while loop busy waits until the client makes a valid seat choice.
    while(true){
        if(whichSeatYouGot[tid] != -1){
            break;
        }
    }
    if(pthread_mutex_lock(&secondMutex) == 0){  //Lock the mutex lock if you can.
        selecedSeats[whichSeatYouGot[tid]-1] = true; //Seat is given  to te client for good.
        //In this for loop we erase the chosen seat from the seats vector.
        for(int p = 0; p  < seats.size(); p++){
            if(seats[p] == whichSeatYouGot[tid]){
                seats.erase(seats.begin()+p);
            }
        }
        x = "Client" + to_string(tid + 1) + " reserves Seat" + to_string(whichSeatYouGot[tid]); //We create the  output string.
        output.push_back(x); //And push the output string into output vector.
    }
    pthread_mutex_unlock(&secondMutex); //We unlock the mutex lock.
    pthread_exit(NULL); //System thread exits.

}
//This function is the client tread function.
void *ClientThreadFunc(void *threadid) {
    long tid; //Id of the tread.
    tid = (long)threadid;
    int randomNumber; //This number will be used in random seat choice.
    int firstChoice;    //This will be the random seat choice.
    void *status;
    int randomSleepTime;    //This will be the random sleep time.
    randomSleepTime = random()%(150)+1 + 50;
    usleep(randomSleepTime);    //Here the client tread  sleeps for a random number of milliseconds between 50 and 200.
    pthread_create(&systemThreads[tid], NULL, SystemThreadFunc, (void *)tid);   //Here we create the system tread.
    randomNumber = random()%seats.size();   //Here we create the indice of the random  seat choice.
    firstChoice = seats[randomNumber]; //This is the random choice of the client.
    if(pthread_mutex_lock(&firstMutex) == 0){ //If can acquire first mutex, do so.
        while(true){
            if(whoOwnsTheSeat[firstChoice-1] != -1){ //If the seat is already taken, make another choice.
                randomNumber = random()%seats.size();
                firstChoice = seats[randomNumber];
            }
            else if(whoOwnsTheSeat[firstChoice-1] == -1){ //If it was not, whoOwnsTheSeat and whichSeatYouGot get updated, meaning other client cannot ask for this seat.
                whoOwnsTheSeat[firstChoice-1] = tid;
                whichSeatYouGot[tid] = firstChoice;
                break;  //And break.
            }
        }
    }
    pthread_mutex_unlock(&firstMutex); //Unlock the mutex lock.
    pthread_join(systemThreads[tid], &status);  //Wait for the system thread to finish execution and join.
    pthread_exit(NULL); //Client thread exits.
}

int main (int argc, char** argv) {
    int f = stoi(argv[1]);  // This integer is the number of clients.
    pthread_t clientThreads[f]; //These are client thhreads.
    int rc;
    int i;
    void *status;   //This will be used as a parameter while joining threads.
    string x;   //This string will be pushed into output vector.

    //In this while loop, we erase the extra slots of the vectors  and shrink them into number of seats size given by the user.
    while(seats.size() > f){
        seats.erase(seats.begin());
        whichSeatYouGot.erase(whichSeatYouGot.begin());
        whoOwnsTheSeat.erase(whoOwnsTheSeat.begin());
        selecedSeats.erase(selecedSeats.begin());
    }
    //In this for loop we initialize our vectors.
    for(int k = 0; k < seats.size(); k++){
        seats[k] = k + 1;
        whichSeatYouGot[k] = -1;
        whoOwnsTheSeat[k] = -1;
        selecedSeats[k] = false;
    }
    //We push the first line of the output.
    x = "Number of total seats: " + to_string(f);
    output.push_back(x);

    //In this for loop we create the client treads with the running function  ClientTreadFunc i, which will be te threadid.
    for( i = 0; i < f; i++ ) {
        rc = pthread_create(&clientThreads[i], NULL, ClientThreadFunc, (void *)i);

        if (rc) {
            cout << "Error:unable to create thread," << rc << endl;
            exit(-1);
        }
    }
    //In this for loop we wait until all client threads finish execution and the join them.
    for(int j = 0; j < f; j++){
        pthread_join(clientThreads[j], &status);
    }
    //We push the last line of the output.
    output.push_back("All seats are reserved.");
    //Here I just wanted to see the output on the console.
    for(int j = 0; j < output.size(); j++){
        cout << output[j] << endl;
    }
    //here, perform the output operation
    //print your results into the file named output.txt
    ofstream myfile;
    myfile.open("output.txt");
    for(int i = 0; i<output.size();i++){
        myfile << output[i]<< endl;
    }
    myfile.close();
        //Main thread exits.
        pthread_exit(NULL);


}