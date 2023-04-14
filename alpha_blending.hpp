#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <SFML/Graphics.hpp>
#include <immintrin.h>
#include <string.h>

typedef struct im_sizes
{
    int bk_width;
    int bk_height;
    int fr_width;
    int fr_height;
} imsizes_t;

const char BKGND_PATH[] = "images/source/background.bmp";
const char FRGND_PATH[] = "images/source/foreground.bmp";
const char RES_PATH[]   = "images/result/result.jpg";

void MakeBlending(sf::Image* bkgnd_im, sf::Image* frgnd_im, int offset_x, int offset_y, imsizes_t* IM_SIZES);
void MakeBlendingAVX(sf::Image* bkgnd_im, sf::Image* frgnd_im, int offset_x, int offset_y, imsizes_t* IM_SIZES);