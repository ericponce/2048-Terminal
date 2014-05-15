#include <ncurses.h>
#include <stdlib.h>
#include <time.h>

#define GRID_SIZE 4
#define TILE_SIZE 4

typedef struct {
	int win;
	int movement;
} MoveState;

int numDigits(int n) {
	int i = 0;

	while (n / 10 != 0) {
		n /= 10;
		i++;
	}

	return i + 1;
}

void drawSquare(int x, int y, int width, int n) {
	int oy, ox;
	getyx(stdscr, oy, ox); //Save old cursor position
	move(y, x);

	attron(COLOR_PAIR(n));
	int i;
	for (i = 0; i < width; i++) {
		int a;

		for (a = 0; a < width * 2; a++) {
			printw(" ");
		}
		move(y + i + 1, x);
	}
	n = pow2(n);
	if (n == 1) {
		n = 0;
	}
	move(y + width / 2, x + width - (numDigits(n) / 2));
	printw("%d", n);
	attroff(COLOR_PAIR(n));

	move(oy, ox); //restore old cursor position
}

void drawBoard(int **board, int squareWidth, int x, int y) {
	int i, j;

	for (i = 0; i < GRID_SIZE; i++) {
		for (j = 0; j < GRID_SIZE; j++) {
			drawSquare(x + i * squareWidth * 2, y + j * squareWidth, squareWidth, board[i][j]);
		}
	}
}

int pow2(int b) {
	int i;
	int n = 1;
	n = n << b;
	return n;
}

void removeSquare(int x, int y, int width) {
	int oy, ox;
	getyx(stdscr, oy, ox); //Save old cursor position
	move(y, x);

	attron(COLOR_PAIR(0));
	int i;
	for (i = 0; i < width; i++) {
		int a;
		for (a = 0; a < width * 2; a++) {
			printw(" ");
		}
		move(y+i+1, x);
	}
	attroff(COLOR_PAIR(0));

	move(oy, ox); //restore old cursor position
}

void shiftRowLeft(int *row, int start) {
	int i;
	for (i = start; i < GRID_SIZE - 1; i++) {
		row[i] = row[i + 1];
		row[i + 1] = 0;
	}
}

int seekTargetRight(int *row, int start, int n) {
	int i;
	for (i = start; i < GRID_SIZE; i++) {
		if (row[i] != 0 && row[i] != n) {
			return -1;
		} else if (row[i] == n) {
			return i;
		}
	}
	return -1;
}

void reverseRow(int *row) {
  int temp;
  int start = 0;
  int end = GRID_SIZE - 1;
  while(start < end)
  {
    temp = row[start];   
    row[start] = row[end];
    row[end] = temp;
    start++;
    end--;
  }   
}  

//Add actual movement functionality

void mergeRow(MoveState* st, int *row, int direction) {
	if (direction == -1) {
		int i;
		for (i = 0; i < GRID_SIZE - 1; i++) {
			if (row[i] != 0) {
				int n = seekTargetRight(row, i + 1, row[i]);
				if (n != -1) {
					row[i] += 1;
					if (row[i] == 11) st->win = 1;
					row[n] = 0;
					st->movement = 1;
				}
			} else {
				int j;
				for (j = 0; j < GRID_SIZE - i - 1; j++) {
					shiftRowLeft(row, i);
					if (row[i] != 0) break;
				}
				st->movement = 1;
				if (row[i] == 0) {
					continue;
				}
				i -= 1;
			}
		}
	} else {
		reverseRow(row);
		mergeRow(st, row, -1);
		reverseRow(row);
	}
}

int* getRow(int **board, int n) {
	int i;
	int *row = malloc(GRID_SIZE * sizeof(int));
	for (i = 0; i < GRID_SIZE; i++) {
		row[i] = board[i][n];
	}
	return row;
}

void insertRow(int **board, int *row, int n) {
	int i;
	for (i = 0; i < GRID_SIZE; i++) {
		board[i][n] = row[i];
	}
}

int addRandom(int **board, int max) {
	int len = 0;
	int list[GRID_SIZE*GRID_SIZE][2];

	int i, j;

	for (i = 0; i < GRID_SIZE; i++) {
		for (j = 0; j < GRID_SIZE; j++) {
			if (board[i][j] == 0) {
				list[len][0] = i;
				list[len][1] = j;
				len++;
			}
		}
	}

	if (len == 0) {
		return 1;
	} else {
		if (max == 0) {
			return 0;
		}
		int p = rand() % len;
		board[list[p][0]][list[p][1]] = rand() % max + 1;
		return 0;
	}

}

void actOnBoard(MoveState *st, int **board, int key) {
	st->win = 0;
	st->movement = 0;
	if (key == KEY_LEFT) {
		int i;
		for (i = 0; i < GRID_SIZE; i++) {
			int *row = getRow(board, i);
			mergeRow(st, row, -1);
			insertRow(board, row, i);
			free (row);
		}
	} else if (key == KEY_RIGHT) {
		int i;
		for (i = 0; i < GRID_SIZE; i++) {
			int *row = getRow(board, i);
			mergeRow(st, row, 1);
			insertRow(board, row, i);
			free (row);
		}
	} else if (key == KEY_UP) {
		int i;
		for (i = 0; i < GRID_SIZE; i++) {
			mergeRow(st, board[i], -1);
		}
	} else if (key == KEY_DOWN) {
		int i;
		for (i = 0; i < GRID_SIZE; i++) {
			mergeRow(st, board[i], 1);
		}
	}
}

void init(void) {
	initscr();
	start_color();
	raw();
	noecho();
	keypad(stdscr, TRUE);

	init_pair(0, COLOR_BLACK, COLOR_BLACK);
	init_pair(1, COLOR_BLACK, COLOR_BLUE);
	init_pair(2, COLOR_BLACK, COLOR_RED);
	init_pair(3, COLOR_BLACK, COLOR_YELLOW);
	init_pair(4, COLOR_BLACK, COLOR_GREEN);
	init_pair(5, COLOR_BLACK, COLOR_CYAN);
	init_pair(6, COLOR_BLACK, COLOR_MAGENTA);
	init_pair(7, COLOR_BLACK, COLOR_WHITE);
	init_pair(8, COLOR_WHITE, COLOR_BLACK);
	init_pair(9, COLOR_BLUE, COLOR_YELLOW);
	init_pair(10, COLOR_YELLOW, COLOR_GREEN);
	init_pair(11, COLOR_CYAN, COLOR_WHITE);

	srand(time(NULL));
}

int main(void) {

	init();

	int **board = malloc(GRID_SIZE * sizeof(int*));
	int i;
	for (i = 0; i < GRID_SIZE; i++) {
		board[i] = malloc(GRID_SIZE * sizeof(int));
	}
	addRandom(board, 2);
	drawBoard(board, TILE_SIZE, 2, 2);

	MoveState* st = (MoveState*) malloc(sizeof(MoveState));

//--------------------------------------------
//Main Loop

	int ch = getch();
	int lose = 0;
	while (ch != 'q' && st->win == 0) {
		actOnBoard(st, board, ch);
		if (st->movement) {
			lose = addRandom(board, 2);	
		} else {
			lose = addRandom(board, 0);
		}
		drawBoard(board, TILE_SIZE, 2, 2);
		mvprintw(TILE_SIZE * GRID_SIZE + 2, 0, "Movement: %d. Win: %d. Lose: %d", st->movement, st->win, lose);
		refresh();
		ch = getch();
	}

	endwin();			/* End curses mode		  */

	for (i = 0; i < GRID_SIZE; i++) {
		free (board[i]);
	}

	free (board);
	return 0;
}