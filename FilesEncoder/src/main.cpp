/*
 * �����: ��������� ������� ����������
 * ����: 26.03.2018
 * IDE: visual studio 2017
 * ��������� �������:
 * Encoder.h - ����������� / ������������� ��������.
 * FileReader.h - ������ / ������ � ������. ������� �������, ��������.
 * Timer.h - ����� ������� � ������������.
 * main.cpp - ��� �����������.
 */

#include "Encoder.h"
#include "Timer.h"
#include <iostream>

const string SOURCE_FOLDER = "source/";
const string OUT_FOLDER = "out/";
const string CSV_OUT = "result.csv";
const string CSV_BYTES_OUT = "bytes.csv";

const int FILES_LEN = 1;

// File names in /source folder

const string FILES_PATH[] = { "02" };

const int METHOD_COUNT = 5;

const string METHOD_NAMES[] = { "haff", "shan", "lz775", "lz7710", "lz7720" };

const int n = 1;

int main()
{
    Encoder e;
    Timer t;

    ofstream f(CSV_OUT, ios::out);

    // ������������� ��������� �������� csv �����.

    f << "file name" << ";";
    f << "entropy" << ";";
    for(int i = 0; i < METHOD_COUNT; ++i)
    {
        f << "coef " << METHOD_NAMES[i] << ";";
        f << "time pack " << METHOD_NAMES[i] << ";";
        f << "time unpack " << METHOD_NAMES[i] << ";";
    }

    f << "\n";

    // ��� ������� �����.
    for (int i = 0; i < FILES_LEN; ++i)
    {
        // ���������� ��� �����.
        f << FILES_PATH[i] << ";";

        // ���������� ��������.
        f << FileReader::entropy(SOURCE_FOLDER + FILES_PATH[i]) << ";";

        // ��� ������� ������.
        for (int m = 0; m < METHOD_COUNT; ++m)
        {
            std::cout << "File: " << FILES_PATH[i] << endl << "method: " << METHOD_NAMES[m] << endl;

            long long eTime = 0;
            long long deTime = 0;
            // ��������� n ���.
            for (int c = 0; c < n; ++c)
            {
                // ��������.
                t.start();
                e.encode(METHOD_NAMES[m], SOURCE_FOLDER + FILES_PATH[i], OUT_FOLDER + FILES_PATH[i] + "." + METHOD_NAMES[m]);
                t.stop();
                eTime += t.result();

                // ����������.
                t.start();
                e.decode(METHOD_NAMES[m], OUT_FOLDER + FILES_PATH[i] + "." + METHOD_NAMES[m], OUT_FOLDER + FILES_PATH[i] + ".un" + METHOD_NAMES[m]);
                t.stop();
                deTime += t.result();
            }

            // ���������� ����������� ������.
            f << FileReader::compressRatio(SOURCE_FOLDER + FILES_PATH[i], OUT_FOLDER + FILES_PATH[i] + "." + METHOD_NAMES[m]) << ";";

            eTime /= n;
            deTime /= n;
            
            // ���������� ������� ����� �����������.
            f << eTime << ";";
            // ���������� ������� ����� �������������.
            f << deTime << ";";
        }

        f << "\n";

        FileReader::printBytes(SOURCE_FOLDER + FILES_PATH[i], CSV_BYTES_OUT);
    }

    f.close();
    std::cout << "Done!" << endl;
    getchar();

    return 0;
}
