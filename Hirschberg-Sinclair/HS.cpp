// Hirschberg-Sinclair algorithm
#include <mpi.h>
#include <stdio.h>
#include <iostream>
#include <bits/stdc++.h>
#include <math.h>
#include <stdlib.h>

using namespace std;
#define send_data_tag 2001
#define return_data_tag 2002
#define final_decision_data_tag 2003

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

#define BOLD(x) "\x1B[1m" x RST
#define UNDL(x) "\x1B[4m" x RST

int myrandom(int i)
{
    return rand() % i;
}

int main(int argc, char *argv[])
{
    int my_id, ierr, num_procs = 0;
    MPI_Status status;
    int root_process = 0;
    int d = 0;
    int total_num_messages = 0;

    ierr = MPI_Init(&argc, &argv);
    ierr = MPI_Comm_rank(MPI_COMM_WORLD, &my_id);
    ierr = MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
    int who_leader = my_id;

    vector<int> ranks(num_procs);

    if (my_id == root_process)
    {
        for (int i = 0; i < num_procs; i++)
            ranks[i] = i;

        random_shuffle(ranks.begin(), ranks.end());
    }

    ierr = MPI_Bcast(&ranks[0], num_procs, MPI_INT, root_process, MPI_COMM_WORLD);
    // cout << my_id << " " << ranks[my_id] << endl;

    int is_local_leader = true;
    int phase = -1;
    // bidirectional ring
    MPI_Barrier(MPI_COMM_WORLD);

    do
    {
        phase++;
        d = 0;
        // cout << FCYN("Phase ") << phase << endl;
        int num_hops = 1 << phase;

        if (is_local_leader)
        {
            pair<int, int> message;
            message.first = ranks[my_id];
            message.second = num_hops;

            ierr = MPI_Send(&message, 1, MPI_2INT, (my_id + num_procs - 1) % num_procs, send_data_tag, MPI_COMM_WORLD);
            total_num_messages++;
            ierr = MPI_Send(&message, 1, MPI_2INT, (my_id + 1) % num_procs, send_data_tag, MPI_COMM_WORLD);
            total_num_messages++;

            // cout << FBLU("Local leader ") << ranks[my_id] << FBLU(" sent the pitch messages to ") << ranks[(my_id + num_procs - 1) % num_procs] << FBLU(" and ") << ranks[(my_id + 1) % num_procs] << endl;
        }

        pair<int, int> inmsg;
        int start_time = time(NULL);
        MPI_Request request;

        MPI_Irecv(&inmsg, 1, MPI_2INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &request);

        while (time(NULL) - start_time < 2)
        {
            int flag;
            MPI_Test(&request, &flag, &status);
            if (flag)
            {
                if (status.MPI_TAG == send_data_tag)
                {

                    if (status.MPI_SOURCE == (my_id + num_procs - 1) % num_procs)
                    {
                        if (inmsg.first < ranks[my_id])
                            ;

                        else if (inmsg.second == 1)
                        {
                            // reset number_of_hops left and reverse direction
                            inmsg.second = num_hops;
                            ierr = MPI_Send(&inmsg, 1, MPI_2INT, (my_id + num_procs - 1) % num_procs, return_data_tag, MPI_COMM_WORLD);
                            total_num_messages++;

                            // cout << ranks[my_id] << FMAG(" sent a message with return_data_tag to ") << ranks[(my_id + num_procs - 1) % num_procs] << endl;
                        }
                        else
                        {
                            inmsg.second = inmsg.second - 1;
                            ierr = MPI_Send(&inmsg, 1, MPI_2INT, (my_id + 1) % num_procs, send_data_tag, MPI_COMM_WORLD);
                            total_num_messages++;

                            // cout << ranks[my_id] << FRED(" forwarded a message with send_data_tag from ") << ranks[(my_id + 1) % num_procs] << endl;
                        }
                    }
                    else
                    {
                        // cout << ranks[my_id] << FYEL(" received a message with send_data_tag from ") << ranks[(my_id + 1) % num_procs] << endl;
                        if (inmsg.first < ranks[my_id])
                            ;

                        else if (inmsg.second == 1)
                        {
                            // reset
                            inmsg.second = num_hops;
                            ierr = MPI_Send(&inmsg, 1, MPI_2INT, (my_id + 1) % num_procs, return_data_tag, MPI_COMM_WORLD);
                            total_num_messages++;

                            // cout << ranks[my_id] << FMAG(" sent a message with return_data_tag to ") << ranks[(my_id + 1) % num_procs] << FMAG(" with num_hops = ") << inmsg.second << endl;
                        }
                        else
                        {
                            inmsg.second = inmsg.second - 1;
                            ierr = MPI_Send(&inmsg, 1, MPI_2INT, (my_id + num_procs - 1) % num_procs, send_data_tag, MPI_COMM_WORLD);
                            total_num_messages++;

                            // cout << ranks[my_id] << FRED(" forwarded a message with send_data_tag to ") << ranks[(my_id + num_procs - 1) % num_procs] << FRED(" with num_hops = ") << inmsg.second << endl;
                        }
                    }
                }
                else
                {
                    // cout << ranks[my_id] << FCYN(" received a message with return_data_tag from ") << status.MPI_SOURCE << FCYN(" with num_hops = ") << inmsg.second << endl;

                    if (status.MPI_SOURCE == (my_id + num_procs - 1) % num_procs)
                    {
                        if (inmsg.second == 1 && inmsg.first == ranks[my_id])
                        {
                            // cout << ranks[my_id] << FYEL(" received replies from both sides") << endl;
                            d = d + 1;
                        }
                        else
                        {
                            inmsg.second = inmsg.second - 1;
                            ierr = MPI_Send(&inmsg, 1, MPI_2INT, (my_id + 1) % num_procs, return_data_tag, MPI_COMM_WORLD);
                            total_num_messages++;

                            // cout << my_id << FGRN(" sent a message with return data tag to ") << (my_id + 1) % num_procs << endl;
                        }
                    }
                    else
                    {
                        if (inmsg.second == 1 && inmsg.first == ranks[my_id])
                        {
                            // cout << ranks[my_id] << FYEL(" received replies from both sides") << endl;
                            d = d + 1;
                        }
                        else
                        {
                            inmsg.second = inmsg.second - 1;
                            ierr = MPI_Send(&inmsg, 1, MPI_2INT, (my_id + num_procs - 1) % num_procs, return_data_tag, MPI_COMM_WORLD);
                            total_num_messages++;

                            // cout << my_id << FGRN(" sent a message with return data tag to ") << (my_id + num_procs - 1) % num_procs << endl;
                        }
                    }
                }

                MPI_Irecv(&inmsg, 1, MPI_2INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &request);
            }
        }

        MPI_Cancel(&request);

        if (d != 2)
        {
            // cout << FRED("not a leader ") << ranks[my_id] << endl;
            is_local_leader = false;
        }

        MPI_Barrier(MPI_COMM_WORLD);
    } while (pow(2, phase) <= (num_procs / 2));

    if (is_local_leader)
    {
        // cout << ranks[my_id] << FGRN(" is the leader") << endl;
        ierr = MPI_Send(&ranks[my_id], 1, MPI_INT, (my_id + 1) % num_procs, final_decision_data_tag, MPI_COMM_WORLD);
        total_num_messages++;

        ierr = MPI_Recv(&who_leader, 1, MPI_INT, (my_id - 1 + num_procs) % num_procs, final_decision_data_tag, MPI_COMM_WORLD, &status);
    }
    else
    {
        ierr = MPI_Recv(&who_leader, 1, MPI_INT, (my_id - 1 + num_procs) % num_procs, final_decision_data_tag, MPI_COMM_WORLD, &status);
        ierr = MPI_Send(&who_leader, 1, MPI_INT, (my_id + 1) % num_procs, final_decision_data_tag, MPI_COMM_WORLD);
        total_num_messages++;
    }

    // cout << FGRN("For ") << my_id << FGRN(" the leader is ") << who_leader << endl;

    int final_total = 0;
    MPI_Reduce(&total_num_messages, &final_total, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    if (my_id == root_process)
        cout << final_total << endl;

    ierr = MPI_Finalize();
}

// broadcast who is the leader
