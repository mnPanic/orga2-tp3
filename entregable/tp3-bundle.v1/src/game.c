/* ** por compatibilidad se omiten tildes **
================================================================================
 TRABAJO PRACTICO 3 - System Programming - ORGANIZACION DE COMPUTADOR II - FCEN
================================================================================
*/

#include "game.h"

#define MESSAGE_MAX_LEN 20

uint8_t debug_mode_on = 0;
uint8_t debug_chart_shown = 0;

char *eax_str = "eax:";
char *ebx_str = "ebx:";
char *ecx_str = "ecx:";
char *edx_str = "edx:";
char *esi_str = "esi:";
char *edi_str = "edi:";
char *ebp_str = "ebp:";
char *esp_str = "esp:";
char *eip_str = "eip:";
char *cs_str = "cs:";
char *ds_str = "ds:";
char *es_str = "es:";
char *fs_str = "fs:";
char *gs_str = "gs:";
char *ss_str = "ss:";
char *eflags_str = "eflags:";
char *cr0_str = "cr0:";
char *cr2_str = "cr2:";
char *cr3_str = "cr3:";
char *cr4_str = "cr4:";
char *stack_str = "stk:";

char *UNKNOWN_EXCEPTION = "Unknown";
char *EXCEPTION_0 = "Divide by Zero";
char *EXCEPTION_1 = "Debug";
char *EXCEPTION_2 = "Non-maskable interrupt";
char *EXCEPTION_3 = "Breakpoint";
char *EXCEPTION_4 = "Overflow";
char *EXCEPTION_5 = "Bound range exceeded";
char *EXCEPTION_6 = "Invalid Opcode";
char *EXCEPTION_7 = "Device not Available";
char *EXCEPTION_8 = "Double Fault";
char *EXCEPTION_10 = "Invalid TSS";
char *EXCEPTION_11 = "Segment not Present";
char *EXCEPTION_12 = "Stack-segment Fault";
char *EXCEPTION_13 = "General Protection Fault";
char *EXCEPTION_14 = "Page Fault";
char *EXCEPTION_16 = "Floating Point Exc.";
char *EXCEPTION_17 = "Alignment Check";
char *EXCEPTION_18 = "Machine Check";
char *EXCEPTION_30 = "Security Exception";

ca saved_screen[SCREEN_H * SCREEN_W];

uint32_t points_y = 41;

uint32_t message_x[2] = {
    3,
    43,
};

uint32_t message_y[3] = {
    46,
    47,
    48,
};

uint16_t message_color[2] = {
    C_BG_RED + C_FG_WHITE,
    C_BG_BLUE + C_FG_WHITE,
};

uint16_t bg_color = C_BG_LIGHT_GREY + C_FG_LIGHT_GREY;

uint16_t ball_color[2] = {
    C_BG_LIGHT_GREY + C_FG_RED,
    C_BG_LIGHT_GREY + C_FG_BLUE,
};

uint16_t player_color[2] = {
    C_BG_RED + C_FG_BLACK,
    C_BG_BLUE + C_FG_CYAN,
};

uint32_t player_points[2] = {
    0,
    0
};

uint32_t remaining_balls[2] = {
    15,
    15
};

uint32_t player_x[2] = {
    0,
    79
};
uint32_t prev_player_y[2] = {
    BOARD_H / 2 - 3,
    BOARD_H / 2 - 3
};
uint32_t player_y[2] = {
    BOARD_H / 2 - 3,
    BOARD_H / 2 - 3
};

uint32_t prev_ball_x[6];
uint32_t prev_ball_y[6];
uint32_t ball_x[6];
uint32_t ball_y[6];

e_action_t ball_current_actions[6];
short ball_current_directions_x[6];
short ball_current_directions_y[6];

char *ballChar = "O";
char *missingBallChar = "X";
char *playerBgChar = "@";

uint8_t tick = 0;

uint32_t keys[10] = {
    SCAN_CODE_W,
    SCAN_CODE_S,
    SCAN_CODE_Z,
    SCAN_CODE_X,
    SCAN_CODE_C,
    SCAN_CODE_I,
    SCAN_CODE_K,
    SCAN_CODE_B,
    SCAN_CODE_N,
    SCAN_CODE_M,
};

