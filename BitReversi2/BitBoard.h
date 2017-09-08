#pragma once

typedef unsigned long long uint64;

class BitBoard
{
public:
	BitBoard();
	~BitBoard();

	void Reset();
	void Print();
	void Undo();
	uint64 getReverse();
	uint64 getMobility();
	void Put(uint64 pos);

private:

};