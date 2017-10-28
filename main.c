#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <limits.h>
#include <string.h>
#include <stdbool.h>
#include <conio.h>
#include <stdarg.h>
#include "misc.h"

//signatures data declarations and nesting trees

#define LBLOCK '\xb0'
#define MBLOCK '\xb1'
#define DBLOCK '\xb2'
#define BBLOCK '\xdb'
#define UPERHALF '\xdc'
#define LOWERHALF '\xdf'
#define BLANK '\xff'


enum
{
    ESC       = 27,
    ENTER     = 13,
    KEY_DIRECTION = -32,
    KEY_UP    = 72,
    KEY_LEFT  = 75,
    KEY_RIGHT = 77,
    KEY_DOWN  = 80,
    SPACEBAR  = 32,

    DEFAULT_WINDOW_WIDTH = 79,
    DEFAULT_WINDOW_HEIGHT = 24
};

//char matrix of N size
//left-upper corner, diagonal offset from elements's [0][0].
//right-upper corner, column number.
//left-lower corner, line number
/*right-lower corner, matrix column and line number product.->\
//could become the offset's offset from the diagonal.          |
//(0 = diag, n>0 = line over col, n<0 = col over line)       */
typedef struct screen{
    char** elements;
    int lu;
    int ru;
    int ll;
    int rl;
}screen;

void menu();
    screen initMen(size_t cols, size_t lines);

    void options();
    void credits(char** menu, FILE* cred);
    void game();
        int logic(char* mode, char* arg);

        screen load();

        void display(screen image);
            char** initDisp(char **scr, size_t width, size_t height);


int main()
{
    //use sparingly, this is dangerous.

    char buffer[79*24] = {0};
    setvbuf(stdout, buffer, _IOFBF, sizeof(buffer));

    //setvbuf() changes the buffer size so that printf() can write to the command window faster,
    //but(in this case) it only prints with a fflush() or meeting the full 1896 chars on buffer size.
    //It's a major improvement on speed.

    logic("setting", "color");

    menu();

    return 0;
}

///game loop
void game()
{
    screen bg = load();
    screen full = load();

    char* player = "\xDA\xC1\xBF";
    int playerPosition = (strlen(full.elements[22])/2-1);
    char* enemies[] = {"\x94", "\x94", "\x94", "\x94", "\x94", "\x94", "\x94", "\x94", "\x94", "\x94"};

    bool stay = true;
    int i, j;

    centerCpy(full.elements[22], player);
    for(i = 1; i < 11; i++)
        formatCpy(full.elements[i], enemies, 10);


    while(stay)
    {
        checkFocus();

        for(i = 0; i < 24; i++)
            strncpy(full.elements[i], bg.elements[i], 80);

        strncpy(full.elements[22]+playerPosition, player, sizeof(player)-1);


        display(full);

        switch(logic("eval_key", NULL))
        {
            case 0:
                break;
            case 1:
            case 2:
                stay = !stay;
                break;
            case 3:
            case 7:
                playerPosition = (playerPosition+1 < 76)? playerPosition+1: playerPosition;
                break;
            case 4:
            case 8:
                playerPosition = (playerPosition-1 > 0)? playerPosition-1: playerPosition;
                break;
            case 6:
                printf("\a");
                fflush(stdout);
                break;
            default:
                system("color 0C");
                pause();
                logic("setting", "color");
        }


    }

    free(player);

}

