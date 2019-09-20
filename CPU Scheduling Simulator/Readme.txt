PROJECT2
CPU Scheduling

How to:
	We are testing our code with our file which is "processes.txt" that including in the .tar file.

	enter ./main processes.txt [FCFS|RRB|SRTF] quantum(if it's RRB). Scheduling name has to be uppercase. 

	Or you could use our Makefile to test each scheduling algorithm.
	Just enter Make fcfs|rrb|srtf for a desired method.

	our arrival time is 0 1 2 according to P1 P2 P3. and if you test the program with our "processes.txt" file, you see the processes will be executed in that order


Team responsibility:

	Long Pham: FCFS and SRTF, debugging
	Franklin Okomba: RRB and SRTF, debugging

Strength:

	All Scheduling are working. 
	The processes are executed in the right order.
Weakness:

	RESPONSE TIME AND CPU USAGE ARE NOT CORRECT.
	We don't sort arrival time. We assume P1 always executed first then P2 and P3.
