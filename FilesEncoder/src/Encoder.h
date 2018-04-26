#pragma once

#include <vector>
#include <queue>
#include "FileReader.h"
#include <list>

using namespace std;

/** \brief Encoder/decoder for all type of files. */
class Encoder
{

public:

    /** \brief Encodes file with path to file with pathTo by method. */
    void encode(const string& method, const string& path, const string& pathTo)
    {
        ICoder* c;

        if (method == "haff")
            c = new Huffman(path);
        else if (method == "shan")
            c = new ShannonFano(path);
        else if (method == "lz775")
            c = new LZ77(4 * 1024, 1 * 1024);
        else if (method == "lz7710")
            c = new LZ77(8 * 1024, 2 * 1024);
        else if (method == "lz7720")
            c = new LZ77(16 * 1024, 4 * 1024);
        else
            throw logic_error("No supported method to encode: " + method);

        c->encode(path, pathTo);
    }

    /** \brief Decodes file with path to file with pathTo by method. */
    void decode(const string& method, const string& path, const string& pathTo)
    {
        ICoder* c;

        if (method == "haff")
            c = new Huffman(path);
        else if (method == "shan")
            c = new ShannonFano(path);
        else if (method == "lz775")
            c = new LZ77(4 * 1024, 1 * 1024);
        else if (method == "lz7710")
            c = new LZ77(8 * 1024, 2 * 1024);
        else if (method == "lz7720")
            c = new LZ77(16 * 1024, 4 * 1024);
        else
            throw logic_error("No supported method to decode: " + method);

        c->decode(path, pathTo);
    }

private:

    static char binaryToByte(string& bits)
    {
        return static_cast<char>(stoi(bits, nullptr, 2));
    }

    static string byteToBinary(unsigned short c)
    {
        string r;
        for (int i = 0; i < 8; ++i)
        {
            r = (c % 2 == 0 ? "0" : "1") + r;
            c /= 2;
        }
        return r;
    }

public:

    /** \brief Interface for all encoders. */
    class ICoder
    {

    public:

        /** \brief Encodes file with path to file with pathTo by method. */
        virtual void encode(const string& path, const string& pathTo) = 0;

        /** \brief Decodes file with path to file with pathTo by method. */
        virtual void decode(const string& path, const string& pathTo) = 0;

        virtual ~ICoder() {}

    protected:

        ICoder() {}

    };

    /** \brief Huffman method encoder/decoder. */
    class Huffman : public ICoder
    {

    public:

        Huffman(const string& path)
        {
            map<char, int> m;
            FileReader::readSymbolsMap(path, m);

            for (auto& x : m)
                addNode(x.first, x.second);

            build();
        }

    public:

        void encode(const string& path, const string& pathTo) override
        {
            vector<char> encodedText;
            vector<char> text;
            FileReader::readAllBytes(path, text);

            string ser = to_string(_map.size()) + "\n";
            for (auto& x : _map)
            {
                ser += x.second->code;
                ser += ":";
                ser += x.first;
                ser += '\n';
            }

            FileReader::writeString(pathTo, ser);

            ofstream ofs(pathTo, ios::binary | ios::ate | ios::app);
            if (!ofs.good())
                throw runtime_error("Can't write to file: " + pathTo);

            // Codding.
            string buf = "";
            for (int i = 0; i < text.size(); ++i)
            {
                buf += _map[text[i]]->code;
                while (buf.length() >= 8)
                {
                    char c = binaryToByte(buf.substr(0, 8));
                    ofs.write(&c, sizeof(char));
                    buf = buf.substr(8);
                }
            }

            if (buf.length() > 0)
            {
                char c = binaryToByte(buf.substr(0, 8));
                ofs.write(&c, sizeof(char));
            }

            ofs.close();
        }

