#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <limits.h>
#include <string.h>
#include <stdbool.h>
#include <conio.h>
#include <stdarg.h>
#include "misc.h"

//signatures data declarations and nesting 'trees'

#define LBLOCK '\xb0'
#define MBLOCK '\xb1'
#define DBLOCK '\xb2'
#define BBLOCK '\xdb'
#define UPERHALF '\xdc'
#define LOWERHALF '\xdf'
#define BLANK '\xff'

#define PROMPT_WIDTH 80
#define PROMPT_HEIGHT 24


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
typedef struct{
    char** elements;
    int height;
    int width;
    int hOffset;
    int vOffset;
    int size;
}image;

typedef struct{
    int x;
    int y;
}point;

//void oldDisplay(image screen);
//image oldLoad();

void menu();
    image initMen(size_t cols, size_t lines);

    void options();
    void credits(char** menu, FILE* cred);
    void game();
        int logic(char* mode, char* arg);

        int load(FILE* source, image* target);
            int clear(image* target);

        int display(image screen, int height, int width);
            void initDisp(image* init);


int main()
{
    //use sparingly, this is dangerous.

    char buffer[80*24] = {0};
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
    image bg;
    image full;

    load(fopen("./files/background.txt", "r"), &bg);
    load(fopen("./files/background.txt", "r"), &full);

    char* player = "\xDA\xC1\xBF";                                          //player 'sprite'
    point pPos = {full.width/2-1, 23};                                      //place player on center of last line
    char shot = '^';
    point sPos = {0};
    char enemy = '\xCA';                                                    //enemy 'sprite'
    point ePos = {(full.width/20), (full.height/8)};
    int step = 4;

    bool stay = true;
    int i/*, j*/;

    centerCpy(full.elements[pPos.y], player);

    while(stay)
    {
        checkFocus();

        mergeImg(full, bg, &full, 0, 0);
        //for(i = 0; i < 24; i++)
        //    strncpy(full.elements[i], bg.elements[i], 80);

        strncpy(full.elements[pPos.y]+pPos.x, player, sizeof(player)-1);
        full.elements[ePos.y][ePos.x] = enemy;
        if(sPos.x != 0)
            full.elements[sPos.y][sPos.x] = shot;

        display(full, 24, 80);

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
                pPos.x = (pPos.x+1 < 77)? pPos.x+1: pPos.x;
                break;
            case 4:
            case 8:
                pPos.x = (pPos.x-1 > 0)? pPos.x-1: pPos.x;
                break;
            case 6:
                printf("\a");
                fflush(stdout);
                sPos.x = pPos.x + 1;
                sPos.y = pPos.y - 1;
                break;
            default:
                system("color 0C");
                pause();
                logic("setting", "color");
        }

        if((ePos.y & 1) != 0)
        {
            if((ePos.x + step) < full.width-2)
                ePos.x += step;
            else if((ePos.y + 1) < (full.height-1))
                ePos.y++;
            else break;
        }else
        {
            if((ePos.x - step) > 2)
                ePos.x -= step;
            else if((ePos.y + 1) < (full.height-1))
                ePos.y++;
            else break;
        }

        if(sPos.x != 0)
        {
            if((sPos.y - step) > 0)
            {
                for(i = 1; i <= step; i++)
                    if(full.elements[sPos.y-i][sPos.x] == enemy)
                        ePos = (point){2, full.height-2};
                sPos.y -= step;
            }
            else
                sPos.x = 0;
        }
    }


    clear(&bg);
    clear(&full);
}

