#include "algebraic_notation.h"

bool same_move_filter(void* i, void* pars) {
    int index = *((int*)i);
    void** params = pars;
    BOARD* board = (BOARD*)params[0];
    SQUARE* square = (SQUARE*)params[1];
    VECTOR poss_moves = construct_vector(sizeof(MOVE));
    possible_moves(*board, index, true, &poss_moves);
    int move_index = vector_index(&poss_moves, square, square_is_move);
    deconstruct_vector(poss_moves);
    return move_index != -1;
}

bool same_file_filter(void* i, void* pars) {
    int index = *((int*)i);
    void** params = pars;
    BOARD* board = (BOARD*)params[0];
    int jndex = *((int*)params[1]);
    return board->board[index].file == board->board[jndex].file;
}

bool same_rank_filter(void* i, void* pars) {
    int index = *((int*)i);
    void** params = pars;
    BOARD* board = (BOARD*)params[0];
    int jndex = *((int*)params[1]);
    return board->board[index].rank == board->board[jndex].rank;
}

bool is_file_filter(void* i, void* pars) {
    int index = *((int*)i);
    void** params = pars;
    BOARD* board = (BOARD*)params[0];
    COL file = *((int*)params[1]);
    return board->board[index].file == file;
}

bool is_rank_filter(void* i, void* pars) {
    int index = *((int*)i);
    void** params = pars;
    BOARD* board = (BOARD*)params[0];
    unsigned char rank = *((int*)params[1]);
    return board->board[index].rank == rank;
}

char* move_to_algebraic_notation(BOARD board, int index, MOVE move, PIECE promote_to) {
    char* notation = (char*)malloc(sizeof(char) * 10);
    int i = 0;
    COLOR color;
    PIECE piece;
    board_index_to_piece(board, index, &color, &piece);
    switch (piece) {
        case pawn:
            break;
        case rook:
            notation[i] = 'R';
            i++;
            break;
        case knight:
            notation[i] = 'N';
            i++;
            break;
        case bishop:
            notation[i] = 'B';
            i++;
            break;
        case queen:
            notation[i] = 'Q';
            i++;
            break;
        case king:
            notation[i] = 'K';
            i++;
            break;
    }
    if (piece == pawn && move.capture) {
        notation[i] = print_file(board.board[index].file);
        i++;
    } else {
        VECTOR same_pieces = construct_vector(sizeof(int));
        int start = color == white ? 0 : 16;
        int end = color == white ? 16 : 32;
        for (int j = start; j < end; j++) {
            if (j == index)
                continue;
            COLOR _;
            PIECE j_piece;
            board_index_to_piece(board, j, &_, &j_piece);
            if (piece == j_piece)
                vector_enqueue(&same_pieces, &j);
        }
        void* params[2] = {(void*)&board, (void*)&(move.square)};
        VECTOR same_move = vector_filter(&same_pieces, same_move_filter, params);
        if (vector_length(&same_move) > 0) {
            void* params[2] = {(void*)&board, (void*)&index};
            VECTOR same_move_and_file = vector_filter(&same_move, same_file_filter, params);
            if (vector_length(&same_move_and_file) > 0) {
                void* params[2] = {(void*)&board, (void*)&index};
                VECTOR same_move_and_rank = vector_filter(&same_move, same_rank_filter, params);
                if (vector_length(&same_move_and_rank) > 0) {
                    notation[i] = print_file(board.board[index].file);
                    i++;
                    notation[i] = print_rank(board.board[index].rank);
                    i++;
                } else {
                    notation[i] = print_rank(board.board[index].rank);
                    i++;
                }
                deconstruct_vector(same_move_and_rank);
            } else {
                notation[i] = print_file(board.board[index].file);
                i++;
            }
            deconstruct_vector(same_move_and_file);
        }
        deconstruct_vector(same_move);
    }
    if (move.capture) {
        notation[i] = 'x';
        i++;
    }
    notation[i] = print_file(move.square.file);
    i++;
    notation[i] = print_rank(move.square.rank);
    i++;
    if (promote_to != pawn) {
        notation[i] = '=';
        switch (promote_to) {
            case rook:
                notation[i] = 'R';
                i++;
                break;
            case knight:
                notation[i] = 'N';
                i++;
                break;
            case bishop:
                notation[i] = 'B';
                i++;
                break;
            case queen:
                notation[i] = 'Q';
                i++;
                break;
            case pawn:
                break;
            case king:
                break;
        }
    }
    BOARD b = clone_board(board);
    PIECE _;
    move_piece(&b, index, move, NULL, &_);
    if (promote_to != pawn)
        b.promotion[index] = promote_to;
    bool in_check = king_in_check(b, color == white ? black : white);
    bool checkmate = in_check && !can_move(b, color == white ? black : white);
    if (checkmate) {
        notation[i] = '#';
        i++;
    } else if (in_check) {
        notation[i] = '+';
        i++;
    }
    while (i < 9) {
        notation[i] = ' ';
        i++;
    }
    notation[i] = '\0';
    return notation;
}

