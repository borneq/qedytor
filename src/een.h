#ifndef EEN_H
#define EEN_H

#include <QByteArray>
#include <cstring>
#include <stdint.h>

namespace qedytor {
class Een
{
    struct EenHeader
    {
        uint32_t Ident;
        uint32_t ID_number;
        uint32_t header_version;
        uint32_t cipher;
        int64_t data_size;
        uint8_t md5sum[16];
    };
    const uint32_t cIdent = 0x4E4545; //'EEN'#0
    const uint32_t cID_number = 0xE4EEA4AA;
    const uint32_t ecNone = 0;
    const uint32_t ecRijndael = 1;
public:
    Een();
    QByteArray decrypt(QByteArray &content, std::string password);
};
}
#endif // EEN_H
