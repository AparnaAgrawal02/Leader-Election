#include<mpi.h>

// implementation of the echo algorithm with extinction
// types of algo messages
#define token 0
#define leader 1
#define sim 2


#include<bits/stdc++.h>
using namespace std;


int main(int argc,char *argv[])
{
    MPI_Init(&argc,&argv);
    int rank,size;
    MPI_Comm_rank(MPI_COMM_WORLD,&rank);
    MPI_Comm_size(MPI_COMM_WORLD,&size);

    int simulator = 0;
    int num_neighbours = 0;
    int *neighbours;
    if (rank == simulator){
        cout << "Enter the graph" << endl;
        vector<vector<int>> graph(size);
        cout << "Enter the number of edges in the graph" << endl;
        int edges;
        cin >> edges;
        cout << "Enter the edges" << endl;
        for (int i = 0; i < edges; i++){
            int a,b;
            cin >> a >> b;
            graph[a].push_back(b);
            graph[b].push_back(a);
        }
        for (int i = 1 ; i < size; i++){
            int msg = graph[i].size();
            MPI_Send(&msg,1,MPI_INT,i,sim,MPI_COMM_WORLD);
            // cout << "Process " << i << " has " << msg << " neighbours" << endl;
            MPI_Send(&graph[i][0],graph[i].size(),MPI_INT,i,sim,MPI_COMM_WORLD);
        }
    }
    else{
        MPI_Recv(&num_neighbours,1,MPI_INT,simulator,sim,MPI_COMM_WORLD,MPI_STATUS_IGNORE);

        neighbours = (int *)malloc(num_neighbours*sizeof(int));
        MPI_Recv(neighbours,num_neighbours,MPI_INT,simulator,sim,MPI_COMM_WORLD,MPI_STATUS_IGNORE);

        for (int i = 0; i < num_neighbours; i++){
            cout << neighbours[i] << " ";
        }

        cout << endl;
    }
    cout << "Process " << rank << " has " << num_neighbours << " neighbours" << endl;

    MPI_Barrier(MPI_COMM_WORLD);

    int caw = -1;
    int rec = 0;
    int father;
    int lrec = 0;
    int winner = 0;
    int initia[size-1];
    string s;


    if (rank == simulator){
        cout << "Enter the array of size " << size-1 << " with index to be dead as 0 and alive 1: " << endl;
        for(int i=1; i<size; i++)
        {
            cin >> initia[i-1];
        }
    }

    MPI_Bcast(&initia,size-1,MPI_INT,simulator,MPI_COMM_WORLD);
    if (rank ==simulator){
        MPI_Finalize();
        return 0;
    }
    if (initia[rank-1] == 1)
    {
        caw = rank;
        int msg = caw;
        for (int i = 0; i < num_neighbours; i++){
            MPI_Send(&msg,1,MPI_INT,neighbours[i],token,MPI_COMM_WORLD);
        }
    }
    while (lrec < num_neighbours)
    {
        int msg;
        MPI_Status status;
        MPI_Recv(&msg,1,MPI_INT,MPI_ANY_SOURCE,MPI_ANY_TAG,MPI_COMM_WORLD,&status);
        if (status.MPI_TAG == leader){
            if (lrec == 0){
                for (int i =0;i < num_neighbours;i++)
                {
                    MPI_Send(&msg,1,MPI_INT,neighbours[i],leader,MPI_COMM_WORLD);
                }
            }
            lrec++;
            winner = msg;
        }
        else if (status.MPI_TAG == token){
            if (msg < caw || caw == -1){
                caw = msg;
                rec = 0;
                father = status.MPI_SOURCE;
                for (int i = 0; i < num_neighbours; i++){
                    if (neighbours[i] != father){
                        MPI_Send(&msg,1,MPI_INT,neighbours[i],token,MPI_COMM_WORLD);
                    }
                }
            }
            if(msg == caw){
                rec++;
                if (rec == num_neighbours){
                    if (caw == rank){
                        int msg = rank;
                        for (int i = 0; i < num_neighbours; i++){
                            MPI_Send(&msg,1,MPI_INT,neighbours[i],leader,MPI_COMM_WORLD);
                        }
                    }
                    else{
                        MPI_Send(&caw,1,MPI_INT,father,token,MPI_COMM_WORLD);
                    }
                }
            }
        }
    }
    if (winner == rank){
        s = "leader";
    }
    else{
        s = "not leader";
    }
    cout << "Process " << rank << " is " << s << endl;
    MPI_Finalize();
    return 0;

}
