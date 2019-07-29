#include "blockcipher128.h"
#include "exception.h"
#include "util.h"

using namespace crypt;

BlockCipher128::BlockCipher128()
{
}


BlockCipher128::~BlockCipher128()
{
}

void BlockCipher128::incCounter()
{
	CV[15]++;
	int i = 15;
	while (i> 0 && CV[i] == 0)
	{
		CV[i - 1]++;
		i--;
	}
}

void BlockCipher128::init(uint8_t *key, uint32_t size, const char *initVector)
{
	BlockCipher::init(key, size, initVector);
	initKey(key, size);
	if (!initVector)
	{
#ifdef DCP1COMPAT
#define FILLBYTE 0xFF
#else
#define FILLBYTE 0x00
#endif
		memset(IV, FILLBYTE, 16);
		encryptECB(IV, IV);
		reset();
	}
	else
	{
		memmove(IV, initVector, 16);
		reset();
	}
}

void BlockCipher128::setIV(const char *value)
{
	if (!fInitialized)
		throw EDCP_blockcipher("Cipher not initialized");
	memmove(IV, value, 16);
	reset();
}

void BlockCipher128::getIV(char *value)
{
	if (!fInitialized)
		throw EDCP_blockcipher("Cipher not initialized");
	memmove(value, CV, 16);
}

void BlockCipher128::reset()
{
	if (!fInitialized)
		throw EDCP_blockcipher("Cipher not initialized");
	else
		memmove(CV, IV, 16);
}

void BlockCipher128::burn()
{
	memset(IV, 0xFF, 16);
	memset(CV, 0xFF, 16);
}

void BlockCipher128::encryptCBC(uint8_t *inData, uint8_t *outData, uint32_t size)
{
	if (!fInitialized)
		throw EDCP_blockcipher("Cipher not initialized");
	uint8_t *p1 = inData;
	uint8_t *p2 = outData;
	for (int i = 0; i<size / 16; i++)
	{
		memmove(p2, p1, 16);
		xorBlock(p2, CV, 16);
		encryptECB(p2, p2);
		memmove(CV, p2, 16);
		p1 += 16;
		p2 += 16;
	}
	if ((size % 16) != 0)
	{
		encryptECB(CV, CV);
		memmove(p2, p1, size % 16);
		xorBlock(p2, CV, size % 16);
	}
}

void BlockCipher128::decryptCBC(uint8_t *inData, uint8_t *outData, uint32_t size)
{
	uint8_t temp[16];

	if (!fInitialized)
		throw EDCP_blockcipher("Cipher not initialized");
	uint8_t *p1 = inData;
	uint8_t *p2 = outData;
	memset(temp, 0, sizeof(temp));
	for (int i = 0; i < size / 16; i++)
	{
		memmove(p2, p1, 16);
		memmove(temp, p1, 16);
		decryptECB(p2, p2);
		xorBlock(p2, CV, 16);
		memmove(CV, temp, 16);
		p1 += 16;
		p2 += 16;
	}
	if ((size % 16) != 0)
	{
		encryptECB(CV, CV);
		memmove(p2, p1, size % 16);
		xorBlock(p2, CV, size % 16);
	}
}


void BlockCipher128::encryptCFB8bit(uint8_t *inData, uint8_t *outData, uint32_t size)
{

	uint8_t temp[16];

	if (!fInitialized)
		throw EDCP_blockcipher("Cipher not initialized");
	uint8_t *p1 = inData;
	uint8_t *p2 = outData;
	memset(temp, 0, sizeof(temp));
	for (int i = 0; i<size; i++)
	{
		encryptECB(CV, temp);
		*p2 = *p1 ^ temp[0];
		memmove(CV, CV + 1, 15);
		CV[15] = *p2;
		p1++;
		p2++;
	}
}

void BlockCipher128::decryptCFB8bit(uint8_t *inData, uint8_t *outData, uint32_t size)
{
	uint8_t tempByte;
	uint8_t temp[16];

	if (!fInitialized)
		throw EDCP_blockcipher("Cipher not initialized");
	uint8_t *p1 = inData;
	uint8_t *p2 = outData;
	memset(temp, 0, sizeof(temp));
	for (int i = 0; i<size; i++)
	{
		tempByte = *p1;
		encryptECB(CV, temp);
		*p2 = *p1 ^ temp[0];
		memmove(CV, CV + 1, 15);
		CV[15] = tempByte;
		p1++;
		p2++;
	}
}

