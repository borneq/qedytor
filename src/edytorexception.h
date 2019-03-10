#pragma once
#include <stdlib.h>
#include <QString>

namespace qedytor {
class EdytorException
{
private:
    char *msgbuf;
    int n;
public:
    EdytorException(const QString _message, int _n=0);
    virtual ~EdytorException();
    // Returns a C-style character string describing the general cause
    //  of the current error.
    virtual const char* what();
};
}
