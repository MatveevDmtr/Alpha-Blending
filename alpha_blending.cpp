#include "alpha_blending.hpp"

typedef sf::Uint8 INT;
typedef unsigned char BYTE;

const int NUM_MEASURES = 100;

const BYTE ZERO = 0x80;

int main()
{
    imsizes_t IM_SIZES = {};

    sf::Clock clock;
    sf::Time elapsed_time_AVX   = clock.getElapsedTime();
    sf::Time elapsed_time_NoAVX = clock.getElapsedTime();

    int count_measures = 0;
    float time_sum_avx = 0;
    float time_sum_noavx = 0;

    sf::Image bkgnd_im;
    bkgnd_im.loadFromFile(BKGND_PATH);
    IM_SIZES.bk_width  = bkgnd_im.getSize().x;
    IM_SIZES.bk_height = bkgnd_im.getSize().y;

    sf::Image frgnd_im;
    frgnd_im.loadFromFile(FRGND_PATH);
    IM_SIZES.fr_width  = frgnd_im.getSize().x;
    IM_SIZES.fr_height = frgnd_im.getSize().y;  

    sf::RenderWindow window(sf::VideoMode(IM_SIZES.bk_width, IM_SIZES.bk_height), "Alpha-blending", sf::Style::Close);

    sf::Texture texture;
    texture.loadFromImage(bkgnd_im);
    sf::Sprite img_sprite;
    img_sprite.setTexture(texture);

    while (count_measures < NUM_MEASURES)
    {
        clock.restart();
            MakeBlendingAVX(&bkgnd_im, &frgnd_im, 300, 200, &IM_SIZES);
        elapsed_time_AVX = clock.getElapsedTime();
        clock.restart();
            MakeBlending(&bkgnd_im, &frgnd_im, 300, 200, &IM_SIZES);
        elapsed_time_NoAVX = clock.getElapsedTime();
        //printf("elapsed time: %f\n", elapsed_time_AVX.asSeconds());
        time_sum_avx += 1/elapsed_time_AVX.asSeconds()/100;
        time_sum_noavx += 1/elapsed_time_NoAVX.asSeconds()/100;
        count_measures++;
    }
    
    printf("end of measuring\n");

    bkgnd_im.saveToFile(RES_PATH);

    //printf("It's blending time: %f\n", elapsed_time.asSeconds());

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
            {        
                window.close();
            }
        }
        
        #if DRAW
            texture.update(bkgnd_im);
            window.clear();
            window.draw (img_sprite);
            window.display();
        #endif
    }

    printf("Averaged AVX FPS: %.2f\n", time_sum_avx/NUM_MEASURES);
    printf("Averaged No AVX FPS: %.2f\n", time_sum_noavx/NUM_MEASURES);
}

void MakeBlending(sf::Image* bkgnd_im, sf::Image* frgnd_im, int offset_x, int offset_y, imsizes_t* IM_SIZES)
{
    INT* bk_pixels = (INT*) bkgnd_im->getPixelsPtr();
    INT* fr_pixels = (INT*) frgnd_im->getPixelsPtr();

    for (int y_fr = 0; y_fr < IM_SIZES->fr_height; y_fr++)
    {
        int y_bk_shift = (y_fr + offset_y) * IM_SIZES->bk_width;
        int y_fr_shift = y_fr * IM_SIZES->fr_width;
        
        for (int x_fr = 0; x_fr < IM_SIZES->fr_width; x_fr++)
        {
            INT* bkgnd_pixel =  bk_pixels + 4 * (y_bk_shift + x_fr + offset_x);
            INT* frgnd_pixel =  fr_pixels + 4 * (y_fr_shift + x_fr);

            INT alpha = *(frgnd_pixel+3);

            for (int i = 0; i < 3; i++)             // 3 iterations: r, g, b
            {
                *(bkgnd_pixel) = ( ((*(frgnd_pixel) * alpha) + (*(bkgnd_pixel) * (255 - alpha)))) >> 8;
                bkgnd_pixel++;
                frgnd_pixel++;
            }
        }
    }
}

void MakeBlendingAVX(sf::Image* bkgnd_im, sf::Image* frgnd_im, int offset_x, int offset_y, imsizes_t* IM_SIZES)
{
    volatile INT* bk_pixels = (INT*) bkgnd_im->getPixelsPtr();
    volatile INT* fr_pixels = (INT*) frgnd_im->getPixelsPtr();

    volatile __m256i set1_255_arr = _mm256_set1_epi16(255);

    for (int y_fr = 0; y_fr < IM_SIZES->fr_height; y_fr++)
    {
        volatile int y_bk_shift = (y_fr + offset_y) * IM_SIZES->bk_width;
        volatile int y_fr_shift = y_fr * IM_SIZES->fr_width;
        
        for (volatile int x_fr = 0; x_fr < IM_SIZES->fr_width; x_fr+=4)
        {
            volatile INT* bkgnd_pixel_ptr =  bk_pixels + 4 * (y_bk_shift + x_fr + offset_x);
            volatile INT* frgnd_pixel_ptr =  fr_pixels + 4 * (y_fr_shift + x_fr);

            volatile __m128i bkgnd_arr_16 = _mm_loadu_si128 ((__m128i*) bkgnd_pixel_ptr);
            volatile __m128i frgnd_arr_16 = _mm_loadu_si128 ((__m128i*) frgnd_pixel_ptr);

            volatile __m256i bkgnd_arr_32 = _mm256_cvtepi8_epi16 (bkgnd_arr_16);
            volatile __m256i frgnd_arr_32 = _mm256_cvtepi8_epi16 (frgnd_arr_16);

            volatile __m256i alpha_mask = _mm256_set_epi8 (ZERO, ZERO, ZERO, 14, ZERO, 14, ZERO, 14, 
                                                  ZERO, ZERO, ZERO,  6, ZERO,  6, ZERO,  6,
                                                  ZERO, ZERO, ZERO, 14, ZERO, 14, ZERO, 14, 
                                                  ZERO, ZERO, ZERO,  6, ZERO,  6, ZERO,  6);

            volatile __m256i alpha_arr = _mm256_shuffle_epi8(frgnd_arr_32, alpha_mask);

            volatile __m256i bkgnd_255_sub_alpha = _mm256_sub_epi16 (set1_255_arr, alpha_arr);

            volatile __m256i frgnd_multiplied = _mm256_mullo_epi16 (frgnd_arr_32, alpha_arr);
            volatile __m256i bkgnd_multiplied = _mm256_mullo_epi16 (bkgnd_arr_32, bkgnd_255_sub_alpha);

            volatile __m256i sum_bk_fr = _mm256_add_epi16 (frgnd_multiplied, bkgnd_multiplied);

            volatile __m256i sum_mask = _mm256_set_epi8 (15,    13,   11,    9,    7,    5,    3,    1,
                                                ZERO,  ZERO, ZERO,  ZERO, ZERO, ZERO, ZERO, ZERO,
                                                ZERO,  ZERO, ZERO,  ZERO, ZERO, ZERO, ZERO, ZERO,
                                                15,    13,   11,    9,    7,    5,    3,    1);

            volatile __m256i sum = _mm256_shuffle_epi8 (sum_bk_fr, sum_mask);

            volatile __m128i color = _mm_add_epi64(_mm256_extracti128_si256(sum, 1), _mm256_extracti128_si256(sum, 0));


            #if DRAW
                _mm_store_si128 ((__m128i *) bkgnd_pixel_ptr, color);
            #endif
        }
    }
}