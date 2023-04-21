#include "mpi.h"
#include <iostream>
#include <stdlib.h>
using namespace std;

#define RST "\x1B[0m"
#define KRED "\x1B[31m"
#define KGRN "\x1B[32m"
#define KYEL "\x1B[33m"
#define KBLU "\x1B[34m"
#define KMAG "\x1B[35m"
#define KCYN "\x1B[36m"
#define KWHT "\x1B[37m"

#define FRED(x) KRED x RST
#define FGRN(x) KGRN x RST
#define FYEL(x) KYEL x RST
#define FBLU(x) KBLU x RST
#define FMAG(x) KMAG x RST
#define FCYN(x) KCYN x RST
#define FWHT(x) KWHT x RST

int myrandom(int i)
{
    return rand() % i;
}

int main(int argc, char *argv[])
{
    int num_procs, rank, ierr;
    int min, lcounter;

    double t1, t2;

    int inmsg1; // incoming msg from right
    int inmsg2; // incoming msg from left
    int msg;    // outgoing msg

    MPI_Status status;
    MPI_Request send_req, recv_req;

    ierr = MPI_Init(&argc, &argv);

    bool terminated = false;
    bool passive = false;

    if (ierr != 0)
    {
        cout << "Error starting MPI." << endl;
        MPI_Abort(MPI_COMM_WORLD, ierr);
    }

    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank); // my ID

    t1 = MPI_Wtime();
    msg = rank;

    while (!terminated)
    {
        // receive ID with counter from left and right
        if (rank == 0)
        {
            MPI_Isend(&msg, 1, MPI_INT, 1, 1, MPI_COMM_WORLD, &send_req);
            MPI_Recv(&inmsg1, 1, MPI_INT, num_procs - 1, 1, MPI_COMM_WORLD, &status);
            MPI_Isend(&msg, 1, MPI_INT, num_procs - 1, 1, MPI_COMM_WORLD, &send_req);
            MPI_Recv(&inmsg2, 1, MPI_INT, 1, 1, MPI_COMM_WORLD, &status);
        }
        else if (rank == num_procs - 1)
        {
            MPI_Isend(&msg, 1, MPI_INT, 0, 1, MPI_COMM_WORLD, &send_req);
            MPI_Recv(&inmsg1, 1, MPI_INT, rank - 1, 1, MPI_COMM_WORLD, &status);
            MPI_Isend(&msg, 1, MPI_INT, rank - 1, 1, MPI_COMM_WORLD, &send_req);
            MPI_Recv(&inmsg2, 1, MPI_INT, 0, 1, MPI_COMM_WORLD, &status);
        }
        else
        {
            MPI_Isend(&msg, 1, MPI_INT, rank + 1, 1, MPI_COMM_WORLD, &send_req);
            MPI_Recv(&inmsg1, 1, MPI_INT, rank - 1, 1, MPI_COMM_WORLD, &status);
            MPI_Isend(&msg, 1, MPI_INT, rank - 1, 1, MPI_COMM_WORLD, &send_req);
            MPI_Recv(&inmsg2, 1, MPI_INT, rank + 1, 1, MPI_COMM_WORLD, &status);
        }

        // forward the message if passive
        if (passive)
        {
            if (rank == 0)
            {
                MPI_Send(&inmsg1, 1, MPI_INT, 1, 1, MPI_COMM_WORLD);
                MPI_Send(&inmsg2, 1, MPI_INT, num_procs - 1, 1, MPI_COMM_WORLD);
            }
            else if (rank == num_procs - 1)
            {
                MPI_Send(&inmsg1, 1, MPI_INT, 0, 1, MPI_COMM_WORLD);
                MPI_Send(&inmsg2, 1, MPI_INT, rank - 1, 1, MPI_COMM_WORLD);
            }
            else
            {
                MPI_Send(&inmsg1, 1, MPI_INT, rank + 1, 1, MPI_COMM_WORLD);
                MPI_Send(&inmsg2, 1, MPI_INT, rank - 1, 1, MPI_COMM_WORLD);
            }
        }
        else
        {
            if (inmsg1 < rank || inmsg2 < rank)
                passive = true;
            else if (inmsg1 == rank && inmsg2 == rank)
            {
                terminated = true;
                cout << "Node " << rank << " elected as a leader" << endl;
                t2 = MPI_Wtime();
                cout << "Elapsed time: " << (t2 - t1) << " seconds" << endl;
            }
        }

        ierr = MPI_Bcast(&terminated, 1, MPI_INT, 0, MPI_COMM_WORLD);
    }
    MPI_Finalize();
}