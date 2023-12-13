#ifndef RENDER_H
#define RENDER_H

#include "allegro5/allegro5.h"
#include "allegro5/allegro_image.h"
#include "board.h"
#include "move.h"
#include "vector.h"

#define TILE_W 16
#define TILE_H 16

typedef struct SPRITES {
    ALLEGRO_BITMAP* _sheet;
    ALLEGRO_BITMAP* white_pieces[6];
    ALLEGRO_BITMAP* black_pieces[6];
    ALLEGRO_BITMAP* square[2];
    ALLEGRO_BITMAP* possible_move[2];
    ALLEGRO_BITMAP* possible_capture[2];
    ALLEGRO_BITMAP* in_check[2];
    ALLEGRO_BITMAP* checkmate[2];
} SPRITES;

ALLEGRO_BITMAP* sprite_grab(int x, int y, int w, int h);
void sprites_init();
void sprites_deinit();
void render_board();
//int LERP(POINT point1, POINT point2, int t);
void render_piece(COLOR color, PIECE piece, int x, int y);
//void render_piece(PIECE piece, COLOR color, POINT point1, POINT point2, int t, bool interpolate);
bool render_possible_moves(void* m, void* c);
bool render_castling(void* castling, void* c);
void render(BOARD board, VECTOR* poss_moves, VECTOR* castling_moves, COLOR turn, bool in_check[2], bool checkmate[2]);

#endif
