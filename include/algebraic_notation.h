#ifndef ALGEBRAIC_NOTATION
#define ALEGBRAIC_NOTATION

#include "board.h"
#include "move.h"

bool same_move_filter(void* i, void* pars);
bool same_file_filter(void* i, void* pars);
bool same_rank_filter(void* i, void* pars);
char* move_to_algebraic_notation(BOARD board, int index, MOVE move, PIECE promote_to);
char* castling_to_algebraic_notation(CASTLING_SIDE side);

#endif