        void decode(const string& path, const string& pathTo) override
        {
            map<string, char> m;
            vector<char> decodedText;

            ifstream ifs(path, ios::binary);
            if (!ifs.good())
                throw runtime_error("Can't read from file: " + path);

            ofstream ofs(pathTo, ios::binary);
            if (!ofs.good())
                throw runtime_error("Can't write to file: " + pathTo);

            // Считываю мапу.
            string s;
            char c;
            ifs.read(&c, 1);
            while (c != '\n')
            {
                s += c;
                ifs.read(&c, sizeof(char));
            }
            int n = atoi(s.c_str());
            for (int j = 0; j < n; ++j)
            {
                string code;
                char co;

                s = "";
                ifs.read(&c, sizeof(char));
                while (c != '\n')
                {
                    s += c;
                    ifs.read(&c, sizeof(char));
                }

                if (s.size() == 0)
                {
                    --j;
                    continue;
                }

                if (s[s.length() - 1] == ':' && s[s.length() - 2] != ':')
                {
                    co = '\n';
                    code = s.substr(0, s.length() - 1);
                }
                else
                {
                    co = s[s.length() - 1];
                    code = s.substr(0, s.length() - 2);
                }

                m[code] = co;
            }

            //cout << "1" << endl;

            // Декожу файл.

            string code = "";
            string buf = "";
            int k = 0;
            while (!ifs.eof())
            {
                ifs.read(&c, sizeof(char));
                code.append(byteToBinary(c));

                while (code.length() != 0 && k < code.length())
                {
                    buf += code[k];
                    ++k;
                    auto it = m.find(buf);
                    if (it != m.end())
                    {
                        ofs.write(&it.operator*().second, sizeof(char));
                        buf = "";
                        code = code.substr(k);
                        k = 0;
                    }
                }
            }

            //cout << k << endl;

            ifs.close();
            ofs.close();
        }

    private:

        /** \brief Tree node. */
        struct HuffmanNode
        {
            int quantity;
            string code;

            HuffmanNode* left;
            HuffmanNode* right;

            HuffmanNode(int quantity) : quantity(quantity), left(nullptr), right(nullptr) {}
            HuffmanNode(int quantity, HuffmanNode* left, HuffmanNode* right) : quantity(quantity), left(left), right(right) {}

            ~HuffmanNode()
            {
                if (left)
                    delete left;
                if (right)
                    delete right;
            }
        };

        /** \brief Tree node compare. */
        struct Compare
        {
            bool operator() (const HuffmanNode* l, const HuffmanNode* r)
            {
                return l->quantity > r->quantity;
            }
        };

        /** Traverses through the tree and make codes. */
        void traversal(HuffmanNode* root, string code)
        {
            root->code = code;

            if (root->left || root->right)
            {
                traversal(root->left, code + "0");
                traversal(root->right, code + "1");
            }
        }

        void build()
        {
            // Building a tree.
            while (_queue.size() > 1)
            {
                HuffmanNode* l = _queue.top();
                _queue.pop();

                HuffmanNode* r = _queue.top();
                _queue.pop();

                _queue.push(new HuffmanNode(l->quantity + r->quantity, l, r));
            }

            traversal(_queue.top(), "");
        }

        void addNode(char c, int quantity)
        {
            HuffmanNode* nNode = new HuffmanNode(quantity);
            _map[c] = nNode;
            _queue.push(nNode);
        }

        string getCode(char c)
        {
            return _map[c]->code;
        }

        void getCodeMap(map<char, string>& m) const
        {
            m = map<char, string>();
            for (auto& x : _map)
                m[x.first] = x.second->code;
        }

    private:

        map<char, HuffmanNode*> _map;
        priority_queue<HuffmanNode*, vector<HuffmanNode*>, Compare> _queue;
    };

    /** \brief ShannonFano method encoder/decoder. */
    class ShannonFano : public ICoder
    {
    public:

        ShannonFano(const string& path)
        {
            map<char, int> m;
            FileReader::readSymbolsMap(path, m);

            for (auto& x : m)
                addNode(x.first, x.second);

            build();
        }

    private:

        void build()
        {
            fano(0, _list.size() - 1);
            for (auto& x : _list)
                _map[x.first.second] = x.second;
        }

        void fano(int l, int r)
        {
            if (l >= r) return;

            int m = median(l, r);
            fano(l, m);
            fano(m + 1, r);
        }

