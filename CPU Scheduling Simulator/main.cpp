//
//  main.cpp
//  Project2
//
//  Created by Long Pham on 1/27/19.
//  Copyright © 2019 Long Pham. All rights reserved.
//

#include <iostream>
#include <queue>
#include <fstream>
#include <string.h>
#include <sstream>
#include <ctype.h>
#include <bits/stdc++.h>

using namespace std;

struct Process{ // Process Struct
    string pid;
    int arrival_time;
    int burst_time;
};



void FCFS(struct Process* proc, int n);
void SRTF(struct Process * proc, int n);
//void sortProc(Process * proc, int n);
void RRB(struct Process* proc, int n, int quantum,int wt[], int *elapsedTime);
void findWt_FCFS(struct Process* proc, int n,int wt[]);
void findWt_SRTF(struct Process* proc, int n,int wt[]);
void findTat(struct Process*proc, int n, int wt[], int tat[]);
void findAvgWt(struct Process*proc, int n,int check,int wt[]);
void findRst(struct Process*proc, int n, int wt[], int rst[]);
int countProc(char *fileName);
void loadProc(struct Process*proc,char *fileName);
float RRBAverageResponseTime(int wt[], int n,int quantum);
void showCPUUsage(struct Process * proc, int n, int elapsedTime);
void parse(struct Process* proc, string str, int n);

int main(int argc, char * argv[]) {
    // argv[0] is always the file name
    char *fileName;
    char * scheduling;
    int  quantum;
    int elapsedTime = 0;
    
    fileName = argv[1];
    scheduling = argv[2];
    if(argv[3]){
         quantum = atoi(argv[3]);
    }
   
    string sched[3] = {"FCFS","RRB","SRTF"}; //shceduling options
    
    int schedCheck = 0; //int variable to check which scheduling is selected.
    int numOfProc = countProc(fileName);
    Process *proc = new Process[numOfProc];
    loadProc(proc,fileName);
    int wt[numOfProc];
    string schedulingOption(scheduling);
    
    
    if (schedulingOption.compare(sched[0])==0) {
        schedCheck = 1;
        FCFS(proc,numOfProc);
    } else if (schedulingOption.compare(sched[2]) == 0){
        schedCheck = 2;
        //SRTF(proc, numOfProc);
    } else if (schedulingOption.compare(sched[1]) == 0){
        schedCheck = 3;
        RRB(proc,numOfProc,quantum, wt, &elapsedTime);
        showCPUUsage(proc, numOfProc,elapsedTime);
        cout << "\nAverage Response Time = " <<
        RRBAverageResponseTime(wt, numOfProc,quantum) << endl;
    }

    
    findAvgWt(proc, numOfProc, schedCheck, wt);

    
    
    return 0;
}

//Find CPU usage func
void showCPUUsage(struct Process * proc, int n, int elapsedTime){
    
    int totalBurstTime =0;
    for(int i =0; i < n; i ++){
        totalBurstTime += proc[i].burst_time;
    }
    
    cout<< "CPU USAGE "<< ((float)totalBurstTime/elapsedTime)*100 <<endl;
}

//RRB scheduling
void RRB(struct Process* proc, int n, int quantum,int wt[], int *elapsedTime){
    int rt[n];
    bool completedAll = false;
    int jobCompleted;
    int duration = 0;
    for(int i = 0; i < n; i++){
        rt[i] = proc[i].burst_time;
    }
    while(!completedAll){
        jobCompleted = 0;
        for(int i = 0; i < n ; i++){
            *elapsedTime +=1;
            if(rt[i] == 0){
                jobCompleted +=1;
                if(jobCompleted == n){
                    completedAll = true;
                    break;
                }
            }
            else{
                if(rt[i] - quantum >= 0){
                    duration += quantum;
                    if(rt[i] - quantum == 0){
                        wt[i] = duration - proc[i].burst_time;
                        cout<<"process "<<proc[i].pid<<" Finished "<<endl;
                    }
                    else{
                        cout<<"process "<<proc[i].pid<<" Running "<<endl;
                    }
                    rt[i] -= quantum;
                    
                }
                else{
                    duration += rt[i];
                    wt[i] = duration - proc[i].burst_time;
                    rt[i] = 0;
                    cout<<"process "<<proc[i].pid<<" Finished "<<endl;
                }
            }
        }
    }
}



void findWt_FCFS(struct Process* proc, int n,int wt[]){
    //First prcess WT is 0
    wt[0] = 0;
    
    //Calculating waiting time of other processes
    for (int i = 1; i < n; i++) {
        wt[i] = proc[i-1].burst_time + wt[i-1];
    }
    
}

