#include "move.h"

MOVE create_move(SQUARE square, bool capture, bool check, bool en_passant) {
    MOVE move;
    move.square = square;
    move.capture = capture;
    move.check = check;
    move.en_passant = en_passant;
    return move;
}

bool square_is_move(void* square, void* move) {
    SQUARE* s = (SQUARE*)square;
    MOVE* m = (MOVE*)move;
    return squares_equal(*s, m->square);
}

#ifdef DEBUG
bool print_move(void* move, void* _) {
    (void) _;
    MOVE* m = (MOVE*)move;
    printf("%d %d %s %s\n", m->square.file, m->square.rank, m->capture ? "true " : "false", m->check ? "true " : "false");
    return false;
}
#endif

void move_piece(BOARD* board, int index, MOVE move, ALLEGRO_DISPLAY* display, PIECE* promoted_to) {
    *promoted_to = pawn;
    SQUARE old_square = board->board[index];
    if (move.capture) {
        int i;
        if (move.en_passant) {
            SQUARE s = move.square;
            COLOR color;
            PIECE _piece;
            board_index_to_piece(*board, index, &color, &_piece);
            s.rank -= color == white ? 1 : -1;
            i = square_to_board_index(*board, s);
        } else {
            i = square_to_board_index(*board, move.square);
        }
        board->board[i].file = none;
        board->board[i].rank = 0;
    }
    board->board[index] = move.square;
    for (int i = 0; i < 32; i++) {
        if (board->movement_state[i] == has_just_moved_up_two) {
            board->movement_state[i] = has_moved;
        }
    }
    COLOR color;
    PIECE piece;
    board_index_to_piece(*board, index, &color, &piece);
    if (piece == pawn && old_square.rank == (color == white ? 2 : 7) && move.square.rank == (color == white ? 4 : 5))
        board->movement_state[index] = has_just_moved_up_two;
    else
        board->movement_state[index] = has_moved;
    
    if (piece == pawn && move.square.rank == (color == white ? 8 : 1) && display != NULL) {
        board->promotion[index] = al_show_native_message_box(
            display,
            "Pawn Promotion",
            "Pawn Promotion",
            "Promote your pawn:",
            "Rook|Knight|Bishop|Queen",
            ALLEGRO_MESSAGEBOX_QUESTION
        );
        *promoted_to = board->promotion[index];
    }
}

void castle(BOARD* board, COLOR color, CASTLING_SIDE side) {
    SQUARE square;
    square.file = side == queenside ? c : g;
    square.rank = color == white ? 1 : 8;
    board->board[color == white ? 12 : 28] = square;
    square.file = side == queenside ? d : f;
    board->board[color == white ? (side == queenside ? 8 : 15) : (side == queenside ? 24 : 31)] = square;

    for (int i = 0; i < 32; i++) {
        if (board->movement_state[i] == has_just_moved_up_two) {
            board->movement_state[i] = has_moved;
        }
    }
    board->movement_state[color == white ? 12 : 28] = has_moved;
    board->movement_state[color == white ? (side == queenside ? 8 : 15) : (side == queenside ? 24 : 31)] = has_moved;
}

bool check_for_checks(void* move, void* is_check) {
    MOVE* m = (MOVE*)move;
    bool* c = (bool*)is_check;
    if (m->check == true) {
        *c = true;
        return true;
    }
    return false;
}

bool king_in_check(BOARD board, COLOR king_color) {
    VECTOR opponent_possible_moves = construct_vector(sizeof(MOVE));
    int start = king_color == white ? 16 : 0;
    int end = start + 16;
    for (int i = start; i < end; i++)
        possible_moves(board, i, false, &opponent_possible_moves);

    bool is_in_check = false;
    vector_iterate(&opponent_possible_moves, check_for_checks, &is_in_check);
    
    deconstruct_vector(opponent_possible_moves);
    return is_in_check;
}