///Returns useful integers for logic and calls certain functions; processing mode and args.
int logic(char* mode, char* arg)
{
    if(!strcmp(mode, "eval_key"))
    {
        if(kbhit())
        {
            char key = getch();
            switch(key)
            {
                case ESC:
                    return 1;
                case 'x':
                    return 2;
                case 'd':
                    return 3;
                case 'a':
                    return 4;
                case ENTER:
                    return 5;
                case 'e':
                case SPACEBAR:
                    return 6;
                case KEY_DIRECTION:
                    switch(key = getch())
                    {
                        case KEY_RIGHT:
                            return 7;
                        case KEY_LEFT:
                            return 8;
                    }
                default:
                    return 9;
            }

            dump();
        }
    }
    if(!strcmp(mode, "setting"))
    {
        FILE* cfg = fopen("settings.ini", "r+");
        if(!strcmp(arg, "color"))
        {
            char* option = malloc(30);
            while(fscanf(cfg, "%s", option) != EOF)
                if(strcmp(option, "color"))
                {
                    fscanf(cfg, "%s", option);
                        if(!strcmp(option, "green"))
                        {
                            option = "color 0A";
                            system(option);
                            printf("\a");
                        }
                        else
                        {
                            char* out = malloc(9);
                            sprintf(out, "color %s\n", option);
                            system(out);
                            free(out);
                        }
                }
            free(option);
        }
        fclose(cfg);
    }
    /*if(!strcmp(mode, "debug"))
    {
        if();
    }*/
    return 0;
}

///Renders any properly formatted screen
void display(screen image)
{
    int i, j;
    system("cls");
    for(i = image.lu; i < image.ll; i++){
        for(j = image.lu; j < image.ru; j++)
            printf("%c", image.elements[i][j]);
        printf("\n");
    }
    fflush(stdout);
}

///Loads initial screen where none is present, ergo: initializes display.
char** initDisp(char **scr, size_t width, size_t height)
{
    int i, j;
    if(!scr)
    {
        scr = (char**)malloc(height*sizeof(char*));
        for(i = 0; i < height; i++)
            scr[i] = (char*)malloc(width);

        for(i = 0; i < height; i++){
            for(j = 0; j < width; j++){
                scr[i][j] = (((i==0) || (i==height-1))?
                                            ((j==0) || (j==width-1))?
                                                '*'
                                                :'-'
                                            :((j==0) || (j==width-1))?
                                                '|'
                                                :176);
            }
        }
    }
    return scr;
}

///Calls and handles menu screen. Possibly, the logic and visual functions can be made to handle respective operations on scope.
void menu(){
    screen title = initMen(79, 24);
    char** opts = retrieveOptions(title.elements[21]+1, '\xB0', '|');
    int option = 0;
    *opts[option] = 178;

    bool stay = true;

    while(stay)
    {
        checkFocus();
        display(title);
        switch(logic("eval_key", NULL))
        {
            case 0:
                break;
            case 1:
            case 2:
                stay = !stay;
                break;
            case 3:
            case 7:

                if(option+1 == 3){
                    changeC(opts[option], opts[0]);
                    option = 0;
                }else{
                    changeC(opts[option], opts[option+1]);
                    option++;
                }break;

            case 4:
            case 8:

                if(option-1 == -1){
                    changeC(opts[option], opts[2]);
                    option = 2;
                }else{
                    changeC(opts[option], opts[option-1]);
                    option--;
                }break;

            case 5:
            case 6:
                switch(option)
                {
                    case 0:
                        game();
                        break;
                    case 1:
                        options();
                        break;
                    case 2:
                        credits(title.elements, fopen("./levels/credits.txt", "r"));
                        break;
                }
                break;
            default:
                system("color 0C");
                pause();
                logic("setting", "color");
        }
        delay(33);
    }
    free(opts);
}

