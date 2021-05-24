#pragma once
#include <string>
#include <vector>

using namespace std;

bool parseArguments(int argc, char* argv[], unsigned int& len, string& mask, string& hash);
bool splitCharset(vector<size_t>& st, vector<size_t>& end, int pr_rank, int pr_num);
