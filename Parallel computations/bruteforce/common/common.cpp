#include "common.h"
#include <iostream>

using namespace std;

bool parseArguments(int argc, char* argv[], unsigned int& len, string& mask, string& hash) {
    bool len_arg = false, hash_arg = false;
    len = 0;
    mask = "";
    for (int i = 0; i < argc; i++) {
        if (strcmp(argv[i], "-len") == 0) {
            if (i + 1 < argc && argv[i+1][0] != '-') {
                len = atoi(argv[++i]);
                len_arg = true;
            }
        } 
        else  if (strcmp(argv[i], "-mask") == 0) {
            if (i + 1 < argc && argv[i + 1][0] != '-') {
                mask = string(argv[++i]);
                len_arg = true;
            }
        }
        else if (strcmp(argv[i], "-hash") == 0) {
            if (i + 1 < argc && argv[i + 1][0] != '-') {
                hash = string(argv[++i]);
                hash_arg = true;
            }
        }
    }
    return len_arg && hash_arg;
}

bool splitCharset(vector<size_t>& st, vector<size_t>& end, int pr_rank, int pr_num) {
    if (st.size() != end.size())
        return false;
    int rank = pr_rank, n = pr_num, rpp = 0, rem = 0;
    for (int i = 0; i < st.size() && rpp <= 0 && rank >= 0; ++i) {
        int range = end[i] - st[i] + 1;
        rpp = range / n;
        rem = range % n - (rpp <= 0);
        int ps = st[i] + rpp * rank + (rem > rank ? rank : rem), pe = ps;
        if (rpp > 0)
            pe += rpp + (rem > rank) - 1;
        st[i] = ps;
        end[i] = pe;
        n -= rem;
        rank -= rem;
    }
    if (rpp <= 0 && rank > 0) {
        st = vector<size_t>();
        end = vector<size_t>();
        return false;
    }
    return true;
}