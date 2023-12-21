#include "algebraic_notation.h"

bool same_move_filter(void* i, void* pars) {
    int index = *((int*)i);
    void** params = (void*)pars;
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
    void** params = (void*)pars;
    BOARD* board = (BOARD*)params[0];
    int jndex = *((int*)params[1]);
    return board->board[index].file == board->board[jndex].file;
}

bool same_rank_filter(void* i, void* pars) {
    int index = *((int*)i);
    void** params = (void*)pars;
    BOARD* board = (BOARD*)params[0];
    int jndex = *((int*)params[1]);
    return board->board[index].rank == board->board[jndex].rank;
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
    bool in_check = king_in_check(b, color);
    bool checkmate = in_check && !can_move(b, color);
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
