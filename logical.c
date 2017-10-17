//********************************************************************
//
// Rahul Bethi
// Advanced Operating Systems
// Project #2: Berkeley Algorithm, Logical Clocks, and Vector Clocks
// October 27, 2016
// Instructor: Dr. Ajay K. Katangur
//
// Part 2: Logical Clocks
//
//********************************************************************

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <string.h> // to use strings

int main(int argc, char *argv[])
{
	int rank, i, size, root = 0;
	int clock = 0, clocks[100], signal[3], t_processes, sProcess, ack, l, ln;
	FILE *inputFile;
	char line[500], message[500], number[4];
	
	MPI_Status status;
	
	MPI_Init(&argc, &argv); // Initializing MPI
	MPI_Comm_rank(MPI_COMM_WORLD, &rank); // to know the rank of each process
	MPI_Comm_size(MPI_COMM_WORLD, &size); // to know the total number of processes
	
	if(rank == root)
	{
		inputFile = fopen(argv[1], "r"); // open file in read only mode, file name is obtained from command line argument
		if(!inputFile)
		{
			printf("Error: Cannot read file!\n");
			end_all_processes(size);
			return;
		}
		fgets(line, 500, inputFile); // read the first line to determine the number of processors being participated
		t_processes = atoi(line);
		printf("There are %d processes in the system\n", t_processes);
		if(size != t_processes+1) // if total number of processors is not equal to the number of processors specified + 1, then end with error
		{
			printf("Error: %d processes are needed to run! But this MPI has %d processes\n", t_processes+1, size);
			end_all_processes(size); // to end all the remaining processes
			fclose(inputFile);
			return;
		}
		while( fgets(line, 500, inputFile) != NULL ) // read each line, repeat untill the last line
		{
			l = strlen(line)-2;
			if( strncmp(line, "end", 3) == 0 ) // check for end
			{
				signal[0] = 3; // end signal
				for(i=1; i<=t_processes; i++)
					MPI_Send(signal, 3, MPI_INT, i, 0, MPI_COMM_WORLD); // request other processes for clock values
				
				for(i=1; i<=t_processes; i++)
				{
					MPI_Recv(&clock, 2, MPI_INT, MPI_ANY_SOURCE, 1, MPI_COMM_WORLD, &status); // recieve clock values
					clocks[status.MPI_SOURCE] = clock;
				}
				for(i=1; i<=t_processes; i++)
					printf("At the end, Logical clock at process %d is %d\n", i, clocks[i]); // display clock values of the processes
				fclose(inputFile);
				MPI_Finalize(); // end
				return;
			}
			else if( strncmp(line, "exec", 4) == 0 ) // check for exec
			{
				signal[0] = 1; // exec signal
				for(i=5; i<l; i++)
					number[i-5] = line[i];
				number[i-5] = '\0';
				sProcess = atoi(number); // process to be executed
				MPI_Send(signal, 3, MPI_INT, sProcess, 0, MPI_COMM_WORLD); // send execute command
			}
			else if( strncmp(line, "send", 4) == 0 ) // check for send
			{
				signal[0] = 2; // send signal
				for(i=0; line[i+5] != ' '; i++)
					number[i] = line[i+5];
				number[i] = '\0';
				ln = i+6;
				sProcess = atoi(number); // process to be executed
				for(i=0; line[i+ln] != ' '; i++)
					number[i] = line[i+ln];
				number[i] = '\0';
				ln = i+ln+2;
				signal[1] = atoi(number); // receiver of message
				for(i=0; line[i+ln] != '"'; i++)
					message[i] = line[i+ln]; // take message
				message[i] = '\0'; // last character of message should be null
				signal[2] = i; // length of the message
				MPI_Send(signal, 3, MPI_INT, sProcess, 0, MPI_COMM_WORLD); // send send command, with receiver and length of the message
				MPI_Send(message, i, MPI_CHAR, sProcess, 2, MPI_COMM_WORLD); // send message
				MPI_Recv(&ack, 1, MPI_INT, MPI_ANY_SOURCE, 3, MPI_COMM_WORLD, &status); // receive acknowledgement from receiver to proceed to next line, to continue simulation
			}
		}
	}
	else // remaining processes
	{
		while(1) // repeat continuously
		{
			MPI_Recv(signal, 3, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status); // wait here for instruction form root
			
			if(signal[0] == 3) // end signal requesting clock values
				MPI_Send(&clock, 1, MPI_INT, root, 1, MPI_COMM_WORLD); // send clock values
			
			if(signal[0] == 0 || signal[0] == 3) // terminate if end signal or terminate signal is given
			{
				MPI_Finalize();
				return;
			}
			clock++; // increment clock
			if(signal[0] == 1) // execute signal
				printf("Execution event in process %d\n", rank);
			
			else if(signal[0] == 2 || signal[0] == 4) // send or receive message signal
			{
				MPI_Recv(message, signal[2], MPI_CHAR, MPI_ANY_SOURCE, 2, MPI_COMM_WORLD, &status); // receive message from sender or root
				message[signal[2]] = '\0'; // append message with null character at the last
				if(signal[0] == 4) // receive signal
				{
					if(clock <= signal[1]) // if clock value is not more than sender, add 1 to the clock value of sender and set it
						clock = signal[1] + 1;
					printf("Message received from process %d by process %d: %s\n", status.MPI_SOURCE, rank, message);
					MPI_Send(&ack, 1, MPI_INT, root, 3, MPI_COMM_WORLD); // send acknowledgement to root
				}
				else // send signal
				{
					signal[0] = 4; // receiver signal
					sProcess = signal[1]; // receiver rank
					signal[1] = clock; // clock value to be sent to receiver
					printf("Message sent from process %d to process %d: %s\n", rank, sProcess, message);
					MPI_Send(signal, 3, MPI_INT, sProcess, 0, MPI_COMM_WORLD); // send receive signal, clock value, message length
					MPI_Send(message, signal[2], MPI_CHAR, sProcess, 2, MPI_COMM_WORLD); // send message to receiver
				}
			}
			printf("Logical time at process %d is %d\n", rank, clock); // print logical clock for every change in clock value of the processes
		}
	}
	
	MPI_Finalize();
	return;
}
//********************************************************************
//
// End all processes Function
//
// This function performs a linear search on an list of student
// records.  If the search key is found in the list, the return
// value is true and false otherwise.
//
//
// Value Parameters
// ----------------
// size			int		  number of processes to be terminated + 1
//
// Local Variables
// ---------------
// i 		int		Loop Iteration Variable
// signal	int		Temporary Array used as signal to terminate
//					all processes
//
//*******************************************************************
int end_all_processes(int size) // to end processes
{
	int i, signal[3];
	for(i=1; i<size; i++)
	{
		signal[0] = 0; // terminate signal
		MPI_Send(signal, 3, MPI_INT, i, 0, MPI_COMM_WORLD); // send terminate signal to processes
	}
	MPI_Finalize();
	return;
}