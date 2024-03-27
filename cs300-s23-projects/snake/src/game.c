#include "game.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "linked_list.h"
#include "mbstrings.h"

/** Updates the game by a single step, and modifies the game information
 * accordingly. Arguments:
 *  - cells: a pointer to the first integer in an array of integers representing
 *    each board cell.
 *  - width: width of the board.
 *  - height: height of the board.
 *  - snake_p: pointer to your snake struct (not used until part 2!)
 *  - input: the next input.
 *  - growing: 0 if the snake does not grow on eating, 1 if it does.
 */
void update(int* cells, size_t width, size_t height, snake_t* snake_p,
            enum input_key input, int growing) {
    // `update` should update the board, your snake's data, and global
    // variables representing game information to reflect new state. If in the
    // updated position, the snake runs into a wall or itself, it will not move
    // and global variable g_game_over will be 1. Otherwise, it will be moved
    // to the new position. If the snake eats food, the game score (`g_score`)
    // increases by 1. This function assumes that the board is surrounded by
    // walls, so it does not handle the case where a snake runs off the board.

    // TODO: implement!
    int eat = 0;
    if (length_list(snake_p->head_pos) >= 2) {
        if ((snake_p->snake_dir == INPUT_UP && input == INPUT_DOWN) ||
            (snake_p->snake_dir == INPUT_DOWN && input == INPUT_UP) ||
            (snake_p->snake_dir == INPUT_LEFT && input == INPUT_RIGHT) ||
            (snake_p->snake_dir == INPUT_RIGHT && input == INPUT_LEFT)) {
        
            input = INPUT_NONE;
        }
    }
    snake_p->snake_dir = (input == INPUT_NONE) ? snake_p->snake_dir : input;
    int origin_pos = *(int*)(get_first(snake_p->head_pos));
    int new_pos = origin_pos;
    if(snake_p->snake_dir == INPUT_UP){
        new_pos -= width;
    } else if(snake_p->snake_dir == INPUT_DOWN){
        new_pos += width;
    } else if(snake_p->snake_dir == INPUT_LEFT){
        new_pos -= 1;
    } else if(snake_p->snake_dir == INPUT_RIGHT){
        new_pos += 1;
    }
    if(cells[new_pos] == FLAG_FOOD){
        place_food(cells, width, height);
        g_score ++;
        eat = 1;
    } else if(cells[new_pos] == FLAG_WALL || cells[new_pos] == FLAG_SNAKE){
        g_game_over = 1;
    }
    int tail_pos = *(int*)get_last(snake_p->head_pos);
    if(g_game_over && (new_pos != tail_pos || (eat && growing))){
        return;
    }
    g_game_over = 0;
    if (!growing || !eat) {
        int* tail_pointer = remove_last(&(snake_p->head_pos));
        cells[*tail_pointer] = FLAG_PLAIN_CELL;
        free(tail_pointer);
    }
    cells[new_pos]  = FLAG_SNAKE;
    int* data = malloc(sizeof(int));
    *data = new_pos;
    insert_first(&(snake_p->head_pos), data, sizeof(int));

}

/** Sets a random space on the given board to food.
 * Arguments:
 *  - cells: a pointer to the first integer in an array of integers representing
 *    each board cell.
 *  - width: the width of the board
 *  - height: the height of the board
 */
void place_food(int* cells, size_t width, size_t height) {
    /* DO NOT MODIFY THIS FUNCTION */
    unsigned food_index = generate_index(width * height);
    if (*(cells + food_index) == FLAG_PLAIN_CELL) {
        *(cells + food_index) = FLAG_FOOD;
    } else {
        place_food(cells, width, height);
    }
    /* DO NOT MODIFY THIS FUNCTION */


/** Prompts the user for their name and saves it in the given buffer.
 * Arguments:
 *  - `write_into`: a pointer to the buffer to be written into.
 */
void read_name(char* write_into) {
    printf("%s", "Name > ");
    fflush(0);
    int len;
    while((len = read(0, write_into, 1000)) == 1){
        printf("%s\n", "Name Invalid: must be longer than 0 characters.");
        fflush(0);
        printf("%s", "Name > ");
        fflush(0);
    }
    if(write_into[len - 1] == '\n'){
        write_into[len - 1] = '\0';
    }
}


/** Cleans up on game over — should free any allocated memory so that the
 * LeakSanitizer doesn't complain.
 * Arguments:
 *  - cells: a pointer to the first integer in an array of integers representing
 *    each board cell.
 *  - snake_p: a pointer to your snake struct. (not needed until part 2)
 */
void teardown(int* cells, snake_t* snake_p) {
    free(cells);

    while (snake_p->head_pos != NULL) {
        void* data = remove_last(&(snake_p->head_pos));
        if (data != NULL) {
            free(data);
        }
    }
}



