#include <iostream>
#include <fstream>
#include <queue>
#include <map>
#include <string>
#include <cstring>
#include <algorithm>
#include <cctype>   // for tolower, isalpha
using namespace std;

typedef int Rank;

class Bitmap {
private:
    unsigned char* M;
    Rank N, _sz;
protected:
    void init(Rank n) {
        M = new unsigned char[N = (n + 7) / 8];
        memset(M, 0, N);
        _sz = 0;
    }
public:
    Bitmap(Rank n = 8) { init(n); }
    
    Bitmap(char* file, Rank n = 8) {
        init(n);
        FILE* fp = fopen(file, "rb");
        if (fp) {
            fread(M, sizeof(char), N, fp);
            fclose(fp);
        }
        Rank cnt = 0;  // ✅ 修复：cnt 声明在循环外
        for (Rank k = 0; k < n; k++) {
            if (test(k)) cnt++;
        }
        _sz = cnt;
    }
    
    ~Bitmap() { delete[] M; M = NULL; _sz = 0; }

    Rank size() { return _sz; }
    void set(Rank k) {
        expand(k);
        _sz++;
        M[k >> 3] |= (0x80 >> (k & 0x07));
    }
    void clear(Rank k) {
        expand(k);
        _sz--;
        M[k >> 3] &= ~(0x80 >> (k & 0x07));
    }
    bool test(Rank k) {
        expand(k);
        return M[k >> 3] & (0x80 >> (k & 0x07));
    }
    void dump(char* file) {
        FILE* fp = fopen(file, "wb");
        fwrite(M, sizeof(char), N, fp);
        fclose(fp);
    }
    char* bits2string(Rank n) {
        expand(n - 1);
        char* s = new char[n + 1];
        s[n] = '\0';
        for (Rank i = 0; i < n; i++)
            s[i] = test(i) ? '1' : '0';
        return s;
    }
    void expand(Rank k) {
        if (k < 8 * N) return;
        Rank oldN = N;
        unsigned char* oldM = M;
        init(2 * k + 1);
        memcpy(M, oldM, oldN);
        delete[] oldM;
    }
};

struct BinNode {
    char ch;
    int freq;
    BinNode* left;
    BinNode* right;
    BinNode(char c = 0, int f = 0) : ch(c), freq(f), left(nullptr), right(nullptr) {}
};

struct Compare {
    bool operator()(BinNode* a, BinNode* b) {
        return a->freq > b->freq;
    }
};

class HuffTree {
private:
    BinNode* root;
    map<char, string> codeMap;

    void buildCodeMap(BinNode* node, string code) {
        if (!node) return;
        if (node->ch != 0) {
            codeMap[node->ch] = code;
            return;
        }
        buildCodeMap(node->left, code + "0");
        buildCodeMap(node->right, code + "1");
    }

public:
    HuffTree(const string& text) : root(nullptr) {
        int freq[26] = {0};
        for (char c : text) {
            if (isalpha(c)) {
                c = tolower(c);
                freq[c - 'a']++;
            }
        }

        priority_queue<BinNode*, vector<BinNode*>, Compare> pq;
        for (int i = 0; i < 26; ++i) {
            if (freq[i] > 0) {
                pq.push(new BinNode('a' + i, freq[i]));
            }
        }

        if (pq.empty()) {
            root = nullptr;
            return;
        }

        if (pq.size() == 1) {
            BinNode* only = pq.top(); pq.pop();
            root = new BinNode(0, only->freq);
            root->left = only;
        } else {
            while (pq.size() > 1) {
                BinNode* l = pq.top(); pq.pop();
                BinNode* r = pq.top(); pq.pop();
                BinNode* parent = new BinNode(0, l->freq + r->freq);
                parent->left = l;
                parent->right = r;
                pq.push(parent);
            }
            root = pq.top();
        }

        buildCodeMap(root, "");
    }

    string encode(const string& word) {
        string result = "";
        for (char c : word) {
            if (isalpha(c)) {
                c = tolower(c);
                if (codeMap.find(c) != codeMap.end()) {
                    result += codeMap[c];
                } else {
                    cerr << "Warning: '" << c << "' not in code map.\n";
                }
            }
        }
        return result;
    }

    void printCodes() {
        cout << "Huffman Codes:\n";
        for (auto& p : codeMap) {
            cout << p.first << ": " << p.second << "\n";
        }
        cout << "\n";
    }
};

int main() {
    string speech =
        "I have a dream that one day this nation will rise up and live out the true meaning of its creed "
        "we hold these truths to be self evident that all men are created equal I have a dream that one day "
        "on the red hills of Georgia the sons of former slaves and the sons of former slave owners will be able "
        "to sit down together at the table of brotherhood";

    HuffTree huff(speech);
    huff.printCodes();

    vector<string> words = {"dream", "freedom", "hope"};
    cout << "Encoded Words:\n";
    for (const string& w : words) {
        string encoded = huff.encode(w);
        cout << w << " -> " << encoded << "\n";

        Bitmap bmp(encoded.length());
        for (size_t i = 0; i < encoded.length(); ++i) {
            if (encoded[i] == '1') bmp.set(i);
        }
        char* bitStr = bmp.bits2string(encoded.length());
   
        delete[] bitStr;
    }

    return 0;
}
