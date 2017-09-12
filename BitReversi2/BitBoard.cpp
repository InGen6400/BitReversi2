
//ƒrƒbƒgƒ{[ƒhA”Õ–Ê‚Ì‘€ì

#include "BitBoard.h"
#include "Macros.h"
#include <stdio.h>

#ifdef __AVX2__

//Flip vertical 8
inline M256I flipV8(M256I in) {
	return M256I(_mm256_shuffle_epi8(in.m256i, flip_v8_table256));
}

//in != 0
inline M256I nonzero(M256I in) {
	return _mm256_add_epi64(_mm256_cmpeq_epi64(in.m256i, _mm256_setzero_si256()), _mm256_set1_epi64x(1));
}

inline uint64 h_or(M256I in) {
	__m128i tmp = _mm_or_si128(_mm256_extractf128_si256(in.m256i, 0), _mm256_extractf128_si256(in.m256i, 1));
	return _mm_extract_epi64(tmp, 0) | _mm_extract_epi64(tmp, 1);
}


inline M256I andnot(const M256I in1, const M256I in2) {
	return _mm256_andnot_si256(in1.m256i, in2.m256i);
}

//leading zero count
inline M256I lzpos(M256I &in) {
	in = in | (in >> 1);
	in = in | (in >> 2);
	in = in | (in >> 4);
	in = andnot(in >> 1, in);
	in = flipV8(in);
	in = in & -in;
	return flipV8(in);
}
#elif __AVX__

inline M128I flipV8(M128I in) {
	return M128I(_mm_shuffle_epi8(in.m128i, flip_v8_table128));
}

inline M128I nonzero(M128I in) {
	return _mm_add_epi64(_mm_cmpeq_epi64(in.m128i, _mm_setzero_si128()), _mm_set1_epi64x(1));
}

//horizontal or
inline uint64 h_or(M128I in) {
	return _mm_extract_epi64(in.m128i, 0) | _mm_extract_epi64(in.m128i, 1);
}

inline M128I andnot(const M128I in1, const M128I in2) {
	return _mm_andnot_si128(in1.m128i, in2.m128i);
}

//leading zero count
inline M128I lzpos(M128I &in) {
	in = in | (in >> 1);
	in = in | (in >> 2);
	in = in | (in >> 4);
	in = andnot(in >> 1, in);
	in = flipV8(in);
	in = in & -in;
	return flipV8(in);
}

#endif

void Board_InitConst() {
#ifdef __AVX2__
	flip_v8_table256 = _mm256_set_epi8(
		24, 25, 26, 27, 28, 29, 30, 31,
		16, 17, 18, 19, 20, 21, 22, 23,
		8, 9, 10, 11, 12, 13, 14, 15,
		0, 1, 2, 3, 4, 5, 6, 7
	);
#elif __AVX__
	flip_v8_table128 = _mm_set_epi8(
		8, 9, 10, 11, 12, 13, 14, 15,
		0, 1, 2, 3, 4, 5, 6, 7
	);
#endif
}

BitBoard::BitBoard()
{
	
}

BitBoard::~BitBoard()
{

}

void BitBoard::Reset() {
	stone[WHITE] = 0x0000001008000000;
	stone[BLACK] = 0x0000000810000000;
}

void BitBoard::Print()
{
	int x,y;
	uint64 cursor = 1;
	printf(" @@@@‚`  ‚a  ‚b  ‚c  ‚d  ‚e  ‚f  ‚g          \n");
	printf(" @{\{\{\{\{\{\{\{\{\{\{\n");
	printf(" @b”b”b”b”b”b”b”b”b”b”b\n");
	printf(" @{\{\{\{\{\{\{\{\{\{\{\n");

	for (y = 0; y < BOARD_SIZE; y++) {
		printf(" %d b”", y+1);
		for (x = 0; x < BOARD_SIZE; x++, cursor*=2) {
			printf("b");
			if ((stone[WHITE] & cursor) != 0) {
				printf("œ");
			}
			else if ((stone[BLACK] & cursor) != 0) {
				printf("Z");
			}
			else {
				printf("@");
			}
		}
		printf("b”b\n");
		printf(" @{\{\{\{\{\{\{\{\{\{\{\n");
	}


	printf(" @b”b”b”b”b”b”b”b”b”b”b\n");
	printf(" @{\{\{\{\{\{\{\{\{\{\{\n");
	printf(" @@@@‚`  ‚a  ‚b  ‚c  ‚d  ‚e  ‚f  ‚g\n");

}

