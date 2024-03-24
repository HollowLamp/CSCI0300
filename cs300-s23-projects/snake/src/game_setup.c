#include "game_setup.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Some handy dandy macros for decompression
#define E_CAP_HEX 0x45
#define E_LOW_HEX 0x65
#define S_CAP_HEX 0x53
#define S_LOW_HEX 0x73
#define W_CAP_HEX 0x57
#define W_LOW_HEX 0x77
#define DIGIT_START 0x30
#define DIGIT_END 0x39

/** Initializes the board with walls around the edge of the board.
 *
 * Modifies values pointed to by cells_p, width_p, and height_p and initializes
 * cells array to reflect this default board.
 *
 * Returns INIT_SUCCESS to indicate that it was successful.
 *
 * Arguments:
 *  - cells_p: a pointer to a memory location where a pointer to the first
 *             element in a newly initialized array of cells should be stored.
 *  - width_p: a pointer to a memory location where the newly initialized
 *             width should be stored.
 *  - height_p: a pointer to a memory location where the newly initialized
 *              height should be stored.
 */
enum board_init_status initialize_default_board(int** cells_p, size_t* width_p,
                                                size_t* height_p) {
    *width_p = 20;
    *height_p = 10;
    int* cells = malloc(20 * 10 * sizeof(int));
    *cells_p = cells;
    for (int i = 0; i < 20 * 10; i++) {
        cells[i] = FLAG_PLAIN_CELL;
    }

    // Set edge cells!
    // Top and bottom edges:
    for (int i = 0; i < 20; ++i) {
        cells[i] = FLAG_WALL;
        cells[i + (20 * (10 - 1))] = FLAG_WALL;
    }
    // Left and right edges:
    for (int i = 0; i < 10; ++i) {
        cells[i * 20] = FLAG_WALL;
        cells[i * 20 + 20 - 1] = FLAG_WALL;
    }

    // Add snake
    cells[20 * 2 + 2] = FLAG_SNAKE;

    return INIT_SUCCESS;
}

/** Initialize variables relevant to the game board.
 * Arguments:
 *  - cells_p: a pointer to a memory location where a pointer to the first
 *             element in a newly initialized array of cells should be stored.
 *  - width_p: a pointer to a memory location where the newly initialized
 *             width should be stored.
 *  - height_p: a pointer to a memory location where the newly initialized
 *              height should be stored.
 *  - snake_p: a pointer to your snake struct (not used until part 2!)
 *  - board_rep: a string representing the initial board. May be NULL for
 * default board.
 */
enum board_init_status initialize_game(int** cells_p, size_t* width_p,
                                       size_t* height_p, snake_t* snake_p,
                                       char* board_rep) {
    // TODO: implement!
    enum board_init_status ret;
    snake_p->head_pos = (node_t*)malloc(sizeof(node_t));
    if(board_rep) {
        ret = decompress_board_str(cells_p, width_p, height_p, snake_p, board_rep);
    }   else{ 
        ret = initialize_default_board(cells_p, width_p, height_p);
        int* data = malloc(sizeof(int));
        *data = 42;
        snake_p->head_pos->data = data;
        snake_p->head_pos->next = NULL;
        snake_p->head_pos->prev = NULL;
    }
    if(ret != INIT_SUCCESS){
        return ret;
    }
    snake_p->snake_dir = INPUT_RIGHT;
    place_food(*cells_p, *width_p, *height_p);
    g_game_over = 0;
    g_score = 0;

    return ret;
}

/** Takes in a string `compressed` and initializes values pointed to by
 * cells_p, width_p, and height_p accordingly. Arguments:
 *      - cells_p: a pointer to the pointer representing the cells array
 *                 that we would like to initialize.
 *      - width_p: a pointer to the width variable we'd like to initialize.
 *      - height_p: a pointer to the height variable we'd like to initialize.
 *      - snake_p: a pointer to your snake struct (not used until part 2!)
 *      - compressed: a string that contains the representation of the board.
 * Note: We assume that the string will be of the following form:
 * B24x80|E5W2E73|E5W2S1E72... To read it, we scan the string row-by-row
 * (delineated by the `|` character), and read out a letter (E, S or W) a number
 * of times dictated by the number that follows the letter.
 */
