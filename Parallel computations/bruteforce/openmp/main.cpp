#include <iostream>
#include <string>
#include <vector>
#include <omp.h>
#include "../common/bf.h"
#include "../common/common.h"

#define N 4

using namespace std;

int main(int argc, char* argv[]) {
    unsigned len = 0;
    string mask, hash;
    if (!parseArguments(argc, argv, len, mask, hash)) {
        cout << "Not enough arguments\n\t check if these arguments are present:"
            << "\n\t\t'-len' - length of sought word (optional if '-mask')"
            << "\n\t\t'-mask' - mask for characters in word (opianal if '-len')"
            << "\n\t\t'-hash' - hex encoded SHA256 digest\n";
        return 0;
    }
    vector<char> charset;
    vector<size_t> st, end;
    if (!BF::GenCharset(mask, len, charset, st, end)) {
        cout << "Unable to generate charset\n\t'-len' or '-mask' is incorrect\n";
        return 0;
    }
    BF* bf;
    cout << "Begin to bRuTeFoRcE SHA256\n->hash: " << hash
        << "\n->len: " << len << "\n->mask: " << mask << "\n\n";


    unsigned int checked = 0;
    bool founded = false;
    string word;
    double start_t = omp_get_wtime();
#pragma omp parallel num_threads(N)
    {
#pragma omp for schedule(static, 1) reduction(+ : checked) firstprivate(st, end, bf)
        for (int i = 0; i < N; ++i) {
            if (splitCharset(st, end, i, N) && (bf = BF::Initialize(hash, charset, st, end))) {
                do {
                    if (bf->Check()) {
                        founded = true;
                        word = bf->Word();
                        break;
                    }
                } while (bf->Next());
                checked += bf->Generated();
            }
        }
    }
    double end_t = omp_get_wtime();
    double exec_t = end_t - start_t;
    if (founded) {
        cout << "!SUCCESS!\nSought word is: " << word
            << "\n->checked " << checked << " variants"
            << "\n->took " << exec_t << " s\n";
    }
    else {
        cout << "!FAIL!\nUnfortunatly sought word was not founded.\nCheck if '-len' and '-mask' are correct.";
    }
    return 0;
}
