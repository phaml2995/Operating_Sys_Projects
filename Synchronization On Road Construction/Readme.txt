CPSC 3500
Project 3
/*
 * README file for project 3
 */

///////// Team member's name and contributions //////////////////////
Team member #1: Long Pham
Contributions: Debug, South and North Car threads.
Percentage of contributions: 50% 

Team member #2: Paul Franklin Okomba
Contributions:Design, Debug, Traffic handler thread.
Percentage of contributions: 50%



/////// Thread information /////////////////////////////////////////
Total threads: 

[Thread #1]
  --> Task: producer that produces north cars
  --> Thread function name: northTraffic

[Thread #2]
  --> Task: producer that produces south cars
  --> Thread function name: southTraffic

[Thread #3]
  --> Task: consumer to handle traffic
  --> Thread function name: handleTraffic




////// Semaphores ////////////////////////////////////////////////
Number of semaphores:

[Sempahore #1]
  --> Variable: carNumber
  --> Initial value:
  --> Purpose: make xxx blocked upon on events xxxx

............



////// Mutex lock ///////////////////////////////////////////////
Number of mutex locks: 3

[Mutex lock #1]
  --> Variable: northQueuelock
  --> Purpose: avoid race condition on shared data structure of northcars queue

[Mutex lock #1]
  --> Variable: southQueuelock
  --> Purpose: avoid race condition on shared data structure of southcars queue

............


///// Strenths  ////////////////////////////////////////////////

successfully compiled and able to output the time correctly into car.log and flagperson.log







//// Weaknesses ///////////////////////////////////////////////