void helper(snake_t* snake_p, int width, char* line,  int* Snum, int* wwrong, int* unknown, int* cells_p, int row, int* Snumtag){
    int sum = 0;
    for(int i = 0; line[i] != '\0'; i++){
        int cnt = 0;
        if(!(line[i] == 'W' || line[i] == 'E' || line[i] == 'S' || (line[i] >= '0' && line[i] <= '9'))){
            *unknown = 1;
        }
        if(line[i] >= 'A' && line[i] <= 'Z'){
            char tmp = line[i];
            i++;
            while(line[i] >= '0' && line[i] <= '9'){
                cnt = cnt * 10 + line[i] - '0';
                i++;
            }
            i--;
            if(tmp == 'S'){
                *Snum += cnt;
            } 
        }
        sum += cnt;

    }
    if(sum != width){
        *wwrong = 1;
    }
    int col = 0;
    for(int i = 0; line[i] != '\0'; i++){
        int cnt = 0;

        if(line[i] == 'W' || line[i] == 'E' || line[i] == 'S'){
            char fill = line[i];
                        i++;
            while(line[i] >= '0' && line[i] <= '9'){
                cnt = cnt * 10 + line[i] - '0';
                i++;
            }
            i--;
            while(cnt--){
                if(fill == 'W'){
                    cells_p[row * width + col] = FLAG_WALL;
                } else if(fill == 'E'){
                    cells_p[row * width + col] = FLAG_PLAIN_CELL;
                } else if(!*Snumtag){
                    cells_p[row * width + col] = FLAG_SNAKE;
                    int* data = malloc(sizeof(int));
                    *data = row * width + col;
                    snake_p->head_pos->data = data;
                    snake_p->head_pos->next = NULL;
                    snake_p->head_pos->prev = NULL;
                    *Snumtag = 1;
                }
                col++;
            }
        }
    }
}
enum board_init_status decompress_board_str(int** cells_p, size_t* width_p,
                                            size_t* height_p, snake_t* snake_p,
                                            char* compressed) {
    int Bwidth = 0;
    int Bheight = 0;
    int Snum = 0;
    const char s1[2] = "|";
    int choose = 0;
    for(int i = 0; compressed[i] != '|'; i++){
        if(compressed[i] >= '0' && compressed[i] <= '9' && !choose){
            Bheight = Bheight * 10 + compressed[i] - '0';
        }
        if(compressed[i] == 'x')
            choose = 1;
        if(compressed[i] >= '0' && compressed[i] <= '9' && choose){
            Bwidth = Bwidth * 10 + compressed[i] - '0';
        }
    }
    *width_p = Bwidth;
    *height_p = Bheight;
    *cells_p = malloc(Bwidth * Bheight * sizeof(int));
    char* token = strtok(compressed, s1);
    int row = 0;
    int wwrong = 0;
    int unknown = 0;
    int Snumtag = 0;
    while(row < Bheight){

        token = strtok(NULL, s1);
        if(token == NULL){
            return INIT_ERR_INCORRECT_DIMENSIONS;    
        }
        helper(snake_p, Bwidth, token, &Snum, &wwrong, &unknown, *cells_p, row, &Snumtag);
        
        row++;
    }
    if(!Snumtag){
        int* data = malloc(sizeof(int));
        *data = 0;
        snake_p->head_pos->data = data;
        snake_p->head_pos->next = NULL;
        snake_p->head_pos->prev = NULL;           
    }
    if(wwrong){
        return INIT_ERR_INCORRECT_DIMENSIONS;
    }
    else if(unknown){
        return INIT_ERR_BAD_CHAR;
    }
    if((token = strtok(NULL, s1)) != NULL){
        return INIT_ERR_INCORRECT_DIMENSIONS;
    }
    if(Snum != 1){

        return INIT_ERR_WRONG_SNAKE_NUM;
    }
    return INIT_SUCCESS;
}
