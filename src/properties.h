#ifndef PROPERTIES_H
#define PROPERTIES_H

namespace qedytor {
class Properties
{
public:
    Properties(){};
    bool isUtfBOM = false;
    bool isUtfDetected = false;
    bool isLocale = false;
};
}
#endif // PROPERTIES_H
