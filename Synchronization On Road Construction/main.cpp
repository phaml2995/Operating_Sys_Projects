//
//  main.cpp
//  trafficControl
//
//  Created by Franklin okomba on 2/14/19.
//  Copyright © 2019 Franklin okomba. All rights reserved.
//
//File path /Users/franklinokomba/Movies/CPlus_XCode/trafficControl/data.txt
#include <iostream>
#include <pthread.h>
#include <stdio.h>
#include <fstream>
#include <cstdlib>
#include <queue>
#include <semaphore.h>
#include <ctime>
#include <sstream>
#include <csignal>
#define NORTH "NORTH"
#define SOUTH "SOUTH"
#define NEUTRAL "NEUTRAL"

using namespace std;


struct Car{
    int carID;
    string direction;
    time_t arrivalTime;
    time_t startTime;
    time_t endTime;
};


sem_t carNumber;
//sem_t *semaphore;
pthread_mutex_t northQueueLock;
pthread_mutex_t southQueueLock;
pthread_mutex_t firstAndLastCarLock;
bool over = false;
queue <Car> Northcars;
queue <Car> Southcars;
int carIDs = 0;
ofstream carlogFile, flagPersonLogFile;


string timeFormat(time_t t);
void signalHandler(int signum);
int pthread_sleep(int seconds);
void LogFlagPersonStatus(bool working);
void carLog(Car car);
int numberOfCarsWaiting(queue<Car> cars,pthread_mutex_t lock);
string timePriority();
string queuePriority(string currentDirection);
void* northTraffic(void* args);
void* southTraffic(void* args);
void enableTraffic(string direction ,queue<Car> &cars,pthread_mutex_t *lock);
bool noCarsLeft();
void* handleTraffic(void* args);


int main(int argc, const char * argv[]) {
    if (sem_init(&carNumber, 0, 0) == -1){
        cout<<"SEMAPHORE ISSUE"<<endl;
        return -1;
    }
    pthread_mutex_init(&northQueueLock, NULL);
    pthread_mutex_init(&southQueueLock, NULL);
    pthread_mutex_init(&firstAndLastCarLock,NULL);
    pthread_t northBound, southBound, officer;
    flagPersonLogFile.open("flagperson.log");
    carlogFile.open("car.log");
    
    if(flagPersonLogFile){
        
        flagPersonLogFile <<"time"<<"      "<<"state"<<endl;
        flagPersonLogFile.close();
    }
    if(carlogFile){
        
        carlogFile <<"carID   "<<"Direction   "<<"arrival-time   "<<"start-time   "<<"end-time"<<endl;
        carlogFile.close();
    }
    pthread_create(&officer,NULL,handleTraffic,NULL);
    pthread_create(&northBound,NULL,northTraffic,NULL);
    pthread_create(&southBound,NULL,southTraffic,NULL);
    cout<<"SYSTEM RUNNING -- PRESS CTRL+C to STOP "<<endl;
    while(!over){
        signal(SIGINT, signalHandler);
    }
    pthread_join(officer,NULL);
    pthread_join(northBound,NULL);
    pthread_join(southBound,NULL);
    
    
    sem_destroy(&carNumber);
    pthread_mutex_destroy(&southQueueLock);
    pthread_mutex_destroy(&northQueueLock);
    pthread_mutex_destroy(&firstAndLastCarLock);
    return 0;
}

string timeFormat(time_t t){
    stringstream ss;
    tm* tm_local = localtime(&t);
    ss<<tm_local->tm_hour<<":"<<tm_local->tm_min<<":"<<tm_local->tm_sec;
    return ss.str();
}


bool anotherCar(){
    float value = rand()%10;
    return value> 8;
}

void signalHandler(int signum)

{
    over = true;
    exit(signum);
}

int pthread_sleep(int seconds)
{
    pthread_mutex_t mutex;
    pthread_cond_t conditionvar;
    struct timespec timetoexpire;
    if (pthread_mutex_init(&mutex, NULL))
    {
        return -1;
    }
    if (pthread_cond_init(&conditionvar, NULL))
    {
        return -1;
    }
    timetoexpire.tv_sec = (unsigned int)time(NULL) + seconds;
    timetoexpire.tv_nsec = 0;
    return pthread_cond_timedwait(&conditionvar, &mutex, &timetoexpire);
}

void LogFlagPersonStatus(bool working){
    string status = working==true?"woken-up":"sleep";
    time_t currentTime = time(NULL);
    flagPersonLogFile.open("flagperson.log",ios::app);
    if(flagPersonLogFile){
        flagPersonLogFile << timeFormat(currentTime)<<"      "<<status<<endl;
        flagPersonLogFile.close();
    }
}

