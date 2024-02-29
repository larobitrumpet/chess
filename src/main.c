#include <stdio.h>
#include <stdlib.h>
#include "allegro5/allegro5.h"
#include "allegro5/allegro_image.h"
#include "allegro5/allegro_native_dialog.h"
#include "board.h"
#include "render.h"
#include "move.h"
#include "algebraic_notation.h"
#include "vector.h"

void must_init(bool test, const char *description) {
    if (test) return;

    printf("Couldn't initialize %s\n", description);
    exit(1);
}

#define BUFFER_W 128
#define BUFFER_H 128

float DISP_SCALE_W;
float DISP_SCALE_H;
float DISP_W;
float DISP_H;
float DISP_W_OFFSET;
float DISP_H_OFFSET;

ALLEGRO_DISPLAY *disp;
ALLEGRO_BITMAP *buffer;

void disp_init() {
    al_set_new_display_flags(ALLEGRO_FULLSCREEN_WINDOW + ALLEGRO_RESIZABLE);
    al_set_new_display_option(ALLEGRO_SAMPLE_BUFFERS, 1, ALLEGRO_SUGGEST);
    al_set_new_display_option(ALLEGRO_SAMPLES, 8, ALLEGRO_SUGGEST);
    al_set_new_display_option(ALLEGRO_DEPTH_SIZE, 16, ALLEGRO_SUGGEST);

    disp = al_create_display(BUFFER_W, BUFFER_H);
    must_init(disp ,"display");

    buffer = al_create_bitmap(BUFFER_W, BUFFER_H);
    must_init(buffer, "bitmap buffer");
}

void disp_deinit() {
    al_destroy_bitmap(buffer);
    al_destroy_display(disp);
}

void disp_pre_draw() {
    al_set_target_bitmap(buffer);
}

void disp_post_draw() {
    float BUFFER_ASPECT = (float)BUFFER_W / (float)BUFFER_H;
    float D_W = (float)al_get_display_width(disp);
    float D_H = (float)al_get_display_height(disp);
    float DISP_ASPECT = D_W / D_H;

    DISP_SCALE_W = D_W / BUFFER_W;
    DISP_SCALE_H = D_H / BUFFER_H;
    
    if (DISP_ASPECT < BUFFER_ASPECT) {
        DISP_W = BUFFER_W * DISP_SCALE_W;
        DISP_H = BUFFER_H * DISP_SCALE_W;

        float DD_H = BUFFER_H * DISP_SCALE_H;

        DISP_H_OFFSET = (DD_H - DISP_H) / 2;
        DISP_W_OFFSET = 0;
    } else if (DISP_ASPECT > BUFFER_ASPECT) {
        DISP_W = BUFFER_W * DISP_SCALE_H;
        DISP_H = BUFFER_H * DISP_SCALE_H;

        float DD_W = BUFFER_W * DISP_SCALE_W;

        DISP_W_OFFSET = (DD_W - DISP_W) / 2;
        DISP_H_OFFSET = 0;
    } else {
        DISP_W = BUFFER_W * DISP_SCALE_W;
        DISP_H = BUFFER_H * DISP_SCALE_H;

        DISP_W_OFFSET = 0;
        DISP_H_OFFSET = 0;
    }

    al_set_target_backbuffer(disp);
    al_clear_to_color(al_map_rgb(0, 0, 0));
    al_draw_scaled_bitmap(buffer, 0, 0, BUFFER_W, BUFFER_H, DISP_W_OFFSET, DISP_H_OFFSET, DISP_W, DISP_H, 0);

    al_flip_display();
}

#define KEY_SEEN     1
#define KEY_RELEASED 2
unsigned char key[ALLEGRO_KEY_MAX];

void keyboard_init() {
    memset(key, 0, sizeof(key));
}

void keyboard_update(ALLEGRO_EVENT* event) {
    switch (event->type) {
        case ALLEGRO_EVENT_TIMER:
            for (int i = 0; i < ALLEGRO_KEY_MAX; i++)
                key[i] &= KEY_SEEN;
            break;
        case ALLEGRO_EVENT_KEY_DOWN:
            key[event->keyboard.keycode] = KEY_SEEN | KEY_RELEASED;
            break;
        case ALLEGRO_EVENT_KEY_UP:
            key[event->keyboard.keycode] &= KEY_RELEASED;
            break;
    }
}

