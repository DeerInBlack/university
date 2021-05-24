#pragma once
#include <string>
#include <vector>
#include <openssl/sha.h>

using namespace std;

class BF {
private:
	static const size_t digest_len = SHA256_DIGEST_LENGTH;
	SHA256_CTX sha256;
	unsigned char goal[digest_len], digest[digest_len];
	vector<char> charset;
	vector<size_t> curr_ind, begin_ind, end_ind;
	size_t sought_len;
	string word;
	uint64_t generated = 1;
	
public:
	bool Next();
	bool Check();
	string Word() const;
	vector<unsigned char> Digest() const;
	uint64_t Generated() const;

	vector<char> Charset() const;
	vector<unsigned char> Goal() const;
	
	static BF* Initialize(string hash, vector<char> charset, vector<size_t> st, vector<size_t> end);
	static vector<unsigned char> HexToBytes(string hex);
	static bool GenCharset(string& mask, unsigned int& len,
		vector<char>& charset, vector<size_t>& st, vector<size_t>& end);
};
