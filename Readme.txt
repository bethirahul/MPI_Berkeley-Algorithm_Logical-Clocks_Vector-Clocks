//********************************************************************
//
//  Rahul Bethi
//  Advanced Operating Systems
//  Programming Project #2: Berkeley Algorithm, Logical Clocks,
//                          and Vector Clocks
//  October 27, 2016
//  Instructor: Dr. Ajay K. Katangur
//
//  Read me file
//
//********************************************************************
--------------------------
Part 1: Berkeley Algorithm
--------------------------
This is program is to implement berkeley time synchronization algorithm using MPI for communication. Each process has its own time and they all communicate with the coordinator to set their clocks. Input file gives the details of the coordinator process rank and time of each process and then the last line is the time difference to be ignored to calculate average time difference. File name of the input file is given as a command line argument. Every process reads the file, takes its own time and the coordinator process rank. Coordinator process also takes the time difference to be ignored from the last line. After the file is read, the coordinator process broadcasts its time to all the processes. All the processes will receive the coordinator time, and then calculates the time difference with its own time. This time difference is sent to the coordinator process back. Coordinator process collects all the time differences from all the processes and then calculated average. It also determines which process needs to be ignored and ignores them while calculating average time difference. Using this average time, coordinator time sets its own time and then calculates time to be adjusted for each process with the time difference sent by them earlier. This value obtaines is now transmitted to respective processes. ALl the processes then recieve the time to be adjusted and they adjust the time. The time to be adjusted is also calculated for the process which were ignored while calculating average time difference. Finally all the process output their time which is adjusted and terminates.

Note:- Time is converted to minutes while calculating and then changed back to hh:mm format to display at the last. During this calculation, round() function is used to get exact time (eg: to convert 4.99999 to 5). C compiler of std 89 gives a warning message about that function "incompatible implicit declaration of built-in function ‘round’". I came to know that a C compiler of std 99 wouldn't give this warning.

----------------------
Part 2: Logical Clocks
----------------------
This program is to implement Lamport's logical clocks using MPI. Same like the previous program, instructions are taken from an input text file. File name of the input file is given as a command line argument. the root process 0 acts as the organiser, simulating the setup. process 0 reads the input file and sends the instructions to the respective process. three commands exec, send, end are identified. for exec command, the following number after that is the process which needs to be executed. This process is sent a message to execute. That process executes and increments to clock value. For send command, two numbers are given after that. one is the process which needs to send and the other is the process which needs to recieve. After these two numbers, we have the message. the sender increments its colck and sends the message. The receiver receives the message and compares its colck value with the sender. If its value is equal to ot below the sender clock, it will set its clock value to one time unit more than the sender clock value. The thrid command is the end command. This command sends signal to all process to report their colck values to process 0. All the processes sends their clock value to the process 0 and terminate. Process 0 then shows the clock values of all the clocks in their rank order and then terminates.

---------------------
Part 3: Vector Clocks
---------------------
This program is to implement Vector clocks using MPI. This is same as the previous program, but instead of using a single integer value for clock, we have an array for each process to store the value of all clocks. In exec command, the process only increments its value in its clock. When a message is passed, the receiver function will compare all the values of its clock with the sender clock. It updates all the clock values of all the process if they are less than the senders clock's respective process' values. End function will display all the clock values of what all the processes have in their rank order.

//********************************************************************
//  FILE NAMES
//********************************************************************
Readme.txt
Makefile
berkeley.c
berkeley.pdf
berkeley.txt
logical.c
logical.pdf
logical.txt
vector.c
vector.pdf
vector.txt

//********************************************************************
//  EXAMPLE OUTPUT
//********************************************************************
--------------------------
Part 1: Berkeley Algorithm
--------------------------
Input file berkeley.txt >>
3
16:20
13:58
14:12
09:48
02:12

$ mpirun -np 4 berkeley berkeley.txt
I am process with rank 3 acting as the coordinator process
Coordinator process is sending time 09:4802:12
Process 3 has received time differential value of 250
Coordinator Process is ignoring time differential value of 250 from process 1
Process 3 has received time differential value of 392
Coordinator Process is ignoring time differential value of 392 from process 0
Process 3 has received time differential value of 264
Coordinator Process is ignoring time differential value of 264 from process 2
Time differential average is 0.000000
Coordinator process is sending the clock adjustment value of -392.000000 to process 0
Coordinator process is sending the clock adjustment value of -250.000000 to process 1
Coordinator process is sending the clock adjustment value of -264.000000 to process 2
Adjusted local time at process 3 is 09:48
Process 0 has received time 09:48
Process 0 is sending time differential value of 392 to process 3
Process 0 has received the clock adjustment value of -392.000000
Adjusted local time at process 0 is 09:48
Process 1 has received time 09:48
Process 1 is sending time differential value of 250 to process 3
Process 1 has received the clock adjustment value of -250.000000
Adjusted local time at process 1 is 09:48
Process 2 has received time 09:48
Process 2 is sending time differential value of 264 to process 3
Process 2 has received the clock adjustment value of -264.000000
Adjusted local time at process 2 is 09:48

----------------------
Part 2: Logical Clocks
----------------------
Input file logical.txt >>
3
exec 1
exec 2
send 1 2 "silly message"
send 3 2 "flanker"
exec 3
send 2 1 "rally sport"
end

$ mpirun -np 4 logical logical.txt
There are 3 processes in the system
Execution event in process 2
Logical time at process 2 is 1
Message received from process 1 by process 2: silly message
Logical time at process 2 is 3
Message received from process 3 by process 2: flanker
Logical time at process 2 is 4
Message sent from process 2 to process 1: rally sport
Logical time at process 2 is 5
Execution event in process 1
Logical time at process 1 is 1
Message sent from process 1 to process 2: silly message
Logical time at process 1 is 2
Message received from process 2 by process 1: rally sport
Logical time at process 1 is 6
At the end, Logical clock at process 1 is 6
At the end, Logical clock at process 2 is 5
At the end, Logical clock at process 3 is 2
Message sent from process 3 to process 2: flanker
Logical time at process 3 is 1
Execution event in process 3
Logical time at process 3 is 2

---------------------
Part 3: Vector Clocks
---------------------
Input file vector.txt
3
exec 1
exec 2
send 1 2 "silly message"
send 3 2 "flanker"
exec 3
send 2 1 "rally sport"
end

$ mpirun -np 4 vector vector.txt
There are 3 processes in the system
At the end, Logical clock at process 1 is ( 3 4 1 )
At the end, Logical clock at process 2 is ( 2 4 1 )
At the end, Logical clock at process 3 is ( 0 0 2 )
Execution event in process 1
Logical clock at process 1 is ( 1 0 0 )
Message sent from process 1 to process 2: silly message
Logical clock at process 1 is ( 2 0 0 )
Message received from process 2 by process 1: rally sport
Logical clock at process 1 is ( 3 4 1 )
Execution event in process 2
Logical clock at process 2 is ( 0 1 0 )
Message received from process 1 by process 2: silly message
Logical clock at process 2 is ( 2 2 0 )
Message received from process 3 by process 2: flanker
Logical clock at process 2 is ( 2 3 1 )
Message sent from process 2 to process 1: rally sport
Logical clock at process 2 is ( 2 4 1 )
Message sent from process 3 to process 2: flanker
Logical clock at process 3 is ( 0 0 1 )
Execution event in process 3
Logical clock at process 3 is ( 0 0 2 )