        int median(int l, int r)
        {
            int sl = 0;
            for (int i = l; i < r; ++i)
                sl += _list[i].first.first;

            int sr = _list[r].first.first;
            int m = r;
            int d;
            do {
                _list[m--].second += "1";
                d = sl - sr;
                sl -= _list[m].first.first;
                sr += _list[m].first.first;
            } while (abs(sl - sr) <= d);

            for (int i = l; i <= m; ++i)
                _list[i].second += "0";

            return m;
        }

        void addNode(char c, int quantity)
        {
            _list.push_back(pair< pair<int, char>, string >(pair<int, char>(quantity, c), ""));
        }

    private:

        void encode(const string& path, const string& pathTo) override
        {
            vector<char> encodedText;

            // Весь текст тут.
            vector<char> text;
            FileReader::readAllBytes(path, text);

            // Записываю мапу.
            string ser = to_string(_map.size()) + "\n";
            for (auto& x : _map)
            {
                ser += x.second;
                ser += ":";
                ser += x.first;
                ser += '\n';
            }

            FileReader::writeString(pathTo, ser);

            ofstream ofs(pathTo, ios::binary | ios::ate | ios::app);
            if (!ofs.good())
                throw runtime_error("Can't write to file: " + pathTo);

            // Кодирую текст.
            string buf = "";
            for (int i = 0; i < text.size(); ++i)
            {
                buf += _map[text[i]];
                while (buf.length() >= 8)
                {
                    char c = binaryToByte(buf.substr(0, 8));
                    ofs.write(&c, 1);
                    buf = buf.substr(8);
                }
            }

            if (buf.length() > 0)
            {
                char c = binaryToByte(buf.substr(0, 8));
                ofs.write(&c, 1);
            }

            ofs.close();
        }

        void decode(const string& path, const string& pathTo) override
        {
            map<string, char> m;
            vector<char> decodedText;

            ifstream ifs(path, ios::binary);
            if (!ifs.good())
                throw runtime_error("Can't read from file: " + path);

            ofstream ofs(pathTo, ios::binary);
            if (!ofs.good())
                throw runtime_error("Can't write to file: " + pathTo);

            // Считываю мапу.
            string s;
            char c;
            ifs.read(&c, 1);
            while (c != '\n')
            {
                s += c;
                ifs.read(&c, 1);
            }
            int n = atoi(s.c_str());
            for (int j = 0; j < n; ++j)
            {
                string code;
                char co;

                s = "";
                ifs.read(&c, 1);
                while (c != '\n')
                {
                    s += c;
                    ifs.read(&c, 1);
                }

                if (s.size() == 0)
                {
                    --j;
                    continue;
                }

                if (s[s.length() - 1] == ':' && s[s.length() - 2] != ':')
                {
                    co = '\n';
                    code = s.substr(0, s.length() - 1);
                }
                else
                {
                    co = s[s.length() - 1];
                    code = s.substr(0, s.length() - 2);
                }

                m[code] = co;
            }

            //cout << "1" << endl;

            // Декожу файл.

            string code = "";
            string buf = "";
            int k = 0;
            while (!ifs.eof())
            {
                ifs.read(&c, 1);
                code.append(byteToBinary(c));

                while (code.length() != 0 && k < code.length())
                {
                    buf += code[k];
                    ++k;
                    auto it = m.find(buf);
                    if (it != m.end())
                    {
                        ofs.write(&it.operator*().second, 1);
                        buf = "";
                        code = code.substr(k);
                        k = 0;
                    }
                }
            }

            //cout << k << endl;

            ifs.close();
            ofs.close();
        }

    private:

        map<char, string> _map;
        vector< pair< pair<int, char>, string> > _list;
    };

    /** \brief LZ77 method with specific history and preview buffer encoder/decoder. */
    class LZ77 : public ICoder
    {

    public:

        LZ77(size_t hisBufSize, size_t preBufSize)
        {
            this->hisBufSize = hisBufSize;
            this->preBufSize = preBufSize;
        }

