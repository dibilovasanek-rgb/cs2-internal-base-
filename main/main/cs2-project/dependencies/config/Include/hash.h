#pragma once
#pragma warning( disable : 4307 ) // '*': integral constant overflow
#pragma warning( disable : 4244 ) // possible loss of data

#include <cstdint>
#include <string>

// compile time, string hiding
#define fnvc( str )[&]( ){ constexpr uint64_t out = util::hash::hash( str ); return out; }( )

// compiletime or const runtime, not string hiding
#define fnvr( str ) util::hash::hash( str )

namespace util
{
	// compile time, string hiding
	namespace hash
	{
		// https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function#FNV_hash_parameters
		// should change
		constexpr uint64_t fnv64_prime = 0xcbf29ce484222325;
		constexpr uint64_t fnv64_basis = 0x100000001B3;

		// strlen that doesnt include the null terminator
		__forceinline constexpr size_t strlen(const char* str)
		{
			size_t out = 1;

			for (; str[out] != '\0'; ++out);

			return out;
		}

		__forceinline constexpr size_t strlen(const wchar_t* str)
		{
			size_t out = 1;

			for (; str[out] != '\0'; ++out);

			return out;
		}

		__forceinline constexpr uint64_t hash(const char* str)
		{
			uint64_t out = fnv64_basis;
			size_t len = hash::strlen(str);

			for (size_t i = 0; i < len; ++i)
				out = (out ^ str[i]) * fnv64_prime;

			return out;
		}

		__forceinline constexpr uint64_t hash(const wchar_t* str)
		{
			uint64_t out = fnv64_basis;
			size_t len = hash::strlen(str);

			for (size_t i = 0; i < len; ++i)
				out = (out ^ str[i]) * fnv64_prime;

			return out;
		}

		__forceinline constexpr uint64_t hash(const uint64_t num)
		{
			uint64_t out = fnv64_basis;

			auto* p = (uint8_t*)&num;
			for (auto i = 0; i < 8; i++)
				out = (out ^ p[i]) * fnv64_prime;

			return out;
		}
	}
}