uint32_t keyPresses[10];

void game_tick() {
    tick = (tick + 1) % 7;
}

/* ---------------------------- UI UTILS ---------------------------- */
void printIfValid(char* msg, uint32_t x, uint32_t y, uint16_t color) {
    if (x < BOARD_W && y < BOARD_H) {
        print(msg, x, y, color);
    }
}

void printPlayer(uint32_t x, uint32_t y, uint16_t color) {
    for (int i = 0; i < PLAYER_SIZE; i++) {
        printIfValid(playerBgChar, x, y + i, color);
    }
}
/* ---------------------------- UI UTILS ---------------------------- */

void game_printWinMsg() {
    //               Fila desde   ,  Columna desde  , fsize, csize
    screen_drawBox(BOARD_H / 2 - 1, BOARD_W / 2 - 15,   3  ,  30,  '@', C_BG_BLACK + C_FG_LIGHT_GREY);
    print(player_points[0] == 10 ? "Player A won" : "Player B won", BOARD_W / 2 - 6, BOARD_H / 2, C_BG_LIGHT_GREY + C_FG_WHITE);
}

void game_clearBallMessage(int ball) {
    int32_t x = message_x[ball / 3];
    uint32_t y = message_y[ball % 3];
    uint16_t color = player_color[ball / 3];
    print("@@@@@@@@@@@@@@@@@@@@", x, y, color);
}

uint8_t hitPlayerAGoal(uint32_t x, uint32_t y) {
    // Devuelve true si est?? en la posici??n del goal de A y el jugador
    // A no est?? en la posici??n de impacto de la pelota!
    return x == PLAYER_A_GOAL && (y < player_y[0] || y > player_y[0] + PLAYER_SIZE);
}

uint8_t hitPlayerBGoal(uint32_t x, uint32_t y) {
    // Devuelve true si est?? en la posici??n del goal de A y el jugador
    // A no est?? en la posici??n de impacto de la pelota!
    return x >= PLAYER_B_GOAL && (y < player_y[1] || y > player_y[1] + PLAYER_SIZE);
}

void game_movePlayerUp(uint8_t player) {
    // Mueve el jugador para arriba, de ser posible
    if (player_y[player] > 0) {
        player_y[player]--;
    }
}

void game_movePlayerDown(uint8_t player) {
    // Mueve el jugador para abajo, de ser posible
    if (player_y[player] < BOARD_H - PLAYER_SIZE) {
        player_y[player]++;
    }
}

void game_setDefaultBallDirections(PLAYER ballIndex) {
    ball_current_actions[ballIndex] = Center;
    // El jugador A tiene direccion positiva por defecto, el B al reves
    ball_current_directions_x[ballIndex] = (ballIndex / 3) == 0 ? 1 : -1;
    ball_current_directions_y[ballIndex] = 1;
}

void game_launchBall(PLAYER ballType) {
    if (ball_x[ballType] < 1000 || remaining_balls[ballType / 3] == 0) {
        return;
    }
    sched_newBall(ballType);

    // Configuramos las direcciones de la pelota y su posicion
    game_setDefaultBallDirections(ballType);

    ball_x[ballType] = player_x[ballType / 3] + ball_current_directions_x[ballType];
    ball_y[ballType] = player_y[ballType / 3] + PLAYER_SIZE / 2;
    remaining_balls[ballType / 3]--;
}

void game_init() {
    for (uint32_t i = 0; i < 6; i++) {
        prev_ball_x[i] = 1000;
        prev_ball_y[i] = 1000;
        ball_x[i] = 1000;
        ball_y[i] = 1000;

        game_setDefaultBallDirections(i);
    }
}