void BlockCipher128::encryptCFBblock(uint8_t *inData, uint8_t *outData, uint32_t size)
{
	if (!fInitialized)
		throw EDCP_blockcipher("Cipher not initialized");
	uint8_t *p1 = inData;
	uint8_t *p2 = outData;
	for (int i = 0; i<size / 16; i++)
	{
		encryptECB(CV, CV);
		memmove(p2, p1, 16);
		xorBlock(p2, CV, 16);
		memmove(CV, p2, 16);
		p1 += 16;
		p2 += 16;
	}
	if ((size % 16) != 0)
	{
		encryptECB(CV, CV);
		memmove(p2, p1, size % 16);
		xorBlock(p2, CV, size % 16);
	}
}

void BlockCipher128::decryptCFBblock(uint8_t *inData, uint8_t *outData, uint32_t size)
{
	uint8_t temp[16];

	if (!fInitialized)
		throw EDCP_blockcipher("Cipher not initialized");
	uint8_t *p1 = inData;
	uint8_t *p2 = outData;
	memset(temp, 0, sizeof(temp));
	for (int i = 0; i<size / 16; i++)
	{
		memmove(temp, p1, 16);
		encryptECB(CV, CV);
		memmove(p2, p1, 16);
		xorBlock(p2, CV, 16);
		memmove(CV, temp, 16);
		p1 += 16;
		p2 += 16;
	}
	if ((size % 16) != 0)
	{
		encryptECB(CV, CV);
		memmove(p2, p1, size % 16);
		xorBlock(p2, CV, size % 16);
	}
}

void BlockCipher128::encryptOFB(uint8_t *inData, uint8_t *outData, uint32_t size)
{
	if (!fInitialized)
		throw EDCP_blockcipher("Cipher not initialized");
	uint8_t *p1 = inData;
	uint8_t *p2 = outData;
	for (int i = 0; i<size / 16; i++)
	{
		encryptECB(CV, CV);
		memmove(p2, p1, 16);
		xorBlock(p2, CV, 16);
		p1 += 16;
		p2 += 16;
	}
	if ((size % 16) != 0)
	{
		encryptECB(CV, CV);
		memmove(p2, p1, size % 16);
		xorBlock(p2, CV, size % 16);
	}
}

void BlockCipher128::decryptOFB(uint8_t *inData, uint8_t *outData, uint32_t size)
{
	if (!fInitialized)
		throw EDCP_blockcipher("Cipher not initialized");
	uint8_t *p1 = inData;
	uint8_t *p2 = outData;
	for (int i = 0; i<size / 16; i++)
	{
		encryptECB(CV, CV);
		memmove(p2, p1, 16);
		xorBlock(p2, CV, 16);
		p1 += 16;
		p2 += 16;
	}
	if ((size % 16) != 0)
	{
		encryptECB(CV, CV);
		memmove(p2, p1, size % 16);
		xorBlock(p2, CV, size % 16);
	}
}

void BlockCipher128::encryptCTR(uint8_t *inData, uint8_t *outData, uint32_t size)
{
	uint8_t temp[16];
	if (!fInitialized)
		throw EDCP_blockcipher("Cipher not initialized");
	uint8_t *p1 = inData;
	uint8_t *p2 = outData;
	memset(temp, 0, sizeof(temp));
	for (int i = 0; i<size / 16; i++)
	{
		encryptECB(CV, temp);
		incCounter();
		memmove(p2, p1, 16);
		xorBlock(p2, temp, 16);
		p1 += 16;
		p2 += 16;
	}
	if ((size % 16) != 0)
	{
		encryptECB(CV, temp);
		incCounter();
		memmove(p2, p1, size % 16);
		xorBlock(p2, temp, size % 16);
	}
}

void BlockCipher128::decryptCTR(uint8_t *inData, uint8_t *outData, uint32_t size)
{
	uint8_t temp[16];
	if (!fInitialized)
		throw EDCP_blockcipher("Cipher not initialized");
	uint8_t *p1 = inData;
	uint8_t *p2 = outData;
	memset(temp, 0, sizeof(temp));
	for (int i = 0; i<size / 16; i++)
	{
		encryptECB(CV, temp);
		incCounter();
		memmove(p2, p1, 16);
		xorBlock(p2, temp, 16);
		p1 += 16;
		p2 += 16;
	}
	if ((size % 16) != 0)
	{
		encryptECB(CV, temp);
		incCounter();
		memmove(p2, p1, size % 16);
		xorBlock(p2, temp, size % 16);
	}
}