        void encode(const string& path, const string& pathTo) override
        {
            vector<char> text;
            FileReader::readAllBytes(path, text);

            ofstream ofs(pathTo, ofstream::binary);
            if (!ofs.good())
                throw runtime_error("Can't write to file: " + pathTo);

            ofs.write((char*)&hisBufSize, sizeof(size_t));

            size_t hisStart = 0;
            size_t preStart = 0;
            size_t hisEnd = 0;
            size_t preEnd = preBufSize;

            while (preStart < text.size())
            {
                LZ77Node node = findNewNode(text, hisStart, hisEnd, preStart, preEnd);

                ofs.write((char*)&node.offs, sizeof(short));
                ofs.write((char*)&node.len, sizeof(short));
                ofs.write(&node.ch, sizeof(char));

                hisEnd += node.len + 1;
                preStart += node.len + 1;
                preEnd += node.len + 1;

                if (hisEnd - hisStart > hisBufSize)
                    hisStart += hisEnd - hisStart - hisBufSize;
            }
            ofs.close();
        }

        void decode(const string& path, const string& pathTo) override
        {
            ifstream ifs(path, ifstream::binary);
            if (!ifs.good())
                throw runtime_error("Can't read from file: " + path);

            ofstream ofs(pathTo, ofstream::binary);
            if (!ofs.good())
                throw runtime_error("Can't write to file: " + pathTo);

            size_t hisBufSize = 0;
            ifs.read((char*)&hisBufSize, sizeof(size_t));

            string hisBuf;
            while (!ifs.eof())
            {
                ushort offs = 0;
                ushort len = 0;
                char ch = 0;

                ifs.read((char*)&offs, sizeof(short));
                ifs.read((char*)&len, sizeof(short));
                ifs.read(&ch, sizeof(char));

                size_t hisStart = hisBuf.size() - offs;

                for (int j = 0; j < len; ++j)
                    hisBuf += hisBuf[hisStart + j];

                hisBuf += ch;

                ofs.write(&hisBuf[hisStart], len);
                ofs.write(&ch, sizeof(char));

                if (hisBuf.size() > hisBufSize)
                    hisBuf = hisBuf.substr(hisBuf.size() - hisBufSize);
            }
            ifs.close();
            ofs.close();
        }

    private:

        typedef unsigned short ushort;

        // block look like:  <offs, len, ch> example: <3,2,d>
        struct LZ77Node
        {
            ushort offs;
            ushort len;
            char ch;

            LZ77Node(ushort o, ushort l, char c) : offs(o), len(l), ch(c) { }
        };

        LZ77Node findNewNode(const vector<char>& text, size_t hisStart, size_t hisEnd, size_t preStart, size_t preEnd)
        {
            ushort hisSize = hisEnd - hisStart;
            ushort preSize = preEnd - preStart;

            ushort curHisIndex = 0;
            ushort maxSeqStartIndex = 0;
            ushort maxSeqLength = 0;

            while (curHisIndex < hisSize && curHisIndex < preSize)
            {
                ushort tHisIndex = curHisIndex;
                ushort tPreIndex = 0;
                ushort readSeqLength = 0;

                while (tHisIndex < hisSize && tPreIndex < preSize && tPreIndex < text.size() &&
                    text[hisStart + tHisIndex] == text[preStart + tPreIndex])
                {
                    ++tHisIndex;
                    ++tPreIndex;
                    ++readSeqLength;

                    if (tHisIndex == hisSize)
                        tHisIndex = curHisIndex;
                }

                if (readSeqLength > maxSeqLength)
                {
                    maxSeqLength = readSeqLength;
                    maxSeqStartIndex = curHisIndex;
                }
                ++curHisIndex;
            }

            if (maxSeqLength == 0)
                return LZ77Node(0, 0, text[preStart]);

            return LZ77Node(hisSize - maxSeqStartIndex, maxSeqLength, text[preStart + maxSeqLength]);
        }

    protected:
        size_t hisBufSize;
        size_t preBufSize;
    };
};
