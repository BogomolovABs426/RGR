
#ifndef DES_H
#define DES_H

#include <string>

using namespace std;

extern "C" {
    bool EncryptFileDES(const string& inputFile, const string& outputFile);
    bool DecryptFileDES(const string& inputFile, const string& outputFile);
}

#endif