void game_executeBallCalculations() {
    for (int i = 0; i < 6; i++) {
        uint32_t x = prev_ball_x[i];
        uint32_t y = prev_ball_y[i];

        if (x > BOARD_W || y > BOARD_H) {
            continue;
        }

        e_action_t direction = ball_current_actions[i];
        switch (direction) {
            case Up:
                y -= 1 * ball_current_directions_y[i];
                break;

            case Center:
                // NO-OP, la pelota sigue movi??ndose derecho
                break;

            case Down:
                y += 1 * ball_current_directions_y[i];
                break;
        }

        x += 1 * ball_current_directions_x[i];

        if (hitPlayerAGoal(x, y)) {
            // Matar la tarea de la pelota!
            sched_killBall(i);
            player_points[PLAYER_B]++;

            ball_x[i] = 1000;
            ball_y[i] = 1000; // Posiciones invalidas

            game_clearBallMessage(i);
        } else if (hitPlayerBGoal(x, y)) {
            // Matar la tarea de la pelota!
            sched_killBall(i);
            player_points[PLAYER_A]++;

            ball_x[i] = 1000;
            ball_y[i] = 1000; // Posiciones invalidas

            game_clearBallMessage(i);
        } else {
            ball_x[i] = x;
            ball_y[i] = y;

            // Manejar rebotes
            if (x == BOARD_W || x == 0) {
                ball_current_directions_x[i] *= -1;
            }
            if (y == 0 || y == BOARD_H - 1) {
                ball_current_directions_y[i] *= -1;
            }
        }
    }
}

void game_executeInputCalculations() {
    if (tick != 0) {
        return;
    }

    if (keyPresses[PLAYER_A_UP]) game_movePlayerUp(PLAYER_A);
    if (keyPresses[PLAYER_A_DOWN]) game_movePlayerDown(PLAYER_A);
    if (keyPresses[PLAYER_A_BALL_1]) game_launchBall(PLAYER_A_TIPO_1);
    if (keyPresses[PLAYER_A_BALL_2]) game_launchBall(PLAYER_A_TIPO_2);
    if (keyPresses[PLAYER_A_BALL_3]) game_launchBall(PLAYER_A_TIPO_3);
    if (keyPresses[PLAYER_B_UP]) game_movePlayerUp(PLAYER_B);
    if (keyPresses[PLAYER_B_DOWN]) game_movePlayerDown(PLAYER_B);
    if (keyPresses[PLAYER_B_BALL_1]) game_launchBall(PLAYER_B_TIPO_1);
    if (keyPresses[PLAYER_B_BALL_2]) game_launchBall(PLAYER_B_TIPO_2);
    if (keyPresses[PLAYER_B_BALL_3]) game_launchBall(PLAYER_B_TIPO_3);

    for(int i = 0; i < 10; i++) {
        keyPresses[i] = 0;
    }
}

void game_executeFrameCalculations() {
    game_executeBallCalculations();
    game_executeInputCalculations();
}

void game_executeFrame() {
    game_executeFrameCalculations();

    // Pintar puntos
    for (int i = 0; i < 2; i++) {
        // Limpiar anteriores
        uint32_t x = message_x[i];
        uint32_t y = points_y;
        uint16_t color = message_color[i];

        print_dec(player_points[i], 2, x, y, color);
        print_dec(remaining_balls[i], 2, x + 6, y, color);
    }

    if (player_points[0] >= 10 || player_points[1] >= 10) {
        // Alguien ya gan??
        game_printWinMsg();
        return;
    }

    // Pintar pelotas
    for (int i = 0; i < 6; i++) {
        // Limpiar anteriores
        uint32_t x = prev_ball_x[i];
        uint32_t y = prev_ball_y[i];
        uint16_t color = bg_color;
        printIfValid(ballChar, x, y, color);

        // Imprimir nuevas
        x = ball_x[i];
        y = ball_y[i];
        color = ball_color[i / 3];
        printIfValid(ballChar, x, y, color);
        
        // Actualizar pelotas previas
        prev_ball_x[i] = x;
        prev_ball_y[i] = y;
    }

    // Pintar paletas
    for (int i = 0; i < 2; i++) {
        // Limpiar anteriores
        uint32_t x = player_x[i];
        uint32_t y = prev_player_y[i];
        uint16_t color = bg_color;
        printPlayer(x, y, color);

        // Imprimir nuevas
        y = player_y[i];
        color = player_color[i];
        printPlayer(x, y, color);
        
        // Actualizar posici??n previa
        prev_player_y[i] = y;
    }

    // Pintar pelotas disponibles
    for (int i = 0; i < 6; i++) {
        uint32_t x = message_x[i / 3] + 12 + (i % 3);
        uint32_t y = points_y;
        uint16_t color = message_color[i / 3];

        if (ball_x[i] >= 1000) {
            print(ballChar, x, y, color);
        } else {
            print(missingBallChar, x, y, color);
        }
    }
}

