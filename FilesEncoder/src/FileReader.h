#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <map>

// It's ok here.
using namespace std;

/** \brief Simple file reader/writer class. */
class FileReader
{

public:

    /** \brief Reads all bytes from the file into \code vector<char> read \endcode. */
    static void readAllBytes(const string& path, vector<char>& read)
    {
        ifstream ifs(path, ios::binary | ios::ate);
        if (!ifs.good())
            throw runtime_error("Can't read from file: " + path);
        const ifstream::pos_type pos = ifs.tellg();

        read = vector<char>(pos);

        // Read from begining of the file.
        ifs.seekg(0, ios::beg);
        ifs.read(&read[0], pos);

        ifs.close();
    }

    /** \brief Writes all bytes from \code vector<char> write \endcode into the file. */
    static void writeBytes(const string& path, vector<char>& write)
    {
        ofstream ofs(path, ios::binary | ios::ate | ios::app);
        if (!ofs.good())
            throw runtime_error("Can't write to file: " + path);

        // Write to the end of the file.
        ofs.seekp(0, ios::end);
        ofs.write(&write[0], write.size());

        ofs.close();
    }

    /** \brief Reads all file to the char map quantity. */
    static void readSymbolsMap(const string& path, map<char, int>& m)
    {
        ifstream ifs(path, ios::binary | ios::ate);
        if (!ifs.good())
            throw runtime_error("Can't read from file: " + path);
        // Read from begining of the file.
        ifs.seekg(0, ios::beg);

        m = map<char, int>();

        char c;
        while (ifs.read(&c, sizeof(char)))
            ++m[c];

        ifs.close();
    }

    /** \brief Writes string with new line. */
    static void writeString(const string& path, string write)
    {
        ofstream ofs(path, ios::binary | ios::trunc);
        if (!ofs.good())
            throw runtime_error("Can't write to file: " + path);
        ofs.write(write.c_str(), write.length());

        ofs.close();
    }

    /** \brief Gets entropy from file with path. */
    static double entropy(const string& path)
    {
        double entropy = 0;

        map<char, int> m;
        readSymbolsMap(path, m);

        ifstream ifs(path, ios::ate);
        const ifstream::pos_type pos = ifs.tellg();
        ifs.close();

        for (auto& it : m)
        {
            double r = static_cast<double>(it.second) / pos;
            entropy -= r * log2(r);
        }

        return entropy;
    }

    /** \brief Calculates file size / encodedFile size. */
    static double compressRatio(const string& file, const string& encodedFile)
    {
        ifstream ifs1(file, ios::ate);
        const ifstream::pos_type fileLen = ifs1.tellg();
        ifs1.close();

        ifstream ifs2(encodedFile, ios::ate);
        const ifstream::pos_type encodedFileLen = ifs2.tellg();
        ifs2.close();

        return static_cast<double>(static_cast<double>(fileLen) / encodedFileLen);
    }

    /** \brief Prints all bytes and their probabilities. */
    static void printBytes(const string& path, const string& csv_path)
    {
        map<char, int> m;
        readSymbolsMap(path, m);

        ifstream ifs(path, ios::ate);
        const ifstream::pos_type pos = ifs.tellg();
        ifs.close();

        ofstream f(csv_path, ios::out | ios::app);
        for(int i = 0; i < 256; ++i)
        {
            char c = static_cast<char>(i);
            auto it = m.find(c);
            if(it == m.end())
                f << to_string(0) << ";";
            else
            {
                double r = static_cast<double>(it.operator*().second) / pos;
                f << r << ";";
            }
        }
        f << "\n";

        f.close();
    }
};
