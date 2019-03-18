#include "een.h"
#include <hashes/DCPh_md5.h>
#include <ciphers/DCPc_rijndael.h>
#include "edytorexception.h"

using namespace qedytor;

Een::Een()
{
}

QByteArray Een::decrypt(QByteArray &content, std::string password)
{
    if (content.size()<int(sizeof(EenHeader)))
        throw "Bad een file, too short";
    EenHeader header;
    std::memcpy(&header, content.data(), sizeof(header));
    if (header.Ident!=cIdent || header.ID_number!= cID_number
            || header.cipher==ecNone) throw "Bad een header";
    int numRawBytes = content.size()-int(sizeof(EenHeader));
    if (header.header_version!=0 || header.cipher!=ecRijndael
        || header.data_size > int64_t(numRawBytes))
        throw "Bad EEN file (or other version)";
    QByteArray raw = content.mid(sizeof(EenHeader), int(header.data_size));
    DCPh_md5 md5;
    DCPc_rijndael aes;
    md5.init();
    md5.updateStr(password);
    unsigned char digest[128/8];
    md5.final(digest);
    aes.init(digest, sizeof(digest)*8, nullptr);
    QByteArray decodeText;
    decodeText.resize(int(header.data_size));
    aes.decrypt((uint8_t*)raw.data(),(uint8_t*)(decodeText.data()),numRawBytes);
    md5.init();
    md5.update((const unsigned char *)(decodeText.data()), decodeText.length());
    md5.updateStr(password);
    md5.final(digest);
    if (memcmp(digest, header.md5sum, sizeof (header.md5sum))!=0)
        throw EdytorException("Bad password (check capslock) or invalid file");
    return decodeText;
}