void possible_moves_square_enqueue(BOARD board, int index, MOVE move, COLOR piece_color, bool should_check_for_checks, VECTOR* vector) {
    if (should_check_for_checks) {
        BOARD new_board = clone_board(board);
        PIECE _;
        move_piece(&new_board, index, move, NULL, &_);
        if (!king_in_check(new_board, piece_color))
            vector_enqueue(vector, &move);
    } else {
        vector_enqueue(vector, &move);
    }
}

bool possible_moves_square(BOARD board, int index, SQUARE s, COLOR piece_color, bool should_check_for_checks, VECTOR* vector) {
    bool has_piece;
    COLOR color;
    PIECE piece;
    MOVE move;

    if (s.rank < 1 || s.rank > 8 || s.file < a || s.file > h)
        return false;
    square_to_piece(board, s, &has_piece, &color, &piece);
    if (has_piece) {
        if (piece_color != color) {
            move = create_move(s, true, piece == king, false);
            possible_moves_square_enqueue(board, index, move, piece_color, should_check_for_checks, vector);
        }
        return false;
    }
    move = create_move(s, false, false, false);
    possible_moves_square_enqueue(board, index, move, piece_color, should_check_for_checks, vector);
    return true;
}

void pawn_possible_moves(BOARD board, int index, COLOR pawn_color, bool should_check_for_checks, VECTOR* vector) {
    bool has_piece;
    COLOR color;
    PIECE piece;
    SQUARE s = board.board[index];
    MOVE move;
    if (s.rank < 8 && s.rank > 1) {
        // move up one
        s.rank += pawn_color == white ? 1 : -1;
        if (!square_has_piece(board, s)) {
            move = create_move(s, false, false, false);
            possible_moves_square_enqueue(board, index, move, pawn_color, should_check_for_checks, vector);
            // move up two
            if (board.movement_state[index] == has_not_moved) {
                s.rank += pawn_color == white ? 1 : -1;
                if (!square_has_piece(board, s)) {
                    move = create_move(s, false, false, false);
                    possible_moves_square_enqueue(board, index, move, pawn_color, should_check_for_checks, vector);
                }
                s.rank -= pawn_color == white ? 1 : -1;
            }
        }
        // capture
        if (s.file > a) {
            s.file -= 1;
            square_to_piece(board, s, &has_piece, &color, &piece);
            if (has_piece && pawn_color != color) {
                move = create_move(s, true, piece == king, false);
                possible_moves_square_enqueue(board, index, move, pawn_color, should_check_for_checks, vector);
            }
            s.file += 1;
        }
        if (s.file < h) {
            s.file += 1;
            square_to_piece(board, s, &has_piece, &color, &piece);
            if (has_piece && pawn_color != color) {
                move = create_move(s, true, piece == king, false);
                possible_moves_square_enqueue(board, index, move, pawn_color, should_check_for_checks, vector);
            }
        }
    }
    // en passant
    s = board.board[index];
    if (s.rank == (pawn_color == white ? 5 : 4)) {
        s.file -= 1;
        if (s.file >= a) {
            square_to_piece(board, s, &has_piece, &color, &piece);
            int i = square_to_board_index(board, s);
            if (has_piece && piece == pawn && board.promotion[i] == pawn && pawn_color != color && board.movement_state[i] == has_just_moved_up_two) {
                s.rank += pawn_color == white ? 1 : -1;
                move = create_move(s, true, false, true);
                possible_moves_square_enqueue(board, index, move, pawn_color, should_check_for_checks, vector);
                s.rank -= pawn_color == white ? 1 : -1;
            }
        }
        s.file += 2;
        if (s.file <= h) {
            square_to_piece(board, s, &has_piece, &color, &piece);
            int i = square_to_board_index(board, s);
            if (has_piece && piece == pawn && board.promotion[i] == pawn && pawn_color != color && board.movement_state[i] == has_just_moved_up_two) {
                s.rank += pawn_color == white ? 1 : -1;
                move = create_move(s, true, false, true);
                possible_moves_square_enqueue(board, index, move, pawn_color, should_check_for_checks, vector);
                s.rank -= pawn_color == white ? 1 : -1;
            }
        }
    }
}

