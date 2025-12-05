
#ifndef RC2_H
#define RC2_H

#include <string>

using namespace std;

extern "C" {
    bool EncryptFileRC2(const string& inputFile, const string& outputFile);
    bool DecryptFileRC2(const string& inputFile, const string& outputFile);
}

#endif
