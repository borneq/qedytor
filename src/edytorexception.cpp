#include "edytorexception.h"

using namespace qedytor;

EdytorException::EdytorException(const QString _message, int _n): n(_n)
{
  if (_message=="")
  {
      msgbuf = nullptr;
      return;
  }
  int len = _message.length();
  msgbuf = (char*)malloc(len+1);
  if (!msgbuf) return;
  strcpy(msgbuf, _message.toStdString().c_str());
}

EdytorException::~EdytorException()
{
    free(msgbuf);
}

// Returns a C-style character string describing the general cause
//  of the current error.
const char* EdytorException::what()
{
    return msgbuf;
}

