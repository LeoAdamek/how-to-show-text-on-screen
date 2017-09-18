#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

/* ICU Headers */
#include <unicode/utypes.h>    /* ICU Core Types */
#include <unicode/ustring.h>   /* ICU String functions  */

/* FreeType2 Headers */
#include <ft2build.h>          /* FreeType2 Build Configuration */
#include FT_FREETYPE_H         /* FreeType2 headers as defined in ft2build.h */

/* Harfbuzz Headers */
#include <harfbuzz/hb.h>       /* Harfbuzz Core Defintiions */
#include <harfbuzz/hb-ft.h>    /* Harfbuzz FreeType integration */
#include <harfbuzz/hb-icu.h>   /* Harfbuzz ICU Integration */


/* Cairo Headers */
#include <cairo/cairo.h>       /* Cairo Core Definitions */
#include <cairo/cairo-ft.h>    /* Cairo FreeType integration */

/* SDL Headers */
#include <SDL/SDL.h>

#define PT_SIZE 72.0
#define DPI     90

#define VIDEO_WIDTH 800
#define VIDEO_HEIGHT 600
#define VIDEO_FLAGS SDL_SWSURFACE | SDL_RESIZABLE | SDL_DOUBLEBUF

int main(const int argc, const char** argv) {
    /* The text we want to render */
    UChar* text = u"नमस्ते";

    /* Fonts and typeface related variables */
    FT_Library library;
    FT_Face font;
    cairo_font_face_t *cairo_font;
    hb_font_t *hb_font;
    hb_face_t *hb_face;

    /* Rendering Areas */
    SDL_Surface *screen, *surface;
    cairo_surface_t *cairo_surface;
    cairo_t *cr;
    hb_buffer_t *hb_buff;

    SDL_Event event;

    /* Text rendering data */
    unsigned int glyph_count, string_width_in_px;
    hb_glyph_info_t *glyph_info;
    hb_glyph_position_t *glyph_positions;
    cairo_glyph_t *cairo_glyphs;

    unsigned int i,x,y;
    int done = 0;

    assert(!FT_Init_FreeType(&library));

    if (FT_New_Face(library, "font.ttf", 0, &font) != 0) {
        fprintf(stderr, "Unable to load font");
        FT_Done_FreeType(library);
        exit(EXIT_FAILURE);
    }
    
    FT_Set_Char_Size(font, 0, PT_SIZE, DPI, DPI);

    cairo_font = cairo_ft_font_face_create_for_ft_face(font, 0);
    hb_font = hb_ft_font_create(font, NULL);
    hb_face = hb_ft_face_create(font, NULL);

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "Unable to initialize window");
        exit(EXIT_FAILURE);
    }

    SDL_WM_SetCaption("How to show text on the screen","How to show text on the screen");

    screen = SDL_SetVideoMode(VIDEO_WIDTH, VIDEO_HEIGHT, 32, VIDEO_FLAGS);

    SDL_EnableKeyRepeat(300, 100);
    SDL_EnableUNICODE(1);

    surface = SDL_CreateRGBSurface(
                                   VIDEO_FLAGS,
                                   VIDEO_WIDTH,
                                   VIDEO_HEIGHT,
                                   32,
                                   0x00ff0000,
                                   0x0000ff00,
                                   0x000000ff,
                                   0x00000000
                                   );

    /* Fill the space with WHITE (0xFFFFFFFF) */
    SDL_FillRect(surface, NULL, 0xffffffff);

    /* Pass the address of the SDL window data to Cairo so it can be drawn on */
    cairo_surface = cairo_image_surface_create_for_data(
                                                        (unsigned char*)surface->pixels,
                                                        CAIRO_FORMAT_RGB24,
                                                        surface->w,
                                                        surface->h,
                                                        surface->pitch
                                                        );
    
    /* Create a cairo resource to pass to drawing functions */
    cr = cairo_create(cairo_surface);
    
    /* Create a buffer in which to lay out the text */
    hb_buff = hb_buffer_create();
    hb_buffer_set_unicode_funcs(hb_buff, hb_icu_get_unicode_funcs());

    hb_buffer_set_direction(hb_buff, HB_DIRECTION_LTR);
    hb_buffer_set_script(hb_buff, HB_SCRIPT_DEVANAGARI);
    hb_buffer_set_language(hb_buff, hb_language_from_string("hi", strlen("hi")));

    /* Put our text in the buffer so Harfbuzz knows what we want to show */
    hb_buffer_add_utf16(hb_buff, text, u_strlen(text), 0, u_strlen(text));

    /* Lay out our text so we know where to put which glyphs */
    hb_shape(hb_font, hb_buff, NULL, 0);

    /* Get the data so we can use it in cairo */
    glyph_info = hb_buffer_get_glyph_infos(hb_buff, &glyph_count);
    glyph_positions = hb_buffer_get_glyph_positions(hb_buff, &glyph_count);
    cairo_glyphs = malloc(glyph_count * sizeof(cairo_glyph_t));

    x = 80;
    y = 80;
    /* Copy the Harfbuzz Glyph position data to Cairo position data */
    for( i = 0; i < glyph_count; ++i) {
        string_width_in_px += glyph_positions[i].x_advance/64;
        cairo_glyphs[i].index = glyph_info[i].codepoint;
        cairo_glyphs[i].x = x + (glyph_positions[i].x_offset);
        cairo_glyphs[i].y = y - (glyph_positions[i].y_offset);
        x += glyph_positions[i].x_advance;
        y -= glyph_positions[i].y_advance;
    }

    /* We no longer need these values so we can free them */
    free(glyph_info);
    free(glyph_positions);

    /* Set the text colour to black */
    cairo_set_source_rgba(cr, 0 , 0, 0, 1);
    cairo_set_font_face(cr, cairo_font);
    cairo_set_font_size(cr, PT_SIZE);

    cairo_show_glyphs(cr, cairo_glyphs, glyph_count);
    free(cairo_glyphs);

    hb_buffer_destroy(hb_buff);
    hb_font_destroy(hb_font);

    /* Show our rendered text on screen */
    SDL_BlitSurface(surface, NULL, screen, NULL);
    SDL_Flip(screen);

    cairo_surface_destroy(cairo_surface);
    cairo_destroy(cr);

    /* Wait for quit button pressed or ESC key. */
    while(!done) {
        /* Keep the SDL window open until we quit */
        while(SDL_PollEvent(&event)) {
            switch(event.type) {
            case SDL_KEYDOWN:
                if (event.key.keysym.sym == SDLK_ESCAPE) done = 1;
                break;
            case SDL_QUIT:
                done = 1;
                break;

            }
            
            SDL_Delay(1);
        }
    }

    FT_Done_Face(font);
    FT_Done_FreeType(library);

    SDL_FreeSurface(surface);
    SDL_Quit();

    exit(EXIT_SUCCESS);
}
