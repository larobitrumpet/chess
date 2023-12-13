#include <stdio.h>
#include <stdlib.h>
#include "allegro5/allegro5.h"
#include "allegro5/allegro_image.h"
#include "allegro5/allegro_native_dialog.h"
#include "board.h"
#include "render.h"
#include "move.h"
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

void draw(BOARD board, VECTOR* poss_moves, VECTOR* castling_moves, COLOR turn, bool in_check[2], bool checkmate[2]) {
    disp_pre_draw();

    // draw code
    render(board, poss_moves, castling_moves, turn, in_check, checkmate);

    disp_post_draw();
}

COLOR turn;

int main() {
    ALLEGRO_TIMER *timer;
    ALLEGRO_EVENT_QUEUE *queue;

    int redraw = true;
    must_init(al_init(), "Allegro");
    must_init(al_init_image_addon(), "image addon");
    must_init(al_install_keyboard(), "keyboard");
    must_init(al_install_mouse(), "mouse");
    must_init(al_init_native_dialog_addon(), "native dialog addon");

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

    // setup_scene();
    BOARD board = create_board();
    sprites_init();
    turn = white;
    VECTOR poss_moves = construct_vector(sizeof(MOVE));
    VECTOR castling_moves = construct_vector(sizeof(CASTLING));
    bool castling[2];
    int selected_piece_index = -1;
    bool in_check[2] = {false, false};
    bool checkmate[2] = {false, false};

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
                if (event.mouse.button & 1) {
                    SQUARE square;
                    square.file = (mouse_pos_x - DISP_W_OFFSET) / (DISP_W / 8) + 1;
                    square.rank = 9 - ((mouse_pos_y - DISP_H_OFFSET) / (DISP_H / 8));
                    int move_index = vector_index(&poss_moves, &square, square_is_move);
                    if (move_index != -1) {
                        MOVE move;
                        vector_get(&poss_moves, move_index, &move);
                        move_piece(&board, selected_piece_index, move, disp);
                        in_check[turn] = king_in_check(board, turn);
                        checkmate[turn] = in_check[turn] && !can_move(board, turn);
                        turn = turn == white ? black : white;
                        in_check[turn] = king_in_check(board, turn);
                        checkmate[turn] = in_check[turn] && !can_move(board, turn);
                        deconstruct_vector(poss_moves);
                        deconstruct_vector(castling_moves);
                        castling[queenside] = false;
                        castling[kingside] = false;
                        poss_moves = construct_vector(sizeof(MOVE));
                        castling_moves = construct_vector(sizeof(CASTLING));
                    } else if (castling[queenside] && square.file == a && square.rank == (turn == white ? 1 : 8)) {
                        castle(&board, turn, queenside);
                        turn = turn == white ? black : white;
                        deconstruct_vector(poss_moves);
                        deconstruct_vector(castling_moves);
                        castling[queenside] = false;
                        castling[kingside] = false;
                        poss_moves = construct_vector(sizeof(MOVE));
                        castling_moves = construct_vector(sizeof(CASTLING));
                    } else if (castling[kingside] && square.file == h && square.rank == (turn == white ? 1 : 8)) {
                        castle(&board, turn, kingside);
                        turn = turn == white ? black : white;
                        deconstruct_vector(poss_moves);
                        deconstruct_vector(castling_moves);
                        castling[queenside] = false;
                        castling[kingside] = false;
                        poss_moves = construct_vector(sizeof(MOVE));
                        castling_moves = construct_vector(sizeof(CASTLING));
                    } else {
                        deconstruct_vector(poss_moves);
                        deconstruct_vector(castling_moves);
                        castling[queenside] = false;
                        castling[kingside] = false;
                        poss_moves = construct_vector(sizeof(MOVE));
                        castling_moves = construct_vector(sizeof(CASTLING));
                        int tmp_selected_piece_index = square_to_board_index(board, square);
                        if (tmp_selected_piece_index != -1) {
                            COLOR color;
                            PIECE piece;
                            board_index_to_piece(board, tmp_selected_piece_index, &color, &piece);
                            if (color == turn) {
                                selected_piece_index = tmp_selected_piece_index;
                                possible_moves(board, selected_piece_index, true, &poss_moves);
                                if (piece == king) {
                                    castling_possible_moves(board, color, castling);
                                    if (castling[queenside]) {
                                        CASTLING c;
                                        c.side = queenside;
                                        c.color = color;
                                        vector_enqueue(&castling_moves, &c);
                                    }
                                    if (castling[kingside]) {
                                        CASTLING c;
                                        c.side = kingside;
                                        c.color = color;
                                        vector_enqueue(&castling_moves, &c);
                                    }
                                }
                            }
                        } else {
                            selected_piece_index = tmp_selected_piece_index;
                        }
                    }
                    redraw = true;
                }
                break;
        }

        if (done)
            break;

        keyboard_update(&event);

        if (redraw && al_is_event_queue_empty(queue)) {
            // redraw code
            draw(board, &poss_moves, &castling_moves, turn, in_check, checkmate);
            redraw = false;
        }
    }

    // deinit
    deconstruct_vector(poss_moves);
    deconstruct_vector(castling_moves);
    disp_deinit();
    al_destroy_timer(timer);
    al_destroy_event_queue(queue);

    return 0;
}
