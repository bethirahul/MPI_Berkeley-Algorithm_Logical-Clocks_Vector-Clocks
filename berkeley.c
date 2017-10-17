//********************************************************************
//
// Rahul Bethi
// Advanced Operating Systems
// Project #2: Berkeley Algorithm, Logical Clocks, and Vector Clocks
// October 27, 2016
// Instructor: Dr. Ajay K. Katangur
//
// Part 1: Berkeley Algorithm
//
//********************************************************************

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <string.h> // to use strings

int main(int argc, char *argv[])
{
	int rank, i, size, coo, lastLine;
	FILE *inputFile;
	char line[500][5], timeString[5];
	int time[2], min, dMin, cMin, diffMin, pMin[500], t, iFlag[500], sumIFlags, temp1;
	float avgMin, adjMin, temp;
	
	MPI_Status status;
	
	MPI_Init(&argc, &argv); // Initializing MPI
	MPI_Comm_rank(MPI_COMM_WORLD, &rank); // to know the rank of each process
	MPI_Comm_size(MPI_COMM_WORLD, &size); // to know the total number of processes
	
	inputFile = fopen(argv[1], "r"); // open file in read only mode, file name is obtained from command line argument
	if(!inputFile)
	{
		printf("Error: Cannot read file by process %d!\n", rank);
		MPI_Finalize();
		return;
	}
	
	i = 0;
    while( fgets(line[i], 500, inputFile) != NULL ) // read everyline into line 2D array
	{
		if(rank == (i-1))
		{
			time[0] = ((line[i][0]-'0')*10) + (line[i][1]-'0'); // hours
			time[1] = ((line[i][3]-'0')*10) + (line[i][4]-'0'); // minutes
			min = (time[0]*60) + time[1]; // convert each process time from string to minutes integer
		}
		i++;
	}
	lastLine = i-1;
	if(size != lastLine - 1)
	{
		printf("Error: Number of processes must be %d\n", lastLine-1);
		MPI_Finalize();
		return;
	}
	coo = atoi(line[0]);
	//coo = line[0][0]-'0';
	if(rank == coo)
	{
		time[0] = ((line[lastLine][0]-'0')*10) + (line[lastLine][1]-'0'); // hours
		time[1] = ((line[lastLine][3]-'0')*10) + (line[lastLine][4]-'0'); // minutes
		dMin = (time[0]*60) + time[1]; // convert last line, time difference to be ignored from string to minutes integer
		
		printf("I am process with rank %d acting as the coordinator process\n", rank);
	}
	fclose(inputFile);
	
	if(rank == coo) // cordinator process
	{
		strncpy(timeString, line[rank+1], 5);
		timeString[5] = '\0';
		printf("Coordinator process is sending time %s\n", timeString);
		MPI_Bcast(timeString, 5, MPI_CHAR, coo, MPI_COMM_WORLD); // broadcasting coordinator time
		MPI_Barrier(MPI_COMM_WORLD); // barrier is used so that all processors wait for each other after broadcasting is done
		for(i=0; i<size-1; i++)
		{
			MPI_Recv(&t,  1,  MPI_INT,  MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
			pMin[status.MPI_SOURCE] = t;
			printf("Process %d has received time differential value of %d\n", rank, t);
			if(abs(t) > dMin) // check if  the time difference has to ignored
			{
				iFlag[status.MPI_SOURCE] = 0; // iFlag is used to tell if the time is ignored, 0 if ignored, 1 if not ignored
				printf("Coordinator Process is ignoring time differential value of %d from process %d\n", t, status.MPI_SOURCE);
			}
			else
				iFlag[status.MPI_SOURCE] = 1;
		}
		avgMin = 0;
		sumIFlags = size;
		for(i=0; i<size; i++)
			if(i != rank)
			{
				avgMin = avgMin + (pMin[i]*iFlag[i]); // multiply with iFlag to ignore the processes
				sumIFlags = sumIFlags + iFlag[i] - 1; // remove the ignored processes from total processes to calculate average
			}
		avgMin = avgMin / sumIFlags; // average value
		printf("Time differential average is %f\n", avgMin);
		min = min + avgMin; // adjust coordinator time value
		for(i=0; i<size; i++)
			if(i != rank)
			{
				adjMin = avgMin - pMin[i]; // adjust time to be adjusted for each process
				printf("Coordinator process is sending the clock adjustment value of %f to process %d\n", adjMin, i);
				MPI_Send(&adjMin, 1, MPI_FLOAT, i, 1, MPI_COMM_WORLD); // send time to be adjusted to respective processes
			}
	}
	else
	{
		MPI_Bcast(timeString, 5, MPI_CHAR, coo, MPI_COMM_WORLD); // receive broadcasted coordinator time
		MPI_Barrier(MPI_COMM_WORLD);
		printf("Process %d has received time %s\n", rank, timeString);
		time[0] = ((timeString[0]-'0')*10) + (timeString[1]-'0'); // hours
		time[1] = ((timeString[3]-'0')*10) + (timeString[4]-'0'); // minutes
		cMin = (time[0]*60) + time[1]; // convert received coordinator string into minutes integer
		diffMin = min - cMin; // c=calculate difference in time with coordinator time
		printf("Process %d is sending time differential value of %d to process %d\n", rank, diffMin, coo);
		MPI_Send(&diffMin, 1, MPI_INT, coo, 0, MPI_COMM_WORLD); // send time difference to coordinator
		MPI_Recv(&adjMin, 1, MPI_FLOAT, MPI_ANY_SOURCE, 1, MPI_COMM_WORLD, &status); // receive time to be adjusted from coordinator
		printf("Process %d has received the clock adjustment value of %f\n", rank, adjMin);
		min = min + adjMin; // adjust the time
	}
	
	// converting time in minutes to hours and minutes of every process
	temp = min;
	temp = temp/60;
	time[0] = temp;
	temp = (temp - time[0])*60;
	time[1] = round(temp);
	if(time[1] == 60)
	{
		time[1] = 0;
		time[0]++;
	}
	if(time[0] == 24)
		time[0] = 0;
	
	//converting hours to string
	temp = time[0];
	temp = temp/10;
	temp1 = temp;
	timeString[0] = temp1 + '0';
	temp = (temp - temp1)*10;
	temp1 = temp;
	timeString[1] = temp + '0';
	
	timeString[2] = ':';
	
	//converting minutes to string
	temp = time[1];
	temp = temp/10;
	temp1 = temp;
	timeString[3] = temp1 + '0';
	temp = (temp - temp1)*10;
	temp1 = temp;
	timeString[4] = temp + '0';
	printf("Adjusted local time at process %d is %s\n", rank, timeString); // displaying final time
	
	MPI_Finalize();
	return;
}