void rook_possible_moves(BOARD board, int index, COLOR rook_color, bool should_check_for_checks, VECTOR* vector) {
    SQUARE square = board.board[index];

    SQUARE s = square;
    s.rank += 1;
    while (possible_moves_square(board, index, s, rook_color, should_check_for_checks, vector))
        s.rank += 1;

    s = square;
    s.rank -= 1;
    while (possible_moves_square(board, index, s, rook_color, should_check_for_checks, vector))
        s.rank -= 1;

    s = square;
    s.file += 1;
    while (possible_moves_square(board, index, s, rook_color, should_check_for_checks, vector))
        s.file += 1;

    s = square;
    s.file -= 1;
    while (possible_moves_square(board, index, s, rook_color, should_check_for_checks, vector))
        s.file -= 1;
}

void knight_possible_moves(BOARD board, int index, COLOR knight_color, bool should_check_for_checks, VECTOR* vector) {
    SQUARE s = board.board[index];

    s.rank += 2;
    s.file -= 1;
    possible_moves_square(board, index, s, knight_color, should_check_for_checks, vector);
    s.file += 2;
    possible_moves_square(board, index, s, knight_color, should_check_for_checks, vector);
    s.rank -= 1;
    s.file += 1;
    possible_moves_square(board, index, s, knight_color, should_check_for_checks, vector);
    s.rank -= 2;
    possible_moves_square(board, index, s, knight_color, should_check_for_checks, vector);
    s.rank -= 1;
    s.file -= 1;
    possible_moves_square(board, index, s, knight_color, should_check_for_checks, vector);
    s.file -= 2;
    possible_moves_square(board, index, s, knight_color, should_check_for_checks, vector);
    s.rank += 1;
    s.file -= 1;
    possible_moves_square(board, index, s, knight_color, should_check_for_checks, vector);
    s.rank += 2;
    possible_moves_square(board, index, s, knight_color, should_check_for_checks, vector);
}

void bishop_possible_moves(BOARD board, int index, COLOR bishop_color, bool should_check_for_checks, VECTOR* vector) {
    SQUARE square = board.board[index];

    SQUARE s = square;
    s.rank += 1;
    s.file += 1;
    while (possible_moves_square(board, index, s, bishop_color, should_check_for_checks, vector)) {
        s.rank += 1;
        s.file += 1;
    }

    s = square;
    s.rank += 1;
    s.file -= 1;
    while (possible_moves_square(board, index, s, bishop_color, should_check_for_checks, vector)) {
        s.rank += 1;
        s.file -= 1;
    }

    s = square;
    s.rank -= 1;
    s.file += 1;
    while (possible_moves_square(board, index, s, bishop_color, should_check_for_checks, vector)) {
        s.rank -= 1;
        s.file += 1;
    }

    s = square;
    s.rank -= 1;
    s.file -= 1;
    while (possible_moves_square(board, index, s, bishop_color, should_check_for_checks, vector)) {
        s.rank -= 1;
        s.file -= 1;
    }
}

void queen_possible_moves(BOARD board, int index, COLOR queen_color, bool should_check_for_checks, VECTOR* vector) {
    rook_possible_moves(board, index, queen_color, should_check_for_checks, vector);
    bishop_possible_moves(board, index, queen_color, should_check_for_checks, vector);
}

