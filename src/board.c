#include "board.h"

char print_file(COL file) {
    switch (file) {
        case none:
            return ' ';
        case a:
            return 'a';
        case b:
            return 'b';
        case c:
            return 'c';
        case d:
            return 'd';
        case e:
            return 'e';
        case f:
            return 'f';
        case g:
            return 'g';
        case h:
            return 'h';
    }
    return ' ';
}

char print_rank(unsigned char rank) {
    switch (rank) {
        case 1:
            return '1';
        case 2:
            return '2';
        case 3:
            return '3';
        case 4:
            return '4';
        case 5:
            return '5';
        case 6:
            return '6';
        case 7:
            return '7';
        case 8:
            return '8';
    }
    return ' ';
}

COL read_file(char file) {
    switch (file) {
        case 'a':
            return a;
        case 'b':
            return b;
        case 'c':
            return c;
        case 'd':
            return d;
        case 'e':
            return e;
        case 'f':
            return f;
        case 'g':
            return g;
        case 'h':
            return h;
        default:
            return none;
    }
}

unsigned char read_rank(char rank) {
    switch (rank) {
        case '1':
            return 1;
        case '2':
            return 2;
        case '3':
            return 3;
        case '4':
            return 4;
        case '5':
            return 5;
        case '6':
            return 6;
        case '7':
            return 7;
        case '8':
            return 8;
        default:
            return 0;
    }
}

SQUARE create_square(COL file, unsigned char rank) {
    SQUARE square;
    square.file = file;
    square.rank = rank;
    return square;
}

bool squares_equal(SQUARE a, SQUARE b) {
    return a.file == b.file && a.rank == b.rank;
}

BOARD create_board() {
    BOARD board;
    board.board[0] = create_square(a, 2);
    board.board[1] = create_square(b, 2);
    board.board[2] = create_square(c, 2);
    board.board[3] = create_square(d, 2);
    board.board[4] = create_square(e, 2);
    board.board[5] = create_square(f, 2);
    board.board[6] = create_square(g, 2);
    board.board[7] = create_square(h, 2);
    board.board[8] = create_square(a, 1);
    board.board[9] = create_square(b, 1);
    board.board[10] = create_square(c, 1);
    board.board[11] = create_square(d, 1);
    board.board[12] = create_square(e, 1);
    board.board[13] = create_square(f, 1);
    board.board[14] = create_square(g, 1);
    board.board[15] = create_square(h, 1);
    board.board[16] = create_square(a, 7);
    board.board[17] = create_square(b, 7);
    board.board[18] = create_square(c, 7);
    board.board[19] = create_square(d, 7);
    board.board[20] = create_square(e, 7);
    board.board[21] = create_square(f, 7);
    board.board[22] = create_square(g, 7);
    board.board[23] = create_square(h, 7);
    board.board[24] = create_square(a, 8);
    board.board[25] = create_square(b, 8);
    board.board[26] = create_square(c, 8);
    board.board[27] = create_square(d, 8);
    board.board[28] = create_square(e, 8);
    board.board[29] = create_square(f, 8);
    board.board[30] = create_square(g, 8);
    board.board[31] = create_square(h, 8);
    for (int i = 0; i < 32; i++) {
        board.movement_state[i] = has_not_moved;
        board.promotion[i] = pawn;
    }
    return board;
}

BOARD clone_board(BOARD board) {
    BOARD new_board;
    memcpy(new_board.board, board.board, sizeof(board.board));
    memcpy(new_board.movement_state, board.movement_state, sizeof(board.movement_state));
    memcpy(new_board.promotion, board.promotion, sizeof(board.promotion));
    return new_board;
}

VECTOR piece_to_board_index(BOARD board, PIECE piece, COLOR color) {
    VECTOR pieces = construct_vector(sizeof(int));
    int i;
    if (color == white) {
        switch (piece) {
            case pawn:
                for (i = 0; i < 8; i++) {
                    if (board.promotion[i] == pawn)
                        vector_enqueue(&pieces, &i);
                }
                break;
            case rook:
                i = 8;
                vector_enqueue(&pieces, &i);
                i = 15;
                vector_enqueue(&pieces, &i);
                break;
            case knight:
                i = 9;
                vector_enqueue(&pieces, &i);
                i = 14;
                vector_enqueue(&pieces, &i);
                break;
            case bishop:
                i = 10;
                vector_enqueue(&pieces, &i);
                i = 13;
                vector_enqueue(&pieces, &i);
                break;
            case queen:
                i = 11;
                vector_enqueue(&pieces, &i);
                break;
            case king:
                i = 12;
                vector_enqueue(&pieces, &i);
                break;
        }
        if (piece != pawn) {
            for (i = 0; i < 8; i++) {
                if (board.promotion[i] == piece)
                    vector_enqueue(&pieces, &i);
            }
        }
    } else {
        switch (piece) {
            case pawn:
                for (i = 16; i < 24; i++)
                    if (board.promotion[i] == pawn)
                        vector_enqueue(&pieces, &i);
                break;
            case rook:
                i = 24;
                vector_enqueue(&pieces, &i);
                i = 31;
                vector_enqueue(&pieces, &i);
                break;
            case knight:
                i = 25;
                vector_enqueue(&pieces, &i);
                i = 30;
                vector_enqueue(&pieces, &i);
                break;
            case bishop:
                i = 26;
                vector_enqueue(&pieces, &i);
                i = 29;
                vector_enqueue(&pieces, &i);
                break;
            case queen:
                i = 27;
                vector_enqueue(&pieces, &i);
                break;
            case king:
                i = 28;
                vector_enqueue(&pieces, &i);
                break;
        }
        if (piece != pawn) {
            for (i = 16; i < 24; i++) {
                if (board.promotion[i] == piece)
                    vector_enqueue(&pieces, &i);
            }
        }
    }
    return pieces;
}

void board_index_to_piece(BOARD board, int index, COLOR* color, PIECE* piece) {
    *color = index / 16;
    if (index / 8 % 2) {
        switch (index % 8) {
            case 0:
                *piece = rook;
                break;
            case 1:
                *piece = knight;
                break;
            case 2:
                *piece = bishop;
                break;
            case 3:
                *piece = queen;
                break;
            case 4:
                *piece = king;
                break;
            case 5:
                *piece = bishop;
                break;
            case 6:
                *piece = knight;
                break;
            case 7:
                *piece = rook;
                break;
        }
    } else {
        *piece = board.promotion[index];
    }
}

void square_to_pixel(SQUARE square, int tile_w, int tile_h, int* x, int* y) {
    *x = (square.file - 1) * tile_w;
    *y = (8 - square.rank) * tile_h;
}

void square_to_piece(BOARD board, SQUARE square, bool* has_piece, COLOR* color, PIECE* piece) {
    for (int i = 0; i < 32; i++) {
        if (squares_equal(board.board[i], square)) {
            board_index_to_piece(board, i, color, piece);
            *has_piece = true;
            return;
        }
    }
    *has_piece = false;
}

int square_to_board_index(BOARD board, SQUARE square) {
    for (int i = 0; i < 32; i++) {
        if (squares_equal(board.board[i], square)) {
            return i;
        }
    }
    return -1;
}

bool square_has_piece(BOARD board, SQUARE square) {
    bool has_piece;
    COLOR color;
    PIECE piece;
    square_to_piece(board, square, &has_piece, &color, &piece);
    return has_piece;
}