void carLog(Car car){
    carlogFile.open("car.log",ios::app);
    if(carlogFile){
        carlogFile<<car.carID<<"      "<<car.direction<<"      "<<timeFormat(car.arrivalTime)<<"      "<<timeFormat(car.startTime)<<"      "<<timeFormat(car.endTime)<<endl;
        carlogFile.close();
    }
}

string timePriority(){
    Car northBoundFirstCar, southBoundFirstCar;
    pthread_mutex_lock(&southQueueLock);
    southBoundFirstCar = Southcars.front();
    pthread_mutex_unlock(&southQueueLock);
    pthread_mutex_lock(&northQueueLock);
    northBoundFirstCar = Northcars.front();
    pthread_mutex_unlock(&northQueueLock);
    if(difftime(southBoundFirstCar.arrivalTime, northBoundFirstCar.arrivalTime) == 0){
        return NEUTRAL;
    }
    else if(difftime(southBoundFirstCar.arrivalTime, northBoundFirstCar.arrivalTime) > 0){
        return SOUTH;
    }
    return NORTH;
}

string queuePriority(string currentDirection){
    int carsWaitingSouth = numberOfCarsWaiting(Southcars,southQueueLock);
    int carsWaitingNorth = numberOfCarsWaiting(Northcars,northQueueLock);
    if(carsWaitingSouth==0 && carsWaitingNorth==0){
        return NEUTRAL;
    }
    else{
        if(carsWaitingSouth>0 && carsWaitingNorth==0){
            return SOUTH;
        }
        else if( carsWaitingSouth==0 && carsWaitingNorth>0){
            return NORTH;
        }
        else{
            if(currentDirection == SOUTH && carsWaitingNorth>=10){
                return NORTH;
            }
            else if(currentDirection == NORTH && carsWaitingSouth>=10){
                return SOUTH;
            }
            else{
                if(currentDirection == NEUTRAL){
                    string arrivalTimePriority = timePriority();
                    if(arrivalTimePriority == NORTH){
                        return NORTH;
                    }
                    else{
                        return SOUTH;
                    }
                }
                
                return currentDirection;
            }
        }
    }
}

void* northTraffic(void* args){
    while(!over){
        do{
            pthread_mutex_lock(&northQueueLock);
            Car car;
            car.direction = NORTH;
            car.arrivalTime = time(NULL);
            Northcars.push(car);
            pthread_mutex_unlock(&northQueueLock);
            sem_post(&carNumber);
        }
        while(anotherCar());
        pthread_sleep(21);
    }
    pthread_exit(NULL);
}

void* southTraffic(void* args){
    while(!over){
        do{
            pthread_mutex_lock(&southQueueLock);
            Car car;
            car.direction = SOUTH;
            car.arrivalTime = time(NULL);
            Southcars.push(car);
            pthread_mutex_unlock(&southQueueLock);
            sem_post(&carNumber);
        }
        while(anotherCar());
        pthread_sleep(21);
    }
    pthread_exit(NULL);
}

void enableTraffic(string direction ,queue<Car> &cars,pthread_mutex_t lock){
    Car car;
    pthread_mutex_lock(&lock);
    car = cars.front();
    car.carID = carIDs++;
    cars.pop();
    pthread_mutex_unlock(&lock);
    car.startTime = time(NULL);
    pthread_sleep(2);
    car.endTime = time(NULL);
    carLog(car);
}

int numberOfCarsWaiting(queue<Car> cars,pthread_mutex_t lock){
    int count = 0;
    pthread_mutex_lock(&lock);
    count = (int)cars.size();
    pthread_mutex_unlock(&lock);
    return count;
}

bool noCarsLeft(){
    int sum = 0;
    pthread_mutex_lock(&firstAndLastCarLock);
    sum += Northcars.size() + Southcars.size();
    pthread_mutex_unlock(&firstAndLastCarLock);
    return sum == 0?true:false;
}

void* handleTraffic(void* args){
    string currentDirection = NEUTRAL;
    int count =0;
    do{
        if(noCarsLeft()){
            LogFlagPersonStatus(false);
            count = 0;
        }
        sem_wait(&carNumber);
        
        pthread_mutex_lock(&firstAndLastCarLock);
        count++;
        if(count == 1){
            LogFlagPersonStatus(true);
        }
        
        string priorityDirection = queuePriority(currentDirection);
        if(priorityDirection == NEUTRAL){
            LogFlagPersonStatus(false);
        }
        else{
            if(priorityDirection == SOUTH){
                enableTraffic(priorityDirection,Southcars,southQueueLock);
            }
            else{
                enableTraffic(priorityDirection,Northcars,northQueueLock);
            }
        }
        currentDirection = priorityDirection;
        pthread_mutex_unlock(&firstAndLastCarLock);
    }
    while(!over);
    pthread_exit(NULL);
}