///Returns useful integers for logic and calls certain functions; takes processing mode and arguments.
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
                case 'X':
                    return 2;
                case 'd':
                case 'D':
                    return 3;
                case 'a':
                case 'A':
                    return 4;
                case ENTER:
                    return 5;
                case 'e':
                case 'E':
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
        FILE* cfg = fopen("./files/settings.ini", "r+");
        if(!strcmp(arg, "color"))
        {
            char* option = malloc(30);
            while(fscanf(cfg, "%s", option) != EOF)
                if(strcmp(option, "color"))
                {
                    fscanf(cfg, "%s", option);
                        if(!strcmp(option, "matrix"))
                        {
                            option = "color 0A";
                            system(option);
                        }
                        else if(!strcmp(option, "paper"))
                        {
                            option = "color 78";
                            system(option);
                        }
                        else if(!strcmp(option, "turbo"))
                        {
                            option = "color 1E";
                            system(option);
                        }
                        else if(!strcmp(option, "contrast"))
                        {
                            option = "color C9";
                            system(option);
                        }
                        else if(!strcmp(option, "ice"))
                        {
                            option = "color 17";
                            system(option);
                        }
                        else if(!strcmp(option, "scorpion"))
                        {
                            option = "color 0E";
                            system(option);
                        }
                        else if(!strcmp(option, "camo"))
                        {
                            option = "color 26";
                            system(option);
                        }
                        else if(!strcmp(option, "blood"))
                        {
                            option = "color 4C";
                            system(option);
                        }
                        else if(!strcmp(option, "haunted"))
                        {
                            option = "color 40";
                            system(option);
                        }
                        else if(!strcmp(option, "magic"))
                        {
                            option = "color 5C";
                            system(option);
                        }
                        else if(!strcmp(option, "pimp"))
                        {
                            option = "color 5D";
                            system(option);
                        }
                        else if(!strcmp(option, "gold"))
                        {
                            option = "color 6E";
                            system(option);
                        }
                        else if(!strcmp(option, "ink"))
                        {
                            option = "color 80";
                            system(option);
                        }
                        else if(!strcmp(option, "toxic"))
                        {
                            option = "color EA";
                            system(option);
                        }
                        else if(!strcmp(option, "plutonium"))
                        {
                            option = "color A8";
                            system(option);
                        }
                        else if(!strcmp(option, "focus"))
                        {
                            option = "color FC";
                            system(option);
                        }
                        else
                        {
                            char* out = malloc(9);
                            sprintf(out, "color %s\n", option);
                            system(out);
                            free(out);
                        }
                    free(option);
                    fclose(cfg);
                    return 1;
                }
            fclose(cfg);
            free(option);
            return 0;
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
int display(image screen, int height, int width)
{
    system("cls");
    if((height < 1)||(width < 1)||((screen.width - screen.hOffset) < 1)||((screen.height - screen.vOffset) < 1))
        return 0;

    //Attempt at O(n). I have no idea if this really worked.
    int i;

    /*If any of the to-be-printed lengths are greater the size of the prompt,
    use the prompt size instead, else, use the length max. This prevents
    indexes out of bounds.*/
    int wMax = ((screen.width - screen.hOffset) <= PROMPT_WIDTH)? (screen.width - screen.hOffset): PROMPT_WIDTH,
    hMax = ((screen.height - screen.vOffset) <= PROMPT_HEIGHT)? (screen.height - screen.vOffset): PROMPT_HEIGHT;

{
    if(width == PROMPT_WIDTH)
        for(i = 0; i < height*PROMPT_WIDTH; i++)
        {
            if(screen.elements[(int)(i/width)][i%width] != '\0')
                printf("%c", screen.elements[(int)(i/width)][i%width]);//can be shorter than prompt
            else
            {
                printf("\n");
                i += (width-1)-(i % width);
            }
        }
    else
        for(i = 0; i < height*width; i++)
        {
            if(((i % width) != (width-1)) && (screen.elements[(int)(i/width)][i%width] != '\0'))
                printf("%c", screen.elements[(int)(i/width)][i%width]);//thinner than prompt
            else
            {
                printf("%c\n", screen.elements[(int)(i/width)][i%width]);
                i += (width-1)-(i % width);
            }
        }
    }
    fflush(stdout);
    return 1;
}


///Loads initial screen where none is present, ergo: initializes display.
///An proper equivalent should be a load() of the background.txt merged with border.txt
void initDisp(image* init)
{
    image borders;

    load(fopen("./files/background.txt", "r"), init);
    load(fopen("./files/border.txt", "r"), &borders);
    mergeImg(*init, borders, init, 0, 0);
    clear(&borders);
}

int mergeImg(image background, image foreground, image* target, int vOffset, int hOffset)
{
    int i;

    target->elements = (char**)calloc(background.height, sizeof(char*));
    for(i = 0; i < background.height; i++)
        target->elements[i] = (char*)calloc(background.width+1, sizeof(char));

    for(i = 0; i < background.size; i++)
    {
        if(foreground.elements[(int)(i/background.width)][i%background.width] != '\0')
        {
            target->elements[(int)(i/background.width)][(i)%background.width] = (   foreground.elements[(int)(i/background.width)][i%background.width] == ' ')?
                                                                                    background.elements[(int)(i/background.width)][i%background.width]:
                                                                                    foreground.elements[(int)(i/background.width)][i%background.width];
        }//--------------------------------------------------------------------------------------------------------------------------------------------|
    }

    target->height = background.height;
    target->width = background.width;
    target->hOffset = background.hOffset;
    target->vOffset = background.vOffset;
    target->size = background.size;
}


///Calls and handles menu screen. Possibly, the logic and visual functions can be made to handle respective operations on scope.
void menu(){
    image title = initMen(80, 24);
    char** opts = retrieveOptions(title.elements[21]+1, '\xB0', '|');
    int option = 0;
    *opts[option] = 178;

    bool stay = true;

    while(stay)
    {
        checkFocus();
        display(title, 24, 80);
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
                        credits(title.elements, fopen("./files/credits.txt", "r"));
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
    clear(&title);
}


///Returns a "simple" menu with given number lines and their respective prompts. Initializes menu.
image initMen(size_t cols, size_t lines)
{
    image setup;

    initDisp(&setup);


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


///TO DO: main menu options
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

    image credits = initMen(80, 24);
    for(i = 0; i < 22+credsize-1; i++)                                  //credits roll loop
    {
        checkFocus();
        for(j = 1; j < 23; j++)
            strcpy(credits.elements[23-j], menu[23-j]);       //resets the screen

        if(i < 22){

            for(j = 0; j < k; j++)
                centerCpy(credits.elements[22-i+j], creds[j]);          //loads first lines until screen is full with 'em
            if(k < credsize)
                k++;

        }else{

            for(j = 0; j < k-1; j++)
                centerCpy(credits.elements[j+1], creds[(j+i-21)]);      //loads remaining lines and then fades when text hits top.
            if((j+i-21) > (credsize-1))
                k--;

        }
        display(credits, 24, 80);                                       //call render function
        delay(170);                                                     //menu roll speed
        int quit;                                                       //variable that holds the logic evaluation of key press
        if((quit = logic("eval_key", NULL)) == 1 || quit == 2)          //check if user pressed ESC or X
            break;                                                      //quit loop
    }

    clear(&credits);
    for(i = 0; i < credsize; i++)
        free(creds[i]);
    free(creds);
    fclose(cred);
}


/**
*   Loads FILE into image,
*   allocates dynamically depending on file size; sets null-terminators and theoretically creates uneven matrices.
*   returns boolean values for success.
*/
int load(FILE* source, image* target)
{
    int i, height = 0, width = 0, hMax = 0, wMax = 0;
    char c = 0;

    target->elements = (char**)calloc(1, sizeof(char*));//line to be filled
    target->elements[0] = (char*)calloc(1, sizeof(char));//char to be filled

    while((c = getc(source))!= EOF)
    {
        if(c == '#')
        {
            while(c != '\n')
            {
                c = getc(source);
                if(c == EOF)
                    break;
            }
        }
        if(c == '\n')
        {
            //printf("%c", c);
            target->elements[height][width] = '\0';
            width = 0;
            height++;
            target->elements = (char**)realloc(target->elements, (height+1)*sizeof(char*));
            target->elements[height] = (char*)calloc(width+1, sizeof(char));
        }
        else
        {
            //printf("%c", c);
            target->elements[height][width] = c;
            width++;
            if(wMax < width)//{
                wMax = width;
                //printf("%d\n", wMax);
            //}
            target->elements[height] = (char*)realloc(target->elements[height], width+1*sizeof(char));
        }
    }
    target->elements[height][width] = '\0';

    target->width = wMax;
    target->height = height+1;
    target->vOffset = 0;
    target->hOffset = 0;
    target->size = wMax*(height+1);

    return 1;
}


int clear(image* target)
{
    int i;
    for(i = 0; i < target->height; i++)
    {
        free(target->elements[i]);
        target->elements[i] = NULL;
    }

    target->height = 0;
    target->size = target->width;
    free(target->elements);
    target->elements = NULL;
    target->width = 0;
    target->size = 0;
}


/* old functions
void oldDisplay(image screen)
{
    int i, j;
    system("cls");
    for(i = screen.lu; i < screen.height; i++){
        for(j = screen.lu; j < screen.width; j++)
            printf("%c", screen.elements[i][j]);
        printf("\n");
    }
    fflush(stdout);
}

image oldLoad()
{
    FILE* level = fopen("./files/main.txt", "r");
    char** target = (char**)malloc(24*sizeof(char*));
    int i;

    for(i = 0; i < 24; i++)
    {
        target[i] = (char*)malloc(81);
        if(fgets(target[i], 81, level) == NULL)
        {
            free(target[i]);
            break;
        } else {
            if(target[i][strlen(target[i])-1] == '\n')
                target[i][strlen(target[i])-1] = '\0';
        }
    }
    fclose(level);

    return (image){target, 0, strlen(target[0]), i, i*(strlen(target[0]))};
}
*/