int mouse_pos_x;
int mouse_pos_y;

typedef struct GAME_STATE {
    BOARD board;
    COLOR turn;
    VECTOR poss_moves;
    VECTOR castling_moves;
    bool castling[2];
    int selected_piece_index;
    bool in_check[2];
    bool checkmate[2];
} GAME_STATE;

GAME_STATE create_game_state() {
    GAME_STATE game_state;
    game_state.board = create_board();
    game_state.turn = white;
    game_state.poss_moves = construct_vector(sizeof(MOVE));
    game_state.castling_moves = construct_vector(sizeof(CASTLING));
    game_state.castling[queenside] = false;
    game_state.castling[kingside] = false;
    game_state.selected_piece_index = -1;
    game_state.in_check[white] = false;
    game_state.in_check[black] = false;
    game_state.checkmate[white] = false;
    game_state.checkmate[black] = false;
    return game_state;
}

void destroy_game_state(GAME_STATE game_state) {
    deconstruct_vector(game_state.poss_moves);
    deconstruct_vector(game_state.castling_moves);
}

void draw(GAME_STATE game_state) {
    disp_pre_draw();

    // draw code
    render(game_state.board, &game_state.poss_moves, &game_state.castling_moves, game_state.turn, game_state.in_check, game_state.checkmate);

    disp_post_draw();
}

char* strtrim(char* str) {
    size_t length = strlen(str);
    char* new_str = (char*)malloc(sizeof(char) * (length + 1));
    int j = 0;
    for (int i = 0; i < length; i++) {
        switch (str[i]) {
            case '\t':
            case '\n':
            case '\v':
            case '\f':
            case '\r':
            case ' ':
                break;
            default:
                new_str[j] = str[i];
                j++;
                break;
        }
    }
    new_str[j] = '\0';
    return new_str;
}

#define INPUT_ALGEBRAIC_NOTATION_EVENT_TYPE ALLEGRO_GET_EVENT_TYPE('a', 'l', 'g', 'e')

void* input_algerbraic_notation_thread_func(ALLEGRO_THREAD* thread, void* arg) {
    ALLEGRO_EVENT_SOURCE* input_algebraic_notation_source = (ALLEGRO_EVENT_SOURCE*)arg;
    ALLEGRO_EVENT input_algebraic_notation_event;
    char* line = NULL;
    size_t size;
    while (getline(&line, &size, stdin) != -1) {
        if (al_get_thread_should_stop(thread))
            break;
        char* notation = strtrim(line);
        input_algebraic_notation_event.user.type = INPUT_ALGEBRAIC_NOTATION_EVENT_TYPE;
        input_algebraic_notation_event.user.data1 = (intptr_t)notation;
        al_emit_user_event(input_algebraic_notation_source, &input_algebraic_notation_event, NULL);
    }
    free(line);
    return NULL;
}

void make_move(GAME_STATE* game_state, MOVE move, bool *take_mouse_input) {
    BOARD b = clone_board(game_state->board);
    PIECE promoted_to;
    move_piece(&game_state->board, game_state->selected_piece_index, move, disp, &promoted_to);
    char* notation = move_to_algebraic_notation(b, game_state->selected_piece_index, move, promoted_to);
    printf("%s", notation);
    free(notation);
    if (game_state->turn == white) {
        printf(" ");
        fflush(stdout);
    } else {
        printf("\n");
    }
    game_state->in_check[game_state->turn] = king_in_check(game_state->board, game_state->turn);
    game_state->checkmate[game_state->turn] = game_state->in_check[game_state->turn] && !can_move(game_state->board, game_state->turn);
    game_state->turn = game_state->turn == white ? black : white;
    game_state->in_check[game_state->turn] = king_in_check(game_state->board, game_state->turn);
    bool cm = can_move(game_state->board, game_state->turn);
    game_state->checkmate[game_state->turn] = game_state->in_check[game_state->turn] && !cm;
    bool stalemate = !game_state->in_check[game_state->turn] && !cm;
    *take_mouse_input = !(game_state->checkmate[white] || game_state->checkmate[black] || stalemate);
    if (game_state->checkmate[white]) {
        printf("0-1\n");
    }
    if (game_state->checkmate[black]) {
        printf("\n1-0\n");
    }
    if (stalemate) {
        if (game_state->turn == black)
            printf("\n");
        printf("½-½\n");
    }
    deconstruct_vector(game_state->poss_moves);
    deconstruct_vector(game_state->castling_moves);
    game_state->castling[queenside] = false;
    game_state->castling[kingside] = false;
    game_state->poss_moves = construct_vector(sizeof(MOVE));
    game_state->castling_moves = construct_vector(sizeof(CASTLING));
}

