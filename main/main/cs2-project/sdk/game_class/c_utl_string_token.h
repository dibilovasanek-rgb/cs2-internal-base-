#pragma once
#include <cstdint>
#include <corecrt_malloc.h>
#include <string.h>

#define TOLOWERU(c) ((uint32_t)(((c >= 'A') && (c <= 'Z')) ? c + 32 : c))
#define STRINGTOKEN_MURMURHASH_SEED 0x31415926

inline uint32_t MurmurHash2(const void* key, int len, uint32_t seed)
{
	/* 'm' and 'r' are mixing constants generated offline.
	   They're not really 'magic', they just happen to work well.  */

	const uint32_t m = 0x5bd1e995;
	const int r = 24;

	/* Initialize the hash to a 'random' value */

	uint32_t h = seed ^ len;

	/* Mix 4 bytes at a time into the hash */

	const unsigned char* data = (const unsigned char*)key;

	while (len >= 4)
	{
		uint32_t k = *(uint32_t*)data;

		k *= m;
		k ^= k >> r;
		k *= m;

		h *= m;
		h ^= k;

		data += 4;
		len -= 4;
	}

	/* Handle the last few bytes of the input array  */

	switch (len)
	{
	case 3:
		h ^= data[2] << 16;
	case 2:
		h ^= data[1] << 8;
	case 1:
		h ^= data[0];
		h *= m;
	};

	/* Do a few final mixes of the hash to ensure the last few
	// bytes are well-incorporated.  */

	h ^= h >> 13;
	h *= m;
	h ^= h >> 15;

	return h;
}

inline uint32_t MurmurHash2LowerCaseA1(const char* pString, int len, uint32_t nSeed)
{
	char* p = (char*)malloc(len + 1);
	for (int i = 0; i < len; i++)
	{
		p[i] = TOLOWERU(pString[i]);
	}
	const auto hash = MurmurHash2(p, len, nSeed);
    free(p); return hash;
}

#define DEBUG_STRINGTOKENS 0

class CUtlStringToken1
{
public:
	std::uint32_t m_nHashCode;
#if DEBUG_STRINGTOKENS
	const char* m_pDebugName;
#endif

	CUtlStringToken1(const char* szString)
	{
		this->SetHashCode1(this->MakeStringToken1(szString));
	}

	bool operator==(const CUtlStringToken1& other) const
	{
		return (other.m_nHashCode == m_nHashCode);
	}

	bool operator!=(const CUtlStringToken1& other) const
	{
		return (other.m_nHashCode != m_nHashCode);
	}

	bool operator<(const CUtlStringToken1& other) const
	{
		return (m_nHashCode < other.m_nHashCode);
	}

	/// access to the hash code for people who need to store thse as 32-bits, regardless of the
	/// setting of DEBUG_STRINGTOKENS (for instance, for atomic operations).
	uint32_t GetHashCode1(void) const
	{
		return m_nHashCode;
	}

	void SetHashCode1(uint32_t nCode)
	{
		m_nHashCode = nCode;
	}

	__forceinline std::uint32_t MakeStringToken1(const char* szString, int nLen)
	{
		std::uint32_t nHashCode = MurmurHash2LowerCaseA1(szString, nLen, STRINGTOKEN_MURMURHASH_SEED);
		return nHashCode;
	}

	__forceinline std::uint32_t MakeStringToken1(const char* szString)
	{
		return MakeStringToken1(szString, (int)strlen(szString));
	}

	//__forceinline std::uint32_t MakeStringToken(CUtlString& str)
	//{
	//    return MakeStringToken(str.Get(), str.Length());
	//}

	CUtlStringToken1()
	{
		m_nHashCode = 0;
	}
};	

