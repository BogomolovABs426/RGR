
#ifndef ROT13_H
#define ROT13_H

#include <string>

using namespace std;

extern "C" {
    bool EncryptFileROT13(const string& inputFile, const string& outputFile);
    bool DecryptFileROT13(const string& inputFile, const string& outputFile);
}

#endif
