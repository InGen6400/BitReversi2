
//���͊֌W�Ȃ�

#include <stdio.h>
#include <ctype.h>
#include "Input.h"
#include "consts.h"

#define BUF_LENGTH 100

uint64 WaitPlayerInput() {
	char input[BUF_LENGTH];
	int i, x=-1, y=-1;
	
	while (true) {
		printf("a1��4B�̂悤�ɉ����A���t�@�x�b�g�ŁA�c�𐔒l�œ��͂��Ă������� : ");
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
		printf("�悭�m�F���Ă���A������x���͂��Ă݂Ă�������\n");
	}

	return NOMOVE;
}