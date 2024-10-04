#include <string>
#include <fcntl.h>
#include <sstream>
#include <unistd.h> // For stdout and stdin macros
#include <iostream> // For debugging

using namespace std;

int searchString(string searchTerm, string stringToSearch)
{

    size_t res = stringToSearch.string::find(searchTerm);

    if (res != string::npos)
    {
        // Match is found
        return 0;
    }
    else
    {
        return 1;
    }
}

int main(int argc, char *argv[])
{
    int ret;
    int fileDescriptor;
    char buffer[1];
    string str;
    string searchTerm;
    stringstream stringOut;
    stringstream currentLine;
    stringstream wgrepResult;

    // Check if argc is 1 (no arguments)
    if (argc == 1)
    {
        stringOut << "wgrep: searchterm [file ...]\n"; 
        str = stringOut.str();

        // Error checking--no instructions given on what to do here
        if (write(STDOUT_FILENO, str.c_str(), str.length()) < 0)
        {
            return -1;
        }

        return 1;
    }
    else if (argc == 2)
    {
        searchTerm = argv[1];
        currentLine.clear();
        fileDescriptor = STDIN_FILENO;
        string currStr = "";
        while ((ret = read(fileDescriptor, buffer, sizeof(buffer))) > 0)
        {
            if (buffer[0] == '\n')
            {
                // char c = buffer[0];
                // currStr.push_back(c);
                // Newline, so search through this line
                int searchRes = searchString(searchTerm, currStr);
                // Write the search results--if nothing is found, nothing will be added.

                if (searchRes == 0)
                {
                    wgrepResult << currStr << endl;
                }

                currStr = "";
            }
            else
            {
                char c = buffer[0];
                currStr.push_back(c);
            }
        }

        // Done--print out the results to stdout
        str = wgrepResult.str();
        if (write(STDOUT_FILENO, str.c_str(), str.length()) < 0)
        {
            return -1;
        }
    }
    else
    {
        // Search term and file name provided
        fileDescriptor = open(argv[2], O_RDONLY);
        if (fileDescriptor < 0)
        {
            stringOut << "wgrep: cannot open file\n";

            // Convert to cstring so that it can be written out to stdout
            str = stringOut.str();
            if (write(STDOUT_FILENO, str.c_str(), str.length()) < 0)
            {
                return -1;
            }

            return 1;
        }
        else
        {
            // Start reading letter by letter until newline
            searchTerm = argv[1];
            currentLine.clear();
            string currStr = "";

            while ((ret = read(fileDescriptor, buffer, sizeof(buffer))) > 0)
            {
                if (buffer[0] == '\n')
                {
                    // char c = buffer[0];
                    // currStr.push_back(c);                
                    // Newline, so search through this line
                    int searchRes = searchString(searchTerm, currStr);
                    // Write the search results--if nothing is found, nothing will be added.

                    if (searchRes == 0) 
                    {
                        wgrepResult << currStr << endl;
                    }
                    
                    currStr = "";
                }
                else
                {
                    char c = buffer[0];
                    currStr.push_back(c);
                }
            }

            // Done--print out the results to stdout
            str = wgrepResult.str();
            if (write(STDOUT_FILENO, str.c_str(), str.length()) < 0)
            {
                return -1;
            }
        }
    }
    close(fileDescriptor);
    return 0;
}
