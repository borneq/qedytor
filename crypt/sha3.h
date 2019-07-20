/*
* Base on libkeccak-tiny
* https://github.com/coruus/keccak-tiny
* Implementor: David Leon Gil
* License: CC0, attribution kindly requested. Blame taken too,
* but not liability.
*
* Added multiple update one hash
*/
#pragma once
#include <cstdint>
#include "hash.h"

namespace crypt {

class Sha3 : public Hash
{
protected:
	int digestBits;
	size_t rate;
	size_t outlen;
	uint8_t sponge[1600 / 8];
	uint8_t tail[200 - (224 / 4)];//max rate size
	size_t tailIndex;
	static void keccakf(void* state);
public:
	Sha3(int digestBits);
	~Sha3();
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