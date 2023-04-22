//peterson
// The algorithm rst computes the smallest ID and
// makes it known to each process, then the process
// with that ID becomes leader and all others are
// defeated.
#include <stdio.h>
#include <mpi.h>
#include <iostream>
#include <time.h>

using namespace std;

int main(int argc, char *argv[])
{

    int rank, size;
    double start,end;
    int leader_id, initiator, participant = 0, relayer = 0, simulation = 1, message_right[2], message_left[2], message[2], leader_rank;
    int id, leader = 0, num_messages = 0, phase = 0;
    MPI_Init(&argc, &argv);
    MPI_Request send_request, recv_request;
    MPI_Status Stat;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    srand(time(NULL));

    message[0] = rank;
    message[1] = 0; // Phase
    start = MPI_Wtime(); 
    while (simulation)
    { // SEND MEESAGE
        //NonBlocking
        //cout<<(rank + 1) % size;
        MPI_Isend(&message, 2, MPI_INT, (rank + 1) % size, 1, MPI_COMM_WORLD, &send_request);
        MPI_Recv(&message_right, 2, MPI_INT, (rank - 1 + size) % size, 1, MPI_COMM_WORLD, &Stat);
        //cout<<(rank - 1 + rank) % size;
        MPI_Isend(&message, 2, MPI_INT, (rank - 1 + size) % size, 1, MPI_COMM_WORLD, &send_request);
        MPI_Recv(&message_left, 2, MPI_INT, (rank + 1) % size, 1, MPI_COMM_WORLD, &Stat);

        if(message_left[1] == -1){
            simulation = 0;
        }
        else if (message_right[1]== -1){
           simulation = 0;
        }

        // if relayer
        if (relayer)
        {
            MPI_Send(&message_right, 2, MPI_INT, (rank - 1 + size) % size, 1, MPI_COMM_WORLD);
            MPI_Send(&message_left, 2, MPI_INT, (rank + 1) % size, 1, MPI_COMM_WORLD);
        }

        else
        {
            if (message_left[0] < rank || message_right[0] < rank)
            {
                relayer = 1;
            }
            else if (message_left[0] == rank && message_right[0] == rank)
            {   //simulation = 0;
                cout << "process " << rank << " become leader" << endl;
                message[0] = rank;
                message [1] = -1;
            
            }
            else{

                message[1]+=1;
            }
        }
        //MPI_Recv(&simulation, 1, MPI_INT, MPI_ANY_SOURCE, 1, MPI_COMM_WORLD, &Stat);
    }
    end = MPI_Wtime(); 
	
cout<<"Process "<<rank<<" ended "<<endl;
MPI_Finalize();

if (rank ==0){
    cout<<"Time taken: "<< end - start <<endl;
}
}