// for some reason this has to be included first
#include <bits/types/struct_timeval.h>
#include <termios.h>
#include <unistd.h>
#include <sys/select.h>

#include <ctype.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define boardWidth 16
#define boardHeight 16

void displayBoard(uint8_t gameboard[boardHeight][boardWidth]);

void terminalModeRaw(struct termios* original_settings);
void restoreMode(struct termios* original_settings);

int main(void) {
    struct termios previous_settings = {};

    terminalModeRaw(&previous_settings);

    uint8_t gameboard[boardHeight][boardWidth] = {0};

    int8_t dirX = 1;
    int8_t dirY = 0;
    uint8_t headPosX = 8;
    uint8_t headPosY = 8;
    uint8_t snakeLen = 1;

    gameboard[headPosY][headPosX] = 1;
    gameboard[5][5] = 255;


    char lastInput = 0;

    fd_set readfds;
    int numberOfFDs = 1;
    struct timeval timeoutTime = {0, 0};

    // game loop
    while (1) {
        // this has tp be reset each time
        FD_ZERO(&readfds);          // set the set tp zero
        FD_SET(0, &readfds);        // set it as stdin

        int count = select(numberOfFDs, &readfds, NULL, NULL, &timeoutTime);

        // was there any input
        if (count > 0) {
            if (FD_ISSET(0, &readfds)) {
                // get the input
                lastInput = toupper(getchar()); 
            }
        }

        if (lastInput == 'Q')   // q for quit
            break;
        
        // handle user input
        switch (lastInput) {
            case 'W':
                dirX = 0;
                dirY = -1;
                break;
            case 'A':
                dirX = -1;
                dirY = 0;
                break;
            case 'S':
                dirX = 0;
                dirY = 1;
                break;
            case 'D':
                dirX = 1;
                dirY = 0;
                break;
        }

        // potential next positions
        int8_t potNextX = headPosX + dirX;
        int8_t potNextY = headPosY + dirY;

        // check if the next move runs into a wall
        if (potNextX >= boardWidth || potNextX < 0 || potNextY >= boardHeight || potNextY < 0) {
            printf("YOU LOSE!\n");
            break;
        }

        // check if next move moves onto the snake
        if (gameboard[potNextY][potNextX] != 0 && gameboard[potNextY][potNextX] != 255) {
            printf("You also lose!\n");
            break;
        }

        // finally if the next move moves onto an apple. increment the snakelength and place a new apple at a random spot
        if (gameboard[potNextY][potNextX] == 255) {
            snakeLen++;

            uint8_t randomX = 0;
            uint8_t randomY = 0;

            // keep generating random nubers until a free space is found
            while (gameboard[(randomY = rand() % (boardHeight + 1))][(randomX = rand() % (boardWidth + 1))] != 0);

            // then place an apple there
            gameboard[randomY][randomX] = 255;
        }

        // if everything is good update the head
        headPosX = potNextX;
        headPosY = potNextY;

        // then update the player on the board
        gameboard[headPosY][headPosX] = snakeLen+1;

        // then do our fancy decremnts so the snake does not grow endlessly long
        for (int y = 0; y < boardHeight; y++) {
            for (int x = 0; x < boardWidth; x++) {
                // if the game board is part of the snake decrement it
                if (gameboard[y][x] != 0 && gameboard[y][x] != 255)
                    gameboard[y][x]--;
            }
        }

        // clear the terminal
        printf("\033[2J");

        // print the score
        printf("%d\n\r", snakeLen);

        displayBoard(gameboard);

        // check if the player won
        if (snakeLen >= 32) {
            printf("YOU WON!\n\r");
            break;
        }

        usleep((1000/5)*1000); //   update at 5 hz 
    }

    restoreMode(&previous_settings);
    return 0;
}

void displayBoard(uint8_t gameboard[boardHeight][boardWidth]) {
    for (int y = 0; y < boardHeight; y++) {
        for (int x = 0; x < boardWidth; x++) {
            // allways print two chars so it looks square
            switch (gameboard[y][x]) {
                case 0: // zero is empty space
                    printf("\033[47m  \033[40m");   // set the color to white then back to black 
                    break;
                case 255:   // ff is an app
                    printf("\033[41m  \033[40m");   // set the color to red than back to black
                    break;
                default:    // everything else is the snake
                    printf("\033[42m  \033[40m");   // set the color to green then back to black
                    break;

            }
        }
        printf("\n\r");  // print a newline after each line. for some reason we have to write a cf if where in raw mode
    }
}

void terminalModeRaw(struct termios* original_settings) {
    struct termios new_settings = {0};

    tcgetattr(STDIN_FILENO, &new_settings); // get the current settings

    // save them
    *original_settings = new_settings;

    cfmakeraw(&new_settings);   // make it raw
    tcsetattr(STDIN_FILENO, TCSANOW, &new_settings);     // actually change it  
}

void restoreMode(struct termios* original_settings) {
    tcsetattr(STDIN_FILENO, TCSANOW, original_settings);    // set it back to the original settings
}