void king_possible_moves(BOARD board, int index, COLOR king_color, bool should_check_for_checks, VECTOR* vector) {
    SQUARE s = board.board[index];
    s.rank += 1;
    possible_moves_square(board, index, s, king_color, should_check_for_checks, vector);
    s.file += 1;
    possible_moves_square(board, index, s, king_color, should_check_for_checks, vector);
    s.rank -= 1;
    possible_moves_square(board, index, s, king_color, should_check_for_checks, vector);
    s.rank -= 1;
    possible_moves_square(board, index, s, king_color, should_check_for_checks, vector);
    s.file -= 1;
    possible_moves_square(board, index, s, king_color, should_check_for_checks, vector);
    s.file -= 1;
    possible_moves_square(board, index, s, king_color, should_check_for_checks, vector);
    s.rank += 1;
    possible_moves_square(board, index, s, king_color, should_check_for_checks, vector);
    s.rank += 1;
    possible_moves_square(board, index, s, king_color, should_check_for_checks, vector);
}

bool castling_queenside(BOARD board, COLOR color) {
    if (board.movement_state[color == white ? 12 : 28] != has_not_moved)
        return false;
    if (board.movement_state[color == white ? 8 : 24] != has_not_moved)
        return false;
    SQUARE square;
    PIECE _;
    square.file = b;
    square.rank = color == white ? 1 : 8;
    if (square_has_piece(board, square))
        return false;
    square.file = c;
    if (square_has_piece(board, square))
        return false;
    square.file = d;
    if (square_has_piece(board, square))
        return false;
    MOVE move = create_move(square, false, false, false);
    BOARD b = clone_board(board);
    move_piece(&b, color == white ? 12 : 28, move, NULL, &_);
    if (king_in_check(b, color))
        return false;
    square.file = c;
    move = create_move(square, false, false, false);
    b = clone_board(board);
    move_piece(&b, color == white ? 12 : 28, move, NULL, &_);
    return !king_in_check(b, color);
}

bool castling_kingside(BOARD board, COLOR color) {
    if (board.movement_state[color == white ? 12 : 28] != has_not_moved)
        return false;
    if (board.movement_state[color == white ? 15 : 31] != has_not_moved)
        return false;
    SQUARE square;
    PIECE _;
    square.file = g;
    square.rank = color == white ? 1 : 8;
    if (square_has_piece(board, square))
        return false;
    square.file = f;
    if (square_has_piece(board, square))
        return false;
    MOVE move = create_move(square, false, false, false);
    BOARD b = clone_board(board);
    move_piece(&b, color == white ? 12 : 28, move, NULL, &_);
    if (king_in_check(b, color))
        return false;
    square.file = g;
    move = create_move(square, false, false, false);
    b = clone_board(board);
    move_piece(&b, color == white ? 12 : 28, move, NULL, &_);
    return !king_in_check(b, color);
}

void castling_possible_moves(BOARD board, COLOR color, bool castling[2]) {
    castling[queenside] = castling_queenside(board, color);
    castling[kingside] = castling_kingside(board, color);
}

void possible_moves(BOARD board, int index, bool should_check_for_checks, VECTOR* vector) {
    COLOR color;
    PIECE piece;
    if (board.board[index].file == none || board.board[index].rank == 0)
        return;
    board_index_to_piece(board, index, &color, &piece);
    switch (piece) {
        case pawn:
            pawn_possible_moves(board, index, color, should_check_for_checks, vector);
            break;
        case rook:
            rook_possible_moves(board, index, color, should_check_for_checks, vector);
            break;
        case knight:
            knight_possible_moves(board, index, color, should_check_for_checks, vector);
            break;
        case bishop:
            bishop_possible_moves(board, index, color, should_check_for_checks, vector);
            break;
        case queen:
            queen_possible_moves(board, index, color, should_check_for_checks, vector);
            break;
        case king:
            king_possible_moves(board, index, color, should_check_for_checks, vector);
            break;
    }
}

bool can_move(BOARD board, COLOR color) {
    VECTOR poss_moves = construct_vector(sizeof(MOVE));
    
    bool can = false;
    int start = color == white ? 0 : 16;
    int end = color == white ? 16 : 32;
    for (int i = start; i < end; i++) {
        possible_moves(board, i, true, &poss_moves);
        if (vector_length(&poss_moves) != 0) {
            can = true;
            break;
        }
    }
    
    deconstruct_vector(poss_moves);
    return can;
}