void BitBoard::Undo()
{

}

inline unsigned long ntz(uint64 in) {
	unsigned long idx;
	if (_BitScanForward64(&idx, in)) {
		return idx;
	}
	else
	{
		return 64;
	}
}

uint64 BitBoard::getReverse(uint64 pos, char color)
{
	//and
	//
#ifdef __AVX2__	
	//player's stones
	M256I mes = M256I(stone[color]);
	//Masked Opp
	M256I oppM = M256I(0xFFFFFFFFFFFFFFFF, 0x7E7E7E7E7E7E7E7E, 0x7E7E7E7E7E7E7E7E, 0x7E7E7E7E7E7E7E7E) & M256I(stone[oppColor(color)]);

	int posCnt = ntz(pos);
	//mask for UP LEFT U_RIGHT U_LEFT
	M256I mask = M256I(
		0x0101010101010100ULL,
		0x00000000000000FEULL,
		0x0002040810204080ULL,
		0x8040201008040200ULL) << posCnt;
	//outflank
	M256I outf = mask & ((oppM | ~mask) + 1) & mes;
	//will flip
	M256I flip = (outf - nonzero(outf)) & mask;


	//DOWN RIGHT D_LEFT D_RIGHT
	mask = M256I(
		0x0080808080808080ULL,
		0x7F00000000000000ULL,
		0x0102040810204000ULL,
		0x0040201008040201ULL) >> (63 - posCnt);

	//outf = (MSB1 >> lzcnt(~oppM & mask)) & me
	//AVX2‚É‚Í•À—ñ‚ÅLZCNT‚·‚éˆ—‚ª‚È‚¢‚Ì‚Å”z—ñ‚É“WŠJ‚µ‚Älacnt()
	outf = lzpos(andnot(oppM, mask)) & mes;

	//flip = flip | ((-outf << 1) & mask)
	flip = flip | (-outf << 1) & mask;
	//horizontal or 64x4
	return h_or(flip);

#elif __AVX__
	
	//player's stones
	M128I mes = _mm_set1_epi64x(stone[color]);
	//Masked Opp
	M128I oppM = M128I(0xFFFFFFFFFFFFFFFF, 0x7E7E7E7E7E7E7E7E) & M128I(stone[oppColor(color)]);

	int posCnt = ntz(pos);
	//mask for UP LEFT
	M128I mask = M128I(0x0101010101010100ULL, 0x00000000000000FEULL) << posCnt;

	//outflank
	M128I outf = mask & ((oppM | ~mask) + 1) & mes;
	//will flip
	M128I flip = (outf - nonzero(outf)) & mask;

	//DOWN RIGHT
	mask = M128I(0x0080808080808080ULL, 0x7F00000000000000ULL) >> (63 - posCnt);

	outf = lzpos(andnot(oppM, mask)) & mes;

	flip = flip | (-outf << 1) & mask;


	oppM = M128I(0x7E7E7E7E7E7E7E7EULL, 0x7E7E7E7E7E7E7E7EULL) & M128I(*opp);
	//mask for U_RIGHT U_LEFT
	mask = M128I(0x0002040810204080ULL, 0x8040201008040200ULL) << posCnt;

	outf = mask & ((oppM | ~mask) + 1) & mes;
	flip = flip | ((outf - nonzero(outf)) & mask);

	//D_LEFT D_RIGHT
	mask = M128I(0x0102040810204000ULL, 0x0040201008040201ULL) >> (63 - posCnt);

	outf = lzpos(andnot(oppM, mask)) & mes;

	flip = flip | (-outf << 1) & mask;

	//horizontal or 64x4
	return h_or(flip);

#endif
}