char* castling_to_algebraic_notation(CASTLING_SIDE side) {
    return side == queenside ? "0-0-0    " : "0-0      ";
}

void algebraic_notation_to_move(BOARD board, char* notation, COLOR color, int* index, MOVE* move, bool* castling, CASTLING_SIDE* side, PIECE* promote_to, bool* error, char** error_message) {
    move->capture = false;
    move->check = false;
    move->en_passant = false;
    *error = false;
    if (strcmp(notation, "0-0-0") == 0) {
        bool castling[2];
        castling_possible_moves(board, color, castling);
        if (castling[queenside]) {
            *castling = true;
            *side = queenside;
            return;
        } else {
            *error = true;
            *error_message = "Can't castle queenside";
            return;
        }
    } else if (strcmp(notation, "0-0") == 0) {
        bool castling[2];
        castling_possible_moves(board, color, castling);
        if (castling[kingside]) {
            *castling = true;
            *side = kingside;
            return;
        } else {
            *error = true;
            *error_message = "Can't castle queenside";
            return;
        }
    } else {
        *castling = false;
    }
    int i = 0;
    PIECE piece;
    switch (notation[i]) {
        case 'R':
            piece = rook;
            i++;
            break;
        case 'N':
            piece = knight;
            i++;
            break;
        case 'B':
            piece = bishop;
            i++;
            break;
        case 'Q':
            piece = queen;
            i++;
            break;
        case 'K':
            piece = king;
            i++;
            break;
        default:
            piece = pawn;
            break;
    }
    unsigned char rank_disambiguation = none;
    COL file_disambiguation = read_file(notation[i]);
    if (file_disambiguation == none) {
        rank_disambiguation = read_rank(notation[i]);
        if (rank_disambiguation == 0) {
            if (notation[i] == 'x') {
                move->capture = true;
                i++;
            } else {
                *error = true;
                *error_message = "Malformed algebreic notation";
                return;
            }
            move->square.file = read_file(notation[i]);
            if (move->square.file == none) {
                *error = true;
                *error_message = "Must specify destination file";
                return;
            }
            i++;
            move->square.rank = read_rank(notation[i]);
            if (move->square.rank == none) {
                *error = true;
                *error_message = "Must specify destination rank";
                return;
            }
        } else {
            i++;
            if (notation[i] == 'x') {
                move->capture = true;
                i++;
            }
            move->square.file = read_file(notation[i]);
            if (move->square.file == none) {
                *error = true;
                *error_message = "Must specify destination file";
                return;
            }
            i++;
            move->square.rank = read_rank(notation[i]);
            if (move->square.rank == none) {
                *error = true;
                *error_message = "Must specify destination rank";
                return;
            }
        }
    } else {
        i++;
        if (notation[i] == 'x') {
            move->capture = true;
            i++;
        }
        move->square.file = read_file(notation[i]);
        if (move->square.file == none) {
            if (!move->capture) {
                move->square.file = file_disambiguation;
                file_disambiguation = none;
                move->square.rank = read_rank(notation[i]);
                if (move->square.rank == none) {
                    *error = true;
                    *error_message = "Must specify destination rank";
                    return;
                }
            } else {
                *error = true;
                *error_message = "Must specify destination file";
                return;
            }
        } else {
            i++;
            move->square.rank = read_rank(notation[i]);
            if (move->square.rank == none) {
                *error = true;
                *error_message = "Must specify destination rank";
                return;
            }
        }
    }
    i++;
    *promote_to = pawn;
    if (notation[i] == '=') {
        i++;
        switch (notation[i]) {
            case 'R':
                *promote_to = rook;
                i++;
                break;
            case 'N':
                *promote_to = knight;
                i++;
                break;
            case 'B':
                *promote_to = bishop;
                i++;
                break;
            case 'Q':
                *promote_to = queen;
                i++;
                break;
            default:
                *error = true;
                *error_message = "Must specify promotion piece";
                return;
        }
        i++;
    }
    bool check = false;
    bool checkmate = false;
    if (notation[i] == '+') {
        check = true;
        i++;
    } else if (notation[i] == '#') {
        checkmate = true;
        i++;
    }
    if (notation[i] != '\0') {
        *error = true;
        *error_message = "Malformed algebreic notation";
        return;
    }

    VECTOR possible_piece_index = piece_to_board_index(board, piece, color);
    if (file_disambiguation != none) {
        void* params[2] = {(void*)&board, (void*)&file_disambiguation};
        VECTOR possible_piece_is_file = vector_filter(&possible_piece_index, is_file_filter, params);
        deconstruct_vector(possible_piece_index);
        possible_piece_index = possible_piece_is_file;
    }
    if (rank_disambiguation != 0) {
        void* params[2] = {(void*)&board, (void*)&rank_disambiguation};
        VECTOR possible_piece_is_rank = vector_filter(&possible_piece_index, is_rank_filter, params);
        deconstruct_vector(possible_piece_index);
        possible_piece_index = possible_piece_is_rank;
    }
    if (vector_length(&possible_piece_index) == 0) {
        *error = true;
        *error_message = "No piece of that type in that file/rank";
        return;
    }
    void* params[2] = {(void*)&board, (void*)&(move->square)};
    VECTOR same_move = vector_filter(&possible_piece_index, same_move_filter, params);
    if (vector_length(&same_move) == 0) {
        *error = true;
        *error_message = "No piece of that type in that file/rank that can make that move";
        return;
    }
    if (vector_length(&same_move) > 1) {
        *error = true;
        *error_message = "Multiple pieces of that type in that file/rank can make that move";
        return;
    }
    vector_dequeue(&same_move, index);
    deconstruct_vector(same_move);
    deconstruct_vector(possible_piece_index);
    VECTOR poss_moves = construct_vector(sizeof(MOVE));
    possible_moves(board, *index, true, &poss_moves);
    int move_index = vector_index(&poss_moves, &move->square, square_is_move);
    MOVE m;
    vector_get(&poss_moves, move_index, &m);
    deconstruct_vector(poss_moves);
    if (move->capture != m.capture) {
        if (move->capture) {
            *error = true;
            *error_message = "Move is not a capture";
            return;
        } else {
            move->capture = m.capture;
        }
    }
    move->en_passant = m.en_passant;
    if (piece == pawn && move->square.rank == (color == white ? 8 : 1)) {
        if (*promote_to == pawn) {
            *error = true;
            *error_message = "Must specify promotion piece";
            return;
        }
    } else if (*promote_to != pawn) {
        *error = true;
        *error_message = "Can't promote piece";
        return;
    }
    BOARD b = clone_board(board);
    PIECE _;
    move_piece(&b, *index, *move, NULL, &_);
    if (*promote_to != pawn)
        b.promotion[*index] = *promote_to;
    bool true_in_check = king_in_check(b, color == white ? black : white);
    bool true_checkmate = true_in_check && !can_move(b, color == white ? black : white);
    if (checkmate && !true_checkmate) {
        *error = true;
        *error_message = "Move is not checkmate";
        return;
    }
    if (check && !true_in_check) {
        *error = true;
        *error_message = "Move is not check";
        return;
    }
}