void game_talk(const char *msg) {
    uint32_t currentBall = sched_getTareaActual();

    // Truncamos texto, rellenamos con espacios para limpiar
    char displayMsg[MESSAGE_MAX_LEN + 1];
    for (uint8_t i = 0; i < MESSAGE_MAX_LEN; i++) {
        char toWrite = ' ';
        if (*msg != 0) {
            toWrite = *msg;
            msg++;
        }
        displayMsg[i] = toWrite;
    }
    // Agregamos el NULL terminator
    displayMsg[MESSAGE_MAX_LEN] = '\0';
    
    // Las 1ras 3 tareas son del Player A, las otras 3 del Player B
    uint32_t x = message_x[currentBall / 3];
    uint32_t y = message_y[currentBall % 3];
    uint16_t color = message_color[currentBall / 3];
    print(displayMsg, x, y, color);
}

uint32_t game_getCurrentX() {
    uint32_t currentBall = sched_getTareaActual();
    return ball_x[currentBall];
}

uint32_t game_getCurrentY() {
    uint32_t currentBall = sched_getTareaActual();
    return ball_y[currentBall];
}

void game_informAction(e_action_t action) {
    uint32_t currentBall = sched_getTareaActual();
    ball_current_actions[currentBall] = action;
}


void game_kbInput(uint32_t input) {
    if (input == SCAN_CODE_Y) {
        if (debug_mode_on && debug_chart_shown) {
            // Hay que restaurar la pantalla y seguir con el juego!
            restore_screen(&saved_screen);
            debug_chart_shown = 0;
        } else {
            // Descomentar esto para probar f??cil: ---
            // game_showDebugInfo(99, 0);           //
            // ---------------------------------------
            // Toggleamos el modo debug
            debug_mode_on = debug_mode_on ? 0 : 1;

            uint8_t fg = C_FG_BLACK;
            if(debug_mode_on) {
                fg = C_FG_WHITE;
            }
            print("Modo Debug", SCREEN_W / 2 - 5 , SCREEN_H - 1, C_BG_BLACK + fg);
        }
        return;
    }

    // Vemos que tecla se est?? presionando
    for(int i = 0; i < 10; i++) {
        if(keys[i] == input) {
            keyPresses[i] = 1;
        }
    }
}

char *get_exception_str(uint32_t exception);

void game_killCurrentBall(){
    uint32_t player = sched_getTareaActual();
    uint16_t color = bg_color;
    printIfValid(ballChar, ball_x[player], ball_y[player], color);
    prev_ball_x[player] = 1000;
    prev_ball_y[player] = 1000; 
    ball_x[player] = 1000;
    ball_y[player] = 1000;
    game_clearBallMessage(player);
}

