#include<exception>

class CmdException : public std::exception {
public:
    
    const char *what() const throw() {
        return "Something missing!\n";
    }

    const char* filenameMiss() const throw()
    {
        return "Misssing the filename\n";
    }

    const char* dirnameMiss() const throw()
    {
        return "Misssing the directory name\n";
    }

};