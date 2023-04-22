#include <mpi.h>

// implementation of the echo algorithm with extinction
// types of algo messages

#include <bits/stdc++.h>
#define token 0
#define sim 2
MPI_Group group_world;
MPI_Group Non_Simulators;
MPI_Comm Non_Simulators_comm;
int *process_ranks;

using namespace std;
int find_diameter(vector<vector<int>> graph, int size)
{
    int diameter = 0, s;
    for (int i = 0; i < size; i++)
    {
        int l = 0;
        vector<bool> visited;
        int level[size];
        for(int x=0;x<size;x++){
            level[x]=0;
        }
        visited.resize(size, false);

        // Create a queue for BFS
        list<int> queue;
        visited[i] = true;
        level[i]=0;

        queue.push_back(i);
        while (!queue.empty())
        {

            // Dequeue a vertex from queue and print it
            s = queue.front();
            queue.pop_front();

            // Get all adjacent vertices of the dequeued
            // vertex s. If a adjacent has not been visited,
            // then mark it visited and enqueue it
            int level_change = 0;
            for (auto adjacent : graph[s])
            {
                if (!visited[adjacent])
                {
                    visited[adjacent] = true;
                    queue.push_back(adjacent);
                    level[adjacent] = level[s]+1;
                }
            }
        
        }
        for(int x=0;x<size;x++){
            if(l<level[x]){
                l = level[x];
            }
        }
        if (diameter < l)
        {
            diameter = l;
        }
    }
    return diameter;
}

int main(int argc, char *argv[])
{
    MPI_Init(&argc, &argv);
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    process_ranks = (int *)malloc((size - 1) * sizeof(int));
    MPI_Comm_group(MPI_COMM_WORLD, &group_world);

    for (int I = 1; I < size; I++)
        process_ranks[I-1] = I;

    // create the new group
    MPI_Group_incl(group_world, size-1, process_ranks, &Non_Simulators);
    // create the new communicator
    MPI_Comm_create(MPI_COMM_WORLD, Non_Simulators, &Non_Simulators_comm);

    int simulator = 0;
    int num_neighbours = 0;
    int *neighbours;
    int num_recv = 0;
    int num_sent = 0;
    int diameter = 5;
    int leader = 0;
    if (rank == simulator)
    {
        cout << "Enter the graph" << endl;
        vector<vector<int>> graph(size);
        cout << "Enter the number of edges in the graph" << endl;
        int edges;
        cin >> edges;
        cout << "Enter the edges" << endl;
        for (int i = 0; i < edges; i++)
        {
            int a, b;
            cin >> a >> b;
            graph[a].push_back(b);
            graph[b].push_back(a);
        }
        for (int i = 1; i < size; i++)
        {
            int msg = graph[i].size();
            cout << msg;
            MPI_Send(&msg, 1, MPI_INT, i, sim, MPI_COMM_WORLD);
            // cout << "Process " << i << " has " << msg << " neighbours" << endl;
            MPI_Send(&graph[i][0], graph[i].size(), MPI_INT, i, sim, MPI_COMM_WORLD);
        }
        diameter = find_diameter(graph, size);
        
        // cout << "diameter of this graph " << diameter << endl;
        //  MPI_Bcast(&diameter, 1, MPI_INT, 1, MPI_COMM_WORLD);
    }
    else
    {
        MPI_Recv(&num_neighbours, 1, MPI_INT, simulator, sim, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        neighbours = (int *)malloc(num_neighbours * sizeof(int));
        MPI_Recv(neighbours, num_neighbours, MPI_INT, simulator, sim, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        for (int i = 0; i < num_neighbours; i++)
        {
            cout << neighbours[i] << " ";
        }

        cout << endl;
        //   MPI_Bcast(&diameter, 1, MPI_INT, 1, MPI_COMM_WORLD);
    }


    cout << "Process " << rank << " has " << num_neighbours << " neighbours" << endl;
    MPI_Bcast(&diameter,1,MPI_INT,0,MPI_COMM_WORLD);
    MPI_Barrier(MPI_COMM_WORLD);
    int start = time(NULL);
    if (rank != simulator)
    {
        leader = rank;
        printf("diameter %d",diameter);
        while ( diameter )
        {
            diameter--;

            for (int i = 0; i < num_neighbours; i++)
            {
                int msg = leader;
                //cout << "RANK " << rank << " send message" << msg << " to " << neighbours[i] << endl;
                MPI_Send(&msg, 1, MPI_INT, neighbours[i], 1, MPI_COMM_WORLD);
                num_sent++;
            }
            //MPI_Barrier(Non_Simulators_comm);
            //printf("---------------------------------------------");
            for (int i = 0; i < num_neighbours; i++)
            {
                //cout << i << "wainging " << rank << endl;
                int msg;
                MPI_Recv(&msg, 1, MPI_INT, neighbours[i], 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                //cout << "recieved " << msg << "to " << rank;
                leader = max(msg, leader);
            }
            MPI_Barrier(Non_Simulators_comm);
        }

        cout << "Process " << rank << "knows: " << leader << "is leader" << endl;
    }
    if(rank != simulator){
        MPI_Send(&num_sent, 1, MPI_INT, simulator, 1, MPI_COMM_WORLD)
    }
    else{
        int msg;
        MPI_Recv(&msg, 1, MPI_INT, MPI_ANY_SOURCE, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        num_sent+=msg;
        print("Total messages sent: ",num_sent);
    }
    MPI_Finalize();
    return 0;
}