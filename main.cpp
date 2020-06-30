#include <iostream>
#include <fstream>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <queue>
#include <thread>

using namespace std;

pthread_mutex_t atm[11] = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER; // mutex initializer

int number_of_customer; // number of customer(determined from the first line of the file)
// this integers hold the total payments
int total_electricity=0;int total_gas=0;int total_telecommunication=0;int total_cableTV = 0;int total_water=0;

//this struct hold information about customers
struct Customer {
    int customer_id;
    int sleep_time;
    int atm_instance;
    string bill_type;
    int amount;
};

queue <Customer> outputList; // this list use for writing output
Customer customers[300]; // customer adds to this array, max 300 customer

//this struct hold information for send to atm
struct sendToAtm{
    int customer_id;
    int atm_instance;
    int amount;
    string bill_type;
};

//server threads' execution begins
void *serverRunner (void *param){
    // create info according to coming *param and assign it
    struct sendToAtm *info = (struct sendToAtm *) param;
    // lock the mutex and start critical section
    pthread_mutex_lock(&m);
    //assign necessary values
    int id = info->customer_id;
    customers[id].customer_id = info->customer_id;
    customers[id].atm_instance = info->atm_instance;
    customers[id].amount = info->amount;
    customers[id].bill_type = info->bill_type;
    // according to bill type make changes in total payments
    if(customers[id].bill_type == "electricity"){
        total_electricity = total_electricity + customers[id].amount;
    }else if(customers[id].bill_type == "gas"){
        total_gas = total_gas + customers[id].amount;
    }else if(customers[id].bill_type == "water"){
        total_water = total_water + customers[id].amount;
    }else if(customers[id].bill_type == "cableTV"){
        total_cableTV = total_cableTV + customers[id].amount;
    }else{
        total_telecommunication = total_telecommunication + customers[id].amount;
    }
    // push this values to output list
    outputList.push(customers[id]);
    // unlock mutex
    pthread_mutex_unlock(&m);
    //finish thread
    pthread_exit(0);
}

//client threads' execution begins
void *requestAtm(void *param) {
    struct Customer *customerInfo = (struct Customer*) param;
    usleep((unsigned int) customerInfo->sleep_time*1000);  //each thread sleeps for a coming time from input
    // lock the thread and start critical section
    pthread_mutex_lock(&atm[customerInfo->atm_instance]);
    pthread_t serverThread;
    sendToAtm serverArgs;
    serverArgs.customer_id = customerInfo->customer_id;
    serverArgs.atm_instance = customerInfo->atm_instance;
    serverArgs.bill_type = customerInfo->bill_type;
    serverArgs.amount = customerInfo->amount;
    //server thread is created
    pthread_create(&serverThread,NULL,serverRunner,&serverArgs);
    // end of the critical section
    pthread_mutex_unlock(&atm[customerInfo->atm_instance]);
    //thread will wait for server thread to finish
    pthread_join(serverThread,NULL);
    pthread_exit(0);
}

int main(int argc, char *argv[]) {
    // read the file
    ifstream file(argv[1]);
    file >> number_of_customer; // assign number of customer from file
    //create customer vector, take input from file add this vector for temporarily
    vector <Customer> temp;
    Customer c; //create customer
    int counter = 1; // counter for number of customer
    c = {0,0,0,"",0}; // make first customer to 0,0,0,"",0
    temp.push_back(c); // push the temp vector
    while(file.peek() != EOF){ //while file is not on the end, get line one by one and create customers
        string sleep_time,ATM_instance,bill_type,amount;
        getline(file,sleep_time,',');
        getline(file,ATM_instance,',');
        getline(file,bill_type,',');
        getline(file,amount,'\n');
        c.customer_id = counter;
        c.sleep_time = stoi(sleep_time);
        c.atm_instance = stoi(ATM_instance);
        c.bill_type = bill_type;
        c.amount = stoi(amount);
        counter++;
        // push the temp vector
        temp.push_back(c);
    }
    // create thread for number of customers
    pthread_t threads[number_of_customer];
    // assign temp vector to customer vector
    for(int i = 1; i<=number_of_customer; i++){
        customers[i] = temp[i];
    }
    // create all threads
    for(int a =1 ; a<=number_of_customer; a++){
        pthread_create(&threads[a],NULL,requestAtm,&temp[a]);
    }
    // join all threads
    for(int b=1; b<=number_of_customer; b++){
        pthread_join(threads[b],NULL);
    }
    // create output file
    ofstream myfile("output.txt");
    while(!outputList.empty()){ // when list is not empty, write one by one to output.txt and pop from the list
        myfile<<"Customer"<<outputList.front().customer_id<<","<<outputList.front().amount<<","<<outputList.front().bill_type<<endl;
        outputList.pop();
    }
    // write output according to defined log
    myfile<<"All payments are completed"<<endl;
    myfile<<"CableTV: " <<total_cableTV <<"TL"<< endl;
    myfile<<"Electricity: " <<total_electricity <<"TL"<< endl;
    myfile<<"Gas: " <<total_gas <<"TL"<< endl;
    myfile<<"Telecommunication: " <<total_telecommunication <<"TL"<< endl;
    myfile<<"Water: " <<total_water <<"TL"<< endl;
    // close the file and finish the program
    myfile.close();
    return 0;
}