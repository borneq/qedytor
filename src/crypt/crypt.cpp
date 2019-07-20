#include "md5.h"
#include "sha3.h"
#include "rijndael.h"

using namespace crypt;

int main()
{
	Md5 hash;
	hash.selfTest();

	Sha3 hash224(224);
	hash224.selfTest();
	Sha3 hash256(256);
	hash256.selfTest();
	Sha3 hash384(384);
	hash384.selfTest();
	Sha3 hash512(512);
	hash512.selfTest();

	Rijndael crypt;
	crypt.selfTest();

}
