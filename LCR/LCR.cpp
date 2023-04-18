// leader election algorithm
// Le Lann, Chang and Roberts) algorithm
#include <stdio.h>
#include <mpi.h>
#include <iostream>
#include <time.h>

using namespace std;

int main(int argc, char *argv[])
{

    int rank, size;
    int leader_id, initiator, participant = 0, simulation = 1, message[2], leader_rank;
    int id, leader = 0, num_messages = 0, phase = 0;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    srand(time(NULL));
    initiator = atoi(argv[1]);

    // invalid initiator

    if (initiator > size)
    {
        cout << "Invalid initiator" << initiator << endl;
        cout << "Please enter a number between 0 and " << size << endl;
        MPI_Finalize();
        return 0;
    }

    // assigne a random id to each process
    id = rank;
    cout << "Process " << rank << " has id " << id << endl;
    MPI_Barrier(MPI_COMM_WORLD);

    // initiator
    if (rank == initiator)
    {
        // cout << "Process " << rank << " is the initiator with id " << id << endl;
        message[0] = -1; // leader id
        message[1] = id; // participant id
        num_messages++;
        MPI_Send(&message, 2, MPI_INT, (rank + 1) % size, 0, MPI_COMM_WORLD);
        // cout<<"Process "<<rank<<" sent message to process "<<(rank+1)%size<<endl;
    }

    while (simulation)
    { // cout<<"Process "<<rank<<" is waiting for message"<<endl;
        MPI_Recv(&message, 2, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        // cout<<"Process "<<rank<<" received message from process "<<message[1] << id <<endl;

        if (message[0] != -1)
        { // cout<<"Process"<<rank<<" got the message"<<endl;
            simulation = 0;
            leader_id = message[1];
            participant = 0;
            leader_rank = message[0];
            MPI_Send(&message, 2, MPI_INT, (rank + 1) % size, 0, MPI_COMM_WORLD);
        }
        else if (message[1] < id)
        {
            participant = 1;
            message[0] = -1;
            message[1] = id;
            num_messages++;
            MPI_Send(&message, 2, MPI_INT, (rank + 1) % size, 0, MPI_COMM_WORLD);
        }
        else if (id < message[1])
        {
            message[0] = -1;
            message[1] = message[1];
            num_messages++;
            MPI_Send(&message, 2, MPI_INT, (rank + 1) % size, 0, MPI_COMM_WORLD);
        }
        else if (id == message[1])
        {
            // cout<<"Process "<<rank<<" is the leader"<<endl;
            participant = 1;
            leader = 1;
            message[0] = rank;
            num_messages++;
            MPI_Send(&message, 2, MPI_INT, (rank + 1) % size, 0, MPI_COMM_WORLD);
        }
        phase++;
        cout << "Process " << rank << ",phase " << phase << ",Id " << id << ",active " << participant << endl;
    }
 
    // send number of messages to leader
    if (leader == 0)
    {
        MPI_Send(&num_messages, 1, MPI_INT, leader_rank, 0, MPI_COMM_WORLD);
    }
       MPI_Barrier(MPI_COMM_WORLD);
    if (leader == 1)
    {
        int total_m = 0, num_m;
        for (int i = 0; i < size; i++)
        {
            if (i != rank)
            {
                MPI_Recv(&num_m, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                total_m += num_messages;
            }
        }
        total_m += num_messages;
        cout << "Process " << rank << " is the leader with id " << id << endl;
        cout << "Number of messages sent: " << total_m << endl;
    }
   

    MPI_Finalize();
    return 0;
}