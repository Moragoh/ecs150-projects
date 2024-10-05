#include <string>
#include <cstring>
#include <iostream> 
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sstream>
#include <bitset>
using namespace std;

int writeCompressionRes(uint32_t count, char charToWrite)
{
    // Write the count in binary
    if ((write(STDOUT_FILENO, &count, 4)) < 0)
        return -1;
    // Write the character
    if ((write(STDOUT_FILENO, &charToWrite, sizeof(charToWrite))) < 0)
        return -1;

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
        stringOut << "wzip: file1 [file2 ...]\n";
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
                stringOut << "wzip: cannot open file\n";

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

        // Perform the compression
        string totalInputStr = totalInput.str();
        // Init prevChar to null
        char prevChar = '\0';
        uint32_t charCount = 0; // Fixed to 4 bytes
        // Iterate through the string character by character
        for (int i = 0; i < totalInputStr.length(); i++)
        {

            // Check if prevChar is null (to see if this is the fist char)
            if (prevChar == '\0')
            {
                prevChar = totalInputStr[i];
                charCount++;
            }
            else if (totalInputStr[i] == prevChar)
            {
                charCount++;
            }
            else
            {
                // Write the compressed version of this string and reset the chaarCount since we encountered a new character
                writeCompressionRes(charCount, prevChar);

                // Reset
                charCount = 1;
                prevChar = totalInputStr[i];
            }
        }

        // Special case for writing whatever was countiing when the file ends
        writeCompressionRes(charCount, prevChar);
    }
    return 0;
}
