#include "render.h"

SPRITES sprites;

ALLEGRO_BITMAP* sprite_grab(int x, int y, int w, int h) {
    ALLEGRO_BITMAP* sprite = al_create_sub_bitmap(sprites._sheet, x, y, w, h);
    //must_init(sprite, "sprite grab");
    return sprite;
}

void sprites_init() {
    sprites._sheet = al_load_bitmap("chess.png");
    if (sprites._sheet == NULL)
        exit(1);
    //must_init(sprites._sheet, "sprite sheet");
    for (int i = 0; i < 6; i++) {
        sprites.white_pieces[i] = sprite_grab(i * TILE_W, 0, TILE_W, TILE_H);
        sprites.black_pieces[i] = sprite_grab(i * TILE_W, TILE_H, TILE_W, TILE_H);
    }
    sprites.square[white] = sprite_grab(6 * TILE_W, 0, TILE_W, TILE_H);
    sprites.square[black] = sprite_grab(6 * TILE_W, TILE_H, TILE_W, TILE_H);
    sprites.possible_move[white] = sprite_grab(7 * TILE_W, 0, TILE_W, TILE_H);
    sprites.possible_move[black] = sprite_grab(7 * TILE_W, TILE_H, TILE_W, TILE_H);
    sprites.possible_capture[white] = sprite_grab(8 * TILE_W, 0, TILE_W, TILE_H);
    sprites.possible_capture[black] = sprite_grab(8 * TILE_W, TILE_H, TILE_W, TILE_H);
    sprites.in_check[white] = sprite_grab(9 * TILE_W, 0, TILE_W, TILE_H);
    sprites.in_check[black] = sprite_grab(9 * TILE_W, TILE_H, TILE_W, TILE_H);
    sprites.checkmate[white] = sprite_grab(10 * TILE_W, 0, TILE_W, TILE_H);
    sprites.checkmate[black] = sprite_grab(10 * TILE_W, TILE_H, TILE_W, TILE_H);
}

void sprites_deinit() {
}

void render_board() {
    for (int file = 0; file < 8; file++) {
        for (int rank = 0; rank < 8; rank++) {
            al_draw_bitmap(sprites.square[(file + rank) % 2], file * TILE_W, rank * TILE_H, 0);
        }
    }
}

void render_piece(COLOR color, PIECE piece, int x, int y) {
    if (color) {
        al_draw_bitmap(sprites.black_pieces[piece], x, y, 0);
    } else {
        al_draw_bitmap(sprites.white_pieces[piece], x, y, 0);
    }
}

bool render_possible_moves(void* m, void* c) {
    MOVE* move = (MOVE*)m;
    COLOR* color = (COLOR*)c;
    int x;
    int y;
    square_to_pixel(move->square, TILE_W, TILE_H, &x, &y);
    if (move->capture) {
        al_draw_bitmap(sprites.possible_capture[*color], x, y, 0);
    } else {
        al_draw_bitmap(sprites.possible_move[*color], x, y, 0);
    }
    return false;
}

bool render_castling(void* castling, void* c) {
    CASTLING* castle = (CASTLING*)castling;
    COLOR* color = (COLOR*)c;
    int x;
    int y;
    SQUARE s;
    s.file = castle->side == queenside ? a : h;
    s.rank = castle->color == white ? 1 : 8;
    square_to_pixel(s, TILE_W, TILE_H, &x, &y);
    al_draw_bitmap(sprites.possible_move[*color], x, y, 0);
    return false;
}

void render(BOARD board, VECTOR* poss_moves, VECTOR* castling_moves, COLOR turn, bool in_check[2], bool checkmate[2]) {
    render_board();
    int x;
    int y;
    for (int i = 0; i < 32; i++) {
        COLOR color;
        PIECE piece;
        board_index_to_piece(board, i, &color, &piece);
        if (board.board[i].rank != 0 && board.board[i].file != none) {
            if (i == 12) {
                if (checkmate[white]) {
                    square_to_pixel(board.board[12], TILE_W, TILE_H, &x, &y);
                    al_draw_bitmap(sprites.checkmate[white], x, y, 0);
                } else if (in_check[white]) {
                    square_to_pixel(board.board[i], TILE_W, TILE_H, &x, &y);
                    render_piece(color, piece, x, y);
                    square_to_pixel(board.board[12], TILE_W, TILE_H, &x, &y);
                    al_draw_bitmap(sprites.in_check[white], x, y, 0);
                } else {
                    square_to_pixel(board.board[i], TILE_W, TILE_H, &x, &y);
                    render_piece(color, piece, x, y);
                }
            } else if (i == 28) {
                if (checkmate[black]) {
                    square_to_pixel(board.board[28], TILE_W, TILE_H, &x, &y);
                    al_draw_bitmap(sprites.checkmate[black], x, y, 0);
                } else if (in_check[black]) {
                    square_to_pixel(board.board[i], TILE_W, TILE_H, &x, &y);
                    render_piece(color, piece, x, y);
                    square_to_pixel(board.board[28], TILE_W, TILE_H, &x, &y);
                    al_draw_bitmap(sprites.in_check[black], x, y, 0);
                } else {
                    square_to_pixel(board.board[i], TILE_W, TILE_H, &x, &y);
                    render_piece(color, piece, x, y);
                }
            } else {
                square_to_pixel(board.board[i], TILE_W, TILE_H, &x, &y);
                render_piece(color, piece, x, y);
            }
        }
    }
    vector_iterate(poss_moves, render_possible_moves, &turn);
    vector_iterate(castling_moves, render_castling, &turn);
}
