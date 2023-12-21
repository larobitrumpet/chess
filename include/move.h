#ifndef MOVE_H
#define MOVE_H

#ifdef DEBUG
#include <stdio.h>
#endif
#include <stdbool.h>
#include "allegro5/allegro5.h"
#include "allegro5/allegro_native_dialog.h"
#include "vector.h"
#include "board.h"

typedef struct MOVE {
    SQUARE square;
    bool capture;
    bool check;
    bool en_passant;
} MOVE;

typedef enum CASTLING_SIDE {queenside, kingside} CASTLING_SIDE;

typedef struct CASTLING {
    CASTLING_SIDE side;
    COLOR color;
} CASTLING;

MOVE create_move(SQUARE square, bool capture, bool check, bool en_passant);
bool square_is_move(void* square, void* move);
#ifdef DEBUG
bool print_move(void* move, void* _);
#endif
void move_piece(BOARD* board, int index, MOVE move, ALLEGRO_DISPLAY* display, PIECE* promoted_to);
void castle(BOARD* board, COLOR color, CASTLING_SIDE side);
bool check_for_checks(void* move, void* is_check);
bool king_in_check(BOARD board, COLOR king_color);
void possible_moves_square_enqueue(BOARD board, int index, MOVE move, COLOR piece_color, bool should_check_for_checks, VECTOR* vector);
bool possible_moves_square(BOARD board, int index, SQUARE s, COLOR piece_color, bool should_check_for_checks, VECTOR* vector);
void pawn_possible_moves(BOARD board, int index, COLOR pawn_color, bool should_check_for_checks, VECTOR* vector);
void rook_possible_moves(BOARD board, int index, COLOR rook_color, bool should_check_for_checks, VECTOR* vector);
void knight_possible_moves(BOARD board, int index, COLOR knight_color, bool should_check_for_checks, VECTOR* vector);
void bishop_possible_moves(BOARD board, int index, COLOR bishop_color, bool should_check_for_checks, VECTOR* vector);
void queen_possible_moves(BOARD board, int index, COLOR queen_color, bool should_check_for_checks, VECTOR* vector);
void king_possible_moves(BOARD board, int index, COLOR king_color, bool should_check_for_checks, VECTOR* vector);
bool castling_queenside(BOARD board, COLOR color);
bool castling_kingside(BOARD board, COLOR color);
void castling_possible_moves(BOARD board, COLOR color, bool castling[2]);
void possible_moves(BOARD board, int index, bool should_check_for_checks, VECTOR* vector);
bool can_move(BOARD board, COLOR color);

#endif
