#ifndef ALGEBRAIC_NOTATION
#define ALEGBRAIC_NOTATION

#include "board.h"
#include "move.h"

bool same_move_filter(void* i, void* pars);
bool same_file_filter(void* i, void* pars);
bool same_rank_filter(void* i, void* pars);
char* move_to_algebraic_notation(BOARD board, int index, MOVE move, PIECE promote_to);
char* castling_to_algebraic_notation(CASTLING_SIDE side);
void algebraic_notation_to_move(BOARD board, char* notation, COLOR color, int* index, MOVE* move, bool* castling, CASTLING_SIDE* side, PIECE* promote_to, bool* error, char** error_message);

#endif
