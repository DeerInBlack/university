#include <iostream>
#include <string>
#include <vector>
#include <mpi.h>
#include "../common/bf.h"
#include "../common/common.h"

using namespace std;

int main(int argc, char* argv[]) {
    int pr_num, pr_rank, root = 0;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &pr_num);
    MPI_Comm_rank(MPI_COMM_WORLD, &pr_rank);

    unsigned int len;
    string mask, hash;
    vector<char> charset;
    vector<size_t> st, end;
    if (!parseArguments(argc, argv, len, mask, hash)) {
        if (pr_rank == root) {
            cout << "Not enough arguments\n\t check if these arguments are present:"
                << "\n\t\t'-len' - length of sought word (optional if '-mask')"
                << "\n\t\t'-mask' - mask for characters in word (opianal if '-len')"
                << "\n\t\t'-hash' - hex encoded SHA256 digest\n";
        }
        MPI_Finalize();
        return 0;
    }
    if (!BF::GenCharset(mask, len, charset, st, end)) {
        if (pr_rank == root) {
            cout << "Unable to generate charset\n\t'-len' or '-mask' is incorrect\n";
        }
        MPI_Finalize();
        return 0;
    }

    BF* bf = NULL;
    bool any_job = false;
    bool got_job = splitCharset(st, end, pr_rank, pr_num) && (bf = BF::Initialize(hash, charset, st, end));
    MPI_Allreduce(&got_job, &any_job, 1, MPI_C_BOOL, MPI_LOR, MPI_COMM_WORLD);
    if (!any_job) {
        if (pr_rank == root) {
            cout << "Unable to bruteforce\n\t'-hash' is possibly incorrect\n";
        }
        MPI_Finalize();
        return 0;
    }
    if (pr_rank == 0) {
        cout << "\nBegin to bRuTeFoRcE SHA256\n->hash: " << hash
            << "\n->len: " << len << "\n->mask: " << mask << "\n\n";
    }
    int recvFlag = -1, recv_complete = 0;
    MPI_Status recv_status;
    MPI_Request recv_request = MPI_REQUEST_NULL;
    MPI_Irecv(&recvFlag, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &recv_request);
    bool founded = false, global_founded = false;
    unsigned int generated = 0;
    if (got_job) {
        double starttime = MPI_Wtime();
        do {
            if (bf->Check()) {
                founded = true;
                break;
            }
            MPI_Test(&recv_request, &recv_complete, &recv_status);
        } while (bf->Next() && !recv_complete);
        double endtime = MPI_Wtime();
        for (int i = 0; i < pr_num; ++i) {
            if (i != pr_rank)
                MPI_Send(&pr_rank, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
        }
        if (founded) {
            cout << "!SUCCESS!\nSought word is: " << bf->Word()
                << "\n->checked " << bf->Generated() << " variants"
                << "\n->took " << endtime - starttime << " ns\n";
        }
    }
    MPI_Allreduce(&founded, &global_founded, 1, MPI_C_BOOL, MPI_LOR, MPI_COMM_WORLD);
    MPI_Barrier(MPI_COMM_WORLD);
    if (pr_rank == root) {
        if (!global_founded) {
            cout << "!FAIL!\nUnfortunatly sought word was not founded.\nCheck if '-len' and '-mask' are correct.\n";
        }
    }
    MPI_Finalize();
    return 0;
}
