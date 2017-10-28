#include<stdio.h>
#include<stdlib.h>
#include<conio.h>

#define PROMPT_WIDTH 80
#define PROMPT_HEIGHT 24

typedef struct{
    char** elements;
    int width;
    int height;
}image;

int load(FILE* source, image* target, int width, int height);
void display(image screen, int width, int height);

int main(void)
{
    image background;
    image foreground;
    image screen;
    
    load(fopen("./levels/background.txt", "r"), &background, 24, 80);
    display(background, 24, 80);
    while(!kbhit()){};
    
    return 0;
}

int load(FILE* source, image* target, int height, int width)
{
    int i, j;
    char c;
    
    target->elements = (char**)calloc(height, sizeof(char*));
    
    for(i = 0; i < height; i++)
    {
        target->elements[i] = (char*)calloc(width, sizeof(char));
        for(j = 0; j < width; j++)
        {
            c = fgetc(source);
            if(c == EOF)
                return 0;
            if(c == '#')
                while(c != '\n')
                    c = fgetc(source);
            while(c == '\n')
                c = fgetc(source);
            target->elements[i][j] = c;
        }
    }
    
    fclose(source);
    return 0;
}

void display(image screen, int height, int width)
{
    int i;
    
    if(height >= PROMPT_HEIGHT)
        if(width >= PROMPT_WIDTH)
            for(i = 0; i < PROMPT_HEIGHT*PROMPT_WIDTH; i++)
                printf("%c", screen.elements[(int)(i/PROMPT_WIDTH)][i%PROMPT_WIDTH]);//full prompt
        else
            for(i = 0; i < PROMPT_HEIGHT*width; i++)
                if((i % width) != (width-1))//if the rest of the division of the counter and the width is not the same as the width-1
                    printf("\xB0");//thinner than prompt
                else
                    printf("\xB0\n");
    else
        if(width >= PROMPT_WIDTH)
            for(i = 0; i < height*PROMPT_WIDTH; i++)
                printf("%c", screen.elements[(int)(i/PROMPT_WIDTH)][i%PROMPT_WIDTH]);//shorter than prompt
        else
            for(i = 0; i < height*width; i++)
                if((i % width) != (width-1))
                    printf("%c", screen.elements[(int)(i/width)][i%width]);//smaller than prompt(both)
                else
                    printf("\n");
    return;
}
