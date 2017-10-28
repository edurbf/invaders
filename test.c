#include<stdio.h>
#include<stdlib.h>
#include<conio.h>

#define PROMPT_WIDTH 80
#define PROMPT_HEIGHT 24

typedef struct{
    char** elements;
    int height;
    int width;
    int vOffset;
    int hOffset;
    int size;
}image;

int load(FILE* source, image* target, int width, int height);
int display(image screen, int width, int height);
int overlay(image background, image foreground, image* target, int vOffset, int hOffset);
void genBorder();

int main(void)
{
    image background = {0};
    image foreground = {0};
    image screen = {0};
    image invader = {0};
    
    load(fopen("./files/background.txt", "r"), &background, 24, 80);
        printf("background: %d, %d, %d\n", background.height, background.width, background.size);
        display(background, 24, 80);
    load(fopen("./files/border.txt", "r"), &foreground, 24, 80);
        printf("border: %d, %d, %d\n", foreground.height, foreground.width, foreground.size);
        display(foreground, 24, 80);
    load(fopen("./files/invader.txt", "r"), &invader, 24, 80);
        printf("logo: %d, %d, %d\n", invader.height, invader.width, invader.size);
        display(invader, 24, 80);
    overlay(background, foreground, &screen, 0, 0);
        printf("screen with border: %d, %d, %d\n", screen.height, screen.width, screen.size);
        if(display(screen, 24, 80))
            while(!kbhit()){};
    
    return 0;
}

///Places foreground(offset by params.) over background.
int overlay(image background, image foreground, image* target, int vOffset, int hOffset)
{
    int i;
    
    target->elements = (char**)calloc(background.height, sizeof(char*));
    for(i = 0; i < background.height; i++)
        target->elements[i] = (char*)calloc(background.width, sizeof(char));
    
    for(i = 0; i < background.size; i++)
    {
        target->elements[(int)(i/background.width)][i%background.width] = (foreground.elements[(int)(i/background.width)][i%background.width] == ' ')? 
                                                        background.elements[(int)(i/background.width)][i%background.width]:
                                                        foreground.elements[(int)(i/background.width)][i%background.width];
    }
    target->height = background.height;
    target->width = background.width;
    target->hOffset = background.hOffset;
    target->vOffset = background.vOffset;
    target->size = background.size;
}

int load(FILE* source, image* target, int height, int width)
{
    int i;
    char c;
    
    target->elements = (char**)calloc(height, sizeof(char*));
    for(i = 0; i < height; i++)
        target->elements[i] = (char*)calloc(width+1, sizeof(char));
    
    for(i = 0; i < height*width; i++)
    {
        c = fgetc(source);
        if(c == EOF)
        {
            goto success;
        }
        if(c == '#')
            while(c != '\n')
                c = fgetc(source);
        while(c == '\n')
            if(c == EOF)
                goto success;
            else
                c = fgetc(source);
        target->elements[(int)(i/width)][i%width] = c;
    }
    
    //I know. It's to re-utilize code. Could(and should) be done in a function but things just aren't working so well right now.
    //Ends the function returning true and setting important values.
    success:
        fclose(source);
        target->width = (i == 0)? 0: ((i-1)%width)+1;
        target->height = (int)(i/width);
        target->vOffset = 0;
        target->hOffset = 0;
        target->size = width*height;
        return 1;
}

int display(image screen, int height, int width)
{
    if((height < 1)||(width < 1)||((screen.width - screen.hOffset) < 1)||((screen.height - screen.vOffset) < 1))
        return 0;
    
    //Attempt at O(n). I have no idea if this really worked.
    int i;
    
    //If any of the to-be-printed lengths are greater the size of the prompt,
    //use the prompt size instead, else, use the length max. This prevents
    //indexes out of bounds.
    int wMax = (screen.width - screen.hOffset) <= PROMPT_WIDTH? (screen.width - screen.hOffset): PROMPT_WIDTH,
    hMax = (screen.height - screen.vOffset) <= PROMPT_HEIGHT? (screen.height - screen.vOffset): PROMPT_HEIGHT;
    
    //Check for redundancy
    if(height >= hMax)
        if(width >= wMax)
            for(i = 0; i < hMax*wMax; i++)
                printf("%c", screen.elements[(int)(i/wMax)][i%wMax]);//full prompt
        else
            for(i = 0; i < hMax*width; i++)
                if((i % width) != (width-1))//if the rest of the division of the counter and the width is not the same as the width-1
                    printf("\xB0");//thinner than prompt
                else
                    printf("\xB0\n");
    else
        if(width >= wMax)
            for(i = 0; i < height*wMax; i++)
                printf("%c", screen.elements[(int)(i/wMax)][i%wMax]);//shorter than prompt
        else
            for(i = 0; i < height*width; i++)
                if((i % width) != (width-1))
                    printf("%c", screen.elements[(int)(i/width)][i%width]);//smaller than prompt(both)
                else
                    printf("\n");
    return 1;
}

void genBorder()
{
    int i, j;
    
    FILE* overlay = fopen("./files/border.txt", "w");

    for(i = 0; i < 24; i++){
        for(j = 0; j < 80; j++){
            fputc((((i==0) || (i==24-1))?
                                        ((j==0) || (j==80-1))?
                                            '*'
                                            :'-'
                                        :((j==0) || (j==80-1))?
                                            '|'
                                            :' '), overlay);
        }
        fputc('\n', overlay);
    }
    
    fclose(overlay);
}
