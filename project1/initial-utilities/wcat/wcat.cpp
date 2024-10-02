#include <string>
#include <cstring>
#include <iostream> // For debugging
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sstream>
using namespace std;

int main(int argc, char *argv[]) {
    int fileDescriptor;
    int ret;
    int writeRet;
    char buffer[4096];
    string str;
    stringstream stringOut;

    for (int i = 1; i < argc; i++)
    {
        fileDescriptor = open(argv[i], O_RDONLY);
        // Error check
        if (fileDescriptor < 0) {
            // First write what needs to be printed out to stdout in a string stream
            stringOut << "wcat: cannot open file\n";
            
            // Convert to cstring so that it can be written out to stdout
            str = stringOut.str();
            writeRet = write(STDOUT_FILENO, str.c_str(), str.length());
            // Error checking--no instructions given on what to do here
            if (writeRet < 0) {
                return -1;
            }

            // Open failed--return 1
            return 1;
        }
        else
        {
            while ((ret = read(fileDescriptor, buffer, sizeof(buffer))) > 0) {
                writeRet = write(STDOUT_FILENO, buffer, ret);
                if (writeRet < 0)
                {
                    return -1;
                }
            }
        }
        // Close file
        close(fileDescriptor);
    }

    return 0;
}