///Returns a "simple" menu with given number lines and their respective prompts. Initializes menu.
screen initMen(size_t cols, size_t lines)
{
    screen setup = {initDisp(NULL, cols, lines), 0, cols, lines, cols*lines};


    int i;
    char* header[] = {"Space Invaders!","(a bootleg copy by a struggling student)","(\xA6u\xA6)","Eduardo Rodrigues Baldini Filho"};
    char* options[] = {"Start game", "Options", "Credits"};

    char* logo[] = {"\xDB\xDB\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xDB\xDB",
                    "\xDB\xDB\xB0\xB0\xB0\xB0\xB0\xB0\xDB\xDB",
                    "\xDB\xDB\xDF\xDF\xDB\xDB\xDB\xDB\xDB\xDB\xDF\xDF\xDB\xDB",
                    "\xDB\xDB\xDB\xDB\xDC\xDC\xDB\xDB\xDB\xDB\xDB\xDB\xDC\xDC\xDB\xDB\xDB\xDB",
                    "\xDB\xDB\xDF\xDF\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDF\xDF\xDB\xDB",
                    "\xDB\xDB\xB0\xB0\xDB\xDB\xDF\xDF\xDF\xDF\xDF\xDF\xDF\xDF\xDF\xDF\xDB\xDB\xB0\xB0\xDB\xDB",
                    "\xDF\xDF\xB0\xB0\xDF\xDF\xDC\xDC\xDC\xDC\xB0\xB0\xDC\xDC\xDC\xDC\xDF\xDF\xB0\xB0\xDF\xDF",
                    "\xDF\xDF\xDF\xDF\xB0\xB0\xDF\xDF\xDF\xDF"};

    for(i = 0; i < 3; i++)
        formatCpy(setup.elements[i], header+i, 1);
    formatCpy(setup.elements[23], header+3, 1);

    for(i = 0; i < 8; i++)
        centerCpy(setup.elements[i+7], logo[i]);

    formatCpy(setup.elements[21], options, 3);

    return setup;
}

void options()
{
    logic("","");
}


///rolls credits on cred
void credits(char** menu, FILE* cred)
{
    int credsize = 0, i, j, k = 1;

    char** creds = (char**)malloc(sizeof(char*));
    creds[credsize] = malloc(70);
    while(fgets(creds[credsize], 70, cred) != NULL)
    {
        if(creds[credsize][strlen(creds[credsize])-1] == '\n')
            creds[credsize][strlen(creds[credsize])-1] = '\0';
        credsize++;
        creds = (char**)realloc(creds, (credsize+1)*sizeof(char*));
        creds[credsize] = (char*)malloc(70);
    }
    *(creds+credsize) = NULL;

    screen credits = initMen(79, 24);
    for(i = 0; i < 22+credsize-1; i++)                                  //credits roll loop
    {
        checkFocus();
        for(j = 1; j < 23; j++)
            strcpy(credits.elements[23-j], menu[23-j]);       //resets the screen

        if(i < 22){

            for(j = 0; j < k; j++)
                centerCpy(credits.elements[22-i+j], creds[j]);          //loads first lines
            if(k < credsize)
                k++;

        }else{

            for(j = 0; j < k-1; j++)
                centerCpy(credits.elements[j+1], creds[(j+i-21)]);      //loads remaining lines and then fades when text hits top.
            if((j+i-21) > (credsize-1))
                k--;

        }
        display(credits);                                               //call render function
        delay(170);                                                     //menu roll speed
        int quit;                                                       //variable that holds the logic evaluation of key press
        if((quit = logic("eval_key", NULL)) == 1 || quit == 2)          //check if user pressed ESC or X
            break;                                                      //quit loop
    }

    for(i = 0; i < credsize; i++)
        free(creds[i]);
    free(creds);
    fclose(cred);
}

screen load()
{
    FILE* level = fopen("./levels/main.txt", "r");
    char** image = (char**)malloc(24*sizeof(char*));
    int i;

    for(i = 0; i < 24; i++)
    {
        image[i] = (char*)malloc(81);
        if(fgets(image[i], 81, level) == NULL)
        {
            free(image[i]);
            break;
        } else {
            if(image[i][strlen(image[i])-1] == '\n')
                image[i][strlen(image[i])-1] = '\0';
        }
    }
    fclose(level);

    return (screen){image, 0, strlen(image[0]), i, i*(strlen(image[0]))};
}
