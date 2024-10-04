#include <string>
#include <cstring>
#include <iostream> // For debugging
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sstream>
using namespace std;

int main(int argc, char *argv[])
{
    int fileDescriptor;
    int ret;
    int writeRet;
    char buffer[5];
    string str;
    stringstream stringOut;

    if (argc == 1) {
        stringOut << "wzip : file1[file2...]\n";
        // Convert to cstring so that it can be written out to stdout
        str = stringOut.str();
        
        // Error checking--no instructions given on what to do here
        if (writeRet = write(STDOUT_FILENO, str.c_str(), str.length()) < 0)
        {
            return -1;
        }

        return 1;
    } 
    else 
    {
        // Iterate through each file
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
                if (writeRet = write(STDOUT_FILENO, str.c_str(), str.length()) < 0)
                {
                    return -1;
                }
                // Open failed--return 1
                return 1;
            }
            else
            {
                // Init prevChar to null

                // Read each file
                while ((ret = read(fileDescriptor, buffer, sizeof(buffer))) > 0)
                {
                    // Read one byte at a time
                    // If prevChar is null, set prevChar and increment charCount
                    // elif prevChar and this char diff, connvert count to binary in 4 bits and add the character. Reset to 0
                    // else same, increment
                }
            }
        }
        // Close file
        close(fileDescriptor);
    }

    return 0;
}
