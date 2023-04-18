#include<mpi.h>
#include<stdio.h>
#include<stdlib.h>

#include <bits/stdc++.h>

using namespace std;


// implementation of the bully leader election algorithm

// types of algo messages
#define ELECTION 0
#define OK 1
#define COORDINATOR 2
#define ALIVE 3
#define DIE 4

// // types of simulator messages
#define KILL 5
// #define ALIVE 1

int main(int argc, char *argv[])
{

    MPI_Init(&argc, &argv);
    // printf("Enter the number of processes: ");
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    // return 0;
    int leader = size - 1;
    int simulator = 0;
    int alive = 1;
    int change = 0;
    // int initiator = 0;

    while (1)
    {
        if(rank == simulator)
        {
            // printf("Enter the array of size %d with index to be dead as 0 and alive 1: ", size-1);
            cout << "simulator" << endl;
            cout << "Enter the array of size " << size-1 << " with index to be dead as 0 and alive 1: " << endl;
            int arr[size-1];
            for(int i=1; i<size; i++)
            {
                // scanf("%d", &arr[i-1]);
                cin >> arr[i-1];
            }

            for (int i = 1; i < size; i++)
            {
                if(arr[i-1] == 0)
                {
                    int msg = DIE;
                    MPI_Send(&msg, 1, MPI_INT, i, KILL, MPI_COMM_WORLD);
                }
                else
                {
                    int msg = ALIVE;
                    MPI_Send(&msg, 1, MPI_INT, i, ALIVE, MPI_COMM_WORLD);
                }
            }

            // int init_proc;

            // printf("Enter the process to start the election: ");
            // scanf("%d", &init_proc);
            // // send election message to the process
            // int msg = ELECTION;
            // MPI_Send(&msg, 1, MPI_INT, init_proc, ELECTION, MPI_COMM_WORLD);
            // // MPI_Recv(&msg, 1, MPI_INT, init_proc, OK, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
        else {
            cout << "Rank " << rank << " is alive" << endl;
            // cout << "simulator" << simulator << endl;
            int msg;
            MPI_Status status;
            MPI_Recv(&msg, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

            if(msg == DIE)
            {
                change = 1 ? alive == 1 : 0;
                alive = 0;
            }
            if(msg == ALIVE)
            {
                change = 1 ? alive == 0 : 0;
                alive = 1;
            }
        }

        MPI_Barrier(MPI_COMM_WORLD);

        if(rank == simulator)
        {
            int initiator;
            // printf("Enter the process to start the election: ");
            cout << "Enter the process to start the election: ";
            // scanf("%d", &initiator);
            cin >> initiator;
            // send election message to the process
            int msg = ELECTION;
            MPI_Send(&msg, 1, MPI_INT, initiator, ELECTION, MPI_COMM_WORLD);
            // cout << "Rank " << rank << " sent election message to " << initiator << endl;
            MPI_Recv(&msg, 1, MPI_INT, initiator, OK, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        }
        // MPI_Bcast(&initiator, 1, MPI_INT, simulator, MPI_COMM_WORLD);

        // MPI_Barrier(MPI_COMM_WORLD);

        // listen for

        if(rank != simulator)
        {
            // listen for message
            int election = 0;
            MPI_Status status;
            int msg;
            int leader_elctd = 0;

            while (leader_elctd == 0 && alive == 1)
            {
                MPI_Recv(&msg, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
                cout << "Rank " << rank << " received message from " << status.MPI_SOURCE << " with tag " << status.MPI_TAG << endl;
                if (status.MPI_TAG == ELECTION)
                {
                    // send ok message to the sender

                    int reply = OK;
                    MPI_Send(&reply, 1, MPI_INT, status.MPI_SOURCE, OK, MPI_COMM_WORLD);
                    // send election message to all higher ranked processes
                    if (election == 0)
                    {
                        election = 1;

                        for (int i = rank + 1; i < size; i++)
                        {
                            MPI_Send(&msg, 1, MPI_INT, i, ELECTION, MPI_COMM_WORLD);
                        }
                        // listen for ok messages for 0.5 seconds and then send coordinator message
                        int ok_count = 0;
                        int start_time = time(NULL);
                        MPI_Request request;
                        MPI_Irecv(&msg, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &request);
                        while (time(NULL) - start_time < 2)
                        {
                            int flag;
                            MPI_Test(&request, &flag, &status);
                            if (flag)
                            {
                                cout << "Rank " << rank << " received message from " << status.MPI_SOURCE << " with tag " << status.MPI_TAG << endl;
                                if (status.MPI_TAG == OK)
                                {
                                    // cout << "Rank " << rank << " received OK from " << status.MPI_SOURCE << endl;
                                    ok_count++;
                                }
                                else if (status.MPI_TAG == ELECTION)
                                {
                                    int reply = OK;
                                    MPI_Send(&reply, 1, MPI_INT, status.MPI_SOURCE, OK, MPI_COMM_WORLD);
                                }
                                MPI_Irecv(&msg, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &request);
                                // MPI_Irecv(&msg, 1, MPI_INT, MPI_ANY_SOURCE, OK, MPI_COMM_WORLD, &request);                        }
                            }
                        }
                        MPI_Cancel(&request);
                        if (ok_count == 0)
                        {
                            // send coordinator message
                            int reply = COORDINATOR;
                            for (int i = rank - 1; i >= 0; i--)
                            {
                                MPI_Send(&reply, 1, MPI_INT, i, COORDINATOR, MPI_COMM_WORLD);
                            }
                            leader = rank;
                            leader_elctd = 1;
                            // printf("Leader elected: %d by rank %d ", leader, rank);
                            cout << "Leader elected: " << leader << " by rank " << rank << endl;

                        }
                    }



                }
                else if (status.MPI_TAG == COORDINATOR)
                {
                    leader = status.MPI_SOURCE;
                    leader_elctd = 1;
                    // printf("Leader elected: %d by rank %d ", leader, rank);
                    cout << "Leader elected: " << leader << " by rank " << rank << endl;
                }
            }
        }

        MPI_Barrier(MPI_COMM_WORLD);
    }

}

