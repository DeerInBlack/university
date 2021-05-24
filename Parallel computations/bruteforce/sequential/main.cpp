#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include "../common/bf.h"
#include "../common/common.h"

using namespace std;
using namespace chrono;

int main(int argc, char* argv[]) {
    unsigned int len;
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
    BF* bf = BF::Initialize(hash, charset, st, end);
    if (!bf) {
        cout << "Unable to bruteforce\n\t'-hash' is possibly incorrect\n";
        return 0;
    }
    cout << "Begin to bRuTeFoRcE SHA256\n->hash: " << hash 
        << "\n->len: " << len << "\n->mask: " << mask << "\n\n";
    bool founded = false;
    auto start_t = high_resolution_clock::now();
    do {
        if (bf->Check()) {
            founded = true;
            break;
        }
    } while (bf->Next());
    auto stop_t = high_resolution_clock::now();
    auto duration = duration_cast<nanoseconds>(stop_t - start_t);
    if (founded) {
        cout << "!SUCCESS!\nSought word is: " << bf->Word() 
            << "\n->checked " << bf->Generated() << " variants"
            << "\n->took " << duration.count() << " ns\n";
    }
    else {
        cout << "!FAIL!\nUnfortunatly sought word was not founded.\nCheck if '-len' and '-mask' are correct.";
    }
    return 0;
}
