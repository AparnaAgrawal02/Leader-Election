// peterson
//  The algorithm rst computes the smallest ID and
//  makes it known to each process, then the process
//  with that ID becomes leader and all others are
//  defeated.
#include <stdio.h>
#include <mpi.h>
#include <iostream>
#include <time.h>

using namespace std;

int main(int argc, char *argv[])
{

    int rank, size, num_recv = 0;
    double start, end;
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
        // NonBlocking
        // cout << (rank + 1) % size;
        MPI_Isend(&message, 2, MPI_INT, (rank + 1) % size, 1, MPI_COMM_WORLD, &send_request);
        //cout << "Process " << rank << " sent message to process " << (rank + 1) % size << endl;
        MPI_Recv(&message_right, 2, MPI_INT, (rank - 1 + size) % size, 1, MPI_COMM_WORLD, &Stat);
        //cout << "Process " << rank << " received message from process " << message_right[0] << " with phase " << message_right[1] << endl;
        num_recv++;
        // cout << (rank - 1 + rank) % size;
        MPI_Isend(&message, 2, MPI_INT, (rank - 1 + size) % size, 1, MPI_COMM_WORLD, &send_request);
        //cout << "Process " << rank << " sent message to process " << (rank - 1 + size) % size << endl;
        MPI_Recv(&message_left, 2, MPI_INT, (rank + 1) % size, 1, MPI_COMM_WORLD, &Stat);
        //cout << "Process " << rank << " received message from process " << message_left[0] << " with phase " << message_left[1] << endl;
        num_recv++;
        if (message[1] == -1)
        {
            simulation = 0;
            break;
        }
        if (message_left[1] == -1)
        {
            // message[1] = -1;
            //cout << "Process " << rank << " received message from process " << message_left[0] << " with phase " << message_left[1] << endl;
            relayer = 1;
        }
        else if (message_right[1] == -1)
        {
            // message[1] = -1;
            relayer = 1;
            //cout << "Process " << rank << " received message from process " << message_right[0] << " with phase " << message_right[1] << endl;
        }
        // if relayer
        if (relayer)
        {
            
            if (message_left[1] == -1)
            {
                simulation = 0;
                //cout<<"0Process "<<rank<<" came to relayer block and sent message to process "<<(rank + 1) % size<<endl;
                MPI_Send(&message_left, 2, MPI_INT,(rank - 1 + size) % size, 1, MPI_COMM_WORLD);
                break;
            }
            else if (message_right[1] == -1)
            {
                simulation = 0;
                //cout<<"Process "<<rank<<" came to relayer block and sent message to process "<<(rank - 1 + size) % size<<endl;
                MPI_Send(&message_right, 2, MPI_INT, (rank + 1) % size, 1, MPI_COMM_WORLD);
                break;
            }
         
             //cout<<"Process "<<rank<<" came to relayer block"<<endl;
        }

        else
        {
            if (message_left[0] < rank || message_right[0] < rank)
            {
                //cout << "process " << rank << " is relayer " << endl;
                relayer = 1;
                message[0] = min(message_left[0], message_right[0]);
            }
            else if (message_left[0] == rank && message_right[0] == rank)
            { // simulation = 0;
                //cout << "process " << rank << " become leader" << endl;
                message[0] = rank;
                message[1] = -1;
            }
        }

        // MPI_Recv(&simulation, 1, MPI_INT, MPI_ANY_SOURCE, 1, MPI_COMM_WORLD, &Stat);
    }
    end = MPI_Wtime();
     if(rank != 0){
        MPI_Send(&num_recv, 1, MPI_INT,0, 1, MPI_COMM_WORLD);
    }
    else{
        int msg;
        for(int i=1;i<size;i++){
        MPI_Recv(&msg, 1, MPI_INT,i, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        num_recv+=msg;
        }
        printf(" %d\n",num_recv);
    }

    //cout << "Process " << rank << " ended " << endl;

    MPI_Finalize();
    return 0;
}