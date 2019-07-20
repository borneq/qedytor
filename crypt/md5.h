#pragma once
#include "hash.h"
#include <stdint.h>

namespace crypt {

class Md5 : public Hash
{
protected:
	uint32_t lenHi, lenLo;
	uint32_t index;
	uint32_t CurrentHash[4];
	uint8_t HashBuffer[64];
	void compress();
public:
	Md5();
	~Md5();
	/// Get the algorithm name
	std::string getAlgorithm();
	/// Get the size of the digest produced - in bits
	int getHashSize();
	/// Tests the implementation with several test vectors
	bool selfTest();
	/// Initialize the hash algorithm
	void init();
	/// Create the final digest and clear the stored information.
	/// The size of the Digest var must be at least equal to the hash size
	void final(uint8_t *digest);
	/// Clear any stored information with out creating the final digest
	void burn();
	/// Update the hash buffer with Size bytes of data from Buffer
	void update(const unsigned char *buffer, uint32_t size);
};

}