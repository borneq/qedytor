#pragma once
#include "blockcipher128.h"

namespace crypt {
const int BC = 4;
const int MAXROUNDS = 14;

class Rijndael : public BlockCipher128
{
protected:
	int numrounds;
	uint32_t rk[MAXROUNDS + 1][8], drk[MAXROUNDS + 1][8];
	void initKey(uint8_t *key, uint32_t size);
public:
	Rijndael();
	~Rijndael();
	int getMaxKeySize();	
	std::string getAlgorithm();
	void burn();
	void encryptECB(uint8_t *inData, uint8_t *outData);
	void decryptECB(uint8_t *inData, uint8_t *outData);
	bool selfTest();
};

}