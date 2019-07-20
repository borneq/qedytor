#include "een.h"
#include "crypt/md5.h"
#include "crypt/rijndael.h"
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
            || header.cipher==ecNone) throw EdytorException("Bad een header");
    int numRawBytes = content.size()-int(sizeof(EenHeader));
    if (header.header_version!=0 || header.cipher!=ecRijndael
        || header.data_size > int64_t(numRawBytes))
        throw EdytorException("Bad EEN file (or other version)");
    QByteArray raw = content.mid(sizeof(EenHeader), int(header.data_size));
    crypt::Md5 md5;
    crypt::Rijndael aes;
    md5.init();
    md5.updateStr(password);
    unsigned char digest[128/8];
    md5.final(digest);
    aes.init(digest, sizeof(digest)*8, nullptr);
    QByteArray decodedText;
    decodedText.resize(int(header.data_size));
    aes.decrypt((uint8_t*)raw.data(),(uint8_t*)(decodedText.data()),numRawBytes);
    md5.init();
    md5.update((const unsigned char *)(decodedText.data()), decodedText.length());
    md5.updateStr(password);
    md5.final(digest);
    if (memcmp(digest, header.md5sum, sizeof (header.md5sum))!=0)
        throw EdytorException("Bad password (check capslock) or invalid file");
    return decodedText;
}

QByteArray Een::encrypt(QByteArray &content, std::string password)
{
    EenHeader header;
    header.Ident = cIdent;
    header.ID_number = cID_number;
    header.cipher = ecRijndael;
    header.data_size = content.length();
    header.header_version = 0;
    crypt::Md5 md5;
    md5.init();
    md5.update((const unsigned char *)(content.data()), content.length());
    md5.updateStr(password);
    md5.final(header.md5sum);
    md5.init();
    md5.updateStr(password);
    unsigned char digest[128/8];
    md5.final(digest);
    crypt::Rijndael aes;
    aes.init(digest, sizeof(digest)*8, nullptr);
    QByteArray encodedText;
    encodedText.resize(sizeof(header) + int(header.data_size));
    std::memcpy(encodedText.data(), &header, sizeof(header));
    aes.encrypt((unsigned char *)(content.data()), (unsigned char *)encodedText.data() + sizeof(header), content.length());
    return encodedText;
}