void make_castling_move(GAME_STATE* game_state, CASTLING_SIDE side) {
    castle(&game_state->board, game_state->turn, side);
    printf("%s", castling_to_algebraic_notation(side));
    if (game_state->turn == white) {
        printf(" ");
        fflush(stdout);
    } else {
        printf("\n");
    }
    game_state->turn = game_state->turn == white ? black : white;
    deconstruct_vector(game_state->poss_moves);
    deconstruct_vector(game_state->castling_moves);
    game_state->castling[queenside] = false;
    game_state->castling[kingside] = false;
    game_state->poss_moves = construct_vector(sizeof(MOVE));
    game_state->castling_moves = construct_vector(sizeof(CASTLING));
}

int main() {
    ALLEGRO_TIMER *timer;
    ALLEGRO_EVENT_QUEUE *queue;
    ALLEGRO_EVENT_SOURCE input_algebraic_notation_source;

    int redraw = true;
    must_init(al_init(), "Allegro");
    must_init(al_init_image_addon(), "image addon");
    must_init(al_install_keyboard(), "keyboard");
    must_init(al_install_mouse(), "mouse");
    must_init(al_init_native_dialog_addon(), "native dialog addon");
    al_init_user_event_source(&input_algebraic_notation_source);

    // init
    al_set_new_display_option(ALLEGRO_SAMPLE_BUFFERS, 1, ALLEGRO_SUGGEST);
    al_set_new_display_option(ALLEGRO_SAMPLES, 8, ALLEGRO_SUGGEST);
    al_set_new_display_option(ALLEGRO_DEPTH_SIZE, 16, ALLEGRO_SUGGEST);
    al_set_new_display_flags(ALLEGRO_RESIZABLE);

    disp_init();

    keyboard_init();

    mouse_pos_x = 0;
    mouse_pos_y = 0;

    timer = al_create_timer(1.0 / 60);
    must_init(timer, "timer");

    queue = al_create_event_queue();
    must_init(queue, "queue");
    al_register_event_source(queue, al_get_keyboard_event_source());
    al_register_event_source(queue, al_get_mouse_event_source());
    al_register_event_source(queue, al_get_display_event_source(disp));
    al_register_event_source(queue, al_get_timer_event_source(timer));
    al_register_event_source(queue, &input_algebraic_notation_source);

    // setup_scene();
    bool take_mouse_input = true;
    sprites_init();
    GAME_STATE game_state = create_game_state();

    ALLEGRO_THREAD* input_algebraic_notation_thread = al_create_thread(input_algerbraic_notation_thread_func, &input_algebraic_notation_source);
    al_start_thread(input_algebraic_notation_thread);

    bool done = false;
    al_start_timer(timer);
    while(true) {
        ALLEGRO_EVENT event;

        al_wait_for_event(queue, &event);
        switch (event.type) {
            case ALLEGRO_EVENT_TIMER:
                // game logic
                break;
            case ALLEGRO_EVENT_DISPLAY_CLOSE:
                done = true;
                break;
            case ALLEGRO_EVENT_DISPLAY_RESIZE:
                al_acknowledge_resize(disp);
                redraw = true;
                break;
            case ALLEGRO_EVENT_KEY_DOWN:
                switch (event.keyboard.keycode) {
                    case ALLEGRO_KEY_ESCAPE:
                        done = true;
                        break;
                }
                break;
            case ALLEGRO_EVENT_MOUSE_AXES:
                mouse_pos_x = event.mouse.x;
                mouse_pos_y = event.mouse.y;
                break;
            case ALLEGRO_EVENT_MOUSE_BUTTON_DOWN:
                if (take_mouse_input && event.mouse.button & 1) {
                    SQUARE square;
                    square.file = (mouse_pos_x - DISP_W_OFFSET) / (DISP_W / 8) + 1;
                    square.rank = 9 - ((mouse_pos_y - DISP_H_OFFSET) / (DISP_H / 8));
                    int move_index = vector_index(&game_state.poss_moves, &square, square_is_move);
                    if (move_index != -1) {
                        MOVE move;
                        vector_get(&game_state.poss_moves, move_index, &move);
                        make_move(&game_state, move, &take_mouse_input);
                    } else if (game_state.castling[queenside] && square.file == a && square.rank == (game_state.turn == white ? 1 : 8)) {
                        make_castling_move(&game_state, queenside);
                    } else if (game_state.castling[kingside] && square.file == h && square.rank == (game_state.turn == white ? 1 : 8)) {
                        make_castling_move(&game_state, kingside);
                    } else {
                        deconstruct_vector(game_state.poss_moves);
                        deconstruct_vector(game_state.castling_moves);
                        game_state.castling[queenside] = false;
                        game_state.castling[kingside] = false;
                        game_state.poss_moves = construct_vector(sizeof(MOVE));
                        game_state.castling_moves = construct_vector(sizeof(CASTLING));
                        int tmp_selected_piece_index = square_to_board_index(game_state.board, square);
                        if (tmp_selected_piece_index != -1) {
                            COLOR color;
                            PIECE piece;
                            board_index_to_piece(game_state.board, tmp_selected_piece_index, &color, &piece);
                            if (color == game_state.turn) {
                                game_state.selected_piece_index = tmp_selected_piece_index;
                                possible_moves(game_state.board, game_state.selected_piece_index, true, &game_state.poss_moves);
                                if (piece == king) {
                                    castling_possible_moves(game_state.board, color, game_state.castling);
                                    if (game_state.castling[queenside]) {
                                        CASTLING c;
                                        c.side = queenside;
                                        c.color = color;
                                        vector_enqueue(&game_state.castling_moves, &c);
                                    }
                                    if (game_state.castling[kingside]) {
                                        CASTLING c;
                                        c.side = kingside;
                                        c.color = color;
                                        vector_enqueue(&game_state.castling_moves, &c);
                                    }
                                }
                            }
                        } else {
                            game_state.selected_piece_index = tmp_selected_piece_index;
                        }
                    }
                    redraw = true;
                }
                break;
            case INPUT_ALGEBRAIC_NOTATION_EVENT_TYPE:
                char* notation = (char*)event.user.data1;
                MOVE move;
                bool castle;
                CASTLING_SIDE side;
                PIECE promote_to;
                bool error;
                char* error_message;
                algebraic_notation_to_move(game_state.board, notation, game_state.turn, &game_state.selected_piece_index, &move, &castle, &side, &promote_to, &error, &error_message);
                free(notation);
                if (error) {
                    printf("%s\n", error_message);
                } else {
                    if (castle) {
                        make_castling_move(&game_state, side);
                    } else {
                        make_move(&game_state, move, &take_mouse_input);
                    }
                }
                redraw = true;
                break;
        }

        if (done)
            break;

        keyboard_update(&event);

        if (redraw && al_is_event_queue_empty(queue)) {
            // redraw code
            draw(game_state);
            redraw = false;
        }
    }

    // deinit
    al_set_thread_should_stop(input_algebraic_notation_thread);
    disp_deinit();
    destroy_game_state(game_state);
    al_destroy_user_event_source(&input_algebraic_notation_source);
    al_destroy_timer(timer);
    al_destroy_event_queue(queue);
    al_destroy_thread(input_algebraic_notation_thread);

    return 0;
}
