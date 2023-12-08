#ifndef BOARD_H
#define BOARD_H

#include <stdbool.h>
#include <string.h>

typedef enum COLOR {white, black} COLOR;
typedef enum PIECE {pawn, rook, knight, bishop, queen, king} PIECE;
typedef enum COL {none, a, b, c, d, e, f, g, h} COL;
typedef enum MOVEMENT_STATE {has_not_moved, has_just_moved_up_two, has_moved} MOVEMENT_STATE;

typedef struct SQUARE {
    COL file;
    unsigned char rank;
} SQUARE;

typedef struct BOARD {
    SQUARE board[32];
    MOVEMENT_STATE movement_state[32];
    PIECE promotion[32];
} BOARD;

SQUARE create_square(COL file, unsigned char rank);
bool squares_equal(SQUARE a, SQUARE b);
BOARD create_board();
BOARD clone_board(BOARD board);
int piece_to_board_index(COLOR color, bool pawn, COL starting_file);
void board_index_to_piece(BOARD board, int index, COLOR* color, PIECE* piece);
void square_to_pixel(SQUARE square, int tile_w, int tile_h, int* x, int* y);
void square_to_piece(BOARD board, SQUARE square, bool* has_piece, COLOR* color, PIECE* piece);
int square_to_board_index(BOARD board, SQUARE square);
bool square_has_piece(BOARD board, SQUARE square);

#endif