void game_showDebugInfo(
        uint32_t exception,  
        uint32_t pa_edi,
        uint32_t pa_esi,
        uint32_t pa_ebp,
        uint32_t pa_esp,
        uint32_t pa_ebx,
        uint32_t pa_edx,
        uint32_t pa_ecx,
        uint32_t pa_eax,   
        uint32_t pchg_error_code,
        uint32_t pchg_eip,
        uint16_t pchg_cs,
        uint32_t pchg_eflags,
        uint32_t pchg_esp,
        uint16_t pchg_ss) {
    if (!debug_mode_on) {
        return;
    }

    save_screen(&saved_screen);

    screen_drawBox(1, BOARD_W / 2 - 15, BOARD_H - 1, 30, '@', C_BG_BLACK + C_FG_LIGHT_GREY);
    screen_drawBox(2, BOARD_W / 2 - 14, BOARD_H - 3, 28, '@', C_BG_BLACK + C_FG_BLACK);
    
    screen_drawBox(3, BOARD_W / 2 - 14, 1, 28, '@', C_BG_DARK_GREY + C_FG_DARK_GREY);
    screen_drawBox(5, BOARD_W / 2 - 14, BOARD_H - 6, 28, '@', C_BG_LIGHT_GREY + C_FG_LIGHT_GREY);

    print_dec(exception, 2, BOARD_W / 2 - 14, 3, C_BG_DARK_GREY + C_FG_WHITE);
    char *str = get_exception_str(exception);
    print(str, BOARD_W / 2 - 11, 3, C_BG_DARK_GREY + C_FG_WHITE);

    print(    "0x",         BOARD_W / 2 + 10, 3,   C_BG_DARK_GREY + C_FG_WHITE);
    print_hex(pchg_error_code, 2,BOARD_W / 2 + 12, 3,   C_BG_DARK_GREY + C_FG_WHITE);

    print(      eax_str,    BOARD_W / 2 - 13,   6,  C_BG_LIGHT_GREY + C_FG_BLACK);
    print_hex(  pa_eax, 8,  BOARD_W / 2 - 8,    6,  C_BG_LIGHT_GREY + C_FG_WHITE);

    print(      ebx_str,    BOARD_W / 2 - 13,   8,  C_BG_LIGHT_GREY + C_FG_BLACK);
    print_hex(  pa_ebx, 8,  BOARD_W / 2 - 8,    8,  C_BG_LIGHT_GREY + C_FG_WHITE);

    print(      ecx_str,    BOARD_W / 2 - 13,   10,  C_BG_LIGHT_GREY + C_FG_BLACK);
    print_hex(  pa_ecx, 8,  BOARD_W / 2 - 8,    10,  C_BG_LIGHT_GREY + C_FG_WHITE);

    print(      edx_str,    BOARD_W / 2 - 13,   12,  C_BG_LIGHT_GREY + C_FG_BLACK);
    print_hex(  pa_edx, 8,  BOARD_W / 2 - 8,    12,  C_BG_LIGHT_GREY + C_FG_WHITE);

    print(      esi_str,    BOARD_W / 2 - 13,   14,  C_BG_LIGHT_GREY + C_FG_BLACK);
    print_hex(  pa_esi, 8,  BOARD_W / 2 - 8,    14,  C_BG_LIGHT_GREY + C_FG_WHITE);

    print(      edi_str,    BOARD_W / 2 - 13,   16,  C_BG_LIGHT_GREY + C_FG_BLACK);
    print_hex(  pa_edi, 8,  BOARD_W / 2 - 8,    16,  C_BG_LIGHT_GREY + C_FG_WHITE);

    print(      ebp_str,    BOARD_W / 2 - 13,   18,  C_BG_LIGHT_GREY + C_FG_BLACK);
    print_hex(  pa_ebp, 8,  BOARD_W / 2 - 8,    18,  C_BG_LIGHT_GREY + C_FG_WHITE);

    print(      esp_str,    BOARD_W / 2 - 13,   20,  C_BG_LIGHT_GREY + C_FG_BLACK);
    print_hex(  pa_esp, 8,  BOARD_W / 2 - 8,    20,  C_BG_LIGHT_GREY + C_FG_WHITE);

    // print(      eip_str,    BOARD_W / 2 - 13,   22,  C_BG_LIGHT_GREY + C_FG_BLACK);
    // print_hex(  reip(), 8,  BOARD_W / 2 - 8,    22,  C_BG_LIGHT_GREY + C_FG_WHITE);

    print(      cs_str,     BOARD_W / 2 - 13,   24,  C_BG_LIGHT_GREY + C_FG_BLACK);
    print_hex(  pchg_cs, 4,   BOARD_W / 2 - 8,    24,  C_BG_LIGHT_GREY + C_FG_WHITE);

    print(      ds_str,     BOARD_W / 2 - 13,   26,  C_BG_LIGHT_GREY + C_FG_BLACK);
    print_hex(  rds(), 4,   BOARD_W / 2 - 8,    26,  C_BG_LIGHT_GREY + C_FG_WHITE);

    print(      es_str,     BOARD_W / 2 - 13,   28,  C_BG_LIGHT_GREY + C_FG_BLACK);
    print_hex(  res(), 4,   BOARD_W / 2 - 8,    28,  C_BG_LIGHT_GREY + C_FG_WHITE);

    print(      fs_str,     BOARD_W / 2 - 13,   30,  C_BG_LIGHT_GREY + C_FG_BLACK);
    print_hex(  rfs(), 4,   BOARD_W / 2 - 8,    30,  C_BG_LIGHT_GREY + C_FG_WHITE);

    print(      gs_str,     BOARD_W / 2 - 13,   32,  C_BG_LIGHT_GREY + C_FG_BLACK);
    print_hex(  rgs(), 4,   BOARD_W / 2 - 8,    32,  C_BG_LIGHT_GREY + C_FG_WHITE);

    print(      ss_str,     BOARD_W / 2 - 13,   34,  C_BG_LIGHT_GREY + C_FG_BLACK);
    print_hex(  pchg_ss, 4,   BOARD_W / 2 - 8,    34,  C_BG_LIGHT_GREY + C_FG_WHITE);

    print(      eflags_str,     BOARD_W / 2 - 13,   37,  C_BG_LIGHT_GREY + C_FG_BLACK);
    print_hex(  pchg_eflags, 8,   BOARD_W / 2 - 5,    37,  C_BG_LIGHT_GREY + C_FG_WHITE);

    print(      cr0_str,     BOARD_W / 2 + 1,   6,  C_BG_LIGHT_GREY + C_FG_BLACK);
    print_hex(  rcr0(), 8,   BOARD_W / 2 + 6,   6,  C_BG_LIGHT_GREY + C_FG_WHITE);

    print(      cr2_str,     BOARD_W / 2 + 1,   8,  C_BG_LIGHT_GREY + C_FG_BLACK);
    print_hex(  rcr2(), 8,   BOARD_W / 2 + 6,   8,  C_BG_LIGHT_GREY + C_FG_WHITE);

    print(      cr3_str,     BOARD_W / 2 + 1,   10,  C_BG_LIGHT_GREY + C_FG_BLACK);
    print_hex(  rcr3(), 8,   BOARD_W / 2 + 6,   10,  C_BG_LIGHT_GREY + C_FG_WHITE);

    print(      cr4_str,     BOARD_W / 2 + 1,   12,  C_BG_LIGHT_GREY + C_FG_BLACK);
    print_hex(  rcr4(), 8,   BOARD_W / 2 + 6,   12,  C_BG_LIGHT_GREY + C_FG_WHITE);

    print(      stack_str,     BOARD_W / 2 + 1,   24,  C_BG_LIGHT_GREY + C_FG_BLACK);
    print_hex(  *(((uint32_t *)pchg_esp) + 0), 8,   BOARD_W / 2 + 6,    24,  C_BG_LIGHT_GREY + C_FG_WHITE);
    
    print_hex(  *(((uint32_t *)pchg_esp) + 1), 8,   BOARD_W / 2 + 6,    25,  C_BG_LIGHT_GREY + C_FG_WHITE);

    print_hex(  *(((uint32_t *)pchg_esp) + 2), 8,   BOARD_W / 2 + 6,    26,  C_BG_LIGHT_GREY + C_FG_WHITE);

    print_hex(  *(((uint32_t *)pchg_esp) + 3), 8,   BOARD_W / 2 + 6,    27,  C_BG_LIGHT_GREY + C_FG_WHITE);

    print_hex(  *(((uint32_t *)pchg_esp) + 4), 8,   BOARD_W / 2 + 6,    28,  C_BG_LIGHT_GREY + C_FG_WHITE);

    debug_chart_shown = 1;
}

uint32_t game_isDebugChartOn() {
    return debug_chart_shown;
}


char *get_exception_str(uint32_t exception) {
    switch (exception) {
        case 0:
            return EXCEPTION_0;
        case 1:
            return EXCEPTION_1;
        case 2:
            return EXCEPTION_2;
        case 3:
            return EXCEPTION_3;
        case 4:
            return EXCEPTION_4;
        case 5:
            return EXCEPTION_5;
        case 6:
            return EXCEPTION_6;
        case 7:
            return EXCEPTION_7;
        case 8:
            return EXCEPTION_8;
        case 10:
            return EXCEPTION_10;
        case 11:
            return EXCEPTION_11;
        case 12:
            return EXCEPTION_12;
        case 13:
            return EXCEPTION_13;
        case 14:
            return EXCEPTION_14;
        case 16:
            return EXCEPTION_16;
        case 17:
            return EXCEPTION_17;
        case 18:
            return EXCEPTION_18;
        case 30:
            return EXCEPTION_30;

        default:
            return UNKNOWN_EXCEPTION;
    }
}