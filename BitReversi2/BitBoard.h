#pragma once
#include "consts.h"
#include <intrin.h>

#define BOARD_SIZE 8

#ifdef __AVX2__
alignas(64) static __m256i flip_v8_table256;

#elif __AVX__
alignas(64) static __m128i flip_v8_table128;
#endif

void Board_InitConst();

class BitBoard
{
	uint64 stone[2];
public:

	BitBoard();
	~BitBoard();

	//盤面の初期化
	void Reset();
	//盤面を画面に出力
	void Print();
	//戻す
	void Undo();
	//反転する石を求める
	uint64 getReverse(uint64 pos, char color);
	//着手可能位置を求める
	uint64 getMobility(char color);
	//posに着手
	bool Put(uint64 pos, char color);
	//対戦終了か
	bool isFinish();

private:

};

class M256I {
public:
	__m256i m256i;
	M256I() {}
	M256I(__m256i in) : m256i(in) {}
	M256I(uint64 in) : m256i(_mm256_set1_epi64x(in)) {}
	M256I(uint64 x, uint64 y, uint64 z, uint64 w) : m256i(_mm256_set_epi64x(x, y, z, w)) {}

	M256I operator+() { return *this; }
	M256I operator-() { return _mm256_sub_epi64(_mm256_setzero_si256(), this->m256i); }
	M256I operator~() { return _mm256_andnot_si256(m256i, M256I(0xFFFFFFFFFFFFFFFFULL).m256i); }

	M256I operator+(const int in) { return _mm256_add_epi64(this->m256i, M256I(in).m256i); }
	M256I operator+(const M256I &in) { return _mm256_add_epi64(this->m256i, in.m256i); }

	M256I operator-(const int in) { return _mm256_sub_epi64(this->m256i, M256I(in).m256i); }
	M256I operator-(const M256I &in) { return _mm256_sub_epi64(this->m256i, in.m256i); }

	M256I operator >> (const int shift) { return _mm256_srli_epi64(this->m256i, shift); }
	M256I operator >> (const M256I shift) { return _mm256_srlv_epi64(this->m256i, shift.m256i); }
	M256I operator << (const int shift) { return _mm256_slli_epi64(this->m256i, shift); }
	M256I operator << (const M256I shift) { return _mm256_sllv_epi64(this->m256i, shift.m256i); }

	M256I operator| (const M256I &in) { return _mm256_or_si256(this->m256i, in.m256i); }
	M256I operator&(const M256I &in) { return _mm256_and_si256(this->m256i, in.m256i); }

};

//AVXでは用意されていない命令の代わり(右シフト)
inline uint64 sr(uint64 a, uint64 b);
//AVXでは用意されていない命令の代わり(左シフト)
inline uint64 sl(uint64 a, uint64 b);

class M128I {
public:
	__m128i m128i;
	M128I() {}
	M128I(__m128i in) : m128i(in) {}
	M128I(uint64 in) : m128i(_mm_set1_epi64x(in)) {}
	M128I(uint64 x, uint64 y) : m128i(_mm_set_epi64x(x, y)) {}

	M128I operator+() { return *this; }
	M128I operator-() { return _mm_sub_epi64(_mm_setzero_si128(), this->m128i); }
	M128I operator~() { return _mm_andnot_si128(m128i, M128I(0xFFFFFFFFFFFFFFFFULL).m128i); }

	M128I operator+(const int in) { return _mm_add_epi64(this->m128i, M128I(in).m128i); }
	M128I operator+(const M128I &in) { return _mm_add_epi64(this->m128i, in.m128i); }

	M128I operator-(const int in) { return _mm_sub_epi64(this->m128i, M128I(in).m128i); }
	M128I operator-(const M128I &in) { return _mm_sub_epi64(this->m128i, in.m128i); }

	M128I operator >> (const int shift) { return _mm_srli_epi64(this->m128i, shift); }
	M128I operator >> (const M128I shift) {
		return _mm_set_epi64x(sr(_mm_extract_epi64(this->m128i, 1), (uint64)_mm_extract_epi64(shift.m128i, 1)), sr(_mm_extract_epi64(this->m128i, 0), (uint64)_mm_extract_epi64(shift.m128i, 0)));
	}
	M128I operator << (const int shift) { return _mm_slli_epi64(this->m128i, shift); }
	M128I operator << (const M128I shift) {
		return _mm_set_epi64x(sl(_mm_extract_epi64(this->m128i, 1), (uint64)_mm_extract_epi64(shift.m128i, 1)), sl(_mm_extract_epi64(this->m128i, 0), (uint64)_mm_extract_epi64(shift.m128i, 0)));
	}

	M128I operator| (const M128I &in) { return _mm_or_si128(this->m128i, in.m128i); }
	M128I operator& (const M128I &in) { return _mm_and_si128(this->m128i, in.m128i); }
	M128I operator^ (const M128I &in) { return _mm_xor_si128(this->m128i, in.m128i); }

};