uint64 BitBoard::getMobility(char color)
{
#ifdef __AVX2__
	M256I mes(stone[color]);
	M256I oppM = M256I(0xFFFFFFFFFFFFFFFFULL, 0x7E7E7E7E7E7E7E7EULL, 0x7E7E7E7E7E7E7E7EULL, 0x7E7E7E7E7E7E7E7EULL) & M256I(stone[oppColor(color)]);
	M256I empty = M256I(~(stone[color] | stone[oppColor(color)]));
	M256I shift = M256I(8, 1, 9, 7);

	M256I move(oppM & (mes >> shift));
	move = move | (oppM & (move >> shift));
	move = move | (oppM & (move >> shift));
	move = move | (oppM & (move >> shift));
	move = move | (oppM & (move >> shift));
	move = move | (oppM & (move >> shift));
	M256I mobility = empty & (move >> shift);

	move = oppM & (mes << shift);
	move = move | (oppM & (move << shift));
	move = move | (oppM & (move << shift));
	move = move | (oppM & (move << shift));
	move = move | (oppM & (move << shift));
	move = move | (oppM & (move << shift));
	mobility = mobility | (empty & (move << shift));

	return h_or(mobility);

#elif __AVX__

	M128I mes(stone[color]);
	M128I empty = M128I(~(stone[color] | stone[oppColor(color)]));
	M128I oppM = M128I(0xFFFFFFFFFFFFFFFFULL, 0x7E7E7E7E7E7E7E7EULL) & M128I(stone[oppColor(color)]);
	M128I shift = M128I(8, 1);

	M128I move(oppM & (mes >> shift));
	move = move | (oppM & (move >> shift));
	move = move | (oppM & (move >> shift));
	move = move | (oppM & (move >> shift));
	move = move | (oppM & (move >> shift));
	move = move | (oppM & (move >> shift));
	M128I mobility = empty & (move >> shift);

	move = oppM & (mes << shift);
	move = move | (oppM & (move << shift));
	move = move | (oppM & (move << shift));
	move = move | (oppM & (move << shift));
	move = move | (oppM & (move << shift));
	move = move | (oppM & (move << shift));
	mobility = mobility | (empty & (move << shift));


	oppM = M128I(0x7E7E7E7E7E7E7E7EULL, 0x7E7E7E7E7E7E7E7EULL) & M128I(opp);
	shift = M128I(9, 7);

	move = (oppM & (mes >> shift));
	move = move | (oppM & (move >> shift));
	move = move | (oppM & (move >> shift));
	move = move | (oppM & (move >> shift));
	move = move | (oppM & (move >> shift));
	move = move | (oppM & (move >> shift));
	mobility = mobility | (empty & (move >> shift));

	move = oppM & (mes << shift);
	move = move | (oppM & (move << shift));
	move = move | (oppM & (move << shift));
	move = move | (oppM & (move << shift));
	move = move | (oppM & (move << shift));
	move = move | (oppM & (move << shift));
	mobility = mobility | (empty & (move << shift));

	return h_or(mobility);
#endif
}

bool BitBoard::Put(uint64 pos, char color)
{
	uint64 reverse;

	reverse = getReverse(pos, color);

	//”½“]‰Â”\‚È‚ç’…è
	if (reverse != 0) {
		stone[color] ^= pos | reverse;
		stone[oppColor(color)] ^= reverse;
		return true;
	}

	return false;
}

bool BitBoard::isFinish()
{
	if (getMobility(BLACK) == 0 && getMobility(WHITE) == 0) {
		return true;
	}
	return false;
}

inline uint64 sr(uint64 a, uint64 b) {
	return a >> b;
}

inline uint64 sl(uint64 a, uint64 b) {
	return a << b;
}