void findWt_SRTF(struct Process* proc, int n,int wt[]){
    int rt[n];
    
    // Copy the burst time into rt[]
    for (int i = 0; i < n; i++)
        rt[i] = proc[i].burst_time;
    
    int complete = 0, t = 0, min = INT_MAX;
    int shortest = 0, finish_time;
    bool check = false;
    int time = 0;
   
    while (complete != n) {
        
        // Find process with minimum
        // remaining time among the
        // processes that arrives till the
        // current time`
        for (int j = 0; j < n; j++) {
            
            if ((proc[j].arrival_time <= t) &&
                (rt[j] < min) && rt[j] > 0) {
                min = rt[j];
                shortest = j;
                check = true;
                
            }
           
        }
        
        if (check == false) {
            t++;
            continue;
        }
        
        // Decrementing rt by 1
        rt[shortest]--;
        cout<<"Process "<<proc[shortest].pid<<" is running" << endl;
        time++;
        
        // Update minimum
        min = rt[shortest];
        if (min == 0){
            min = INT_MAX;
            cout<<"Process "<<proc[shortest].pid<<" is finished" << endl;
        }
       
        if (rt[shortest] == 0) {
           
            // Increment complete
            complete++;
            check = false;
            
            // Find finish time of current
            // process
            finish_time = t + 1;
            
            // Calculate waiting time
            wt[shortest] = finish_time -
            proc[shortest].burst_time -
            proc[shortest].arrival_time;
            
            if (wt[shortest] < 0)
                wt[shortest] = 0;
        }
        // Increment time
        t++;
    }
}

//Finding Turn around time
void findTat(struct Process*proc, int n, int wt[], int tat[]){
    for (int i = 0; i < n; i++) {
        tat[i] = proc[i].burst_time + wt[i];
    }
}


float RRBAverageResponseTime(int wt[], int n,int quantum){
    int total =0;
    int rs[n];
    rs[0] = 0;
    for(int i =1; i < n ; i++){
        total = quantum+total;
        rs[i] = total;
    }
    total = 0;
    for(int i =0; i < n;i++){
        total += rs[i];
    }
    return total/n;
}



void findAvgWt(struct Process*proc, int n,int check, int wt[]){
    
     int tat[n], total_w = 0, total_ta = 0;
    
    if(check!=3){
        if (check == 1) {
            findWt_FCFS(proc, n, wt);
        } else if (check == 2){
            findWt_SRTF(proc, n ,wt);
        }
    }
   
    
    findTat(proc, n,wt, tat);
    //findRst(proc, n, wt, rst);
    
    cout << "Processes "
    << " Burst time "
    << " Waiting time "
    << " Turn around time\n";
    
    
    for (int  i=0; i<n; i++)
    {
        total_w = total_w + wt[i];
        total_ta = total_ta + tat[i];
        cout << "   " << proc[i].pid << "\t\t" << proc[i].burst_time <<"\t  "
        << wt[i] <<"\t\t  " << tat[i] <<endl;
    }
    
    
    
    cout << "Average waiting time = "
    << (float)total_w / (float)n;
    cout << "\nAverage turn around time = "
    << (float)total_ta / (float)n;
    cout << "\nAverage response time = "
    << (float)total_w / (float)n;
    cout << endl;
}

void parse(struct Process* proc, string str, int n){
    string temp[3];
    int num1 = 0, num2 = 0;
    char cstr[str.length()];
    int count = 0;
    str.copy(cstr, str.length()); // copy string into c-string
    
    for (unsigned int i = 0; i < str.length(); i++){
        
        if (static_cast<int>(cstr[i]) == 32){ //ignore Spacing
            count++;
            continue;
        } else {
            temp[count] += cstr[i];
          
        }
     
    }
    stringstream first(temp[1]);
    stringstream second(temp[2]);

    first >> num1;
    second >> num2;

    proc[n].pid = temp[0];
    proc[n].arrival_time = num1;
    proc[n].burst_time = num2;
}

//CountProc return the number of Processes
int countProc(char* fileName){
    ifstream inFile;
    string line;
    int Pnum = 0;
    inFile.open(fileName);
    if (inFile.is_open()) {
        while (getline(inFile,line)){
            //cout << Pnum+1 << " line is: " << line << endl;
            Pnum++;
        }
    } else {
        cout << "Failed to open file." << endl;
        return 1;
    }
    
    inFile.close();
    return Pnum;
}

//loadProc to load process from processes.txt to struct array
void loadProc(struct Process*proc,char *fileName){
    ifstream inFile;
    string line;
    int Pnum = 0;
    inFile.open(fileName);
    if (inFile.is_open()) {
        while (getline(inFile,line)){
            parse(proc, line, Pnum);
            Pnum++;
        }
    } else {
        cout << "Failed to open file." << endl;
    }
    
    inFile.close();
    
}


//FCFS sched
void FCFS(struct Process* proc, int n){
    int i = 0;
    while (i != n) {
        for (int bt = proc[i].burst_time; bt > 0; bt--) {
            cout << "Running " << proc[i].pid << endl;
            
        }
        cout << "Process " << proc[i].pid << " has finished." << endl;
        i++;
    }
    cout << "All proccesses finish.......\n" << endl;
}

