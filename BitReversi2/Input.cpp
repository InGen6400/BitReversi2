
//入力関係など

#include <stdio.h>
#include <ctype.h>
#include "Input.h"
#include "consts.h"

#define BUF_LENGTH 100

uint64 WaitPlayerInput() {
	char input[BUF_LENGTH];
	int i, x=-1, y=-1;
	
	while (true) {
		printf("a1や4Bのように横をアルファベットで、縦を数値で入力してください : ");
		fgets(input, BUF_LENGTH, stdin);
		for (i = 0; i < BUF_LENGTH; i++) {
			if ((input[i] > 'A' && input[i] < 'H') || (input[i] > 'a' && input[i] < 'h')) {
				x = tolower(input[i]) - 'a';
			}
			else if (input[i] > '1' && input[i] < '8') {
				y = input[i] - '1';
			}
			if (x != -1 && y != -1) {
				return (uint64)0x8000000000000000 >> (x + y * 8);
			}
		}
		printf("よく確認してから、もう一度入力してみてください\n");
	}

	return NOMOVE;
}