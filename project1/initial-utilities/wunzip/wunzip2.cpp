#include <string>
#include <string.h>
#include <cstring>
#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sstream>
#include <bitset>
using namespace std;

int writeDecompressionRes(int count, char charToWrite)
{
    stringstream result;
    string resultStr;
    for (int i = 0; i < count; i++)
    {
        result << charToWrite;
    }

    resultStr = result.str();

    // Write results out
    if ((write(STDOUT_FILENO, resultStr.c_str(), resultStr.length())) < 0)
    {
        return -1;
    }

    return 0;
}

int main(int argc, char *argv[])
{
    int fileDescriptor;
    int ret;
    int writeRet;
    char buffer[1];
    string str;
    stringstream stringOut;
    stringstream totalInput;

    if (argc == 1)
    {
        stringOut << "wunzip: file1 [file2 ...]\n";
        // Convert to cstring so that it can be written out to stdout
        str = stringOut.str();

        // Error checking--no instructions given on what to do here
        if ((writeRet = write(STDOUT_FILENO, str.c_str(), str.length())) < 0)
        {
            return -1;
        }

        return 1;
    }
    else
    {
        // Iterate through each file and concatenate the results first
        for (int i = 1; i < argc; i++)
        {
            fileDescriptor = open(argv[i], O_RDONLY);

            // Error check on opening file
            if (fileDescriptor < 0)
            {
                // First write what needs to be printed out to stdout in a string stream
                stringOut << "wunzip: cannot open file\n";

                // Convert to cstring so that it can be written out to stdout
                str = stringOut.str();
                // Error checking--no instructions given on what to do here
                if ((writeRet = write(STDOUT_FILENO, str.c_str(), str.length())) < 0)
                {
                    return -1;
                }
                // Open failed--return 1
                return 1;
            }
            else
            {
                // Combine into one stringStream
                while ((ret = read(fileDescriptor, buffer, sizeof(buffer))) > 0)
                {
                    totalInput << buffer[0];
                }
            }
            // Close file
            close(fileDescriptor);
        }

        // Perform the decompression
        string totalInputStr = totalInput.str();

        for (unsigned long int i = 0; i < totalInputStr.size(); i += 5)
        {
            string strToDecompress = totalInputStr.substr(i, 5);
            string charCountSubStr = strToDecompress.substr(0, 4);
            char charCount = charCountSubStr[0];
            // char compressedChar = strToDecompress[4];

            // Convert character to int
            // int charCountInt = (int)charCount;

            cout << charCountSubStr << "|" << charCount << " " << endl;

            // Write the results out
            // writeDecompressionRes(charCountInt, compressedChar);
        }
    }
    return 0;
}
