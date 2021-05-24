#include "bf.h"

bool BF::Next() {
	for (int i = sought_len - 1; i >= 0; --i) {
		if (curr_ind[i] < end_ind[i]) {
			word[i] = charset[++curr_ind[i]];
			generated++;
			return true;
		}
		else {
			curr_ind[i] = begin_ind[i];
			word[i] = charset[curr_ind[i]];
		}
	}
	return false;
}

bool BF::Check() {
	SHA256_Init(&sha256);
	SHA256_Update(&sha256, word.c_str(), sought_len);
	SHA256_Final(digest, &sha256);
	return (memcmp(goal, digest, digest_len) == 0);
}

string BF::Word() const { return word; }

vector<unsigned char> BF::Digest() const {
	vector<unsigned char> out(digest, digest + digest_len);
	return out;
}

uint64_t BF::Generated() const { return generated; }

vector<char> BF::Charset() const { return charset; }

vector<unsigned char> BF::Goal() const {
	vector<unsigned char> out(goal, goal + digest_len);
	return out;
}

BF* BF::Initialize(string hash, vector<char> charset, vector<size_t> st, vector<size_t> end) {
	vector<unsigned char> bytes = HexToBytes(hash);
	if (bytes.size() != digest_len || charset.empty() || st.empty() || st.size() != end.size())
		return NULL;
	for (size_t i = 0; i < st.size(); ++i) {
		if (st[i] > end[i] || end[i] >= charset.size())
			return NULL;
	}

	BF* bf = new BF();
	bf->sought_len = st.size();
	bf->begin_ind = st;
	bf->end_ind = end;
	bf->curr_ind = st;
	bf->charset = charset;

	copy(bytes.begin(), bytes.end(), bf->goal);

	bf->word.resize(st.size());
	for (size_t i = 0; i < st.size(); ++i) {
		bf->word[i] = charset[st[i]];
	}

	SHA256_Init(&bf->sha256);
	SHA256_Update(&bf->sha256, bf->word.c_str(), bf->sought_len);
	SHA256_Final(bf->digest, &bf->sha256);

	return bf;
}

vector<unsigned char> BF::HexToBytes(string hex) {
	if (hex.length() % 2 != 0)
		hex.insert(hex.begin(), '0');
	vector<unsigned char> bytes(hex.length() / 2);
	for (unsigned int n, i = 0; i < hex.length() / 2; i++) {
		if (sscanf_s(hex.c_str() + (i * 2), "%2X", &n) != 1)
			return {};
		bytes[i] = n;
	}
	return bytes;
}

bool BF::GenCharset(string& mask, unsigned int& len, 
	vector<char>& charset, vector<size_t>& st, vector<size_t>& end) {
	if (len == 0) {
		if (mask.length() == 0)
			return false;
		len = mask.length();
	}
	if (mask.length() < len) {
		char default_char = (mask.length() == 1) ? mask[0] : 'A';
		mask.append(len - mask.length(), default_char);
	}
	else if (mask.length() > len) {
		mask.resize(len);
	}

	unsigned char charset_conf = 0;
	vector<int> mask_stg(mask.length()), mask_endg(mask.length());
	for (int i = 0; i < mask.length(); ++i) {
		switch (mask[i]) {
		case 'n':
			charset_conf |= 1;
			mask_stg[i] = 0;
			mask_endg[i] = 0;
			break;
		case 'N':
			charset_conf |= 6;
			mask_stg[i] = 1;
			mask_endg[i] = 2;
			break;
		case 'c':
			charset_conf |= 2;
			mask_stg[i] = 1;
			mask_endg[i] = 1;
			break;
		case 'C':
			charset_conf |= 4;
			mask_stg[i] = 2;
			mask_endg[i] = 2;
			break;
		case 'x':
			charset_conf |= 3;
			mask_stg[i] = 0;
			mask_endg[i] = 1;
			break;
		case 'A':
			charset_conf |= 7;
			mask_stg[i] = 0;
			mask_endg[i] = 2;
			break;
		default: return false;
		}
	}

	vector<int> ascii_group_start{ 48, 97, 65 }, ascii_group_len{ 10, 26, 26 };
	charset = vector<char>();
	for (int g = 0; g < 3; ++g) {
		if (charset_conf & (1 << (g))) {
			int stch = ascii_group_start[g], endch = stch + ascii_group_len[g];
			for (int ch = stch; ch < endch; ++ch)
				charset.push_back(ch);
		}
	}
	st = vector<size_t>(len, 0);
	end = vector<size_t>(len, 0);
	for (unsigned int i = 0; i < len; ++i) {
		for (int g = 0; g < 3; ++g) {
			if (charset_conf & (1 << (g))) {
				if (g < mask_stg[i])
					st[i] += ascii_group_len[g];
				if (g <= mask_endg[i])
					end[i] += ascii_group_len[g];
			}
		}
		end[i] -= 1;
	}
	return true;
}
