//
// LuaBindings.cpp
// LuaRunner2 - Lua bindings for SuperTerminal API
//
// This file provides Lua bindings for all SuperTerminal C API functions.
// Functions are registered in the global Lua namespace.
//

#include "LuaBindings.h"
#include "../Framework/API/superterminal_api.h"
#include "../Framework/API/st_api_circles.h"
#include "../Framework/API/st_api_polygons.h"
#include "../Framework/API/st_api_collision.h"
#include "../Framework/API/st_api_video_mode.h"
#include "../Framework/API/st_api_video_palette.h"
#include "../Framework/Particles/ParticleSystem.h"
#include "../FBRunner3/IndexedTileBindings.h"
#include <lua.hpp>
#include <string>
#include <cstring>

using SuperTerminal::ParticleMode;

namespace LuaRunner2 {

// =============================================================================
// Helper Functions
// =============================================================================

static void luaL_setglobalfunction(lua_State* L, const char* name, lua_CFunction func) {
    lua_pushcfunction(L, func);
    lua_setglobal(L, name);
}

static void luaL_setglobalnumber(lua_State* L, const char* name, lua_Number value) {
    lua_pushnumber(L, value);
    lua_setglobal(L, name);
}

// =============================================================================
// Text API Bindings
// =============================================================================

static int lua_st_text_putchar(lua_State* L) {
    int x = luaL_checkinteger(L, 1);
    int y = luaL_checkinteger(L, 2);
    const char* str = luaL_checkstring(L, 3);
    uint32_t fg = luaL_optinteger(L, 4, 0xFFFFFFFF);
    uint32_t bg = luaL_optinteger(L, 5, 0xFF000000);

    if (str && str[0]) {
        st_text_putchar(x, y, str[0], fg, bg);
    }
    return 0;
}

// poke_text: directly write a 32-bit character code to text grid
static int lua_poke_text(lua_State* L) {
    int x = luaL_checkinteger(L, 1);
    int y = luaL_checkinteger(L, 2);
    uint32_t character = luaL_checkinteger(L, 3);
    uint32_t fg = luaL_optinteger(L, 4, 0xFFFFFFFF);
    uint32_t bg = luaL_optinteger(L, 5, 0xFF000000);

    st_text_putchar(x, y, character, fg, bg);
    return 0;
}

static int lua_st_text_put(lua_State* L) {
    int x = luaL_checkinteger(L, 1);
    int y = luaL_checkinteger(L, 2);
    const char* text = luaL_checkstring(L, 3);
    uint32_t fg = luaL_optinteger(L, 4, 0xFFFFFFFF);
    uint32_t bg = luaL_optinteger(L, 5, 0xFF000000);

    st_text_put(x, y, text, fg, bg);
    return 0;
}

static int lua_st_text_clear(lua_State* L) {
    (void)L;
    st_text_clear();
    return 0;
}

static int lua_st_text_clear_region(lua_State* L) {
    int x = luaL_checkinteger(L, 1);
    int y = luaL_checkinteger(L, 2);
    int width = luaL_checkinteger(L, 3);
    int height = luaL_checkinteger(L, 4);

    st_text_clear_region(x, y, width, height);
    return 0;
}

static int lua_st_text_set_size(lua_State* L) {
    int width = luaL_checkinteger(L, 1);
    int height = luaL_checkinteger(L, 2);

    st_text_set_size(width, height);
    return 0;
}

static int lua_st_text_get_size(lua_State* L) {
    int width, height;
    st_text_get_size(&width, &height);

    lua_pushinteger(L, width);
    lua_pushinteger(L, height);
    return 2;
}

static int lua_st_text_scroll(lua_State* L) {
    int lines = luaL_checkinteger(L, 1);
    st_text_scroll(lines);
    return 0;
}

// =============================================================================
// Sixel Graphics API Bindings
// =============================================================================

static int lua_st_text_putsixel(lua_State* L) {
    int x = luaL_checkinteger(L, 1);
    int y = luaL_checkinteger(L, 2);
    uint32_t sixel_char = luaL_checkinteger(L, 3);
    uint32_t bg = luaL_optinteger(L, 5, 0xFF000000);

    // Get the colors table (6 elements)
    luaL_checktype(L, 4, LUA_TTABLE);
    uint8_t colors[6];
    for (int i = 0; i < 6; i++) {
        lua_rawgeti(L, 4, i + 1); // Lua arrays are 1-indexed
        colors[i] = (uint8_t)luaL_checkinteger(L, -1);
        lua_pop(L, 1);
    }

    st_text_putsixel(x, y, sixel_char, colors, bg);
    return 0;
}

static int lua_st_sixel_pack_colors(lua_State* L) {
    // Get the colors table (6 elements)
    luaL_checktype(L, 1, LUA_TTABLE);
    uint8_t colors[6];
    for (int i = 0; i < 6; i++) {
        lua_rawgeti(L, 1, i + 1); // Lua arrays are 1-indexed
        colors[i] = (uint8_t)luaL_checkinteger(L, -1);
        lua_pop(L, 1);
    }

    uint32_t packed = st_sixel_pack_colors(colors);
    lua_pushinteger(L, packed);
    return 1;
}

static int lua_st_text_putsixel_packed(lua_State* L) {
    int x = luaL_checkinteger(L, 1);
    int y = luaL_checkinteger(L, 2);
    uint32_t sixel_char = luaL_checkinteger(L, 3);
    uint32_t packed_colors = luaL_checkinteger(L, 4);
    uint32_t bg = luaL_optinteger(L, 5, 0xFF000000);

    st_text_putsixel_packed(x, y, sixel_char, packed_colors, bg);
    return 0;
}

static int lua_st_sixel_set_stripe(lua_State* L) {
    int x = luaL_checkinteger(L, 1);
    int y = luaL_checkinteger(L, 2);
    int stripe_index = luaL_checkinteger(L, 3);
    uint8_t color_index = (uint8_t)luaL_checkinteger(L, 4);

    st_sixel_set_stripe(x, y, stripe_index, color_index);
    return 0;
}

static int lua_st_sixel_get_stripe(lua_State* L) {
    int x = luaL_checkinteger(L, 1);
    int y = luaL_checkinteger(L, 2);
    int stripe_index = luaL_checkinteger(L, 3);

    uint8_t color = st_sixel_get_stripe(x, y, stripe_index);
    lua_pushinteger(L, color);
    return 1;
}

static int lua_st_sixel_gradient(lua_State* L) {
    int x = luaL_checkinteger(L, 1);
    int y = luaL_checkinteger(L, 2);
    uint8_t top_color = (uint8_t)luaL_checkinteger(L, 3);
    uint8_t bottom_color = (uint8_t)luaL_checkinteger(L, 4);
    uint32_t bg = luaL_optinteger(L, 5, 0xFF000000);

    st_sixel_gradient(x, y, top_color, bottom_color, bg);
    return 0;
}

static int lua_st_sixel_hline(lua_State* L) {
    int x = luaL_checkinteger(L, 1);
    int y = luaL_checkinteger(L, 2);
    int width = luaL_checkinteger(L, 3);
    uint32_t bg = luaL_optinteger(L, 5, 0xFF000000);

    // Get the colors table (6 elements)
    luaL_checktype(L, 4, LUA_TTABLE);
    uint8_t colors[6];
    for (int i = 0; i < 6; i++) {
        lua_rawgeti(L, 4, i + 1);
        colors[i] = (uint8_t)luaL_checkinteger(L, -1);
        lua_pop(L, 1);
    }

    st_sixel_hline(x, y, width, colors, bg);
    return 0;
}

static int lua_st_sixel_fill_rect(lua_State* L) {
    int x = luaL_checkinteger(L, 1);
    int y = luaL_checkinteger(L, 2);
    int width = luaL_checkinteger(L, 3);
    int height = luaL_checkinteger(L, 4);
    uint32_t bg = luaL_optinteger(L, 6, 0xFF000000);

    // Get the colors table (6 elements)
    luaL_checktype(L, 5, LUA_TTABLE);
    uint8_t colors[6];
    for (int i = 0; i < 6; i++) {
        lua_rawgeti(L, 5, i + 1);
        colors[i] = (uint8_t)luaL_checkinteger(L, -1);
        lua_pop(L, 1);
    }

    st_sixel_fill_rect(x, y, width, height, colors, bg);
    return 0;
}

// =============================================================================
// Graphics Mode Switching API Bindings
// =============================================================================

// Switch to TEXT mode (mode 0)
static int lua_st_text_mode(lua_State* L) {
    (void)L;
    st_mode(0);
    return 0;
}

// Switch to LORES mode (mode 1: 160×75 pixels)
static int lua_st_lores(lua_State* L) {
    (void)L;
    st_mode(1);
    return 0;
}

// Switch to MEDIUMRES mode (mode 2: 320×150 pixels)
static int lua_st_mediumres(lua_State* L) {
    (void)L;
    st_mode(2);
    return 0;
}

// Switch to HIGHRES mode (mode 3: 640×300 pixels)
static int lua_st_highres(lua_State* L) {
    (void)L;
    st_mode(3);
    return 0;
}

// Switch to ULTRARES mode (mode 4: 1280×720 direct color)
static int lua_st_ultrares(lua_State* L) {
    (void)L;
    st_mode(4);
    return 0;
}

// Switch to XRES mode (mode 5: 320×240 with 256-color palette)
static int lua_st_xres(lua_State* L) {
    (void)L;
    st_mode(5);
    return 0;
}

static int lua_st_wres(lua_State* L) {
    (void)L;
    st_mode(6);
    return 0;
}

// Generic mode switch function that takes a mode number
static int lua_st_mode(lua_State* L) {
    int mode = luaL_checkinteger(L, 1);
    st_mode(mode);
    return 0;
}

// =============================================================================
// LORES Pixel Buffer API Bindings
// =============================================================================

// Set a pixel in LORES mode (works in LORES/MEDIUMRES/HIRES)
static int lua_st_lores_pset(lua_State* L) {
    int x = luaL_checkinteger(L, 1);
    int y = luaL_checkinteger(L, 2);
    uint8_t color = (uint8_t)luaL_checkinteger(L, 3);
    uint32_t bg = luaL_optinteger(L, 4, 0xFF000000);  // default black
    st_lores_pset(x, y, color, bg);
    return 0;
}

// Draw a line in LORES mode
static int lua_st_lores_line(lua_State* L) {
    int x1 = luaL_checkinteger(L, 1);
    int y1 = luaL_checkinteger(L, 2);
    int x2 = luaL_checkinteger(L, 3);
    int y2 = luaL_checkinteger(L, 4);
    uint8_t color = (uint8_t)luaL_checkinteger(L, 5);
    uint32_t bg = luaL_optinteger(L, 6, 0xFF000000);
    st_lores_line(x1, y1, x2, y2, color, bg);
    return 0;
}

// Draw a rectangle outline in LORES mode
static int lua_st_lores_rect(lua_State* L) {
    int x = luaL_checkinteger(L, 1);
    int y = luaL_checkinteger(L, 2);
    int width = luaL_checkinteger(L, 3);
    int height = luaL_checkinteger(L, 4);
    uint8_t color = (uint8_t)luaL_checkinteger(L, 5);
    uint32_t bg = luaL_optinteger(L, 6, 0xFF000000);
    st_lores_rect(x, y, width, height, color, bg);
    return 0;
}

// Draw a filled rectangle in LORES mode
static int lua_st_lores_fillrect(lua_State* L) {
    int x = luaL_checkinteger(L, 1);
    int y = luaL_checkinteger(L, 2);
    int width = luaL_checkinteger(L, 3);
    int height = luaL_checkinteger(L, 4);
    uint8_t color = (uint8_t)luaL_checkinteger(L, 5);
    uint32_t bg = luaL_optinteger(L, 6, 0xFF000000);
    st_lores_fillrect(x, y, width, height, color, bg);
    return 0;
}

// Draw a horizontal line in LORES mode
static int lua_st_lores_hline(lua_State* L) {
    int x = luaL_checkinteger(L, 1);
    int y = luaL_checkinteger(L, 2);
    int width = luaL_checkinteger(L, 3);
    uint8_t color = (uint8_t)luaL_checkinteger(L, 4);
    uint32_t bg = luaL_optinteger(L, 5, 0xFF000000);
    st_lores_hline(x, y, width, color, bg);
    return 0;
}

// Draw a vertical line in LORES mode
static int lua_st_lores_vline(lua_State* L) {
    int x = luaL_checkinteger(L, 1);
    int y = luaL_checkinteger(L, 2);
    int height = luaL_checkinteger(L, 3);
    uint8_t color = (uint8_t)luaL_checkinteger(L, 4);
    uint32_t bg = luaL_optinteger(L, 5, 0xFF000000);
    st_lores_vline(x, y, height, color, bg);
    return 0;
}

// Clear LORES buffer
static int lua_st_lores_clear(lua_State* L) {
    uint32_t bg = luaL_optinteger(L, 1, 0xFF000000);
    st_lores_clear(bg);
    return 0;
}

// Get LORES resolution
static int lua_st_lores_resolution(lua_State* L) {
    int width, height;
    st_lores_resolution(&width, &height);
    lua_pushinteger(L, width);
    lua_pushinteger(L, height);
    return 2;
}

// Select active LORES buffer (0 or 1)
static int lua_st_lores_buffer(lua_State* L) {
    int bufferID = luaL_checkinteger(L, 1);
    st_lores_buffer(bufferID);
    return 0;
}

// Get active LORES buffer ID
static int lua_st_lores_buffer_get(lua_State* L) {
    int bufferID = st_lores_buffer_get();
    lua_pushinteger(L, bufferID);
    return 1;
}

// Flip LORES buffers (double buffering)
static int lua_st_lores_flip(lua_State* L) {
    (void)L;
    st_lores_flip();
    return 0;
}

// Blit within LORES buffer
static int lua_st_lores_blit(lua_State* L) {
    int src_x = luaL_checkinteger(L, 1);
    int src_y = luaL_checkinteger(L, 2);
    int width = luaL_checkinteger(L, 3);
    int height = luaL_checkinteger(L, 4);
    int dst_x = luaL_checkinteger(L, 5);
    int dst_y = luaL_checkinteger(L, 6);
    st_lores_blit(src_x, src_y, width, height, dst_x, dst_y);
    return 0;
}

// Blit with transparency
static int lua_st_lores_blit_trans(lua_State* L) {
    int src_x = luaL_checkinteger(L, 1);
    int src_y = luaL_checkinteger(L, 2);
    int width = luaL_checkinteger(L, 3);
    int height = luaL_checkinteger(L, 4);
    int dst_x = luaL_checkinteger(L, 5);
    int dst_y = luaL_checkinteger(L, 6);
    uint8_t transparent_color = (uint8_t)luaL_checkinteger(L, 7);
    st_lores_blit_trans(src_x, src_y, width, height, dst_x, dst_y, transparent_color);
    return 0;
}

// Set LORES palette
static int lua_st_lores_palette_set(lua_State* L) {
    const char* mode = luaL_checkstring(L, 1);
    st_lores_palette_set(mode);
    return 0;
}

// Poke LORES palette entry
static int lua_st_lores_palette_poke(lua_State* L) {
    int row = luaL_checkinteger(L, 1);
    int index = luaL_checkinteger(L, 2);
    uint32_t rgba = luaL_checkinteger(L, 3);
    st_lores_palette_poke(row, index, rgba);
    return 0;
}

// Peek LORES palette entry
static int lua_st_lores_palette_peek(lua_State* L) {
    int row = luaL_checkinteger(L, 1);
    int index = luaL_checkinteger(L, 2);
    uint32_t rgba = st_lores_palette_peek(row, index);
    lua_pushinteger(L, rgba);
    return 1;
}

// =============================================================================
// URES (Ultra Resolution) API Bindings
// =============================================================================

// Set a pixel in URES mode (1280×720 direct color)
static int lua_st_ures_pset(lua_State* L) {
    int x = luaL_checkinteger(L, 1);
    int y = luaL_checkinteger(L, 2);
    int color = luaL_checkinteger(L, 3);
    st_ures_pset(x, y, color);
    return 0;
}

// Get a pixel in URES mode
static int lua_st_ures_pget(lua_State* L) {
    int x = luaL_checkinteger(L, 1);
    int y = luaL_checkinteger(L, 2);
    int color = st_ures_pget(x, y);
    lua_pushinteger(L, color);
    return 1;
}

// Clear URES buffer
static int lua_st_ures_clear(lua_State* L) {
    int color = luaL_checkinteger(L, 1);
    st_ures_clear(color);
    return 0;
}

// Fill rectangle in URES mode
static int lua_st_ures_fillrect(lua_State* L) {
    int x = luaL_checkinteger(L, 1);
    int y = luaL_checkinteger(L, 2);
    int width = luaL_checkinteger(L, 3);
    int height = luaL_checkinteger(L, 4);
    int color = luaL_checkinteger(L, 5);
    st_ures_fillrect(x, y, width, height, color);
    return 0;
}

// Draw horizontal line in URES mode
static int lua_st_ures_hline(lua_State* L) {
    int x = luaL_checkinteger(L, 1);
    int y = luaL_checkinteger(L, 2);
    int width = luaL_checkinteger(L, 3);
    int color = luaL_checkinteger(L, 4);
    st_ures_hline(x, y, width, color);
    return 0;
}

// Draw vertical line in URES mode
static int lua_st_ures_vline(lua_State* L) {
    int x = luaL_checkinteger(L, 1);
    int y = luaL_checkinteger(L, 2);
    int height = luaL_checkinteger(L, 3);
    int color = luaL_checkinteger(L, 4);
    st_ures_vline(x, y, height, color);
    return 0;
}

// URES buffer management
static int lua_st_ures_buffer(lua_State* L) {
    int bufferID = luaL_checkinteger(L, 1);
    st_ures_buffer(bufferID);
    return 0;
}

static int lua_st_ures_buffer_get(lua_State* L) {
    int bufferID = st_ures_buffer_get();
    lua_pushinteger(L, bufferID);
    return 1;
}

static int lua_st_ures_flip(lua_State* L) {
    st_ures_flip();
    return 0;
}

static int lua_st_ures_gpu_flip(lua_State* L) {
    st_ures_gpu_flip();
    return 0;
}

static int lua_st_ures_sync(lua_State* L) {
    int bufferID = luaL_checkinteger(L, 1);
    st_ures_sync(bufferID);
    return 0;
}

static int lua_st_ures_swap(lua_State* L) {
    int bufferID = luaL_checkinteger(L, 1);
    st_ures_swap(bufferID);
    return 0;
}

static int lua_st_ures_blit_from(lua_State* L) {
    int srcBufferID = luaL_checkinteger(L, 1);
    int srcX = luaL_checkinteger(L, 2);
    int srcY = luaL_checkinteger(L, 3);
    int width = luaL_checkinteger(L, 4);
    int height = luaL_checkinteger(L, 5);
    int dstX = luaL_checkinteger(L, 6);
    int dstY = luaL_checkinteger(L, 7);
    st_ures_blit_from(srcBufferID, srcX, srcY, width, height, dstX, dstY);
    return 0;
}

static int lua_st_ures_blit_from_trans(lua_State* L) {
    int srcBufferID = luaL_checkinteger(L, 1);
    int srcX = luaL_checkinteger(L, 2);
    int srcY = luaL_checkinteger(L, 3);
    int width = luaL_checkinteger(L, 4);
    int height = luaL_checkinteger(L, 5);
    int dstX = luaL_checkinteger(L, 6);
    int dstY = luaL_checkinteger(L, 7);
    st_ures_blit_from_trans(srcBufferID, srcX, srcY, width, height, dstX, dstY);
    return 0;
}

// Create URES RGB color (opaque)
static int lua_st_urgb(lua_State* L) {
    int r = luaL_checkinteger(L, 1);
    int g = luaL_checkinteger(L, 2);
    int b = luaL_checkinteger(L, 3);
    int color = st_urgb(r, g, b);
    lua_pushinteger(L, color);
    return 1;
}

// Create URES RGBA color
static int lua_st_urgba(lua_State* L) {
    int r = luaL_checkinteger(L, 1);
    int g = luaL_checkinteger(L, 2);
    int b = luaL_checkinteger(L, 3);
    int a = luaL_checkinteger(L, 4);
    int color = st_urgba(r, g, b, a);
    lua_pushinteger(L, color);
    return 1;
}

// =============================================================================
// XRES (Mode X) API Bindings - 320×240 with 256-color palette
// =============================================================================

// Set a pixel in XRES mode (320×240, 8-bit palette index)
static int lua_st_xres_pset(lua_State* L) {
    int x = luaL_checkinteger(L, 1);
    int y = luaL_checkinteger(L, 2);
    int colorIndex = luaL_checkinteger(L, 3);
    st_xres_pset(x, y, colorIndex);
    return 0;
}

// Get a pixel in XRES mode
static int lua_st_xres_pget(lua_State* L) {
    int x = luaL_checkinteger(L, 1);
    int y = luaL_checkinteger(L, 2);
    int colorIndex = st_xres_pget(x, y);
    lua_pushinteger(L, colorIndex);
    return 1;
}

// Clear XRES buffer
static int lua_st_xres_clear(lua_State* L) {
    int colorIndex = luaL_checkinteger(L, 1);
    st_xres_clear(colorIndex);
    return 0;
}

// Fill rectangle in XRES mode
static int lua_st_xres_fillrect(lua_State* L) {
    int x = luaL_checkinteger(L, 1);
    int y = luaL_checkinteger(L, 2);
    int width = luaL_checkinteger(L, 3);
    int height = luaL_checkinteger(L, 4);
    int colorIndex = luaL_checkinteger(L, 5);
    st_xres_fillrect(x, y, width, height, colorIndex);
    return 0;
}

// Draw horizontal line in XRES mode
static int lua_st_xres_hline(lua_State* L) {
    int x = luaL_checkinteger(L, 1);
    int y = luaL_checkinteger(L, 2);
    int width = luaL_checkinteger(L, 3);
    int colorIndex = luaL_checkinteger(L, 4);
    st_xres_hline(x, y, width, colorIndex);
    return 0;
}

// Draw vertical line in XRES mode
static int lua_st_xres_vline(lua_State* L) {
    int x = luaL_checkinteger(L, 1);
    int y = luaL_checkinteger(L, 2);
    int height = luaL_checkinteger(L, 3);
    int colorIndex = luaL_checkinteger(L, 4);
    st_xres_vline(x, y, height, colorIndex);
    return 0;
}

// Set active XRES buffer
static int lua_st_xres_buffer(lua_State* L) {
    int bufferID = luaL_checkinteger(L, 1);
    st_xres_buffer(bufferID);
    return 0;
}

// Flip XRES buffers
static int lua_st_xres_flip(lua_State* L) {
    (void)L;
    st_xres_flip();
    return 0;
}

// Blit within XRES buffer
static int lua_st_xres_blit(lua_State* L) {
    int srcX = luaL_checkinteger(L, 1);
    int srcY = luaL_checkinteger(L, 2);
    int width = luaL_checkinteger(L, 3);
    int height = luaL_checkinteger(L, 4);
    int dstX = luaL_checkinteger(L, 5);
    int dstY = luaL_checkinteger(L, 6);
    st_xres_blit(srcX, srcY, width, height, dstX, dstY);
    return 0;
}

// Blit with transparency
static int lua_st_xres_blit_trans(lua_State* L) {
    int srcX = luaL_checkinteger(L, 1);
    int srcY = luaL_checkinteger(L, 2);
    int width = luaL_checkinteger(L, 3);
    int height = luaL_checkinteger(L, 4);
    int dstX = luaL_checkinteger(L, 5);
    int dstY = luaL_checkinteger(L, 6);
    st_xres_blit_trans(srcX, srcY, width, height, dstX, dstY);
    return 0;
}

// Blit from another buffer
static int lua_st_xres_blit_from(lua_State* L) {
    int srcBufferID = luaL_checkinteger(L, 1);
    int srcX = luaL_checkinteger(L, 2);
    int srcY = luaL_checkinteger(L, 3);
    int width = luaL_checkinteger(L, 4);
    int height = luaL_checkinteger(L, 5);
    int dstX = luaL_checkinteger(L, 6);
    int dstY = luaL_checkinteger(L, 7);
    st_xres_blit_from(srcBufferID, srcX, srcY, width, height, dstX, dstY);
    return 0;
}

// Blit from another buffer with transparency
static int lua_st_xres_blit_from_trans(lua_State* L) {
    int srcBufferID = luaL_checkinteger(L, 1);
    int srcX = luaL_checkinteger(L, 2);
    int srcY = luaL_checkinteger(L, 3);
    int width = luaL_checkinteger(L, 4);
    int height = luaL_checkinteger(L, 5);
    int dstX = luaL_checkinteger(L, 6);
    int dstY = luaL_checkinteger(L, 7);
    st_xres_blit_from_trans(srcBufferID, srcX, srcY, width, height, dstX, dstY);
    return 0;
}

// =============================================================================
// GPU-accelerated LORES blit functions
// =============================================================================

static int lua_st_lores_blit_gpu(lua_State* L) {
    int srcBufferID = luaL_checkinteger(L, 1);
    int dstBufferID = luaL_checkinteger(L, 2);
    int srcX = luaL_checkinteger(L, 3);
    int srcY = luaL_checkinteger(L, 4);
    int width = luaL_checkinteger(L, 5);
    int height = luaL_checkinteger(L, 6);
    int dstX = luaL_checkinteger(L, 7);
    int dstY = luaL_checkinteger(L, 8);
    st_lores_blit_gpu(srcBufferID, dstBufferID, srcX, srcY, width, height, dstX, dstY);
    return 0;
}

static int lua_st_lores_blit_trans_gpu(lua_State* L) {
    int srcBufferID = luaL_checkinteger(L, 1);
    int dstBufferID = luaL_checkinteger(L, 2);
    int srcX = luaL_checkinteger(L, 3);
    int srcY = luaL_checkinteger(L, 4);
    int width = luaL_checkinteger(L, 5);
    int height = luaL_checkinteger(L, 6);
    int dstX = luaL_checkinteger(L, 7);
    int dstY = luaL_checkinteger(L, 8);
    int transparentColor = luaL_checkinteger(L, 9);
    st_lores_blit_trans_gpu(srcBufferID, dstBufferID, srcX, srcY, width, height, dstX, dstY, transparentColor);
    return 0;
}

static int lua_st_lores_clear_gpu(lua_State* L) {
    int bufferID = luaL_checkinteger(L, 1);
    int colorIndex = luaL_checkinteger(L, 2);
    st_lores_clear_gpu(bufferID, colorIndex);
    return 0;
}

static int lua_st_lores_rect_fill_gpu(lua_State* L) {
    int bufferID = luaL_checkinteger(L, 1);
    int x = luaL_checkinteger(L, 2);
    int y = luaL_checkinteger(L, 3);
    int width = luaL_checkinteger(L, 4);
    int height = luaL_checkinteger(L, 5);
    int colorIndex = luaL_checkinteger(L, 6);
    st_lores_rect_fill_gpu(bufferID, x, y, width, height, colorIndex);
    return 0;
}

static int lua_st_lores_circle_fill_gpu(lua_State* L) {
    int bufferID = luaL_checkinteger(L, 1);
    int cx = luaL_checkinteger(L, 2);
    int cy = luaL_checkinteger(L, 3);
    int radius = luaL_checkinteger(L, 4);
    int colorIndex = luaL_checkinteger(L, 5);
    st_lores_circle_fill_gpu(bufferID, cx, cy, radius, colorIndex);
    return 0;
}

static int lua_st_lores_line_gpu(lua_State* L) {
    int bufferID = luaL_checkinteger(L, 1);
    int x0 = luaL_checkinteger(L, 2);
    int y0 = luaL_checkinteger(L, 3);
    int x1 = luaL_checkinteger(L, 4);
    int y1 = luaL_checkinteger(L, 5);
    int colorIndex = luaL_checkinteger(L, 6);
    st_lores_line_gpu(bufferID, x0, y0, x1, y1, colorIndex);
    return 0;
}

// =============================================================================
// GPU-accelerated XRES blit functions
// =============================================================================

static int lua_st_xres_blit_gpu(lua_State* L) {
    int srcBufferID = luaL_checkinteger(L, 1);
    int dstBufferID = luaL_checkinteger(L, 2);
    int srcX = luaL_checkinteger(L, 3);
    int srcY = luaL_checkinteger(L, 4);
    int width = luaL_checkinteger(L, 5);
    int height = luaL_checkinteger(L, 6);
    int dstX = luaL_checkinteger(L, 7);
    int dstY = luaL_checkinteger(L, 8);
    st_xres_blit_gpu(srcBufferID, dstBufferID, srcX, srcY, width, height, dstX, dstY);
    return 0;
}

static int lua_st_xres_blit_trans_gpu(lua_State* L) {
    int srcBufferID = luaL_checkinteger(L, 1);
    int dstBufferID = luaL_checkinteger(L, 2);
    int srcX = luaL_checkinteger(L, 3);
    int srcY = luaL_checkinteger(L, 4);
    int width = luaL_checkinteger(L, 5);
    int height = luaL_checkinteger(L, 6);
    int dstX = luaL_checkinteger(L, 7);
    int dstY = luaL_checkinteger(L, 8);
    int transparentColor = luaL_checkinteger(L, 9);
    st_xres_blit_trans_gpu(srcBufferID, dstBufferID, srcX, srcY, width, height, dstX, dstY, transparentColor);
    return 0;
}

// GPU synchronization
static int lua_st_gpu_sync(lua_State* L) {
    st_gpu_sync();
    return 0;
}

// GPU clear operations
static int lua_st_xres_clear_gpu(lua_State* L) {
    int bufferID = luaL_checkinteger(L, 1);
    int colorIndex = luaL_checkinteger(L, 2);
    st_xres_clear_gpu(bufferID, colorIndex);
    return 0;
}

static int lua_st_wres_clear_gpu(lua_State* L) {
    int bufferID = luaL_checkinteger(L, 1);
    int colorIndex = luaL_checkinteger(L, 2);
    st_wres_clear_gpu(bufferID, colorIndex);
    return 0;
}

// GPU Primitive Drawing
static int lua_st_xres_rect_fill_gpu(lua_State* L) {
    int bufferID = luaL_checkinteger(L, 1);
    int x = luaL_checkinteger(L, 2);
    int y = luaL_checkinteger(L, 3);
    int width = luaL_checkinteger(L, 4);
    int height = luaL_checkinteger(L, 5);
    int colorIndex = luaL_checkinteger(L, 6);
    st_xres_rect_fill_gpu(bufferID, x, y, width, height, colorIndex);
    return 0;
}

static int lua_st_wres_rect_fill_gpu(lua_State* L) {
    int bufferID = luaL_checkinteger(L, 1);
    int x = luaL_checkinteger(L, 2);
    int y = luaL_checkinteger(L, 3);
    int width = luaL_checkinteger(L, 4);
    int height = luaL_checkinteger(L, 5);
    int colorIndex = luaL_checkinteger(L, 6);
    st_wres_rect_fill_gpu(bufferID, x, y, width, height, colorIndex);
    return 0;
}

static int lua_st_xres_circle_fill_gpu(lua_State* L) {
    int bufferID = luaL_checkinteger(L, 1);
    int cx = luaL_checkinteger(L, 2);
    int cy = luaL_checkinteger(L, 3);
    int radius = luaL_checkinteger(L, 4);
    int colorIndex = luaL_checkinteger(L, 5);
    st_xres_circle_fill_gpu(bufferID, cx, cy, radius, colorIndex);
    return 0;
}

static int lua_st_wres_circle_fill_gpu(lua_State* L) {
    int bufferID = luaL_checkinteger(L, 1);
    int cx = luaL_checkinteger(L, 2);
    int cy = luaL_checkinteger(L, 3);
    int radius = luaL_checkinteger(L, 4);
    int colorIndex = luaL_checkinteger(L, 5);
    st_wres_circle_fill_gpu(bufferID, cx, cy, radius, colorIndex);
    return 0;
}

static int lua_st_xres_line_gpu(lua_State* L) {
    int bufferID = luaL_checkinteger(L, 1);
    int x0 = luaL_checkinteger(L, 2);
    int y0 = luaL_checkinteger(L, 3);
    int x1 = luaL_checkinteger(L, 4);
    int y1 = luaL_checkinteger(L, 5);
    int colorIndex = luaL_checkinteger(L, 6);
    st_xres_line_gpu(bufferID, x0, y0, x1, y1, colorIndex);
    return 0;
}

static int lua_st_wres_line_gpu(lua_State* L) {
    int bufferID = luaL_checkinteger(L, 1);
    int x0 = luaL_checkinteger(L, 2);
    int y0 = luaL_checkinteger(L, 3);
    int x1 = luaL_checkinteger(L, 4);
    int y1 = luaL_checkinteger(L, 5);
    int colorIndex = luaL_checkinteger(L, 6);
    st_wres_line_gpu(bufferID, x0, y0, x1, y1, colorIndex);
    return 0;
}

// GPU Anti-Aliased Primitive Drawing
static int lua_st_xres_circle_fill_aa(lua_State* L) {
    int bufferID = luaL_checkinteger(L, 1);
    int cx = luaL_checkinteger(L, 2);
    int cy = luaL_checkinteger(L, 3);
    int radius = luaL_checkinteger(L, 4);
    int colorIndex = luaL_checkinteger(L, 5);
    st_xres_circle_fill_aa(bufferID, cx, cy, radius, colorIndex);
    return 0;
}

static int lua_st_wres_circle_fill_aa(lua_State* L) {
    int bufferID = luaL_checkinteger(L, 1);
    int cx = luaL_checkinteger(L, 2);
    int cy = luaL_checkinteger(L, 3);
    int radius = luaL_checkinteger(L, 4);
    int colorIndex = luaL_checkinteger(L, 5);
    st_wres_circle_fill_aa(bufferID, cx, cy, radius, colorIndex);
    return 0;
}

static int lua_st_xres_line_aa(lua_State* L) {
    int bufferID = luaL_checkinteger(L, 1);
    int x0 = luaL_checkinteger(L, 2);
    int y0 = luaL_checkinteger(L, 3);
    int x1 = luaL_checkinteger(L, 4);
    int y1 = luaL_checkinteger(L, 5);
    int colorIndex = luaL_checkinteger(L, 6);
    float lineWidth = (float)luaL_optnumber(L, 7, 1.0);
    st_xres_line_aa(bufferID, x0, y0, x1, y1, colorIndex, lineWidth);
    return 0;
}

static int lua_st_wres_line_aa(lua_State* L) {
    int bufferID = luaL_checkinteger(L, 1);
    int x0 = luaL_checkinteger(L, 2);
    int y0 = luaL_checkinteger(L, 3);
    int x1 = luaL_checkinteger(L, 4);
    int y1 = luaL_checkinteger(L, 5);
    int colorIndex = luaL_checkinteger(L, 6);
    float lineWidth = (float)luaL_optnumber(L, 7, 1.0);
    st_wres_line_aa(bufferID, x0, y0, x1, y1, colorIndex, lineWidth);
    return 0;
}

// =============================================================================
// URES GPU Blitter Lua wrappers (Direct Color ARGB4444)
// =============================================================================

static int lua_st_ures_blit_copy_gpu(lua_State* L) {
    int srcBufferID = luaL_checkinteger(L, 1);
    int dstBufferID = luaL_checkinteger(L, 2);
    int srcX = luaL_checkinteger(L, 3);
    int srcY = luaL_checkinteger(L, 4);
    int width = luaL_checkinteger(L, 5);
    int height = luaL_checkinteger(L, 6);
    int dstX = luaL_checkinteger(L, 7);
    int dstY = luaL_checkinteger(L, 8);
    st_ures_blit_copy_gpu(srcBufferID, dstBufferID, srcX, srcY, width, height, dstX, dstY);
    return 0;
}

static int lua_st_ures_blit_transparent_gpu(lua_State* L) {
    int srcBufferID = luaL_checkinteger(L, 1);
    int dstBufferID = luaL_checkinteger(L, 2);
    int srcX = luaL_checkinteger(L, 3);
    int srcY = luaL_checkinteger(L, 4);
    int width = luaL_checkinteger(L, 5);
    int height = luaL_checkinteger(L, 6);
    int dstX = luaL_checkinteger(L, 7);
    int dstY = luaL_checkinteger(L, 8);
    st_ures_blit_transparent_gpu(srcBufferID, dstBufferID, srcX, srcY, width, height, dstX, dstY);
    return 0;
}

static int lua_st_ures_blit_alpha_composite_gpu(lua_State* L) {
    int srcBufferID = luaL_checkinteger(L, 1);
    int dstBufferID = luaL_checkinteger(L, 2);
    int srcX = luaL_checkinteger(L, 3);
    int srcY = luaL_checkinteger(L, 4);
    int width = luaL_checkinteger(L, 5);
    int height = luaL_checkinteger(L, 6);
    int dstX = luaL_checkinteger(L, 7);
    int dstY = luaL_checkinteger(L, 8);
    st_ures_blit_alpha_composite_gpu(srcBufferID, dstBufferID, srcX, srcY, width, height, dstX, dstY);
    return 0;
}

static int lua_st_ures_clear_gpu(lua_State* L) {
    int bufferID = luaL_checkinteger(L, 1);
    int color = luaL_checkinteger(L, 2);
    st_ures_clear_gpu(bufferID, color);
    return 0;
}

// =============================================================================
// URES GPU Primitive Drawing Lua wrappers (Direct Color ARGB4444)
// =============================================================================

static int lua_st_ures_rect_fill_gpu(lua_State* L) {
    int bufferID = luaL_checkinteger(L, 1);
    int x = luaL_checkinteger(L, 2);
    int y = luaL_checkinteger(L, 3);
    int width = luaL_checkinteger(L, 4);
    int height = luaL_checkinteger(L, 5);
    int color = luaL_checkinteger(L, 6);
    st_ures_rect_fill_gpu(bufferID, x, y, width, height, color);
    return 0;
}

static int lua_st_ures_circle_fill_gpu(lua_State* L) {
    int bufferID = luaL_checkinteger(L, 1);
    int cx = luaL_checkinteger(L, 2);
    int cy = luaL_checkinteger(L, 3);
    int radius = luaL_checkinteger(L, 4);
    int color = luaL_checkinteger(L, 5);
    st_ures_circle_fill_gpu(bufferID, cx, cy, radius, color);
    return 0;
}

static int lua_st_ures_line_gpu(lua_State* L) {
    int bufferID = luaL_checkinteger(L, 1);
    int x0 = luaL_checkinteger(L, 2);
    int y0 = luaL_checkinteger(L, 3);
    int x1 = luaL_checkinteger(L, 4);
    int y1 = luaL_checkinteger(L, 5);
    int color = luaL_checkinteger(L, 6);
    st_ures_line_gpu(bufferID, x0, y0, x1, y1, color);
    return 0;
}

// =============================================================================
// URES GPU Anti-Aliased Primitive Drawing Lua wrappers (TRUE alpha blending!)
// =============================================================================

static int lua_st_ures_circle_fill_aa(lua_State* L) {
    int bufferID = luaL_checkinteger(L, 1);
    int cx = luaL_checkinteger(L, 2);
    int cy = luaL_checkinteger(L, 3);
    int radius = luaL_checkinteger(L, 4);
    int color = luaL_checkinteger(L, 5);
    st_ures_circle_fill_aa(bufferID, cx, cy, radius, color);
    return 0;
}

static int lua_st_ures_line_aa(lua_State* L) {
    int bufferID = luaL_checkinteger(L, 1);
    int x0 = luaL_checkinteger(L, 2);
    int y0 = luaL_checkinteger(L, 3);
    int x1 = luaL_checkinteger(L, 4);
    int y1 = luaL_checkinteger(L, 5);
    int color = luaL_checkinteger(L, 6);
    float lineWidth = (float)luaL_optnumber(L, 7, 1.0);
    st_ures_line_aa(bufferID, x0, y0, x1, y1, color, lineWidth);
    return 0;
}

// =============================================================================
// URES GPU Gradient Primitive Drawing Lua wrappers
// =============================================================================

static int lua_st_ures_rect_fill_gradient_gpu(lua_State* L) {
    int bufferID = luaL_checkinteger(L, 1);
    int x = luaL_checkinteger(L, 2);
    int y = luaL_checkinteger(L, 3);
    int width = luaL_checkinteger(L, 4);
    int height = luaL_checkinteger(L, 5);
    int colorTopLeft = luaL_checkinteger(L, 6);
    int colorTopRight = luaL_checkinteger(L, 7);
    int colorBottomLeft = luaL_checkinteger(L, 8);
    int colorBottomRight = luaL_checkinteger(L, 9);
    st_ures_rect_fill_gradient_gpu(bufferID, x, y, width, height,
                                   colorTopLeft, colorTopRight,
                                   colorBottomLeft, colorBottomRight);
    return 0;
}

static int lua_st_ures_circle_fill_gradient_gpu(lua_State* L) {
    int bufferID = luaL_checkinteger(L, 1);
    int cx = luaL_checkinteger(L, 2);
    int cy = luaL_checkinteger(L, 3);
    int radius = luaL_checkinteger(L, 4);
    int centerColor = luaL_checkinteger(L, 5);
    int edgeColor = luaL_checkinteger(L, 6);
    st_ures_circle_fill_gradient_gpu(bufferID, cx, cy, radius, centerColor, edgeColor);
    return 0;
}

static int lua_st_ures_circle_fill_gradient_aa(lua_State* L) {
    int bufferID = luaL_checkinteger(L, 1);
    int cx = luaL_checkinteger(L, 2);
    int cy = luaL_checkinteger(L, 3);
    int radius = luaL_checkinteger(L, 4);
    int centerColor = luaL_checkinteger(L, 5);
    int edgeColor = luaL_checkinteger(L, 6);
    st_ures_circle_fill_gradient_aa(bufferID, cx, cy, radius, centerColor, edgeColor);
    return 0;
}

static int lua_st_begin_blit_batch(lua_State* L) {
    st_begin_blit_batch();
    return 0;
}

static int lua_st_end_blit_batch(lua_State* L) {
    st_end_blit_batch();
    return 0;
}

// =============================================================================
// URES Color Utility Lua wrappers
// =============================================================================

static int lua_st_ures_pack_argb4(lua_State* L) {
    int a = luaL_checkinteger(L, 1);
    int r = luaL_checkinteger(L, 2);
    int g = luaL_checkinteger(L, 3);
    int b = luaL_checkinteger(L, 4);
    int color = st_ures_pack_argb4(a, r, g, b);
    lua_pushinteger(L, color);
    return 1;
}

static int lua_st_ures_pack_argb8(lua_State* L) {
    int a = luaL_checkinteger(L, 1);
    int r = luaL_checkinteger(L, 2);
    int g = luaL_checkinteger(L, 3);
    int b = luaL_checkinteger(L, 4);
    int color = st_ures_pack_argb8(a, r, g, b);
    lua_pushinteger(L, color);
    return 1;
}

static int lua_st_ures_unpack_argb4(lua_State* L) {
    int color = luaL_checkinteger(L, 1);
    int a, r, g, b;
    st_ures_unpack_argb4(color, &a, &r, &g, &b);
    lua_pushinteger(L, a);
    lua_pushinteger(L, r);
    lua_pushinteger(L, g);
    lua_pushinteger(L, b);
    return 4;
}

static int lua_st_ures_unpack_argb8(lua_State* L) {
    int color = luaL_checkinteger(L, 1);
    int a, r, g, b;
    st_ures_unpack_argb8(color, &a, &r, &g, &b);
    lua_pushinteger(L, a);
    lua_pushinteger(L, r);
    lua_pushinteger(L, g);
    lua_pushinteger(L, b);
    return 4;
}

static int lua_st_ures_blend_colors(lua_State* L) {
    int src = luaL_checkinteger(L, 1);
    int dst = luaL_checkinteger(L, 2);
    int result = st_ures_blend_colors(src, dst);
    lua_pushinteger(L, result);
    return 1;
}

static int lua_st_ures_lerp_colors(lua_State* L) {
    int color1 = luaL_checkinteger(L, 1);
    int color2 = luaL_checkinteger(L, 2);
    float t = (float)luaL_checknumber(L, 3);
    int result = st_ures_lerp_colors(color1, color2, t);
    lua_pushinteger(L, result);
    return 1;
}

static int lua_st_ures_color_from_hsv(lua_State* L) {
    float h = (float)luaL_checknumber(L, 1);
    float s = (float)luaL_checknumber(L, 2);
    float v = (float)luaL_checknumber(L, 3);
    int a = luaL_checkinteger(L, 4);
    int color = st_ures_color_from_hsv(h, s, v, a);
    lua_pushinteger(L, color);
    return 1;
}

static int lua_st_ures_adjust_brightness(lua_State* L) {
    int color = luaL_checkinteger(L, 1);
    float factor = (float)luaL_checknumber(L, 2);
    int result = st_ures_adjust_brightness(color, factor);
    lua_pushinteger(L, result);
    return 1;
}

static int lua_st_ures_set_alpha(lua_State* L) {
    int color = luaL_checkinteger(L, 1);
    int alpha = luaL_checkinteger(L, 2);
    int result = st_ures_set_alpha(color, alpha);
    lua_pushinteger(L, result);
    return 1;
}

static int lua_st_ures_get_alpha(lua_State* L) {
    int color = luaL_checkinteger(L, 1);
    int alpha = st_ures_get_alpha(color);
    lua_pushinteger(L, alpha);
    return 1;
}

// Set per-row palette color
static int lua_st_xres_palette_row(lua_State* L) {
    int row = luaL_checkinteger(L, 1);
    int index = luaL_checkinteger(L, 2);
    int r = luaL_checkinteger(L, 3);
    int g = luaL_checkinteger(L, 4);
    int b = luaL_checkinteger(L, 5);
    st_xres_palette_row(row, index, r, g, b);
    return 0;
}

// Set global palette color
static int lua_st_xres_palette_global(lua_State* L) {
    int index = luaL_checkinteger(L, 1);
    int r = luaL_checkinteger(L, 2);
    int g = luaL_checkinteger(L, 3);
    int b = luaL_checkinteger(L, 4);
    st_xres_palette_global(index, r, g, b);
    return 0;
}

// XRES Palette Animation - Rotate per-row palette
static int lua_st_xres_palette_rotate_row(lua_State* L) {
    int row = luaL_checkinteger(L, 1);
    int startIndex = luaL_checkinteger(L, 2);
    int endIndex = luaL_checkinteger(L, 3);
    int direction = luaL_checkinteger(L, 4);
    st_xres_palette_rotate_row(row, startIndex, endIndex, direction);
    return 0;
}

// XRES Palette Animation - Rotate global palette
static int lua_st_xres_palette_rotate_global(lua_State* L) {
    int startIndex = luaL_checkinteger(L, 1);
    int endIndex = luaL_checkinteger(L, 2);
    int direction = luaL_checkinteger(L, 3);
    st_xres_palette_rotate_global(startIndex, endIndex, direction);
    return 0;
}

// XRES Palette Animation - Copy row palette
static int lua_st_xres_palette_copy_row(lua_State* L) {
    int srcRow = luaL_checkinteger(L, 1);
    int dstRow = luaL_checkinteger(L, 2);
    st_xres_palette_copy_row(srcRow, dstRow);
    return 0;
}

// XRES Palette Animation - Interpolate per-row palette
static int lua_st_xres_palette_lerp_row(lua_State* L) {
    int row = luaL_checkinteger(L, 1);
    int index = luaL_checkinteger(L, 2);
    int r1 = luaL_checkinteger(L, 3);
    int g1 = luaL_checkinteger(L, 4);
    int b1 = luaL_checkinteger(L, 5);
    int r2 = luaL_checkinteger(L, 6);
    int g2 = luaL_checkinteger(L, 7);
    int b2 = luaL_checkinteger(L, 8);
    float t = (float)luaL_checknumber(L, 9);
    st_xres_palette_lerp_row(row, index, r1, g1, b1, r2, g2, b2, t);
    return 0;
}

// XRES Palette Animation - Interpolate global palette
static int lua_st_xres_palette_lerp_global(lua_State* L) {
    int index = luaL_checkinteger(L, 1);
    int r1 = luaL_checkinteger(L, 2);
    int g1 = luaL_checkinteger(L, 3);
    int b1 = luaL_checkinteger(L, 4);
    int r2 = luaL_checkinteger(L, 5);
    int g2 = luaL_checkinteger(L, 6);
    int b2 = luaL_checkinteger(L, 7);
    float t = (float)luaL_checknumber(L, 8);
    st_xres_palette_lerp_global(index, r1, g1, b1, r2, g2, b2, t);
    return 0;
}

// --- XRES Gradient Drawing Functions ---

static int lua_st_xres_palette_make_ramp(lua_State* L) {
    int row = luaL_checkinteger(L, 1);
    int startIndex = luaL_checkinteger(L, 2);
    int endIndex = luaL_checkinteger(L, 3);
    int r1 = luaL_checkinteger(L, 4);
    int g1 = luaL_checkinteger(L, 5);
    int b1 = luaL_checkinteger(L, 6);
    int r2 = luaL_checkinteger(L, 7);
    int g2 = luaL_checkinteger(L, 8);
    int b2 = luaL_checkinteger(L, 9);
    st_xres_palette_make_ramp(row, startIndex, endIndex, r1, g1, b1, r2, g2, b2);
    return 0;
}

static int lua_st_xres_gradient_h(lua_State* L) {
    int x = luaL_checkinteger(L, 1);
    int y = luaL_checkinteger(L, 2);
    int width = luaL_checkinteger(L, 3);
    int height = luaL_checkinteger(L, 4);
    int startIndex = luaL_checkinteger(L, 5);
    int endIndex = luaL_checkinteger(L, 6);
    st_xres_gradient_h(x, y, width, height, (uint8_t)startIndex, (uint8_t)endIndex);
    return 0;
}

static int lua_st_xres_gradient_v(lua_State* L) {
    int x = luaL_checkinteger(L, 1);
    int y = luaL_checkinteger(L, 2);
    int width = luaL_checkinteger(L, 3);
    int height = luaL_checkinteger(L, 4);
    int startIndex = luaL_checkinteger(L, 5);
    int endIndex = luaL_checkinteger(L, 6);
    st_xres_gradient_v(x, y, width, height, (uint8_t)startIndex, (uint8_t)endIndex);
    return 0;
}

static int lua_st_xres_gradient_radial(lua_State* L) {
    int cx = luaL_checkinteger(L, 1);
    int cy = luaL_checkinteger(L, 2);
    int radius = luaL_checkinteger(L, 3);
    int centerIndex = luaL_checkinteger(L, 4);
    int edgeIndex = luaL_checkinteger(L, 5);
    st_xres_gradient_radial(cx, cy, radius, (uint8_t)centerIndex, (uint8_t)edgeIndex);
    return 0;
}

static int lua_st_xres_gradient_corners(lua_State* L) {
    int x = luaL_checkinteger(L, 1);
    int y = luaL_checkinteger(L, 2);
    int width = luaL_checkinteger(L, 3);
    int height = luaL_checkinteger(L, 4);
    int tlIndex = luaL_checkinteger(L, 5);
    int trIndex = luaL_checkinteger(L, 6);
    int blIndex = luaL_checkinteger(L, 7);
    int brIndex = luaL_checkinteger(L, 8);
    st_xres_gradient_corners(x, y, width, height, (uint8_t)tlIndex, (uint8_t)trIndex, 
                             (uint8_t)blIndex, (uint8_t)brIndex);
    return 0;
}

// Calculate XRES global palette index from RGB
static int lua_st_xrgb(lua_State* L) {
    int r = luaL_checkinteger(L, 1);
    int g = luaL_checkinteger(L, 2);
    int b = luaL_checkinteger(L, 3);
    int index = st_xrgb(r, g, b);
    lua_pushinteger(L, index);
    return 1;
}

// =============================================================================
// WRES Buffer API Bindings (432×240, 256-color palette)
// =============================================================================

// Set a pixel in WRES mode
static int lua_st_wres_pset(lua_State* L) {
    int x = luaL_checkinteger(L, 1);
    int y = luaL_checkinteger(L, 2);
    int colorIndex = luaL_checkinteger(L, 3);
    st_wres_pset(x, y, colorIndex);
    return 0;
}

// Get a pixel in WRES mode
static int lua_st_wres_pget(lua_State* L) {
    int x = luaL_checkinteger(L, 1);
    int y = luaL_checkinteger(L, 2);
    int colorIndex = st_wres_pget(x, y);
    lua_pushinteger(L, colorIndex);
    return 1;
}

// Clear WRES buffer
static int lua_st_wres_clear(lua_State* L) {
    int colorIndex = luaL_checkinteger(L, 1);
    st_wres_clear(colorIndex);
    return 0;
}

// Fill rectangle in WRES mode
static int lua_st_wres_fillrect(lua_State* L) {
    int x = luaL_checkinteger(L, 1);
    int y = luaL_checkinteger(L, 2);
    int width = luaL_checkinteger(L, 3);
    int height = luaL_checkinteger(L, 4);
    int colorIndex = luaL_checkinteger(L, 5);
    st_wres_fillrect(x, y, width, height, colorIndex);
    return 0;
}

// Draw horizontal line in WRES mode
static int lua_st_wres_hline(lua_State* L) {
    int x = luaL_checkinteger(L, 1);
    int y = luaL_checkinteger(L, 2);
    int width = luaL_checkinteger(L, 3);
    int colorIndex = luaL_checkinteger(L, 4);
    st_wres_hline(x, y, width, colorIndex);
    return 0;
}

// Draw vertical line in WRES mode
static int lua_st_wres_vline(lua_State* L) {
    int x = luaL_checkinteger(L, 1);
    int y = luaL_checkinteger(L, 2);
    int height = luaL_checkinteger(L, 3);
    int colorIndex = luaL_checkinteger(L, 4);
    st_wres_vline(x, y, height, colorIndex);
    return 0;
}

// Set active WRES buffer
static int lua_st_wres_buffer(lua_State* L) {
    int bufferID = luaL_checkinteger(L, 1);
    st_wres_buffer(bufferID);
    return 0;
}

// Flip WRES buffers
static int lua_st_wres_flip(lua_State* L) {
    (void)L;
    st_wres_flip();
    return 0;
}

// Blit within WRES buffer
static int lua_st_wres_blit(lua_State* L) {
    int srcX = luaL_checkinteger(L, 1);
    int srcY = luaL_checkinteger(L, 2);
    int width = luaL_checkinteger(L, 3);
    int height = luaL_checkinteger(L, 4);
    int dstX = luaL_checkinteger(L, 5);
    int dstY = luaL_checkinteger(L, 6);
    st_wres_blit(srcX, srcY, width, height, dstX, dstY);
    return 0;
}

// Blit with transparency
static int lua_st_wres_blit_trans(lua_State* L) {
    int srcX = luaL_checkinteger(L, 1);
    int srcY = luaL_checkinteger(L, 2);
    int width = luaL_checkinteger(L, 3);
    int height = luaL_checkinteger(L, 4);
    int dstX = luaL_checkinteger(L, 5);
    int dstY = luaL_checkinteger(L, 6);
    st_wres_blit_trans(srcX, srcY, width, height, dstX, dstY);
    return 0;
}

// Blit from another buffer
static int lua_st_wres_blit_from(lua_State* L) {
    int srcBufferID = luaL_checkinteger(L, 1);
    int srcX = luaL_checkinteger(L, 2);
    int srcY = luaL_checkinteger(L, 3);
    int width = luaL_checkinteger(L, 4);
    int height = luaL_checkinteger(L, 5);
    int dstX = luaL_checkinteger(L, 6);
    int dstY = luaL_checkinteger(L, 7);
    st_wres_blit_from(srcBufferID, srcX, srcY, width, height, dstX, dstY);
    return 0;
}

// Blit from another buffer with transparency
static int lua_st_wres_blit_from_trans(lua_State* L) {
    int srcBufferID = luaL_checkinteger(L, 1);
    int srcX = luaL_checkinteger(L, 2);
    int srcY = luaL_checkinteger(L, 3);
    int width = luaL_checkinteger(L, 4);
    int height = luaL_checkinteger(L, 5);
    int dstX = luaL_checkinteger(L, 6);
    int dstY = luaL_checkinteger(L, 7);
    st_wres_blit_from_trans(srcBufferID, srcX, srcY, width, height, dstX, dstY);
    return 0;
}

// GPU-accelerated WRES blit functions
static int lua_st_wres_blit_gpu(lua_State* L) {
    int srcBufferID = luaL_checkinteger(L, 1);
    int dstBufferID = luaL_checkinteger(L, 2);
    int srcX = luaL_checkinteger(L, 3);
    int srcY = luaL_checkinteger(L, 4);
    int width = luaL_checkinteger(L, 5);
    int height = luaL_checkinteger(L, 6);
    int dstX = luaL_checkinteger(L, 7);
    int dstY = luaL_checkinteger(L, 8);
    st_wres_blit_gpu(srcBufferID, dstBufferID, srcX, srcY, width, height, dstX, dstY);
    return 0;
}

static int lua_st_wres_blit_trans_gpu(lua_State* L) {
    int srcBufferID = luaL_checkinteger(L, 1);
    int dstBufferID = luaL_checkinteger(L, 2);
    int srcX = luaL_checkinteger(L, 3);
    int srcY = luaL_checkinteger(L, 4);
    int width = luaL_checkinteger(L, 5);
    int height = luaL_checkinteger(L, 6);
    int dstX = luaL_checkinteger(L, 7);
    int dstY = luaL_checkinteger(L, 8);
    int transparentColor = luaL_checkinteger(L, 9);
    st_wres_blit_trans_gpu(srcBufferID, dstBufferID, srcX, srcY, width, height, dstX, dstY, transparentColor);
    return 0;
}

// Set per-row palette color
static int lua_st_wres_palette_row(lua_State* L) {
    int row = luaL_checkinteger(L, 1);
    int index = luaL_checkinteger(L, 2);
    int r = luaL_checkinteger(L, 3);
    int g = luaL_checkinteger(L, 4);
    int b = luaL_checkinteger(L, 5);
    st_wres_palette_row(row, index, r, g, b);
    return 0;
}

// Set global palette color
static int lua_st_wres_palette_global(lua_State* L) {
    int index = luaL_checkinteger(L, 1);
    int r = luaL_checkinteger(L, 2);
    int g = luaL_checkinteger(L, 3);
    int b = luaL_checkinteger(L, 4);
    st_wres_palette_global(index, r, g, b);
    return 0;
}

// WRES Palette Animation - Rotate per-row palette
static int lua_st_wres_palette_rotate_row(lua_State* L) {
    int row = luaL_checkinteger(L, 1);
    int startIndex = luaL_checkinteger(L, 2);
    int endIndex = luaL_checkinteger(L, 3);
    int direction = luaL_checkinteger(L, 4);
    st_wres_palette_rotate_row(row, startIndex, endIndex, direction);
    return 0;
}

// WRES Palette Animation - Rotate global palette
static int lua_st_wres_palette_rotate_global(lua_State* L) {
    int startIndex = luaL_checkinteger(L, 1);
    int endIndex = luaL_checkinteger(L, 2);
    int direction = luaL_checkinteger(L, 3);
    st_wres_palette_rotate_global(startIndex, endIndex, direction);
    return 0;
}

// WRES Palette Animation - Copy row palette
static int lua_st_wres_palette_copy_row(lua_State* L) {
    int srcRow = luaL_checkinteger(L, 1);
    int dstRow = luaL_checkinteger(L, 2);
    st_wres_palette_copy_row(srcRow, dstRow);
    return 0;
}

// WRES Palette Animation - Interpolate per-row palette
static int lua_st_wres_palette_lerp_row(lua_State* L) {
    int row = luaL_checkinteger(L, 1);
    int index = luaL_checkinteger(L, 2);
    int r1 = luaL_checkinteger(L, 3);
    int g1 = luaL_checkinteger(L, 4);
    int b1 = luaL_checkinteger(L, 5);
    int r2 = luaL_checkinteger(L, 6);
    int g2 = luaL_checkinteger(L, 7);
    int b2 = luaL_checkinteger(L, 8);
    float t = (float)luaL_checknumber(L, 9);
    st_wres_palette_lerp_row(row, index, r1, g1, b1, r2, g2, b2, t);
    return 0;
}

// WRES Palette Animation - Interpolate global palette
static int lua_st_wres_palette_lerp_global(lua_State* L) {
    int index = luaL_checkinteger(L, 1);
    int r1 = luaL_checkinteger(L, 2);
    int g1 = luaL_checkinteger(L, 3);
    int b1 = luaL_checkinteger(L, 4);
    int r2 = luaL_checkinteger(L, 5);
    int g2 = luaL_checkinteger(L, 6);
    int b2 = luaL_checkinteger(L, 7);
    float t = (float)luaL_checknumber(L, 8);
    st_wres_palette_lerp_global(index, r1, g1, b1, r2, g2, b2, t);
    return 0;
}

// --- WRES Gradient Drawing Functions ---

static int lua_st_wres_palette_make_ramp(lua_State* L) {
    int row = luaL_checkinteger(L, 1);
    int startIndex = luaL_checkinteger(L, 2);
    int endIndex = luaL_checkinteger(L, 3);
    int r1 = luaL_checkinteger(L, 4);
    int g1 = luaL_checkinteger(L, 5);
    int b1 = luaL_checkinteger(L, 6);
    int r2 = luaL_checkinteger(L, 7);
    int g2 = luaL_checkinteger(L, 8);
    int b2 = luaL_checkinteger(L, 9);
    st_wres_palette_make_ramp(row, startIndex, endIndex, r1, g1, b1, r2, g2, b2);
    return 0;
}

static int lua_st_wres_gradient_h(lua_State* L) {
    int x = luaL_checkinteger(L, 1);
    int y = luaL_checkinteger(L, 2);
    int width = luaL_checkinteger(L, 3);
    int height = luaL_checkinteger(L, 4);
    int startIndex = luaL_checkinteger(L, 5);
    int endIndex = luaL_checkinteger(L, 6);
    st_wres_gradient_h(x, y, width, height, (uint8_t)startIndex, (uint8_t)endIndex);
    return 0;
}

static int lua_st_wres_gradient_v(lua_State* L) {
    int x = luaL_checkinteger(L, 1);
    int y = luaL_checkinteger(L, 2);
    int width = luaL_checkinteger(L, 3);
    int height = luaL_checkinteger(L, 4);
    int startIndex = luaL_checkinteger(L, 5);
    int endIndex = luaL_checkinteger(L, 6);
    st_wres_gradient_v(x, y, width, height, (uint8_t)startIndex, (uint8_t)endIndex);
    return 0;
}

static int lua_st_wres_gradient_radial(lua_State* L) {
    int cx = luaL_checkinteger(L, 1);
    int cy = luaL_checkinteger(L, 2);
    int radius = luaL_checkinteger(L, 3);
    int centerIndex = luaL_checkinteger(L, 4);
    int edgeIndex = luaL_checkinteger(L, 5);
    st_wres_gradient_radial(cx, cy, radius, (uint8_t)centerIndex, (uint8_t)edgeIndex);
    return 0;
}

static int lua_st_wres_gradient_corners(lua_State* L) {
    int x = luaL_checkinteger(L, 1);
    int y = luaL_checkinteger(L, 2);
    int width = luaL_checkinteger(L, 3);
    int height = luaL_checkinteger(L, 4);
    int tlIndex = luaL_checkinteger(L, 5);
    int trIndex = luaL_checkinteger(L, 6);
    int blIndex = luaL_checkinteger(L, 7);
    int brIndex = luaL_checkinteger(L, 8);
    st_wres_gradient_corners(x, y, width, height, (uint8_t)tlIndex, (uint8_t)trIndex, 
                             (uint8_t)blIndex, (uint8_t)brIndex);
    return 0;
}

// Calculate WRES global palette index from RGB
static int lua_st_wrgb(lua_State* L) {
    int r = luaL_checkinteger(L, 1);
    int g = luaL_checkinteger(L, 2);
    int b = luaL_checkinteger(L, 3);
    int index = st_wrgb(r, g, b);
    lua_pushinteger(L, index);
    return 1;
}

// =============================================================================
// PRES Buffer API Bindings (Premium Resolution 1280×720, 256-color palette)
// =============================================================================

// Set a pixel in PRES mode
static int lua_st_pres_pset(lua_State* L) {
    int x = luaL_checkinteger(L, 1);
    int y = luaL_checkinteger(L, 2);
    int colorIndex = luaL_checkinteger(L, 3);
    st_pres_pset(x, y, colorIndex);
    return 0;
}

// Get a pixel in PRES mode
static int lua_st_pres_pget(lua_State* L) {
    int x = luaL_checkinteger(L, 1);
    int y = luaL_checkinteger(L, 2);
    int colorIndex = st_pres_pget(x, y);
    lua_pushinteger(L, colorIndex);
    return 1;
}

// Clear PRES buffer
static int lua_st_pres_clear(lua_State* L) {
    int colorIndex = luaL_checkinteger(L, 1);
    st_pres_clear(colorIndex);
    return 0;
}

// Fill rectangle in PRES mode
static int lua_st_pres_fillrect(lua_State* L) {
    int x = luaL_checkinteger(L, 1);
    int y = luaL_checkinteger(L, 2);
    int width = luaL_checkinteger(L, 3);
    int height = luaL_checkinteger(L, 4);
    int colorIndex = luaL_checkinteger(L, 5);
    st_pres_fillrect(x, y, width, height, colorIndex);
    return 0;
}

// Draw horizontal line in PRES mode
static int lua_st_pres_hline(lua_State* L) {
    int x = luaL_checkinteger(L, 1);
    int y = luaL_checkinteger(L, 2);
    int width = luaL_checkinteger(L, 3);
    int colorIndex = luaL_checkinteger(L, 4);
    st_pres_hline(x, y, width, colorIndex);
    return 0;
}

// Draw vertical line in PRES mode
static int lua_st_pres_vline(lua_State* L) {
    int x = luaL_checkinteger(L, 1);
    int y = luaL_checkinteger(L, 2);
    int height = luaL_checkinteger(L, 3);
    int colorIndex = luaL_checkinteger(L, 4);
    st_pres_vline(x, y, height, colorIndex);
    return 0;
}

// Set active PRES buffer
static int lua_st_pres_buffer(lua_State* L) {
    int bufferID = luaL_checkinteger(L, 1);
    st_pres_buffer(bufferID);
    return 0;
}

// Flip PRES buffers
static int lua_st_pres_flip(lua_State* L) {
    st_pres_flip();
    return 0;
}

// Blit within same PRES buffer
static int lua_st_pres_blit(lua_State* L) {
    int srcX = luaL_checkinteger(L, 1);
    int srcY = luaL_checkinteger(L, 2);
    int width = luaL_checkinteger(L, 3);
    int height = luaL_checkinteger(L, 4);
    int dstX = luaL_checkinteger(L, 5);
    int dstY = luaL_checkinteger(L, 6);
    st_pres_blit(srcX, srcY, width, height, dstX, dstY);
    return 0;
}

// Blit with transparency
static int lua_st_pres_blit_trans(lua_State* L) {
    int srcX = luaL_checkinteger(L, 1);
    int srcY = luaL_checkinteger(L, 2);
    int width = luaL_checkinteger(L, 3);
    int height = luaL_checkinteger(L, 4);
    int dstX = luaL_checkinteger(L, 5);
    int dstY = luaL_checkinteger(L, 6);
    st_pres_blit_trans(srcX, srcY, width, height, dstX, dstY);
    return 0;
}

// Blit from another PRES buffer
static int lua_st_pres_blit_from(lua_State* L) {
    int srcBufferID = luaL_checkinteger(L, 1);
    int srcX = luaL_checkinteger(L, 2);
    int srcY = luaL_checkinteger(L, 3);
    int width = luaL_checkinteger(L, 4);
    int height = luaL_checkinteger(L, 5);
    int dstX = luaL_checkinteger(L, 6);
    int dstY = luaL_checkinteger(L, 7);
    st_pres_blit_from(srcBufferID, srcX, srcY, width, height, dstX, dstY);
    return 0;
}

// Blit from another PRES buffer with transparency
static int lua_st_pres_blit_from_trans(lua_State* L) {
    int srcBufferID = luaL_checkinteger(L, 1);
    int srcX = luaL_checkinteger(L, 2);
    int srcY = luaL_checkinteger(L, 3);
    int width = luaL_checkinteger(L, 4);
    int height = luaL_checkinteger(L, 5);
    int dstX = luaL_checkinteger(L, 6);
    int dstY = luaL_checkinteger(L, 7);
    st_pres_blit_from_trans(srcBufferID, srcX, srcY, width, height, dstX, dstY);
    return 0;
}

// GPU-accelerated blit
static int lua_st_pres_blit_gpu(lua_State* L) {
    int srcBufferID = luaL_checkinteger(L, 1);
    int dstBufferID = luaL_checkinteger(L, 2);
    int srcX = luaL_checkinteger(L, 3);
    int srcY = luaL_checkinteger(L, 4);
    int width = luaL_checkinteger(L, 5);
    int height = luaL_checkinteger(L, 6);
    int dstX = luaL_checkinteger(L, 7);
    int dstY = luaL_checkinteger(L, 8);
    st_pres_blit_gpu(srcBufferID, dstBufferID, srcX, srcY, width, height, dstX, dstY);
    return 0;
}

// GPU-accelerated transparent blit
static int lua_st_pres_blit_trans_gpu(lua_State* L) {
    int srcBufferID = luaL_checkinteger(L, 1);
    int dstBufferID = luaL_checkinteger(L, 2);
    int srcX = luaL_checkinteger(L, 3);
    int srcY = luaL_checkinteger(L, 4);
    int width = luaL_checkinteger(L, 5);
    int height = luaL_checkinteger(L, 6);
    int dstX = luaL_checkinteger(L, 7);
    int dstY = luaL_checkinteger(L, 8);
    int transparentColor = luaL_checkinteger(L, 9);
    st_pres_blit_trans_gpu(srcBufferID, dstBufferID, srcX, srcY, width, height, dstX, dstY, transparentColor);
    return 0;
}

// GPU-accelerated clear
static int lua_st_pres_clear_gpu(lua_State* L) {
    int bufferID = luaL_checkinteger(L, 1);
    int colorIndex = luaL_checkinteger(L, 2);
    st_pres_clear_gpu(bufferID, colorIndex);
    return 0;
}

// GPU-accelerated filled rectangle
static int lua_st_pres_rect_fill_gpu(lua_State* L) {
    int bufferID = luaL_checkinteger(L, 1);
    int x = luaL_checkinteger(L, 2);
    int y = luaL_checkinteger(L, 3);
    int width = luaL_checkinteger(L, 4);
    int height = luaL_checkinteger(L, 5);
    int colorIndex = luaL_checkinteger(L, 6);
    st_pres_rect_fill_gpu(bufferID, x, y, width, height, colorIndex);
    return 0;
}

// GPU-accelerated filled circle
static int lua_st_pres_circle_fill_gpu(lua_State* L) {
    int bufferID = luaL_checkinteger(L, 1);
    int cx = luaL_checkinteger(L, 2);
    int cy = luaL_checkinteger(L, 3);
    int radius = luaL_checkinteger(L, 4);
    int colorIndex = luaL_checkinteger(L, 5);
    st_pres_circle_fill_gpu(bufferID, cx, cy, radius, colorIndex);
    return 0;
}

// GPU-accelerated line
static int lua_st_pres_line_gpu(lua_State* L) {
    int bufferID = luaL_checkinteger(L, 1);
    int x0 = luaL_checkinteger(L, 2);
    int y0 = luaL_checkinteger(L, 3);
    int x1 = luaL_checkinteger(L, 4);
    int y1 = luaL_checkinteger(L, 5);
    int colorIndex = luaL_checkinteger(L, 6);
    st_pres_line_gpu(bufferID, x0, y0, x1, y1, colorIndex);
    return 0;
}

// GPU-accelerated anti-aliased circle
static int lua_st_pres_circle_fill_aa(lua_State* L) {
    int bufferID = luaL_checkinteger(L, 1);
    int cx = luaL_checkinteger(L, 2);
    int cy = luaL_checkinteger(L, 3);
    int radius = luaL_checkinteger(L, 4);
    int colorIndex = luaL_checkinteger(L, 5);
    st_pres_circle_fill_aa(bufferID, cx, cy, radius, colorIndex);
    return 0;
}

// GPU-accelerated anti-aliased line
static int lua_st_pres_line_aa(lua_State* L) {
    int bufferID = luaL_checkinteger(L, 1);
    int x0 = luaL_checkinteger(L, 2);
    int y0 = luaL_checkinteger(L, 3);
    int x1 = luaL_checkinteger(L, 4);
    int y1 = luaL_checkinteger(L, 5);
    int colorIndex = luaL_checkinteger(L, 6);
    float lineWidth = luaL_checknumber(L, 7);
    st_pres_line_aa(bufferID, x0, y0, x1, y1, colorIndex, lineWidth);
    return 0;
}

// Set per-row palette color
static int lua_st_pres_palette_row(lua_State* L) {
    int row = luaL_checkinteger(L, 1);
    int index = luaL_checkinteger(L, 2);
    int r = luaL_checkinteger(L, 3);
    int g = luaL_checkinteger(L, 4);
    int b = luaL_checkinteger(L, 5);
    st_pres_palette_row(row, index, r, g, b);
    return 0;
}

// Set global palette color
static int lua_st_pres_palette_global(lua_State* L) {
    int index = luaL_checkinteger(L, 1);
    int r = luaL_checkinteger(L, 2);
    int g = luaL_checkinteger(L, 3);
    int b = luaL_checkinteger(L, 4);
    st_pres_palette_global(index, r, g, b);
    return 0;
}

// PRES Palette Animation - Rotate per-row palette
static int lua_st_pres_palette_rotate_row(lua_State* L) {
    int row = luaL_checkinteger(L, 1);
    int startIndex = luaL_checkinteger(L, 2);
    int endIndex = luaL_checkinteger(L, 3);
    int direction = luaL_checkinteger(L, 4);
    st_pres_palette_rotate_row(row, startIndex, endIndex, direction);
    return 0;
}

// PRES Palette Animation - Rotate global palette
static int lua_st_pres_palette_rotate_global(lua_State* L) {
    int startIndex = luaL_checkinteger(L, 1);
    int endIndex = luaL_checkinteger(L, 2);
    int direction = luaL_checkinteger(L, 3);
    st_pres_palette_rotate_global(startIndex, endIndex, direction);
    return 0;
}

// PRES Palette Animation - Copy row palette
static int lua_st_pres_palette_copy_row(lua_State* L) {
    int srcRow = luaL_checkinteger(L, 1);
    int dstRow = luaL_checkinteger(L, 2);
    st_pres_palette_copy_row(srcRow, dstRow);
    return 0;
}

// PRES Palette Animation - Interpolate per-row palette
static int lua_st_pres_palette_lerp_row(lua_State* L) {
    int row = luaL_checkinteger(L, 1);
    int index = luaL_checkinteger(L, 2);
    int r1 = luaL_checkinteger(L, 3);
    int g1 = luaL_checkinteger(L, 4);
    int b1 = luaL_checkinteger(L, 5);
    int r2 = luaL_checkinteger(L, 6);
    int g2 = luaL_checkinteger(L, 7);
    int b2 = luaL_checkinteger(L, 8);
    float t = (float)luaL_checknumber(L, 9);
    st_pres_palette_lerp_row(row, index, r1, g1, b1, r2, g2, b2, t);
    return 0;
}

// PRES Palette Animation - Interpolate global palette
static int lua_st_pres_palette_lerp_global(lua_State* L) {
    int index = luaL_checkinteger(L, 1);
    int r1 = luaL_checkinteger(L, 2);
    int g1 = luaL_checkinteger(L, 3);
    int b1 = luaL_checkinteger(L, 4);
    int r2 = luaL_checkinteger(L, 5);
    int g2 = luaL_checkinteger(L, 6);
    int b2 = luaL_checkinteger(L, 7);
    float t = (float)luaL_checknumber(L, 8);
    st_pres_palette_lerp_global(index, r1, g1, b1, r2, g2, b2, t);
    return 0;
}

// --- PRES Gradient Drawing Functions ---

static int lua_st_pres_palette_make_ramp(lua_State* L) {
    int row = luaL_checkinteger(L, 1);
    int startIndex = luaL_checkinteger(L, 2);
    int endIndex = luaL_checkinteger(L, 3);
    int r1 = luaL_checkinteger(L, 4);
    int g1 = luaL_checkinteger(L, 5);
    int b1 = luaL_checkinteger(L, 6);
    int r2 = luaL_checkinteger(L, 7);
    int g2 = luaL_checkinteger(L, 8);
    int b2 = luaL_checkinteger(L, 9);
    st_pres_palette_make_ramp(row, startIndex, endIndex, r1, g1, b1, r2, g2, b2);
    return 0;
}

static int lua_st_pres_gradient_h(lua_State* L) {
    int bufferID = luaL_checkinteger(L, 1);
    int x = luaL_checkinteger(L, 2);
    int y = luaL_checkinteger(L, 3);
    int width = luaL_checkinteger(L, 4);
    int height = luaL_checkinteger(L, 5);
    int startIndex = luaL_checkinteger(L, 6);
    int endIndex = luaL_checkinteger(L, 7);
    st_pres_gradient_h(bufferID, x, y, width, height, (uint8_t)startIndex, (uint8_t)endIndex);
    return 0;
}

static int lua_st_pres_gradient_v(lua_State* L) {
    int bufferID = luaL_checkinteger(L, 1);
    int x = luaL_checkinteger(L, 2);
    int y = luaL_checkinteger(L, 3);
    int width = luaL_checkinteger(L, 4);
    int height = luaL_checkinteger(L, 5);
    int startIndex = luaL_checkinteger(L, 6);
    int endIndex = luaL_checkinteger(L, 7);
    st_pres_gradient_v(bufferID, x, y, width, height, (uint8_t)startIndex, (uint8_t)endIndex);
    return 0;
}

static int lua_st_pres_gradient_radial(lua_State* L) {
    int bufferID = luaL_checkinteger(L, 1);
    int cx = luaL_checkinteger(L, 2);
    int cy = luaL_checkinteger(L, 3);
    int radius = luaL_checkinteger(L, 4);
    int centerIndex = luaL_checkinteger(L, 5);
    int edgeIndex = luaL_checkinteger(L, 6);
    st_pres_gradient_radial(bufferID, cx, cy, radius, (uint8_t)centerIndex, (uint8_t)edgeIndex);
    return 0;
}

static int lua_st_pres_gradient_corners(lua_State* L) {
    int bufferID = luaL_checkinteger(L, 1);
    int x = luaL_checkinteger(L, 2);
    int y = luaL_checkinteger(L, 3);
    int width = luaL_checkinteger(L, 4);
    int height = luaL_checkinteger(L, 5);
    int tlIndex = luaL_checkinteger(L, 6);
    int trIndex = luaL_checkinteger(L, 7);
    int blIndex = luaL_checkinteger(L, 8);
    int brIndex = luaL_checkinteger(L, 9);
    st_pres_gradient_corners(bufferID, x, y, width, height, (uint8_t)tlIndex, (uint8_t)trIndex, 
                             (uint8_t)blIndex, (uint8_t)brIndex);
    return 0;
}

// Calculate PRES global palette index from RGB
static int lua_st_prgb(lua_State* L) {
    int r = luaL_checkinteger(L, 1);
    int g = luaL_checkinteger(L, 2);
    int b = luaL_checkinteger(L, 3);
    int index = st_prgb(r, g, b);
    lua_pushinteger(L, index);
    return 1;
}

// =============================================================================
// Unified Video Palette API Bindings
// =============================================================================

// Query Functions
static int lua_video_get_color_depth(lua_State* L) {
    int depth = st_video_get_color_depth();
    lua_pushinteger(L, depth);
    return 1;
}

static int lua_video_has_palette(lua_State* L) {
    bool hasPalette = st_video_has_palette();
    lua_pushboolean(L, hasPalette);
    return 1;
}

static int lua_video_has_per_row_palette(lua_State* L) {
    bool hasPerRow = st_video_has_per_row_palette();
    lua_pushboolean(L, hasPerRow);
    return 1;
}

static int lua_video_get_palette_info(lua_State* L) {
    st_video_palette_info_t info;
    st_video_get_palette_info(&info);
    
    lua_createtable(L, 0, 6);
    
    lua_pushboolean(L, info.hasPalette);
    lua_setfield(L, -2, "has_palette");
    
    lua_pushboolean(L, info.hasPerRowPalette);
    lua_setfield(L, -2, "has_per_row_palette");
    
    lua_pushinteger(L, info.colorDepth);
    lua_setfield(L, -2, "color_depth");
    
    lua_pushinteger(L, info.perRowColorCount);
    lua_setfield(L, -2, "per_row_color_count");
    
    lua_pushinteger(L, info.globalColorCount);
    lua_setfield(L, -2, "global_color_count");
    
    lua_pushinteger(L, info.rowCount);
    lua_setfield(L, -2, "row_count");
    
    return 1;
}

// Set Palette Functions
static int lua_video_set_palette(lua_State* L) {
    int index = luaL_checkinteger(L, 1);
    int r = luaL_checkinteger(L, 2);
    int g = luaL_checkinteger(L, 3);
    int b = luaL_checkinteger(L, 4);
    st_video_set_palette(index, r, g, b);
    return 0;
}

static int lua_video_set_palette_row(lua_State* L) {
    int row = luaL_checkinteger(L, 1);
    int index = luaL_checkinteger(L, 2);
    int r = luaL_checkinteger(L, 3);
    int g = luaL_checkinteger(L, 4);
    int b = luaL_checkinteger(L, 5);
    st_video_set_palette_row(row, index, r, g, b);
    return 0;
}

// Get Palette Functions
static int lua_video_get_palette(lua_State* L) {
    int index = luaL_checkinteger(L, 1);
    uint32_t color = st_video_get_palette(index);
    lua_pushinteger(L, color);
    return 1;
}

static int lua_video_get_palette_row(lua_State* L) {
    int row = luaL_checkinteger(L, 1);
    int index = luaL_checkinteger(L, 2);
    uint32_t color = st_video_get_palette_row(row, index);
    lua_pushinteger(L, color);
    return 1;
}

// Batch Operations (removed - using file-based API below)

// Preset Palette Functions
static int lua_video_load_preset_palette(lua_State* L) {
    int preset = luaL_checkinteger(L, 1);
    st_video_load_preset_palette((st_video_palette_preset_t)preset);
    return 0;
}

static int lua_video_load_preset_palette_rows(lua_State* L) {
    int preset = luaL_checkinteger(L, 1);
    int startRow = luaL_checkinteger(L, 2);
    int endRow = luaL_checkinteger(L, 3);
    st_video_load_preset_palette_rows((st_video_palette_preset_t)preset, startRow, endRow);
    return 0;
}

// Helper Functions
static int lua_video_pack_rgb(lua_State* L) {
    int r = luaL_checkinteger(L, 1);
    int g = luaL_checkinteger(L, 2);
    int b = luaL_checkinteger(L, 3);
    uint32_t color = st_video_pack_rgb(r, g, b);
    lua_pushinteger(L, color);
    return 1;
}

static int lua_video_unpack_rgb(lua_State* L) {
    uint32_t color = (uint32_t)luaL_checkinteger(L, 1);
    int r, g, b;
    st_video_unpack_rgb(color, &r, &g, &b);
    lua_pushinteger(L, r);
    lua_pushinteger(L, g);
    lua_pushinteger(L, b);
    return 3;
}

// =============================================================================
// Unified Video Mode API Bindings
// =============================================================================

static int lua_video_mode(lua_State* L) {
    int mode = luaL_checkinteger(L, 1);
    int result = st_video_mode_set((STVideoMode)mode);
    lua_pushboolean(L, result);
    return 1;
}

static int lua_video_mode_name(lua_State* L) {
    const char* modeName = luaL_checkstring(L, 1);
    int result = st_video_mode_name(modeName);
    lua_pushboolean(L, result);
    return 1;
}

static int lua_video_mode_get(lua_State* L) {
    STVideoMode mode = st_video_mode_get();
    lua_pushinteger(L, (int)mode);
    return 1;
}

static int lua_video_mode_disable(lua_State* L) {
    (void)L;
    st_video_mode_disable();
    return 0;
}

static int lua_video_pset(lua_State* L) {
    int x = luaL_checkinteger(L, 1);
    int y = luaL_checkinteger(L, 2);
    uint32_t color = (uint32_t)luaL_checkinteger(L, 3);
    st_video_pset(x, y, color);
    return 0;
}

static int lua_video_pget(lua_State* L) {
    int x = luaL_checkinteger(L, 1);
    int y = luaL_checkinteger(L, 2);
    uint32_t color = st_video_pget(x, y);
    lua_pushinteger(L, color);
    return 1;
}

static int lua_video_clear(lua_State* L) {
    uint32_t color = (uint32_t)luaL_checkinteger(L, 1);
    st_video_clear(color);
    return 0;
}

static int lua_video_clear_gpu(lua_State* L) {
    int buffer_id = luaL_checkinteger(L, 1);
    uint32_t color = (uint32_t)luaL_checkinteger(L, 2);
    st_video_clear_gpu(buffer_id, color);
    return 0;
}

static int lua_video_rect(lua_State* L) {
    int x = luaL_checkinteger(L, 1);
    int y = luaL_checkinteger(L, 2);
    int width = luaL_checkinteger(L, 3);
    int height = luaL_checkinteger(L, 4);
    uint32_t color = (uint32_t)luaL_checkinteger(L, 5);
    st_video_rect(x, y, width, height, color);
    return 0;
}

static int lua_video_rect_gpu(lua_State* L) {
    int buffer_id = luaL_checkinteger(L, 1);
    int x = luaL_checkinteger(L, 2);
    int y = luaL_checkinteger(L, 3);
    int width = luaL_checkinteger(L, 4);
    int height = luaL_checkinteger(L, 5);
    uint32_t color = (uint32_t)luaL_checkinteger(L, 6);
    st_video_rect_gpu(buffer_id, x, y, width, height, color);
    return 0;
}

static int lua_video_circle(lua_State* L) {
    int x = luaL_checkinteger(L, 1);
    int y = luaL_checkinteger(L, 2);
    int radius = luaL_checkinteger(L, 3);
    uint32_t color = (uint32_t)luaL_checkinteger(L, 4);
    st_video_circle(x, y, radius, color);
    return 0;
}

static int lua_video_circle_gpu(lua_State* L) {
    int buffer_id = luaL_checkinteger(L, 1);
    int x = luaL_checkinteger(L, 2);
    int y = luaL_checkinteger(L, 3);
    int radius = luaL_checkinteger(L, 4);
    uint32_t color = (uint32_t)luaL_checkinteger(L, 5);
    st_video_circle_gpu(buffer_id, x, y, radius, color);
    return 0;
}

static int lua_video_circle_aa(lua_State* L) {
    int buffer_id = luaL_checkinteger(L, 1);
    int x = luaL_checkinteger(L, 2);
    int y = luaL_checkinteger(L, 3);
    int radius = luaL_checkinteger(L, 4);
    uint32_t color = (uint32_t)luaL_checkinteger(L, 5);
    st_video_circle_aa(buffer_id, x, y, radius, color);
    return 0;
}

static int lua_video_line(lua_State* L) {
    int x1 = luaL_checkinteger(L, 1);
    int y1 = luaL_checkinteger(L, 2);
    int x2 = luaL_checkinteger(L, 3);
    int y2 = luaL_checkinteger(L, 4);
    uint32_t color = (uint32_t)luaL_checkinteger(L, 5);
    st_video_line(x1, y1, x2, y2, color);
    return 0;
}

static int lua_video_line_gpu(lua_State* L) {
    int buffer_id = luaL_checkinteger(L, 1);
    int x1 = luaL_checkinteger(L, 2);
    int y1 = luaL_checkinteger(L, 3);
    int x2 = luaL_checkinteger(L, 4);
    int y2 = luaL_checkinteger(L, 5);
    uint32_t color = (uint32_t)luaL_checkinteger(L, 6);
    st_video_line_gpu(buffer_id, x1, y1, x2, y2, color);
    return 0;
}

static int lua_video_line_aa(lua_State* L) {
    int buffer_id = luaL_checkinteger(L, 1);
    int x1 = luaL_checkinteger(L, 2);
    int y1 = luaL_checkinteger(L, 3);
    int x2 = luaL_checkinteger(L, 4);
    int y2 = luaL_checkinteger(L, 5);
    uint32_t color = (uint32_t)luaL_checkinteger(L, 6);
    st_video_line_aa(buffer_id, x1, y1, x2, y2, color);
    return 0;
}

static int lua_video_blit(lua_State* L) {
    int src_buffer = luaL_checkinteger(L, 1);
    int dst_buffer = luaL_checkinteger(L, 2);
    int src_x = luaL_checkinteger(L, 3);
    int src_y = luaL_checkinteger(L, 4);
    int dst_x = luaL_checkinteger(L, 5);
    int dst_y = luaL_checkinteger(L, 6);
    int width = luaL_checkinteger(L, 7);
    int height = luaL_checkinteger(L, 8);
    st_video_blit(src_buffer, dst_buffer, src_x, src_y, dst_x, dst_y, width, height);
    return 0;
}

static int lua_video_blit_trans(lua_State* L) {
    int src_buffer = luaL_checkinteger(L, 1);
    int dst_buffer = luaL_checkinteger(L, 2);
    int src_x = luaL_checkinteger(L, 3);
    int src_y = luaL_checkinteger(L, 4);
    int dst_x = luaL_checkinteger(L, 5);
    int dst_y = luaL_checkinteger(L, 6);
    int width = luaL_checkinteger(L, 7);
    int height = luaL_checkinteger(L, 8);
    uint32_t transparent_color = (uint32_t)luaL_checkinteger(L, 9);
    st_video_blit_trans(src_buffer, dst_buffer, src_x, src_y, dst_x, dst_y, width, height, transparent_color);
    return 0;
}

static int lua_video_blit_gpu(lua_State* L) {
    int src_buffer = luaL_checkinteger(L, 1);
    int dst_buffer = luaL_checkinteger(L, 2);
    int src_x = luaL_checkinteger(L, 3);
    int src_y = luaL_checkinteger(L, 4);
    int dst_x = luaL_checkinteger(L, 5);
    int dst_y = luaL_checkinteger(L, 6);
    int width = luaL_checkinteger(L, 7);
    int height = luaL_checkinteger(L, 8);
    st_video_blit_gpu(src_buffer, dst_buffer, src_x, src_y, dst_x, dst_y, width, height);
    return 0;
}

static int lua_video_blit_trans_gpu(lua_State* L) {
    int src_buffer = luaL_checkinteger(L, 1);
    int dst_buffer = luaL_checkinteger(L, 2);
    int src_x = luaL_checkinteger(L, 3);
    int src_y = luaL_checkinteger(L, 4);
    int dst_x = luaL_checkinteger(L, 5);
    int dst_y = luaL_checkinteger(L, 6);
    int width = luaL_checkinteger(L, 7);
    int height = luaL_checkinteger(L, 8);
    st_video_blit_trans_gpu(src_buffer, dst_buffer, src_x, src_y, dst_x, dst_y, width, height);
    return 0;
}

static int lua_video_buffer(lua_State* L) {
    int buffer_id = luaL_checkinteger(L, 1);
    st_video_buffer(buffer_id);
    return 0;
}

static int lua_video_buffer_get(lua_State* L) {
    int buffer_id = st_video_buffer_get();
    lua_pushinteger(L, buffer_id);
    return 1;
}

static int lua_video_get_back_buffer(lua_State* L) {
    int buffer_id = st_video_get_back_buffer();
    lua_pushinteger(L, buffer_id);
    return 1;
}

static int lua_video_load_image(lua_State* L) {
    const char* filePath = luaL_checkstring(L, 1);
    int bufferID = luaL_checkinteger(L, 2);
    int destX = luaL_optinteger(L, 3, 0);
    int destY = luaL_optinteger(L, 4, 0);
    int maxWidth = luaL_optinteger(L, 5, 0);
    int maxHeight = luaL_optinteger(L, 6, 0);
    bool success = st_video_load_image(filePath, bufferID, destX, destY, maxWidth, maxHeight);
    lua_pushboolean(L, success);
    return 1;
}

static int lua_video_save_image(lua_State* L) {
    const char* filePath = luaL_checkstring(L, 1);
    int bufferID = luaL_checkinteger(L, 2);
    bool success = st_video_save_image(filePath, bufferID);
    lua_pushboolean(L, success);
    return 1;
}

static int lua_video_load_palette(lua_State* L) {
    const char* filePath = luaL_checkstring(L, 1);
    bool success = st_video_load_palette_file(filePath);
    lua_pushboolean(L, success);
    return 1;
}

static int lua_video_save_palette(lua_State* L) {
    const char* filePath = luaL_checkstring(L, 1);
    bool success = st_video_save_palette_file(filePath);
    lua_pushboolean(L, success);
    return 1;
}

static int lua_video_get_front_buffer(lua_State* L) {
    int buffer_id = st_video_get_front_buffer();
    lua_pushinteger(L, buffer_id);
    return 1;
}

static int lua_video_flip(lua_State* L) {
    (void)L;
    st_video_flip();
    return 0;
}

static int lua_video_gpu_flip(lua_State* L) {
    (void)L;
    st_video_gpu_flip();
    return 0;
}

static int lua_video_rect_gradient_gpu(lua_State* L) {
    int buffer_id = luaL_checkinteger(L, 1);
    int x = luaL_checkinteger(L, 2);
    int y = luaL_checkinteger(L, 3);
    int width = luaL_checkinteger(L, 4);
    int height = luaL_checkinteger(L, 5);
    uint32_t topLeft = (uint32_t)luaL_checkinteger(L, 6);
    uint32_t topRight = (uint32_t)luaL_checkinteger(L, 7);
    uint32_t bottomLeft = (uint32_t)luaL_checkinteger(L, 8);
    uint32_t bottomRight = (uint32_t)luaL_checkinteger(L, 9);
    st_video_rect_gradient_gpu(buffer_id, x, y, width, height, 
                               topLeft, topRight, bottomLeft, bottomRight);
    return 0;
}

static int lua_video_circle_gradient_gpu(lua_State* L) {
    int buffer_id = luaL_checkinteger(L, 1);
    int cx = luaL_checkinteger(L, 2);
    int cy = luaL_checkinteger(L, 3);
    int radius = luaL_checkinteger(L, 4);
    uint32_t centerColor = (uint32_t)luaL_checkinteger(L, 5);
    uint32_t edgeColor = (uint32_t)luaL_checkinteger(L, 6);
    st_video_circle_gradient_gpu(buffer_id, cx, cy, radius, centerColor, edgeColor);
    return 0;
}

static int lua_video_supports_gradients(lua_State* L) {
    int supported = st_video_supports_gradients();
    lua_pushboolean(L, supported);
    return 1;
}

static int lua_video_enable_antialias(lua_State* L) {
    int enable = lua_toboolean(L, 1);
    int supported = st_video_enable_antialias(enable);
    lua_pushboolean(L, supported);
    return 1;
}

static int lua_video_supports_antialias(lua_State* L) {
    int supported = st_video_supports_antialias();
    lua_pushboolean(L, supported);
    return 1;
}

static int lua_video_set_line_width(lua_State* L) {
    float width = (float)luaL_checknumber(L, 1);
    st_video_set_line_width(width);
    return 0;
}

static int lua_video_get_line_width(lua_State* L) {
    float width = st_video_get_line_width();
    lua_pushnumber(L, width);
    return 1;
}

static int lua_video_sync(lua_State* L) {
    int buffer_id = luaL_checkinteger(L, 1);
    st_video_sync(buffer_id);
    return 0;
}

static int lua_video_begin_batch(lua_State* L) {
    (void)L;
    st_video_begin_batch();
    return 0;
}

static int lua_video_end_batch(lua_State* L) {
    (void)L;
    st_video_end_batch();
    return 0;
}

static int lua_video_swap(lua_State* L) {
    int buffer_id = luaL_checkinteger(L, 1);
    st_video_swap(buffer_id);
    return 0;
}

static int lua_video_resolution(lua_State* L) {
    int width, height;
    st_video_mode_get_resolution(&width, &height);
    lua_pushinteger(L, width);
    lua_pushinteger(L, height);
    return 2;
}

// =============================================================================
// Unified API - Buffer Management (Phase 1)
// =============================================================================

static int lua_video_get_max_buffers(lua_State* L) {
    int max = st_video_get_max_buffers();
    lua_pushinteger(L, max);
    return 1;
}

static int lua_video_is_valid_buffer(lua_State* L) {
    int buffer_id = luaL_checkinteger(L, 1);
    int valid = st_video_is_valid_buffer(buffer_id);
    lua_pushboolean(L, valid);
    return 1;
}

static int lua_video_get_current_buffer(lua_State* L) {
    int buffer_id = st_video_get_current_buffer();
    lua_pushinteger(L, buffer_id);
    return 1;
}

// =============================================================================
// Unified API - Feature Detection (Phase 1)
// =============================================================================

static int lua_video_get_feature_flags(lua_State* L) {
    uint32_t flags = st_video_get_feature_flags();
    lua_pushinteger(L, flags);
    return 1;
}

static int lua_video_uses_palette(lua_State* L) {
    int uses = st_video_uses_palette();
    lua_pushboolean(L, uses);
    return 1;
}

static int lua_video_has_gpu(lua_State* L) {
    int has = st_video_has_gpu();
    lua_pushboolean(L, has);
    return 1;
}

// =============================================================================
// Unified API - Memory Queries (Phase 2)
// =============================================================================

static int lua_video_get_memory_per_buffer(lua_State* L) {
    size_t memory = st_video_get_memory_per_buffer();
    lua_pushinteger(L, (lua_Integer)memory);
    return 1;
}

static int lua_video_get_memory_usage(lua_State* L) {
    size_t memory = st_video_get_memory_usage();
    lua_pushinteger(L, (lua_Integer)memory);
    return 1;
}

static int lua_video_get_pixel_count(lua_State* L) {
    size_t count = st_video_get_pixel_count();
    lua_pushinteger(L, (lua_Integer)count);
    return 1;
}

// =============================================================================
// Unified API - Palette Management (Phase 2)
// =============================================================================

static int lua_video_reset_palette_to_default(lua_State* L) {
    (void)L;
    st_video_reset_palette_to_default();
    return 0;
}

// =============================================================================
// Graphics Layer API Bindings
// =============================================================================

static int lua_st_gfx_clear(lua_State* L) {
    (void)L;
    st_gfx_clear();
    return 0;
}

static int lua_st_gfx_rect(lua_State* L) {
    int x = luaL_checkinteger(L, 1);
    int y = luaL_checkinteger(L, 2);
    int width = luaL_checkinteger(L, 3);
    int height = luaL_checkinteger(L, 4);
    uint32_t color = luaL_checkinteger(L, 5);

    st_gfx_rect(x, y, width, height, color);
    return 0;
}

static int lua_st_gfx_rect_outline(lua_State* L) {
    int x = luaL_checkinteger(L, 1);
    int y = luaL_checkinteger(L, 2);
    int width = luaL_checkinteger(L, 3);
    int height = luaL_checkinteger(L, 4);
    uint32_t color = luaL_checkinteger(L, 5);
    int thickness = luaL_optinteger(L, 6, 1);

    st_gfx_rect_outline(x, y, width, height, color, thickness);
    return 0;
}

static int lua_st_gfx_circle(lua_State* L) {
    int x = luaL_checkinteger(L, 1);
    int y = luaL_checkinteger(L, 2);
    int radius = luaL_checkinteger(L, 3);
    uint32_t color = luaL_checkinteger(L, 4);

    st_gfx_circle(x, y, radius, color);
    return 0;
}

static int lua_st_gfx_circle_outline(lua_State* L) {
    int x = luaL_checkinteger(L, 1);
    int y = luaL_checkinteger(L, 2);
    int radius = luaL_checkinteger(L, 3);
    uint32_t color = luaL_checkinteger(L, 4);
    int thickness = luaL_optinteger(L, 5, 1);

    st_gfx_circle_outline(x, y, radius, color, thickness);
    return 0;
}

static int lua_st_gfx_line(lua_State* L) {
    int x1 = luaL_checkinteger(L, 1);
    int y1 = luaL_checkinteger(L, 2);
    int x2 = luaL_checkinteger(L, 3);
    int y2 = luaL_checkinteger(L, 4);
    uint32_t color = luaL_checkinteger(L, 5);
    int thickness = luaL_optinteger(L, 6, 1);

    st_gfx_line(x1, y1, x2, y2, color, thickness);
    return 0;
}

static int lua_st_gfx_point(lua_State* L) {
    int x = luaL_checkinteger(L, 1);
    int y = luaL_checkinteger(L, 2);
    uint32_t color = luaL_checkinteger(L, 3);

    st_gfx_point(x, y, color);
    return 0;
}

// =============================================================================
// Rectangle API Bindings
// =============================================================================

static int lua_st_rect_create(lua_State* L) {
    float x = luaL_checknumber(L, 1);
    float y = luaL_checknumber(L, 2);
    float width = luaL_checknumber(L, 3);
    float height = luaL_checknumber(L, 4);
    uint32_t color = luaL_checkinteger(L, 5);

    int id = st_rect_create(x, y, width, height, color);
    lua_pushinteger(L, id);
    return 1;
}

static int lua_st_rect_create_gradient(lua_State* L) {
    float x = luaL_checknumber(L, 1);
    float y = luaL_checknumber(L, 2);
    float width = luaL_checknumber(L, 3);
    float height = luaL_checknumber(L, 4);
    uint32_t color1 = luaL_checkinteger(L, 5);
    uint32_t color2 = luaL_checkinteger(L, 6);
    int mode = luaL_checkinteger(L, 7);

    int id = st_rect_create_gradient(x, y, width, height, color1, color2, (STRectangleGradientMode)mode);
    lua_pushinteger(L, id);
    return 1;
}

static int lua_st_rect_create_three_point(lua_State* L) {
    float x = luaL_checknumber(L, 1);
    float y = luaL_checknumber(L, 2);
    float width = luaL_checknumber(L, 3);
    float height = luaL_checknumber(L, 4);
    uint32_t color1 = luaL_checkinteger(L, 5);
    uint32_t color2 = luaL_checkinteger(L, 6);
    uint32_t color3 = luaL_checkinteger(L, 7);
    int mode = luaL_checkinteger(L, 8);

    int id = st_rect_create_three_point(x, y, width, height, color1, color2, color3, (STRectangleGradientMode)mode);
    lua_pushinteger(L, id);
    return 1;
}

static int lua_st_rect_create_four_corner(lua_State* L) {
    float x = luaL_checknumber(L, 1);
    float y = luaL_checknumber(L, 2);
    float width = luaL_checknumber(L, 3);
    float height = luaL_checknumber(L, 4);
    uint32_t topLeft = luaL_checkinteger(L, 5);
    uint32_t topRight = luaL_checkinteger(L, 6);
    uint32_t bottomRight = luaL_checkinteger(L, 7);
    uint32_t bottomLeft = luaL_checkinteger(L, 8);

    int id = st_rect_create_four_corner(x, y, width, height, topLeft, topRight, bottomRight, bottomLeft);
    lua_pushinteger(L, id);
    return 1;
}

static int lua_st_rect_set_position(lua_State* L) {
    int id = luaL_checkinteger(L, 1);
    float x = luaL_checknumber(L, 2);
    float y = luaL_checknumber(L, 3);

    bool result = st_rect_set_position(id, x, y);
    lua_pushboolean(L, result);
    return 1;
}

static int lua_st_rect_set_size(lua_State* L) {
    int id = luaL_checkinteger(L, 1);
    float width = luaL_checknumber(L, 2);
    float height = luaL_checknumber(L, 3);

    bool result = st_rect_set_size(id, width, height);
    lua_pushboolean(L, result);
    return 1;
}

static int lua_st_rect_set_color(lua_State* L) {
    int id = luaL_checkinteger(L, 1);
    uint32_t color = luaL_checkinteger(L, 2);

    bool result = st_rect_set_color(id, color);
    lua_pushboolean(L, result);
    return 1;
}

static int lua_st_rect_set_colors(lua_State* L) {
    int id = luaL_checkinteger(L, 1);
    uint32_t color1 = luaL_checkinteger(L, 2);
    uint32_t color2 = luaL_checkinteger(L, 3);
    uint32_t color3 = luaL_checkinteger(L, 4);
    uint32_t color4 = luaL_checkinteger(L, 5);

    bool result = st_rect_set_colors(id, color1, color2, color3, color4);
    lua_pushboolean(L, result);
    return 1;
}

static int lua_st_rect_set_mode(lua_State* L) {
    int id = luaL_checkinteger(L, 1);
    int mode = luaL_checkinteger(L, 2);

    bool result = st_rect_set_mode(id, (STRectangleGradientMode)mode);
    lua_pushboolean(L, result);
    return 1;
}

static int lua_st_rect_set_rotation(lua_State* L) {
    int id = luaL_checkinteger(L, 1);
    double angleDegrees = luaL_checknumber(L, 2);

    bool result = st_rect_set_rotation(id, (float)angleDegrees);
    lua_pushboolean(L, result);
    return 1;
}

static int lua_st_rect_set_visible(lua_State* L) {
    int id = luaL_checkinteger(L, 1);
    bool visible = lua_toboolean(L, 2);

    bool result = st_rect_set_visible(id, visible);
    lua_pushboolean(L, result);
    return 1;
}

static int lua_st_rect_exists(lua_State* L) {
    int id = luaL_checkinteger(L, 1);

    bool result = st_rect_exists(id);
    lua_pushboolean(L, result);
    return 1;
}

static int lua_st_rect_is_visible(lua_State* L) {
    int id = luaL_checkinteger(L, 1);

    bool result = st_rect_is_visible(id);
    lua_pushboolean(L, result);
    return 1;
}

static int lua_st_rect_delete(lua_State* L) {
    int id = luaL_checkinteger(L, 1);

    bool result = st_rect_delete(id);
    lua_pushboolean(L, result);
    return 1;
}

static int lua_st_rect_delete_all(lua_State* L) {
    (void)L;
    st_rect_delete_all();
    return 0;
}

// Rectangle Procedural Pattern API (helper functions using pattern modes)

static int lua_st_rect_create_outline(lua_State* L) {
    float x = luaL_checknumber(L, 1);
    float y = luaL_checknumber(L, 2);
    float width = luaL_checknumber(L, 3);
    float height = luaL_checknumber(L, 4);
    uint32_t fillColor = luaL_checkinteger(L, 5);
    uint32_t outlineColor = luaL_checkinteger(L, 6);

    int id = st_rect_create_gradient(x, y, width, height, fillColor, outlineColor, ST_PATTERN_OUTLINE);
    lua_pushinteger(L, id);
    return 1;
}

static int lua_st_rect_create_horizontal_stripes(lua_State* L) {
    float x = luaL_checknumber(L, 1);
    float y = luaL_checknumber(L, 2);
    float width = luaL_checknumber(L, 3);
    float height = luaL_checknumber(L, 4);
    uint32_t color1 = luaL_checkinteger(L, 5);
    uint32_t color2 = luaL_checkinteger(L, 6);

    int id = st_rect_create_gradient(x, y, width, height, color1, color2, ST_PATTERN_HORIZONTAL_STRIPES);
    lua_pushinteger(L, id);
    return 1;
}

static int lua_st_rect_create_vertical_stripes(lua_State* L) {
    float x = luaL_checknumber(L, 1);
    float y = luaL_checknumber(L, 2);
    float width = luaL_checknumber(L, 3);
    float height = luaL_checknumber(L, 4);
    uint32_t color1 = luaL_checkinteger(L, 5);
    uint32_t color2 = luaL_checkinteger(L, 6);

    int id = st_rect_create_gradient(x, y, width, height, color1, color2, ST_PATTERN_VERTICAL_STRIPES);
    lua_pushinteger(L, id);
    return 1;
}

static int lua_st_rect_create_diagonal_stripes(lua_State* L) {
    float x = luaL_checknumber(L, 1);
    float y = luaL_checknumber(L, 2);
    float width = luaL_checknumber(L, 3);
    float height = luaL_checknumber(L, 4);
    uint32_t color1 = luaL_checkinteger(L, 5);
    uint32_t color2 = luaL_checkinteger(L, 6);

    int id = st_rect_create_gradient(x, y, width, height, color1, color2, ST_PATTERN_DIAGONAL_STRIPES);
    lua_pushinteger(L, id);
    return 1;
}

static int lua_st_rect_create_checkerboard(lua_State* L) {
    float x = luaL_checknumber(L, 1);
    float y = luaL_checknumber(L, 2);
    float width = luaL_checknumber(L, 3);
    float height = luaL_checknumber(L, 4);
    uint32_t color1 = luaL_checkinteger(L, 5);
    uint32_t color2 = luaL_checkinteger(L, 6);

    int id = st_rect_create_gradient(x, y, width, height, color1, color2, ST_PATTERN_CHECKERBOARD);
    lua_pushinteger(L, id);
    return 1;
}

static int lua_st_rect_create_dots(lua_State* L) {
    float x = luaL_checknumber(L, 1);
    float y = luaL_checknumber(L, 2);
    float width = luaL_checknumber(L, 3);
    float height = luaL_checknumber(L, 4);
    uint32_t dotColor = luaL_checkinteger(L, 5);
    uint32_t backgroundColor = luaL_checkinteger(L, 6);

    int id = st_rect_create_gradient(x, y, width, height, dotColor, backgroundColor, ST_PATTERN_DOTS);
    lua_pushinteger(L, id);
    return 1;
}

static int lua_st_rect_create_grid(lua_State* L) {
    float x = luaL_checknumber(L, 1);
    float y = luaL_checknumber(L, 2);
    float width = luaL_checknumber(L, 3);
    float height = luaL_checknumber(L, 4);
    uint32_t lineColor = luaL_checkinteger(L, 5);
    uint32_t backgroundColor = luaL_checkinteger(L, 6);

    int id = st_rect_create_gradient(x, y, width, height, lineColor, backgroundColor, ST_PATTERN_GRID);
    lua_pushinteger(L, id);
    return 1;
}




// =============================================================================
// Circle API Bindings
// =============================================================================

static int lua_st_circle_create(lua_State* L) {
    float x = luaL_checknumber(L, 1);
    float y = luaL_checknumber(L, 2);
    float radius = luaL_checknumber(L, 3);
    uint32_t color = luaL_checkinteger(L, 4);

    int id = st_circle_create(x, y, radius, color);
    lua_pushinteger(L, id);
    return 1;
}

static int lua_st_circle_create_radial(lua_State* L) {
    float x = luaL_checknumber(L, 1);
    float y = luaL_checknumber(L, 2);
    float radius = luaL_checknumber(L, 3);
    uint32_t centerColor = luaL_checkinteger(L, 4);
    uint32_t edgeColor = luaL_checkinteger(L, 5);

    int id = st_circle_create_radial(x, y, radius, centerColor, edgeColor);
    lua_pushinteger(L, id);
    return 1;
}

static int lua_st_circle_create_radial_3(lua_State* L) {
    float x = luaL_checknumber(L, 1);
    float y = luaL_checknumber(L, 2);
    float radius = luaL_checknumber(L, 3);
    uint32_t color1 = luaL_checkinteger(L, 4);
    uint32_t color2 = luaL_checkinteger(L, 5);
    uint32_t color3 = luaL_checkinteger(L, 6);

    int id = st_circle_create_radial_3(x, y, radius, color1, color2, color3);
    lua_pushinteger(L, id);
    return 1;
}

static int lua_st_circle_create_radial_4(lua_State* L) {
    float x = luaL_checknumber(L, 1);
    float y = luaL_checknumber(L, 2);
    float radius = luaL_checknumber(L, 3);
    uint32_t color1 = luaL_checkinteger(L, 4);
    uint32_t color2 = luaL_checkinteger(L, 5);
    uint32_t color3 = luaL_checkinteger(L, 6);
    uint32_t color4 = luaL_checkinteger(L, 7);

    int id = st_circle_create_radial_4(x, y, radius, color1, color2, color3, color4);
    lua_pushinteger(L, id);
    return 1;
}

static int lua_st_circle_create_outline(lua_State* L) {
    float x = luaL_checknumber(L, 1);
    float y = luaL_checknumber(L, 2);
    float radius = luaL_checknumber(L, 3);
    uint32_t fillColor = luaL_checkinteger(L, 4);
    uint32_t outlineColor = luaL_checkinteger(L, 5);
    float lineWidth = luaL_optnumber(L, 6, 2.0);

    int id = st_circle_create_outline(x, y, radius, fillColor, outlineColor, lineWidth);
    lua_pushinteger(L, id);
    return 1;
}

static int lua_st_circle_create_dashed_outline(lua_State* L) {
    float x = luaL_checknumber(L, 1);
    float y = luaL_checknumber(L, 2);
    float radius = luaL_checknumber(L, 3);
    uint32_t fillColor = luaL_checkinteger(L, 4);
    uint32_t outlineColor = luaL_checkinteger(L, 5);
    float lineWidth = luaL_optnumber(L, 6, 2.0);
    float dashLength = luaL_optnumber(L, 7, 10.0);

    int id = st_circle_create_dashed_outline(x, y, radius, fillColor, outlineColor, lineWidth, dashLength);
    lua_pushinteger(L, id);
    return 1;
}

static int lua_st_circle_create_ring(lua_State* L) {
    float x = luaL_checknumber(L, 1);
    float y = luaL_checknumber(L, 2);
    float outerRadius = luaL_checknumber(L, 3);
    float innerRadius = luaL_checknumber(L, 4);
    uint32_t color = luaL_checkinteger(L, 5);

    int id = st_circle_create_ring(x, y, outerRadius, innerRadius, color);
    lua_pushinteger(L, id);
    return 1;
}

static int lua_st_circle_create_pie_slice(lua_State* L) {
    float x = luaL_checknumber(L, 1);
    float y = luaL_checknumber(L, 2);
    float radius = luaL_checknumber(L, 3);
    float startAngle = luaL_checknumber(L, 4);
    float endAngle = luaL_checknumber(L, 5);
    uint32_t color = luaL_checkinteger(L, 6);

    int id = st_circle_create_pie_slice(x, y, radius, startAngle, endAngle, color);
    lua_pushinteger(L, id);
    return 1;
}

static int lua_st_circle_create_arc(lua_State* L) {
    float x = luaL_checknumber(L, 1);
    float y = luaL_checknumber(L, 2);
    float radius = luaL_checknumber(L, 3);
    float startAngle = luaL_checknumber(L, 4);
    float endAngle = luaL_checknumber(L, 5);
    uint32_t color = luaL_checkinteger(L, 6);
    float lineWidth = luaL_optnumber(L, 7, 2.0);

    int id = st_circle_create_arc(x, y, radius, startAngle, endAngle, color, lineWidth);
    lua_pushinteger(L, id);
    return 1;
}

static int lua_st_circle_create_dots_ring(lua_State* L) {
    float x = luaL_checknumber(L, 1);
    float y = luaL_checknumber(L, 2);
    float radius = luaL_checknumber(L, 3);
    uint32_t dotColor = luaL_checkinteger(L, 4);
    uint32_t backgroundColor = luaL_checkinteger(L, 5);
    float dotRadius = luaL_checknumber(L, 6);
    int numDots = luaL_checkinteger(L, 7);

    int id = st_circle_create_dots_ring(x, y, radius, dotColor, backgroundColor, dotRadius, numDots);
    lua_pushinteger(L, id);
    return 1;
}

static int lua_st_circle_create_star_burst(lua_State* L) {
    float x = luaL_checknumber(L, 1);
    float y = luaL_checknumber(L, 2);
    float radius = luaL_checknumber(L, 3);
    uint32_t color1 = luaL_checkinteger(L, 4);
    uint32_t color2 = luaL_checkinteger(L, 5);
    int numRays = luaL_checkinteger(L, 6);

    int id = st_circle_create_star_burst(x, y, radius, color1, color2, numRays);
    lua_pushinteger(L, id);
    return 1;
}

static int lua_st_circle_set_position(lua_State* L) {
    int id = luaL_checkinteger(L, 1);
    float x = luaL_checknumber(L, 2);
    float y = luaL_checknumber(L, 3);

    bool result = st_circle_set_position(id, x, y);
    lua_pushboolean(L, result);
    return 1;
}

static int lua_st_circle_set_radius(lua_State* L) {
    int id = luaL_checkinteger(L, 1);
    float radius = luaL_checknumber(L, 2);

    bool result = st_circle_set_radius(id, radius);
    lua_pushboolean(L, result);
    return 1;
}

static int lua_st_circle_set_color(lua_State* L) {
    int id = luaL_checkinteger(L, 1);
    uint32_t color = luaL_checkinteger(L, 2);

    bool result = st_circle_set_color(id, color);
    lua_pushboolean(L, result);
    return 1;
}

static int lua_st_circle_set_colors(lua_State* L) {
    int id = luaL_checkinteger(L, 1);
    uint32_t color1 = luaL_checkinteger(L, 2);
    uint32_t color2 = luaL_checkinteger(L, 3);
    uint32_t color3 = luaL_checkinteger(L, 4);
    uint32_t color4 = luaL_checkinteger(L, 5);

    bool result = st_circle_set_colors(id, color1, color2, color3, color4);
    lua_pushboolean(L, result);
    return 1;
}

static int lua_st_circle_set_parameters(lua_State* L) {
    int id = luaL_checkinteger(L, 1);
    float param1 = luaL_checknumber(L, 2);
    float param2 = luaL_checknumber(L, 3);
    float param3 = luaL_checknumber(L, 4);

    bool result = st_circle_set_parameters(id, param1, param2, param3);
    lua_pushboolean(L, result);
    return 1;
}

static int lua_st_circle_set_visible(lua_State* L) {
    int id = luaL_checkinteger(L, 1);
    bool visible = lua_toboolean(L, 2);

    bool result = st_circle_set_visible(id, visible);
    lua_pushboolean(L, result);
    return 1;
}

static int lua_st_circle_exists(lua_State* L) {
    int id = luaL_checkinteger(L, 1);
    lua_pushboolean(L, st_circle_exists(id));
    return 1;
}

static int lua_st_circle_is_visible(lua_State* L) {
    int id = luaL_checkinteger(L, 1);
    lua_pushboolean(L, st_circle_is_visible(id));
    return 1;
}

static int lua_st_circle_delete(lua_State* L) {
    int id = luaL_checkinteger(L, 1);
    bool result = st_circle_delete(id);
    lua_pushboolean(L, result);
    return 1;
}

static int lua_st_circle_delete_all(lua_State* L) {
    (void)L;
    st_circle_delete_all();
    return 0;
}

static int lua_st_circle_count(lua_State* L) {
    lua_pushinteger(L, st_circle_count());
    return 1;
}

static int lua_st_circle_is_empty(lua_State* L) {
    lua_pushboolean(L, st_circle_is_empty());
    return 1;
}

static int lua_st_circle_set_max(lua_State* L) {
    size_t max = luaL_checkinteger(L, 1);
    st_circle_set_max(max);
    return 0;
}

static int lua_st_circle_get_max(lua_State* L) {
    lua_pushinteger(L, st_circle_get_max());
    return 1;
}

// =============================================================================
// Line API Bindings
// =============================================================================

static int lua_st_line_create(lua_State* L) {
    float x1 = luaL_checknumber(L, 1);
    float y1 = luaL_checknumber(L, 2);
    float x2 = luaL_checknumber(L, 3);
    float y2 = luaL_checknumber(L, 4);
    uint32_t color = luaL_checkinteger(L, 5);
    float thickness = luaL_optnumber(L, 6, 1.0);

    int id = st_line_create(x1, y1, x2, y2, color, thickness);
    lua_pushinteger(L, id);
    return 1;
}

static int lua_st_line_create_gradient(lua_State* L) {
    float x1 = luaL_checknumber(L, 1);
    float y1 = luaL_checknumber(L, 2);
    float x2 = luaL_checknumber(L, 3);
    float y2 = luaL_checknumber(L, 4);
    uint32_t color1 = luaL_checkinteger(L, 5);
    uint32_t color2 = luaL_checkinteger(L, 6);
    float thickness = luaL_optnumber(L, 7, 1.0);

    int id = st_line_create_gradient(x1, y1, x2, y2, color1, color2, thickness);
    lua_pushinteger(L, id);
    return 1;
}

static int lua_st_line_create_dashed(lua_State* L) {
    float x1 = luaL_checknumber(L, 1);
    float y1 = luaL_checknumber(L, 2);
    float x2 = luaL_checknumber(L, 3);
    float y2 = luaL_checknumber(L, 4);
    uint32_t color = luaL_checkinteger(L, 5);
    float thickness = luaL_optnumber(L, 6, 1.0);
    float dashLength = luaL_optnumber(L, 7, 10.0);
    float gapLength = luaL_optnumber(L, 8, 5.0);

    int id = st_line_create_dashed(x1, y1, x2, y2, color, thickness, dashLength, gapLength);
    lua_pushinteger(L, id);
    return 1;
}

static int lua_st_line_create_dotted(lua_State* L) {
    float x1 = luaL_checknumber(L, 1);
    float y1 = luaL_checknumber(L, 2);
    float x2 = luaL_checknumber(L, 3);
    float y2 = luaL_checknumber(L, 4);
    uint32_t color = luaL_checkinteger(L, 5);
    float thickness = luaL_optnumber(L, 6, 1.0);
    float dotSpacing = luaL_optnumber(L, 7, 5.0);

    int id = st_line_create_dotted(x1, y1, x2, y2, color, thickness, dotSpacing);
    lua_pushinteger(L, id);
    return 1;
}

static int lua_st_line_set_endpoints(lua_State* L) {
    int id = luaL_checkinteger(L, 1);
    float x1 = luaL_checknumber(L, 2);
    float y1 = luaL_checknumber(L, 3);
    float x2 = luaL_checknumber(L, 4);
    float y2 = luaL_checknumber(L, 5);

    bool result = st_line_set_endpoints(id, x1, y1, x2, y2);
    lua_pushboolean(L, result);
    return 1;
}

static int lua_st_line_set_thickness(lua_State* L) {
    int id = luaL_checkinteger(L, 1);
    float thickness = luaL_checknumber(L, 2);

    bool result = st_line_set_thickness(id, thickness);
    lua_pushboolean(L, result);
    return 1;
}

static int lua_st_line_set_color(lua_State* L) {
    int id = luaL_checkinteger(L, 1);
    uint32_t color = luaL_checkinteger(L, 2);

    bool result = st_line_set_color(id, color);
    lua_pushboolean(L, result);
    return 1;
}

static int lua_st_line_set_colors(lua_State* L) {
    int id = luaL_checkinteger(L, 1);
    uint32_t color1 = luaL_checkinteger(L, 2);
    uint32_t color2 = luaL_checkinteger(L, 3);

    bool result = st_line_set_colors(id, color1, color2);
    lua_pushboolean(L, result);
    return 1;
}

static int lua_st_line_set_dash_pattern(lua_State* L) {
    int id = luaL_checkinteger(L, 1);
    float dashLength = luaL_checknumber(L, 2);
    float gapLength = luaL_checknumber(L, 3);

    bool result = st_line_set_dash_pattern(id, dashLength, gapLength);
    lua_pushboolean(L, result);
    return 1;
}

static int lua_st_line_set_visible(lua_State* L) {
    int id = luaL_checkinteger(L, 1);
    bool visible = lua_toboolean(L, 2);

    bool result = st_line_set_visible(id, visible);
    lua_pushboolean(L, result);
    return 1;
}

static int lua_st_line_exists(lua_State* L) {
    int id = luaL_checkinteger(L, 1);
    lua_pushboolean(L, st_line_exists(id));
    return 1;
}

static int lua_st_line_is_visible(lua_State* L) {
    int id = luaL_checkinteger(L, 1);
    lua_pushboolean(L, st_line_is_visible(id));
    return 1;
}

static int lua_st_line_delete(lua_State* L) {
    int id = luaL_checkinteger(L, 1);
    bool result = st_line_delete(id);
    lua_pushboolean(L, result);
    return 1;
}

static int lua_st_line_delete_all(lua_State* L) {
    (void)L;
    st_line_delete_all();
    return 0;
}

static int lua_st_line_count(lua_State* L) {
    lua_pushinteger(L, st_line_count());
    return 1;
}

static int lua_st_line_is_empty(lua_State* L) {
    lua_pushboolean(L, st_line_is_empty());
    return 1;
}

static int lua_st_line_set_max(lua_State* L) {
    size_t max = luaL_checkinteger(L, 1);
    st_line_set_max(max);
    return 0;
}

static int lua_st_line_get_max(lua_State* L) {
    lua_pushinteger(L, st_line_get_max());
    return 1;
}



// =============================================================================
// Polygon API Bindings
// =============================================================================

static int lua_st_polygon_create(lua_State* L) {
    float x = luaL_checknumber(L, 1);
    float y = luaL_checknumber(L, 2);
    float radius = luaL_checknumber(L, 3);
    int numSides = luaL_checkinteger(L, 4);
    uint32_t color = luaL_checkinteger(L, 5);

    int id = st_polygon_create(x, y, radius, numSides, color);
    lua_pushinteger(L, id);
    return 1;
}

static int lua_st_polygon_create_gradient(lua_State* L) {
    float x = luaL_checknumber(L, 1);
    float y = luaL_checknumber(L, 2);
    float radius = luaL_checknumber(L, 3);
    int numSides = luaL_checkinteger(L, 4);
    uint32_t color1 = luaL_checkinteger(L, 5);
    uint32_t color2 = luaL_checkinteger(L, 6);
    int mode = luaL_checkinteger(L, 7);

    int id = st_polygon_create_gradient(x, y, radius, numSides, color1, color2, (STPolygonGradientMode)mode);
    lua_pushinteger(L, id);
    return 1;
}

static int lua_st_polygon_set_position(lua_State* L) {
    int id = luaL_checkinteger(L, 1);
    float x = luaL_checknumber(L, 2);
    float y = luaL_checknumber(L, 3);
    bool result = st_polygon_set_position(id, x, y);
    lua_pushboolean(L, result);
    return 1;
}

static int lua_st_polygon_set_radius(lua_State* L) {
    int id = luaL_checkinteger(L, 1);
    float radius = luaL_checknumber(L, 2);
    bool result = st_polygon_set_radius(id, radius);
    lua_pushboolean(L, result);
    return 1;
}

static int lua_st_polygon_set_sides(lua_State* L) {
    int id = luaL_checkinteger(L, 1);
    int numSides = luaL_checkinteger(L, 2);
    bool result = st_polygon_set_sides(id, numSides);
    lua_pushboolean(L, result);
    return 1;
}

static int lua_st_polygon_set_color(lua_State* L) {
    int id = luaL_checkinteger(L, 1);
    uint32_t color = luaL_checkinteger(L, 2);
    bool result = st_polygon_set_color(id, color);
    lua_pushboolean(L, result);
    return 1;
}

static int lua_st_polygon_set_rotation(lua_State* L) {
    int id = luaL_checkinteger(L, 1);
    double angleDegrees = luaL_checknumber(L, 2);
    bool result = st_polygon_set_rotation(id, (float)angleDegrees);
    lua_pushboolean(L, result);
    return 1;
}

static int lua_st_polygon_set_visible(lua_State* L) {
    int id = luaL_checkinteger(L, 1);
    bool visible = lua_toboolean(L, 2);
    bool result = st_polygon_set_visible(id, visible);
    lua_pushboolean(L, result);
    return 1;
}

static int lua_st_polygon_delete(lua_State* L) {
    int id = luaL_checkinteger(L, 1);
    bool result = st_polygon_delete(id);
    lua_pushboolean(L, result);
    return 1;
}

static int lua_st_polygon_delete_all(lua_State* L) {
    (void)L;
    st_polygon_delete_all();
    return 0;
}

static int lua_st_polygon_count(lua_State* L) {
    lua_pushinteger(L, st_polygon_count());
    return 1;
}

// =============================================================================
// Star API Bindings
// =============================================================================

static int lua_st_star_create(lua_State* L) {
    float x = luaL_checknumber(L, 1);
    float y = luaL_checknumber(L, 2);
    float outerRadius = luaL_checknumber(L, 3);
    int numPoints = luaL_checkinteger(L, 4);
    uint32_t color = luaL_checkinteger(L, 5);

    int id = st_star_create(x, y, outerRadius, numPoints, color);
    lua_pushinteger(L, id);
    return 1;
}

static int lua_st_star_create_custom(lua_State* L) {
    float x = luaL_checknumber(L, 1);
    float y = luaL_checknumber(L, 2);
    float outerRadius = luaL_checknumber(L, 3);
    float innerRadius = luaL_checknumber(L, 4);
    int numPoints = luaL_checkinteger(L, 5);
    uint32_t color = luaL_checkinteger(L, 6);

    int id = st_star_create_custom(x, y, outerRadius, innerRadius, numPoints, color);
    lua_pushinteger(L, id);
    return 1;
}

static int lua_st_star_create_gradient(lua_State* L) {
    float x = luaL_checknumber(L, 1);
    float y = luaL_checknumber(L, 2);
    float outerRadius = luaL_checknumber(L, 3);
    int numPoints = luaL_checkinteger(L, 4);
    uint32_t color1 = luaL_checkinteger(L, 5);
    uint32_t color2 = luaL_checkinteger(L, 6);
    int mode = luaL_checkinteger(L, 7);

    int id = st_star_create_gradient(x, y, outerRadius, numPoints, color1, color2, (STStarGradientMode)mode);
    lua_pushinteger(L, id);
    return 1;
}

static int lua_st_star_create_outline(lua_State* L) {
    float x = luaL_checknumber(L, 1);
    float y = luaL_checknumber(L, 2);
    float outerRadius = luaL_checknumber(L, 3);
    int numPoints = luaL_checkinteger(L, 4);
    uint32_t fillColor = luaL_checkinteger(L, 5);
    uint32_t outlineColor = luaL_checkinteger(L, 6);
    float lineWidth = luaL_optnumber(L, 7, 2.0);

    int id = st_star_create_outline(x, y, outerRadius, numPoints, fillColor, outlineColor, lineWidth);
    lua_pushinteger(L, id);
    return 1;
}

static int lua_st_star_set_position(lua_State* L) {
    int id = luaL_checkinteger(L, 1);
    float x = luaL_checknumber(L, 2);
    float y = luaL_checknumber(L, 3);
    bool result = st_star_set_position(id, x, y);
    lua_pushboolean(L, result);
    return 1;
}

static int lua_st_star_set_radius(lua_State* L) {
    int id = luaL_checkinteger(L, 1);
    float outerRadius = luaL_checknumber(L, 2);
    bool result = st_star_set_radius(id, outerRadius);
    lua_pushboolean(L, result);
    return 1;
}

static int lua_st_star_set_radii(lua_State* L) {
    int id = luaL_checkinteger(L, 1);
    float outerRadius = luaL_checknumber(L, 2);
    float innerRadius = luaL_checknumber(L, 3);
    bool result = st_star_set_radii(id, outerRadius, innerRadius);
    lua_pushboolean(L, result);
    return 1;
}

static int lua_st_star_set_points(lua_State* L) {
    int id = luaL_checkinteger(L, 1);
    int numPoints = luaL_checkinteger(L, 2);
    bool result = st_star_set_points(id, numPoints);
    lua_pushboolean(L, result);
    return 1;
}

static int lua_st_star_set_color(lua_State* L) {
    int id = luaL_checkinteger(L, 1);
    uint32_t color = luaL_checkinteger(L, 2);
    bool result = st_star_set_color(id, color);
    lua_pushboolean(L, result);
    return 1;
}

static int lua_st_star_set_colors(lua_State* L) {
    int id = luaL_checkinteger(L, 1);
    uint32_t color1 = luaL_checkinteger(L, 2);
    uint32_t color2 = luaL_checkinteger(L, 3);
    bool result = st_star_set_colors(id, color1, color2);
    lua_pushboolean(L, result);
    return 1;
}

static int lua_st_star_set_rotation(lua_State* L) {
    int id = luaL_checkinteger(L, 1);
    float angleDegrees = luaL_checknumber(L, 2);
    bool result = st_star_set_rotation(id, angleDegrees);
    lua_pushboolean(L, result);
    return 1;
}

static int lua_st_star_set_visible(lua_State* L) {
    int id = luaL_checkinteger(L, 1);
    bool visible = lua_toboolean(L, 2);
    bool result = st_star_set_visible(id, visible);
    lua_pushboolean(L, result);
    return 1;
}

static int lua_st_star_exists(lua_State* L) {
    int id = luaL_checkinteger(L, 1);
    lua_pushboolean(L, st_star_exists(id));
    return 1;
}

static int lua_st_star_is_visible(lua_State* L) {
    int id = luaL_checkinteger(L, 1);
    lua_pushboolean(L, st_star_is_visible(id));
    return 1;
}

static int lua_st_star_delete(lua_State* L) {
    int id = luaL_checkinteger(L, 1);
    bool result = st_star_delete(id);
    lua_pushboolean(L, result);
    return 1;
}

static int lua_st_star_delete_all(lua_State* L) {
    (void)L;
    st_star_delete_all();
    return 0;
}

static int lua_st_star_count(lua_State* L) {
    lua_pushinteger(L, st_star_count());
    return 1;
}

static int lua_st_star_is_empty(lua_State* L) {
    lua_pushboolean(L, st_star_is_empty());
    return 1;
}

// =============================================================================
// Audio API Bindings
// =============================================================================

static int lua_st_music_play(lua_State* L) {
    const char* abc = luaL_checkstring(L, 1);
    st_music_play(abc);
    return 0;
}

static int lua_st_music_play_file(lua_State* L) {
    const char* path = luaL_checkstring(L, 1);
    st_music_play_file(path);
    return 0;
}

static int lua_st_music_stop(lua_State* L) {
    (void)L;
    st_music_stop();
    return 0;
}

static int lua_st_music_pause(lua_State* L) {
    (void)L;
    st_music_pause();
    return 0;
}

static int lua_st_music_resume(lua_State* L) {
    (void)L;
    st_music_resume();
    return 0;
}

static int lua_st_music_is_playing(lua_State* L) {
    lua_pushboolean(L, st_music_is_playing());
    return 1;
}

static int lua_st_music_set_volume(lua_State* L) {
    float volume = luaL_checknumber(L, 1);
    st_music_set_volume(volume);
    return 0;
}

static int lua_st_synth_note(lua_State* L) {
    int note = luaL_checkinteger(L, 1);
    float duration = luaL_checknumber(L, 2);
    float volume = luaL_optnumber(L, 3, 0.5f);

    st_synth_note(note, duration, volume);
    return 0;
}

static int lua_st_synth_set_instrument(lua_State* L) {
    int instrument = luaL_checkinteger(L, 1);
    st_synth_set_instrument(instrument);
    return 0;
}

// Sound Bank API - Create sound effects
static int lua_st_sound_create_beep(lua_State* L) {
    float frequency = (float)luaL_checknumber(L, 1);
    float duration = (float)luaL_checknumber(L, 2);
    uint32_t sound_id = st_sound_create_beep(frequency, duration);
    lua_pushinteger(L, sound_id);
    return 1;
}

static int lua_st_sound_create_blip(lua_State* L) {
    float pitch = (float)luaL_checknumber(L, 1);
    float duration = (float)luaL_checknumber(L, 2);
    uint32_t sound_id = st_sound_create_blip(pitch, duration);
    lua_pushinteger(L, sound_id);
    return 1;
}

static int lua_st_sound_create_click(lua_State* L) {
    float sharpness = (float)luaL_checknumber(L, 1);
    float duration = (float)luaL_checknumber(L, 2);
    uint32_t sound_id = st_sound_create_click(sharpness, duration);
    lua_pushinteger(L, sound_id);
    return 1;
}

static int lua_st_sound_create_zap(lua_State* L) {
    float frequency = (float)luaL_checknumber(L, 1);
    float duration = (float)luaL_checknumber(L, 2);
    uint32_t sound_id = st_sound_create_zap(frequency, duration);
    lua_pushinteger(L, sound_id);
    return 1;
}

static int lua_st_sound_create_explode(lua_State* L) {
    float size = (float)luaL_checknumber(L, 1);
    float duration = (float)luaL_checknumber(L, 2);
    uint32_t sound_id = st_sound_create_explode(size, duration);
    lua_pushinteger(L, sound_id);
    return 1;
}

static int lua_st_sound_create_pickup(lua_State* L) {
    float brightness = (float)luaL_checknumber(L, 1);
    float duration = (float)luaL_checknumber(L, 2);
    uint32_t sound_id = st_sound_create_pickup(brightness, duration);
    lua_pushinteger(L, sound_id);
    return 1;
}

static int lua_st_sound_create_hurt(lua_State* L) {
    float severity = (float)luaL_checknumber(L, 1);
    float duration = (float)luaL_checknumber(L, 2);
    uint32_t sound_id = st_sound_create_hurt(severity, duration);
    lua_pushinteger(L, sound_id);
    return 1;
}

static int lua_st_sound_create_sweep_down(lua_State* L) {
    float start_freq = (float)luaL_checknumber(L, 1);
    float end_freq = (float)luaL_checknumber(L, 2);
    float duration = (float)luaL_checknumber(L, 3);
    uint32_t sound_id = st_sound_create_sweep_down(start_freq, end_freq, duration);
    lua_pushinteger(L, sound_id);
    return 1;
}

static int lua_st_sound_create_coin(lua_State* L) {
    float pitch = (float)luaL_checknumber(L, 1);
    float duration = (float)luaL_checknumber(L, 2);
    uint32_t sound_id = st_sound_create_coin(pitch, duration);
    lua_pushinteger(L, sound_id);
    return 1;
}

static int lua_st_sound_create_powerup(lua_State* L) {
    float intensity = (float)luaL_checknumber(L, 1);
    float duration = (float)luaL_checknumber(L, 2);
    uint32_t sound_id = st_sound_create_powerup(intensity, duration);
    lua_pushinteger(L, sound_id);
    return 1;
}

// Play sound by ID
static int lua_st_sound_play_id(lua_State* L) {
    uint32_t sound_id = (uint32_t)luaL_checkinteger(L, 1);
    float volume = (float)luaL_optnumber(L, 2, 1.0);
    float pan = (float)luaL_optnumber(L, 3, 0.0); // 0.0 = center
    st_sound_play_id(sound_id, volume, pan);
    return 0;
}

// Check if sound exists
static int lua_st_sound_exists(lua_State* L) {
    uint32_t sound_id = (uint32_t)luaL_checkinteger(L, 1);
    bool exists = st_sound_exists(sound_id);
    lua_pushboolean(L, exists);
    return 1;
}

// Delete sound from bank
static int lua_st_sound_delete(lua_State* L) {
    uint32_t sound_id = (uint32_t)luaL_checkinteger(L, 1);
    st_sound_unload(sound_id);
    return 0;
}

static int lua_st_synth_frequency(lua_State* L) {
    float frequency = luaL_checknumber(L, 1);
    float duration = luaL_checknumber(L, 2);
    float volume = luaL_optnumber(L, 3, 0.5f);

    st_synth_frequency(frequency, duration, volume);
    return 0;
}

// =============================================================================
// Voice Controller API Bindings
// =============================================================================

// Voice basic controls
static int lua_st_voice_set_waveform(lua_State* L) {
    int voiceNum = luaL_checkinteger(L, 1);
    int waveform = luaL_checkinteger(L, 2);
    st_voice_set_waveform(voiceNum, waveform);
    return 0;
}

static int lua_st_voice_set_frequency(lua_State* L) {
    int voiceNum = luaL_checkinteger(L, 1);
    float frequency = (float)luaL_checknumber(L, 2);
    st_voice_set_frequency(voiceNum, frequency);
    return 0;
}

static int lua_st_voice_set_note(lua_State* L) {
    int voiceNum = luaL_checkinteger(L, 1);
    int midiNote = luaL_checkinteger(L, 2);
    st_voice_set_note(voiceNum, midiNote);
    return 0;
}

static int lua_st_voice_set_note_name(lua_State* L) {
    int voiceNum = luaL_checkinteger(L, 1);
    const char* noteName = luaL_checkstring(L, 2);
    st_voice_set_note_name(voiceNum, noteName);
    return 0;
}

static int lua_st_voice_set_envelope(lua_State* L) {
    int voiceNum = luaL_checkinteger(L, 1);
    float attack = (float)luaL_checknumber(L, 2);
    float decay = (float)luaL_checknumber(L, 3);
    float sustain = (float)luaL_checknumber(L, 4);
    float release = (float)luaL_checknumber(L, 5);
    st_voice_set_envelope(voiceNum, attack, decay, sustain, release);
    return 0;
}

static int lua_st_voice_set_gate(lua_State* L) {
    int voiceNum = luaL_checkinteger(L, 1);
    int gateOn = lua_toboolean(L, 2);
    st_voice_set_gate(voiceNum, gateOn);
    return 0;
}

static int lua_st_voice_set_volume(lua_State* L) {
    int voiceNum = luaL_checkinteger(L, 1);
    float volume = (float)luaL_checknumber(L, 2);
    st_voice_set_volume(voiceNum, volume);
    return 0;
}

static int lua_st_voice_set_pulse_width(lua_State* L) {
    int voiceNum = luaL_checkinteger(L, 1);
    float pulseWidth = (float)luaL_checknumber(L, 2);
    st_voice_set_pulse_width(voiceNum, pulseWidth);
    return 0;
}

static int lua_st_voice_set_pan(lua_State* L) {
    int voiceNum = luaL_checkinteger(L, 1);
    float pan = (float)luaL_checknumber(L, 2);
    st_voice_set_pan(voiceNum, pan);
    return 0;
}

// Voice filter controls
static int lua_st_voice_set_filter_routing(lua_State* L) {
    int voiceNum = luaL_checkinteger(L, 1);
    int enabled = lua_toboolean(L, 2);
    st_voice_set_filter_routing(voiceNum, enabled);
    return 0;
}

static int lua_st_voice_set_filter_type(lua_State* L) {
    int filterType = luaL_checkinteger(L, 1);
    st_voice_set_filter_type(filterType);
    return 0;
}

static int lua_st_voice_set_filter_cutoff(lua_State* L) {
    float cutoff = (float)luaL_checknumber(L, 1);
    st_voice_set_filter_cutoff(cutoff);
    return 0;
}

static int lua_st_voice_set_filter_resonance(lua_State* L) {
    float resonance = (float)luaL_checknumber(L, 1);
    st_voice_set_filter_resonance(resonance);
    return 0;
}

static int lua_st_voice_set_filter_enabled(lua_State* L) {
    int enabled = lua_toboolean(L, 1);
    st_voice_set_filter_enabled(enabled);
    return 0;
}

// Voice master controls
static int lua_st_voice_set_master_volume(lua_State* L) {
    float volume = (float)luaL_checknumber(L, 1);
    st_voice_set_master_volume(volume);
    return 0;
}

static int lua_st_voice_get_master_volume(lua_State* L) {
    float volume = st_voice_get_master_volume();
    lua_pushnumber(L, volume);
    return 1;
}

static int lua_st_voice_reset_all(lua_State* L) {
    (void)L;
    st_voice_reset_all();
    return 0;
}

static int lua_st_voice_wait(lua_State* L) {
    float beats = (float)luaL_checknumber(L, 1);
    st_voice_wait(beats);
    return 0;
}

// Physical Modeling
static int lua_st_voice_set_physical_model(lua_State* L) {
    int voiceNum = luaL_checkinteger(L, 1);
    int modelType = luaL_checkinteger(L, 2);
    st_voice_set_physical_model(voiceNum, modelType);
    return 0;
}

static int lua_st_voice_set_physical_damping(lua_State* L) {
    int voiceNum = luaL_checkinteger(L, 1);
    float damping = (float)luaL_checknumber(L, 2);
    st_voice_set_physical_damping(voiceNum, damping);
    return 0;
}

static int lua_st_voice_set_physical_brightness(lua_State* L) {
    int voiceNum = luaL_checkinteger(L, 1);
    float brightness = (float)luaL_checknumber(L, 2);
    st_voice_set_physical_brightness(voiceNum, brightness);
    return 0;
}

static int lua_st_voice_set_physical_excitation(lua_State* L) {
    int voiceNum = luaL_checkinteger(L, 1);
    float excitation = (float)luaL_checknumber(L, 2);
    st_voice_set_physical_excitation(voiceNum, excitation);
    return 0;
}

static int lua_st_voice_set_physical_resonance(lua_State* L) {
    int voiceNum = luaL_checkinteger(L, 1);
    float resonance = (float)luaL_checknumber(L, 2);
    st_voice_set_physical_resonance(voiceNum, resonance);
    return 0;
}

static int lua_st_voice_set_physical_tension(lua_State* L) {
    int voiceNum = luaL_checkinteger(L, 1);
    float tension = (float)luaL_checknumber(L, 2);
    st_voice_set_physical_tension(voiceNum, tension);
    return 0;
}

static int lua_st_voice_set_physical_pressure(lua_State* L) {
    int voiceNum = luaL_checkinteger(L, 1);
    float pressure = (float)luaL_checknumber(L, 2);
    st_voice_set_physical_pressure(voiceNum, pressure);
    return 0;
}

static int lua_st_voice_physical_trigger(lua_State* L) {
    int voiceNum = luaL_checkinteger(L, 1);
    st_voice_physical_trigger(voiceNum);
    return 0;
}

// SID-style modulation
static int lua_st_voice_set_ring_mod(lua_State* L) {
    int voiceNum = luaL_checkinteger(L, 1);
    int sourceVoice = luaL_checkinteger(L, 2);
    st_voice_set_ring_mod(voiceNum, sourceVoice);
    return 0;
}

static int lua_st_voice_set_sync(lua_State* L) {
    int voiceNum = luaL_checkinteger(L, 1);
    int sourceVoice = luaL_checkinteger(L, 2);
    st_voice_set_sync(voiceNum, sourceVoice);
    return 0;
}

static int lua_st_voice_set_portamento(lua_State* L) {
    int voiceNum = luaL_checkinteger(L, 1);
    float time = (float)luaL_checknumber(L, 2);
    st_voice_set_portamento(voiceNum, time);
    return 0;
}

static int lua_st_voice_set_detune(lua_State* L) {
    int voiceNum = luaL_checkinteger(L, 1);
    float cents = (float)luaL_checknumber(L, 2);
    st_voice_set_detune(voiceNum, cents);
    return 0;
}

// Delay effects
static int lua_st_voice_set_delay_enable(lua_State* L) {
    int voiceNum = luaL_checkinteger(L, 1);
    int enabled = luaL_checkinteger(L, 2);
    st_voice_set_delay_enable(voiceNum, enabled);
    return 0;
}

static int lua_st_voice_set_delay_time(lua_State* L) {
    int voiceNum = luaL_checkinteger(L, 1);
    float time = (float)luaL_checknumber(L, 2);
    st_voice_set_delay_time(voiceNum, time);
    return 0;
}

static int lua_st_voice_set_delay_feedback(lua_State* L) {
    int voiceNum = luaL_checkinteger(L, 1);
    float feedback = (float)luaL_checknumber(L, 2);
    st_voice_set_delay_feedback(voiceNum, feedback);
    return 0;
}

static int lua_st_voice_set_delay_mix(lua_State* L) {
    int voiceNum = luaL_checkinteger(L, 1);
    float mix = (float)luaL_checknumber(L, 2);
    st_voice_set_delay_mix(voiceNum, mix);
    return 0;
}

// LFO controls
static int lua_st_lfo_set_waveform(lua_State* L) {
    int lfoNum = luaL_checkinteger(L, 1);
    int waveform = luaL_checkinteger(L, 2);
    st_lfo_set_waveform(lfoNum, waveform);
    return 0;
}

static int lua_st_lfo_set_rate(lua_State* L) {
    int lfoNum = luaL_checkinteger(L, 1);
    float rateHz = (float)luaL_checknumber(L, 2);
    st_lfo_set_rate(lfoNum, rateHz);
    return 0;
}

static int lua_st_lfo_reset(lua_State* L) {
    int lfoNum = luaL_checkinteger(L, 1);
    st_lfo_reset(lfoNum);
    return 0;
}

static int lua_st_lfo_to_pitch(lua_State* L) {
    int voiceNum = luaL_checkinteger(L, 1);
    int lfoNum = luaL_checkinteger(L, 2);
    float depthCents = (float)luaL_checknumber(L, 3);
    st_lfo_to_pitch(voiceNum, lfoNum, depthCents);
    return 0;
}

static int lua_st_lfo_to_volume(lua_State* L) {
    int voiceNum = luaL_checkinteger(L, 1);
    int lfoNum = luaL_checkinteger(L, 2);
    float depth = (float)luaL_checknumber(L, 3);
    st_lfo_to_volume(voiceNum, lfoNum, depth);
    return 0;
}

static int lua_st_lfo_to_filter(lua_State* L) {
    int voiceNum = luaL_checkinteger(L, 1);
    int lfoNum = luaL_checkinteger(L, 2);
    float depthHz = (float)luaL_checknumber(L, 3);
    st_lfo_to_filter(voiceNum, lfoNum, depthHz);
    return 0;
}

static int lua_st_lfo_to_pulsewidth(lua_State* L) {
    int voiceNum = luaL_checkinteger(L, 1);
    int lfoNum = luaL_checkinteger(L, 2);
    float depth = (float)luaL_checknumber(L, 3);
    st_lfo_to_pulsewidth(voiceNum, lfoNum, depth);
    return 0;
}

// Voice timeline/rendering
static int lua_st_voices_start(lua_State* L) {
    (void)L;
    st_voices_start();
    return 0;
}

static int lua_st_voices_set_tempo(lua_State* L) {
    float bpm = (float)luaL_checknumber(L, 1);
    st_voices_set_tempo(bpm);
    return 0;
}

static int lua_st_voices_end_slot(lua_State* L) {
    int slot = luaL_checkinteger(L, 1);
    float volume = (float)luaL_optnumber(L, 2, 1.0);
    st_voices_end_slot(slot, volume);
    return 0;
}

static int lua_st_voices_next_slot(lua_State* L) {
    float volume = (float)luaL_optnumber(L, 1, 1.0);
    uint32_t soundId = st_voices_next_slot(volume);
    lua_pushinteger(L, soundId);
    return 1;
}

static int lua_st_voices_end_play(lua_State* L) {
    (void)L;
    st_voices_end_play();
    return 0;
}

static int lua_st_voices_end_save(lua_State* L) {
    const char* filename = luaL_checkstring(L, 1);
    st_voices_end_save(filename);
    return 0;
}

static int lua_st_voices_are_playing(lua_State* L) {
    int playing = st_voices_are_playing();
    lua_pushboolean(L, playing);
    return 1;
}

static int lua_st_voice_direct(lua_State* L) {
    const char* destination = luaL_checkstring(L, 1);
    st_voice_direct(destination);
    return 0;
}

static int lua_st_voice_direct_slot(lua_State* L) {
    int slot = luaL_checkinteger(L, 1);
    float volume = (float)luaL_optnumber(L, 2, 1.0);
    float duration = (float)luaL_optnumber(L, 3, 0.0);
    uint32_t sound_id = st_voice_direct_slot(slot, volume, duration);
    lua_pushinteger(L, sound_id);
    return 1;
}

// VoiceScript
static int lua_st_vscript_save_to_bank(lua_State* L) {
    const char* scriptName = luaL_checkstring(L, 1);
    float duration = (float)luaL_optnumber(L, 2, 0.0);
    uint32_t sound_id = st_vscript_save_to_bank(scriptName, duration);
    lua_pushinteger(L, sound_id);
    return 1;
}

// =============================================================================
// Input API Bindings
// =============================================================================

static int lua_st_key_pressed(lua_State* L) {
    int key = luaL_checkinteger(L, 1);
    lua_pushboolean(L, st_key_pressed((STKeyCode)key));
    return 1;
}

static int lua_st_key_just_pressed(lua_State* L) {
    int key = luaL_checkinteger(L, 1);
    lua_pushboolean(L, st_key_just_pressed((STKeyCode)key));
    return 1;
}

static int lua_st_key_just_released(lua_State* L) {
    int key = luaL_checkinteger(L, 1);
    lua_pushboolean(L, st_key_just_released((STKeyCode)key));
    return 1;
}

static int lua_st_key_get_char(lua_State* L) {
    uint32_t ch = st_key_get_char();
    if (ch) {
        char str[2] = { (char)ch, 0 };
        lua_pushstring(L, str);
    } else {
        lua_pushnil(L);
    }
    return 1;
}

static int lua_st_key_clear_buffer(lua_State* L) {
    (void)L;
    st_key_clear_buffer();
    return 0;
}

static int lua_st_mouse_position(lua_State* L) {
    int x, y;
    st_mouse_position(&x, &y);
    lua_pushinteger(L, x);
    lua_pushinteger(L, y);
    return 2;
}

static int lua_st_mouse_grid_position(lua_State* L) {
    int x, y;
    st_mouse_grid_position(&x, &y);
    lua_pushinteger(L, x);
    lua_pushinteger(L, y);
    return 2;
}

static int lua_st_mouse_button(lua_State* L) {
    int button = luaL_checkinteger(L, 1);
    lua_pushboolean(L, st_mouse_button((STMouseButton)button));
    return 1;
}

static int lua_st_mouse_button_just_pressed(lua_State* L) {
    int button = luaL_checkinteger(L, 1);
    lua_pushboolean(L, st_mouse_button_just_pressed((STMouseButton)button));
    return 1;
}

static int lua_st_mouse_button_just_released(lua_State* L) {
    int button = luaL_checkinteger(L, 1);
    lua_pushboolean(L, st_mouse_button_just_released((STMouseButton)button));
    return 1;
}

// =============================================================================
// Frame Control API Bindings
// =============================================================================

static int lua_st_wait_frame(lua_State* L) {
    (void)L;
    st_wait_frame();
    return 0;
}

static int lua_st_wait_frames(lua_State* L) {
    int count = luaL_checkinteger(L, 1);
    st_wait_frames(count);
    return 0;
}

static int lua_st_wait_key(lua_State* L) {
    // Wait for any key press with optional timeout
    int timeout_frames = luaL_optinteger(L, 1, -1);  // -1 = no timeout
    int frames_waited = 0;
    
    while (true) {
        st_wait_frame();
        frames_waited++;
        
        // Check timeout
        if (timeout_frames > 0 && frames_waited >= timeout_frames) {
            lua_pushboolean(L, 0);  // Return false on timeout
            return 1;
        }
        
        // Check if ANY key is currently pressed
        for (int key = 0; key < 512; key++) {
            if (st_key_pressed(static_cast<STKeyCode>(key))) {
                lua_pushboolean(L, 1);  // Return true on key press
                return 1;
            }
        }
    }
    return 0;
}

static int lua_st_wait(lua_State* L) {
    float seconds = (float)luaL_checknumber(L, 1);
    int frames = (int)(seconds * 60.0f);  // Assume 60 FPS
    for (int i = 0; i < frames; i++) {
        st_wait_frame();
    }
    return 0;
}

static int lua_st_sleep(lua_State* L) {
    float milliseconds = (float)luaL_checknumber(L, 1);
    float seconds = milliseconds / 1000.0f;
    int frames = (int)(seconds * 60.0f);  // Assume 60 FPS
    for (int i = 0; i < frames; i++) {
        st_wait_frame();
    }
    return 0;
}

static int lua_st_frame_count(lua_State* L) {
    lua_pushinteger(L, st_frame_count());
    return 1;
}

static int lua_st_time(lua_State* L) {
    lua_pushnumber(L, st_time());
    return 1;
}

static int lua_st_delta_time(lua_State* L) {
    lua_pushnumber(L, st_delta_time());
    return 1;
}

// =============================================================================
// Utility API Bindings
// =============================================================================

static int lua_st_rgb(lua_State* L) {
    int r = luaL_checkinteger(L, 1);
    int g = luaL_checkinteger(L, 2);
    int b = luaL_checkinteger(L, 3);

    lua_pushinteger(L, st_rgb(r, g, b));
    return 1;
}

static int lua_st_rgba(lua_State* L) {
    int r = luaL_checkinteger(L, 1);
    int g = luaL_checkinteger(L, 2);
    int b = luaL_checkinteger(L, 3);
    int a = luaL_checkinteger(L, 4);

    lua_pushinteger(L, st_rgba(r, g, b, a));
    return 1;
}

static int lua_st_hsv(lua_State* L) {
    float h = luaL_checknumber(L, 1);
    float s = luaL_checknumber(L, 2);
    float v = luaL_checknumber(L, 3);

    lua_pushinteger(L, st_hsv(h, s, v));
    return 1;
}

static int lua_st_debug_print(lua_State* L) {
    const char* msg = luaL_checkstring(L, 1);
    st_debug_print(msg);
    return 0;
}

// =============================================================================
// Display API Bindings
// =============================================================================

static int lua_st_display_size(lua_State* L) {
    int width, height;
    st_display_size(&width, &height);
    lua_pushinteger(L, width);
    lua_pushinteger(L, height);
    return 2;
}

static int lua_st_cell_size(lua_State* L) {
    int width, height;
    st_cell_size(&width, &height);
    lua_pushinteger(L, width);
    lua_pushinteger(L, height);
    return 2;
}

// =============================================================================
// Registration Function
// =============================================================================

// Chunky Pixel Graphics API Bindings
// =============================================================================

// DISABLED: chunky API no longer exists
/*
static int lua_st_chunky_pset(lua_State* L) {
    int pixel_x = luaL_checkinteger(L, 1);
    int pixel_y = luaL_checkinteger(L, 2);
    uint8_t color_index = (uint8_t)luaL_checkinteger(L, 3);
    uint32_t bg = luaL_optinteger(L, 4, 0xFF000000);

    st_chunky_pset(pixel_x, pixel_y, color_index, bg);
    return 0;
}
*/

/*
static int lua_st_chunky_line(lua_State* L) {
    int x1 = luaL_checkinteger(L, 1);
    int y1 = luaL_checkinteger(L, 2);
    int x2 = luaL_checkinteger(L, 3);
    int y2 = luaL_checkinteger(L, 4);
    uint8_t color_index = (uint8_t)luaL_checkinteger(L, 5);
    uint32_t bg = luaL_optinteger(L, 6, 0xFF000000);

    st_chunky_line(x1, y1, x2, y2, color_index, bg);
    return 0;
}
*/

/*
static int lua_st_chunky_rect(lua_State* L) {
    int x = luaL_checkinteger(L, 1);
    int y = luaL_checkinteger(L, 2);
    int width = luaL_checkinteger(L, 3);
    int height = luaL_checkinteger(L, 4);
    uint8_t color_index = (uint8_t)luaL_checkinteger(L, 5);
    uint32_t bg = luaL_optinteger(L, 6, 0xFF000000);

    st_chunky_rect(x, y, width, height, color_index, bg);
    return 0;
}
*/

/*
static int lua_st_chunky_fillrect(lua_State* L) {
    int x = luaL_checkinteger(L, 1);
    int y = luaL_checkinteger(L, 2);
    int width = luaL_checkinteger(L, 3);
    int height = luaL_checkinteger(L, 4);
    uint8_t color_index = (uint8_t)luaL_checkinteger(L, 5);
    uint32_t bg = luaL_optinteger(L, 6, 0xFF000000);

    st_chunky_fillrect(x, y, width, height, color_index, bg);
    return 0;
}
*/

/*
static int lua_st_chunky_hline(lua_State* L) {
    int x = luaL_checkinteger(L, 1);
    int y = luaL_checkinteger(L, 2);
    int width = luaL_checkinteger(L, 3);
    uint8_t color_index = (uint8_t)luaL_checkinteger(L, 4);
    uint32_t bg = luaL_optinteger(L, 5, 0xFF000000);

    st_chunky_hline(x, y, width, color_index, bg);
    return 0;
}
*/

/*
static int lua_st_chunky_vline(lua_State* L) {
    int x = luaL_checkinteger(L, 1);
    int y = luaL_checkinteger(L, 2);
    int height = luaL_checkinteger(L, 3);
    uint8_t color_index = (uint8_t)luaL_checkinteger(L, 4);
    uint32_t bg = luaL_optinteger(L, 5, 0xFF000000);

    st_chunky_vline(x, y, height, color_index, bg);
    return 0;
}
*/

/*
static int lua_st_chunky_clear(lua_State* L) {
    uint32_t bg = luaL_optinteger(L, 1, 0xFF000000);
    st_chunky_clear(bg);
    return 0;
}
*/

/*
static int lua_st_chunky_get_resolution(lua_State* L) {
    int width, height;
    st_chunky_get_resolution(&width, &height);

    lua_pushinteger(L, width);
    lua_pushinteger(L, height);
    return 2;
}
*/

// =============================================================================
// Particle System API Bindings
// =============================================================================

static int lua_st_sprite_explode(lua_State* L) {
    float x = luaL_checknumber(L, 1);
    float y = luaL_checknumber(L, 2);
    int particleCount = luaL_checkinteger(L, 3);
    uint32_t color = luaL_checkinteger(L, 4);

    bool result = st_sprite_explode(x, y, particleCount, color);
    lua_pushboolean(L, result);
    return 1;
}

static int lua_st_sprite_explode_advanced(lua_State* L) {
    float x = luaL_checknumber(L, 1);
    float y = luaL_checknumber(L, 2);
    int particleCount = luaL_checkinteger(L, 3);
    uint32_t color = luaL_checkinteger(L, 4);
    float force = luaL_checknumber(L, 5);
    float gravity = luaL_checknumber(L, 6);
    float fadeTime = luaL_checknumber(L, 7);

    bool result = st_sprite_explode_advanced(x, y, particleCount, color, force, gravity, fadeTime);
    lua_pushboolean(L, result);
    return 1;
}

static int lua_st_sprite_explode_directional(lua_State* L) {
    float x = luaL_checknumber(L, 1);
    float y = luaL_checknumber(L, 2);
    int particleCount = luaL_checkinteger(L, 3);
    uint32_t color = luaL_checkinteger(L, 4);
    float forceX = luaL_checknumber(L, 5);
    float forceY = luaL_checknumber(L, 6);

    bool result = st_sprite_explode_directional(x, y, particleCount, color, forceX, forceY);
    lua_pushboolean(L, result);
    return 1;
}

static int lua_st_particle_clear(lua_State* L) {
    st_particle_clear();
    return 0;
}

static int lua_st_particle_pause(lua_State* L) {
    st_particle_pause();
    return 0;
}

static int lua_st_particle_resume(lua_State* L) {
    st_particle_resume();
    return 0;
}

static int lua_st_particle_set_time_scale(lua_State* L) {
    float scale = luaL_checknumber(L, 1);
    st_particle_set_time_scale(scale);
    return 0;
}

static int lua_st_particle_set_world_bounds(lua_State* L) {
    float width = luaL_checknumber(L, 1);
    float height = luaL_checknumber(L, 2);
    st_particle_set_world_bounds(width, height);
    return 0;
}

static int lua_st_particle_set_enabled(lua_State* L) {
    bool enabled = lua_toboolean(L, 1);
    st_particle_set_enabled(enabled);
    return 0;
}

static int lua_st_particle_get_active_count(lua_State* L) {
    uint32_t count = st_particle_get_active_count();
    lua_pushinteger(L, count);
    return 1;
}

static int lua_st_particle_get_total_created(lua_State* L) {
    uint64_t total = st_particle_get_total_created();
    lua_pushinteger(L, total);
    return 1;
}

static int lua_st_particle_dump_stats(lua_State* L) {
    st_particle_dump_stats();
    return 0;
}

// =============================================================================
// Sprite Management API
// =============================================================================

static int lua_st_sprite_load(lua_State* L) {
    const char* path = luaL_checkstring(L, 1);
    int sprite_id = st_sprite_load(path);
    lua_pushinteger(L, sprite_id);
    return 1;
}

static int lua_st_sprite_load_builtin(lua_State* L) {
    const char* name = luaL_checkstring(L, 1);
    int sprite_id = st_sprite_load_builtin(name);
    lua_pushinteger(L, sprite_id);
    return 1;
}

static int lua_st_sprite_load_sprtz(lua_State* L) {
    const char* path = luaL_checkstring(L, 1);
    int sprite_id = st_sprite_load_sprtz(path);
    lua_pushinteger(L, sprite_id);
    return 1;
}

static int lua_st_sprite_show(lua_State* L) {
    int sprite_id = luaL_checkinteger(L, 1);
    int x = luaL_checkinteger(L, 2);
    int y = luaL_checkinteger(L, 3);
    st_sprite_show(sprite_id, x, y);
    return 0;
}

static int lua_st_sprite_hide(lua_State* L) {
    int sprite_id = luaL_checkinteger(L, 1);
    st_sprite_hide(sprite_id);
    return 0;
}

static int lua_st_sprite_transform(lua_State* L) {
    int sprite_id = luaL_checkinteger(L, 1);
    int x = luaL_checkinteger(L, 2);
    int y = luaL_checkinteger(L, 3);
    float rotation = luaL_checknumber(L, 4);
    float scale_x = luaL_checknumber(L, 5);
    float scale_y = luaL_checknumber(L, 6);
    st_sprite_transform(sprite_id, x, y, rotation, scale_x, scale_y);
    return 0;
}

static int lua_st_sprite_tint(lua_State* L) {
    int sprite_id = luaL_checkinteger(L, 1);
    uint32_t color = luaL_checkinteger(L, 2);
    st_sprite_tint(sprite_id, color);
    return 0;
}

static int lua_st_sprite_unload(lua_State* L) {
    int sprite_id = luaL_checkinteger(L, 1);
    st_sprite_unload(sprite_id);
    return 0;
}

// =============================================================================
// Indexed Sprite API
// =============================================================================

static int lua_st_sprite_load_indexed_from_rgba(lua_State* L) {
    // Get RGBA pixel data table
    luaL_checktype(L, 1, LUA_TTABLE);
    int width = luaL_checkinteger(L, 2);
    int height = luaL_checkinteger(L, 3);
    
    int pixel_count = width * height;
    int expected_size = pixel_count * 4;
    
    // Allocate buffer for RGBA pixels
    std::vector<uint8_t> pixels(expected_size);
    
    // Read pixels from Lua table
    for (int i = 0; i < expected_size; i++) {
        lua_rawgeti(L, 1, i + 1);
        pixels[i] = (uint8_t)luaL_checkinteger(L, -1);
        lua_pop(L, 1);
    }
    
    // Optional: return generated palette
    uint8_t palette[64];
    int sprite_id = st_sprite_load_indexed_from_rgba(pixels.data(), width, height, palette);
    
    if (sprite_id < 0) {
        lua_pushnil(L);
        return 1;
    }
    
    lua_pushinteger(L, sprite_id);
    
    // Return palette as second return value
    lua_createtable(L, 64, 0);
    for (int i = 0; i < 64; i++) {
        lua_pushinteger(L, palette[i]);
        lua_rawseti(L, -2, i + 1);
    }
    
    return 2;
}

static int lua_st_sprite_is_indexed(lua_State* L) {
    int sprite_id = luaL_checkinteger(L, 1);
    bool is_indexed = st_sprite_is_indexed(sprite_id);
    lua_pushboolean(L, is_indexed);
    return 1;
}

static int lua_st_sprite_set_palette(lua_State* L) {
    int sprite_id = luaL_checkinteger(L, 1);
    luaL_checktype(L, 2, LUA_TTABLE);
    
    uint8_t palette[64];
    for (int i = 0; i < 64; i++) {
        lua_rawgeti(L, 2, i + 1);
        palette[i] = (uint8_t)luaL_checkinteger(L, -1);
        lua_pop(L, 1);
    }
    
    bool success = st_sprite_set_palette(sprite_id, palette);
    lua_pushboolean(L, success);
    return 1;
}

static int lua_st_sprite_get_palette(lua_State* L) {
    int sprite_id = luaL_checkinteger(L, 1);
    
    uint8_t palette[64];
    bool success = st_sprite_get_palette(sprite_id, palette);
    
    if (!success) {
        lua_pushnil(L);
        return 1;
    }
    
    lua_createtable(L, 64, 0);
    for (int i = 0; i < 64; i++) {
        lua_pushinteger(L, palette[i]);
        lua_rawseti(L, -2, i + 1);
    }
    
    return 1;
}

static int lua_st_sprite_set_palette_color(lua_State* L) {
    int sprite_id = luaL_checkinteger(L, 1);
    int color_index = luaL_checkinteger(L, 2);
    uint8_t r = (uint8_t)luaL_checkinteger(L, 3);
    uint8_t g = (uint8_t)luaL_checkinteger(L, 4);
    uint8_t b = (uint8_t)luaL_checkinteger(L, 5);
    uint8_t a = (uint8_t)luaL_optinteger(L, 6, 255);
    
    bool success = st_sprite_set_palette_color(sprite_id, color_index, r, g, b, a);
    lua_pushboolean(L, success);
    return 1;
}

static int lua_st_sprite_lerp_palette(lua_State* L) {
    int sprite_id = luaL_checkinteger(L, 1);
    luaL_checktype(L, 2, LUA_TTABLE);
    luaL_checktype(L, 3, LUA_TTABLE);
    float t = (float)luaL_checknumber(L, 4);
    
    uint8_t palette_a[64];
    uint8_t palette_b[64];
    
    for (int i = 0; i < 64; i++) {
        lua_rawgeti(L, 2, i + 1);
        palette_a[i] = (uint8_t)luaL_checkinteger(L, -1);
        lua_pop(L, 1);
        
        lua_rawgeti(L, 3, i + 1);
        palette_b[i] = (uint8_t)luaL_checkinteger(L, -1);
        lua_pop(L, 1);
    }
    
    bool success = st_sprite_lerp_palette(sprite_id, palette_a, palette_b, t);
    lua_pushboolean(L, success);
    return 1;
}

static int lua_st_sprite_rotate_palette(lua_State* L) {
    int sprite_id = luaL_checkinteger(L, 1);
    int start_index = luaL_checkinteger(L, 2);
    int end_index = luaL_checkinteger(L, 3);
    int amount = luaL_checkinteger(L, 4);
    
    bool success = st_sprite_rotate_palette(sprite_id, start_index, end_index, amount);
    lua_pushboolean(L, success);
    return 1;
}

static int lua_st_sprite_adjust_brightness(lua_State* L) {
    int sprite_id = luaL_checkinteger(L, 1);
    float brightness = (float)luaL_checknumber(L, 2);
    
    bool success = st_sprite_adjust_brightness(sprite_id, brightness);
    lua_pushboolean(L, success);
    return 1;
}

static int lua_st_sprite_copy_palette(lua_State* L) {
    int src_sprite_id = luaL_checkinteger(L, 1);
    int dst_sprite_id = luaL_checkinteger(L, 2);
    
    bool success = st_sprite_copy_palette(src_sprite_id, dst_sprite_id);
    lua_pushboolean(L, success);
    return 1;
}

static int lua_st_sprite_set_standard_palette(lua_State* L) {
    int sprite_id = luaL_checkinteger(L, 1);
    int palette_id = luaL_checkinteger(L, 2);
    
    if (palette_id < 0 || palette_id > 31) {
        lua_pushboolean(L, false);
        return 1;
    }
    
    bool success = st_sprite_set_standard_palette(sprite_id, (uint8_t)palette_id);
    lua_pushboolean(L, success);
    return 1;
}

// =============================================================================
// Sprite-based Particle Explosion API (v1 compatible)
// =============================================================================

static int lua_sprite_explode(lua_State* L) {
    int sprite_id = luaL_checkinteger(L, 1);
    int particle_count = 32; // Default

    if (lua_gettop(L) >= 2) {
        particle_count = luaL_checkinteger(L, 2);
    }

    // Optional mode parameter (default to SPRITE_FRAGMENT for v1 compatibility)
    ParticleMode mode = ParticleMode::SPRITE_FRAGMENT;
    if (lua_gettop(L) >= 3) {
        int mode_int = luaL_checkinteger(L, 3);
        mode = (mode_int == 1) ? ParticleMode::SPRITE_FRAGMENT : ParticleMode::POINT_SPRITE;
    }

    // Validate parameters
    if (sprite_id < 1 || sprite_id > 1024) {
        return luaL_error(L, "sprite_explode: sprite_id must be between 1 and 1024");
    }

    if (particle_count < 1 || particle_count > 500) {
        return luaL_error(L, "sprite_explode: particle_count must be between 1 and 500");
    }

    bool result = sprite_explode((uint16_t)sprite_id, (uint16_t)particle_count);
    lua_pushboolean(L, result);
    return 1;
}

static int lua_sprite_explode_advanced(lua_State* L) {
    int sprite_id = luaL_checkinteger(L, 1);
    int particle_count = luaL_checkinteger(L, 2);

    // Get optional parameters with defaults
    float explosion_force = 200.0f;
    float gravity = 100.0f;
    float fade_time = 2.0f;

    if (lua_gettop(L) >= 3) explosion_force = luaL_checknumber(L, 3);
    if (lua_gettop(L) >= 4) gravity = luaL_checknumber(L, 4);
    if (lua_gettop(L) >= 5) fade_time = luaL_checknumber(L, 5);

    // Optional mode parameter (default to SPRITE_FRAGMENT for v1 compatibility)
    ParticleMode mode = ParticleMode::SPRITE_FRAGMENT;
    if (lua_gettop(L) >= 6) {
        int mode_int = luaL_checkinteger(L, 6);
        mode = (mode_int == 1) ? ParticleMode::SPRITE_FRAGMENT : ParticleMode::POINT_SPRITE;
    }

    // Validate parameters
    if (sprite_id < 1 || sprite_id > 1024) {
        return luaL_error(L, "sprite_explode_advanced: sprite_id must be between 1 and 1024");
    }

    if (particle_count < 1 || particle_count > 500) {
        return luaL_error(L, "sprite_explode_advanced: particle_count must be between 1 and 500");
    }

    bool result = sprite_explode_advanced((uint16_t)sprite_id, (uint16_t)particle_count,
                                         explosion_force, gravity, fade_time);
    lua_pushboolean(L, result);
    return 1;
}

static int lua_sprite_explode_directional(lua_State* L) {
    int sprite_id = luaL_checkinteger(L, 1);
    int particle_count = luaL_checkinteger(L, 2);
    float force_x = luaL_checknumber(L, 3);
    float force_y = luaL_checknumber(L, 4);

    // Validate parameters
    if (sprite_id < 1 || sprite_id > 1024) {
        return luaL_error(L, "sprite_explode_directional: sprite_id must be between 1 and 1024");
    }

    if (particle_count < 1 || particle_count > 500) {
        return luaL_error(L, "sprite_explode_directional: particle_count must be between 1 and 500");
    }

    bool result = sprite_explode_directional((uint16_t)sprite_id, (uint16_t)particle_count,
                                            force_x, force_y);
    lua_pushboolean(L, result);
    return 1;
}

static int lua_sprite_explode_mode(lua_State* L) {
    int sprite_id = luaL_checkinteger(L, 1);
    int explosion_mode = luaL_checkinteger(L, 2);

    // Validate sprite ID
    if (sprite_id < 1 || sprite_id > 1024) {
        return luaL_error(L, "sprite_explode_mode: sprite_id must be between 1 and 1024");
    }

    // Validate explosion mode
    if (explosion_mode < 1 || explosion_mode > 6) {
        return luaL_error(L, "sprite_explode_mode: explosion_mode must be between 1 and 6");
    }

    bool success = false;

    // Apply the appropriate explosion mode
    switch (explosion_mode) {
        case 1: // BASIC_EXPLOSION
            success = sprite_explode_advanced((uint16_t)sprite_id, 48, 200.0f, 100.0f, 2.0f);
            break;
        case 2: // MASSIVE_BLAST
            success = sprite_explode_advanced((uint16_t)sprite_id, 128, 350.0f, 80.0f, 3.0f);
            break;
        case 3: // GENTLE_DISPERSAL
            success = sprite_explode_advanced((uint16_t)sprite_id, 64, 120.0f, 40.0f, 4.0f);
            break;
        case 4: // RIGHTWARD_BLAST
            success = sprite_explode_directional((uint16_t)sprite_id, 80, 180.0f, -30.0f);
            break;
        case 5: // UPWARD_ERUPTION
            success = sprite_explode_directional((uint16_t)sprite_id, 96, 0.0f, -250.0f);
            break;
        case 6: // RAPID_BURST
            success = sprite_explode_advanced((uint16_t)sprite_id, 32, 400.0f, 200.0f, 1.0f);
            break;
        default:
            return luaL_error(L, "sprite_explode_mode: invalid explosion_mode");
    }

    lua_pushboolean(L, success);
    return 1;
}

// =============================================================================
// Asset Management API Bindings
// =============================================================================

// Initialization
static int lua_st_asset_init(lua_State* L) {
    const char* db_path = luaL_checkstring(L, 1);
    size_t max_cache_size = luaL_optinteger(L, 2, 0);

    bool result = st_asset_init(db_path, max_cache_size);
    lua_pushboolean(L, result);
    return 1;
}

static int lua_st_asset_shutdown(lua_State* L) {
    (void)L;
    st_asset_shutdown();
    return 0;
}

static int lua_st_asset_is_initialized(lua_State* L) {
    (void)L;
    bool result = st_asset_is_initialized();
    lua_pushboolean(L, result);
    return 1;
}

// Loading/Unloading
static int lua_st_asset_load(lua_State* L) {
    const char* name = luaL_checkstring(L, 1);

    STAssetID asset = st_asset_load(name);
    lua_pushinteger(L, asset);
    return 1;
}

static int lua_st_asset_load_file(lua_State* L) {
    const char* path = luaL_checkstring(L, 1);
    int type = luaL_checkinteger(L, 2);

    STAssetID asset = st_asset_load_file(path, (STAssetType)type);
    lua_pushinteger(L, asset);
    return 1;
}

static int lua_st_asset_unload(lua_State* L) {
    STAssetID asset = luaL_checkinteger(L, 1);
    st_asset_unload(asset);
    return 0;
}

static int lua_st_asset_is_loaded(lua_State* L) {
    const char* name = luaL_checkstring(L, 1);

    bool result = st_asset_is_loaded(name);
    lua_pushboolean(L, result);
    return 1;
}

// Import/Export
static int lua_st_asset_import(lua_State* L) {
    const char* file_path = luaL_checkstring(L, 1);
    const char* asset_name = luaL_checkstring(L, 2);
    int type = luaL_optinteger(L, 3, -1);

    bool result = st_asset_import(file_path, asset_name, type);
    lua_pushboolean(L, result);
    return 1;
}

static int lua_st_asset_import_directory(lua_State* L) {
    const char* directory = luaL_checkstring(L, 1);
    bool recursive = lua_toboolean(L, 2);

    int count = st_asset_import_directory(directory, recursive);
    lua_pushinteger(L, count);
    return 1;
}

static int lua_st_asset_export(lua_State* L) {
    const char* asset_name = luaL_checkstring(L, 1);
    const char* file_path = luaL_checkstring(L, 2);

    bool result = st_asset_export(asset_name, file_path);
    lua_pushboolean(L, result);
    return 1;
}

static int lua_st_asset_delete(lua_State* L) {
    const char* asset_name = luaL_checkstring(L, 1);

    bool result = st_asset_delete(asset_name);
    lua_pushboolean(L, result);
    return 1;
}

// Data Access
static int lua_st_asset_get_data(lua_State* L) {
    STAssetID asset = luaL_checkinteger(L, 1);

    const void* data = st_asset_get_data(asset);
    size_t size = st_asset_get_size(asset);

    if (data && size > 0) {
        lua_pushlstring(L, (const char*)data, size);
        return 1;
    }

    lua_pushnil(L);
    return 1;
}

static int lua_st_asset_get_size(lua_State* L) {
    STAssetID asset = luaL_checkinteger(L, 1);

    size_t size = st_asset_get_size(asset);
    lua_pushinteger(L, size);
    return 1;
}

static int lua_st_asset_get_type(lua_State* L) {
    STAssetID asset = luaL_checkinteger(L, 1);

    int type = st_asset_get_type(asset);
    lua_pushinteger(L, type);
    return 1;
}

static int lua_st_asset_get_name(lua_State* L) {
    STAssetID asset = luaL_checkinteger(L, 1);

    const char* name = st_asset_get_name(asset);
    if (name) {
        lua_pushstring(L, name);
    } else {
        lua_pushnil(L);
    }
    return 1;
}

// Queries
static int lua_st_asset_exists(lua_State* L) {
    const char* name = luaL_checkstring(L, 1);

    bool result = st_asset_exists(name);
    lua_pushboolean(L, result);
    return 1;
}

static int lua_st_asset_list(lua_State* L) {
    int type = luaL_optinteger(L, 1, -1);

    // Get count first
    int count = st_asset_list(type, nullptr, 0);

    if (count <= 0) {
        lua_newtable(L);
        return 1;
    }

    // Allocate array for names
    const char** names = new const char*[count];
    st_asset_list(type, names, count);

    // Create Lua table
    lua_createtable(L, count, 0);
    for (int i = 0; i < count; i++) {
        lua_pushstring(L, names[i]);
        lua_rawseti(L, -2, i + 1);  // Lua arrays are 1-indexed
    }

    delete[] names;
    return 1;
}

static int lua_st_asset_search(lua_State* L) {
    const char* pattern = luaL_checkstring(L, 1);

    // Get count first
    int count = st_asset_search(pattern, nullptr, 0);

    if (count <= 0) {
        lua_newtable(L);
        return 1;
    }

    // Allocate array for names
    const char** names = new const char*[count];
    st_asset_search(pattern, names, count);

    // Create Lua table
    lua_createtable(L, count, 0);
    for (int i = 0; i < count; i++) {
        lua_pushstring(L, names[i]);
        lua_rawseti(L, -2, i + 1);
    }

    delete[] names;
    return 1;
}

static int lua_st_asset_get_count(lua_State* L) {
    int type = luaL_optinteger(L, 1, -1);

    int count = st_asset_get_count(type);
    lua_pushinteger(L, count);
    return 1;
}

// Cache Management
static int lua_st_asset_clear_cache(lua_State* L) {
    (void)L;
    st_asset_clear_cache();
    return 0;
}

static int lua_st_asset_get_cache_size(lua_State* L) {
    (void)L;
    size_t size = st_asset_get_cache_size();
    lua_pushinteger(L, size);
    return 1;
}

static int lua_st_asset_get_cached_count(lua_State* L) {
    (void)L;
    int count = st_asset_get_cached_count();
    lua_pushinteger(L, count);
    return 1;
}

static int lua_st_asset_set_max_cache_size(lua_State* L) {
    size_t max_size = luaL_checkinteger(L, 1);
    st_asset_set_max_cache_size(max_size);
    return 0;
}

// Statistics
static int lua_st_asset_get_hit_rate(lua_State* L) {
    (void)L;
    double rate = st_asset_get_hit_rate();
    lua_pushnumber(L, rate);
    return 1;
}

static int lua_st_asset_get_database_size(lua_State* L) {
    (void)L;
    size_t size = st_asset_get_database_size();
    lua_pushinteger(L, size);
    return 1;
}

// Error Handling
static int lua_st_asset_get_error(lua_State* L) {
    (void)L;
    const char* error = st_asset_get_error();
    if (error) {
        lua_pushstring(L, error);
    } else {
        lua_pushnil(L);
    }
    return 1;
}

static int lua_st_asset_clear_error(lua_State* L) {
    (void)L;
    st_asset_clear_error();
    return 0;
}

// =============================================================================
// Tilemap API
// =============================================================================

static int lua_st_tilemap_init(lua_State* L) {
    float width = (float)luaL_checknumber(L, 1);
    float height = (float)luaL_checknumber(L, 2);
    bool result = st_tilemap_init(width, height);
    lua_pushboolean(L, result);
    return 1;
}

static int lua_st_tilemap_shutdown(lua_State* L) {
    (void)L;
    st_tilemap_shutdown();
    return 0;
}

static int lua_st_tilemap_create(lua_State* L) {
    int32_t width = (int32_t)luaL_checkinteger(L, 1);
    int32_t height = (int32_t)luaL_checkinteger(L, 2);
    int32_t tileWidth = (int32_t)luaL_checkinteger(L, 3);
    int32_t tileHeight = (int32_t)luaL_checkinteger(L, 4);
    STTilemapID id = st_tilemap_create(width, height, tileWidth, tileHeight);
    lua_pushinteger(L, id);
    return 1;
}

static int lua_st_tilemap_destroy(lua_State* L) {
    STTilemapID id = (STTilemapID)luaL_checkinteger(L, 1);
    st_tilemap_destroy(id);
    return 0;
}

static int lua_st_tilemap_get_size(lua_State* L) {
    STTilemapID id = (STTilemapID)luaL_checkinteger(L, 1);
    int32_t width, height;
    st_tilemap_get_size(id, &width, &height);
    lua_pushinteger(L, width);
    lua_pushinteger(L, height);
    return 2;
}

static int lua_st_tilemap_create_layer(lua_State* L) {
    const char* name = lua_isstring(L, 1) ? lua_tostring(L, 1) : nullptr;
    STLayerID id = st_tilemap_create_layer(name);
    lua_pushinteger(L, id);
    return 1;
}

static int lua_st_tilemap_destroy_layer(lua_State* L) {
    STLayerID id = (STLayerID)luaL_checkinteger(L, 1);
    st_tilemap_destroy_layer(id);
    return 0;
}

static int lua_st_tilemap_layer_set_tilemap(lua_State* L) {
    STLayerID layer = (STLayerID)luaL_checkinteger(L, 1);
    STTilemapID tilemap = (STTilemapID)luaL_checkinteger(L, 2);
    st_tilemap_layer_set_tilemap(layer, tilemap);
    return 0;
}

static int lua_st_tilemap_layer_set_parallax(lua_State* L) {
    STLayerID layer = (STLayerID)luaL_checkinteger(L, 1);
    float parallaxX = (float)luaL_checknumber(L, 2);
    float parallaxY = (float)luaL_checknumber(L, 3);
    st_tilemap_layer_set_parallax(layer, parallaxX, parallaxY);
    return 0;
}

static int lua_st_tilemap_layer_set_opacity(lua_State* L) {
    STLayerID layer = (STLayerID)luaL_checkinteger(L, 1);
    float opacity = (float)luaL_checknumber(L, 2);
    st_tilemap_layer_set_opacity(layer, opacity);
    return 0;
}

static int lua_st_tilemap_layer_set_visible(lua_State* L) {
    STLayerID layer = (STLayerID)luaL_checkinteger(L, 1);
    bool visible = lua_toboolean(L, 2);
    st_tilemap_layer_set_visible(layer, visible);
    return 0;
}

static int lua_st_tilemap_layer_set_z_order(lua_State* L) {
    STLayerID layer = (STLayerID)luaL_checkinteger(L, 1);
    int32_t zOrder = (int32_t)luaL_checkinteger(L, 2);
    st_tilemap_layer_set_z_order(layer, zOrder);
    return 0;
}

static int lua_st_tilemap_layer_set_auto_scroll(lua_State* L) {
    STLayerID layer = (STLayerID)luaL_checkinteger(L, 1);
    float scrollX = (float)luaL_checknumber(L, 2);
    float scrollY = (float)luaL_checknumber(L, 3);
    st_tilemap_layer_set_auto_scroll(layer, scrollX, scrollY);
    return 0;
}

static int lua_st_tilemap_set_tile(lua_State* L) {
    STLayerID layer = (STLayerID)luaL_checkinteger(L, 1);
    int32_t x = (int32_t)luaL_checkinteger(L, 2);
    int32_t y = (int32_t)luaL_checkinteger(L, 3);
    uint16_t tileID = (uint16_t)luaL_checkinteger(L, 4);
    st_tilemap_set_tile(layer, x, y, tileID);
    return 0;
}

static int lua_st_tilemap_get_tile(lua_State* L) {
    STLayerID layer = (STLayerID)luaL_checkinteger(L, 1);
    int32_t x = (int32_t)luaL_checkinteger(L, 2);
    int32_t y = (int32_t)luaL_checkinteger(L, 3);
    uint16_t tileID = st_tilemap_get_tile(layer, x, y);
    lua_pushinteger(L, tileID);
    return 1;
}

static int lua_st_tilemap_fill_rect(lua_State* L) {
    STLayerID layer = (STLayerID)luaL_checkinteger(L, 1);
    int32_t x = (int32_t)luaL_checkinteger(L, 2);
    int32_t y = (int32_t)luaL_checkinteger(L, 3);
    int32_t width = (int32_t)luaL_checkinteger(L, 4);
    int32_t height = (int32_t)luaL_checkinteger(L, 5);
    uint16_t tileID = (uint16_t)luaL_checkinteger(L, 6);
    st_tilemap_fill_rect(layer, x, y, width, height, tileID);
    return 0;
}

static int lua_st_tilemap_clear(lua_State* L) {
    STLayerID layer = (STLayerID)luaL_checkinteger(L, 1);
    st_tilemap_clear(layer);
    return 0;
}

static int lua_st_tilemap_set_camera(lua_State* L) {
    float x = (float)luaL_checknumber(L, 1);
    float y = (float)luaL_checknumber(L, 2);
    st_tilemap_set_camera(x, y);
    return 0;
}

static int lua_st_tilemap_move_camera(lua_State* L) {
    float dx = (float)luaL_checknumber(L, 1);
    float dy = (float)luaL_checknumber(L, 2);
    st_tilemap_move_camera(dx, dy);
    return 0;
}

static int lua_st_tilemap_get_camera(lua_State* L) {
    float x, y;
    st_tilemap_get_camera(&x, &y);
    lua_pushnumber(L, x);
    lua_pushnumber(L, y);
    return 2;
}

static int lua_st_tilemap_set_zoom(lua_State* L) {
    float zoom = (float)luaL_checknumber(L, 1);
    st_tilemap_set_zoom(zoom);
    return 0;
}

static int lua_st_tilemap_camera_follow(lua_State* L) {
    float targetX = (float)luaL_checknumber(L, 1);
    float targetY = (float)luaL_checknumber(L, 2);
    float smoothness = (float)luaL_checknumber(L, 3);
    st_tilemap_camera_follow(targetX, targetY, smoothness);
    return 0;
}

static int lua_st_tilemap_set_camera_bounds(lua_State* L) {
    float x = (float)luaL_checknumber(L, 1);
    float y = (float)luaL_checknumber(L, 2);
    float width = (float)luaL_checknumber(L, 3);
    float height = (float)luaL_checknumber(L, 4);
    st_tilemap_set_camera_bounds(x, y, width, height);
    return 0;
}

static int lua_st_tilemap_camera_shake(lua_State* L) {
    float magnitude = (float)luaL_checknumber(L, 1);
    float duration = (float)luaL_checknumber(L, 2);
    st_tilemap_camera_shake(magnitude, duration);
    return 0;
}

static int lua_st_tilemap_update(lua_State* L) {
    float dt = (float)luaL_checknumber(L, 1);
    st_tilemap_update(dt);
    return 0;
}

static int lua_st_tilemap_world_to_tile(lua_State* L) {
    STLayerID layer = (STLayerID)luaL_checkinteger(L, 1);
    float worldX = (float)luaL_checknumber(L, 2);
    float worldY = (float)luaL_checknumber(L, 3);
    int32_t tileX, tileY;
    st_tilemap_world_to_tile(layer, worldX, worldY, &tileX, &tileY);
    lua_pushinteger(L, tileX);
    lua_pushinteger(L, tileY);
    return 2;
}

static int lua_st_tilemap_tile_to_world(lua_State* L) {
    STLayerID layer = (STLayerID)luaL_checkinteger(L, 1);
    int32_t tileX = (int32_t)luaL_checkinteger(L, 2);
    int32_t tileY = (int32_t)luaL_checkinteger(L, 3);
    float worldX, worldY;
    st_tilemap_tile_to_world(layer, tileX, tileY, &worldX, &worldY);
    lua_pushnumber(L, worldX);
    lua_pushnumber(L, worldY);
    return 2;
}

// Error handling API
static int lua_st_get_error(lua_State* L) {
    const char* error = st_get_last_error();
    if (error && error[0] != '\0') {
        lua_pushstring(L, error);
    } else {
        lua_pushnil(L);
    }
    return 1;
}

static int lua_st_clear_error(lua_State* L) {
    (void)L;
    st_clear_error();
    return 0;
}

// Tileset API
static int lua_st_tileset_load(lua_State* L) {
    const char* imagePath = luaL_checkstring(L, 1);
    int32_t tileWidth = (int32_t)luaL_checkinteger(L, 2);
    int32_t tileHeight = (int32_t)luaL_checkinteger(L, 3);
    int32_t margin = (int32_t)luaL_optinteger(L, 4, 0);
    int32_t spacing = (int32_t)luaL_optinteger(L, 5, 0);
    STTilesetID id = st_tileset_load(imagePath, tileWidth, tileHeight, margin, spacing);
    lua_pushinteger(L, id);
    return 1;
}

static int lua_st_tileset_load_asset(lua_State* L) {
    const char* assetName = luaL_checkstring(L, 1);
    int32_t tileWidth = (int32_t)luaL_checkinteger(L, 2);
    int32_t tileHeight = (int32_t)luaL_checkinteger(L, 3);
    int32_t margin = (int32_t)luaL_optinteger(L, 4, 0);
    int32_t spacing = (int32_t)luaL_optinteger(L, 5, 0);
    STTilesetID id = st_tileset_load_asset(assetName, tileWidth, tileHeight, margin, spacing);
    lua_pushinteger(L, id);
    return 1;
}

static int lua_st_tileset_destroy(lua_State* L) {
    STTilesetID id = (STTilesetID)luaL_checkinteger(L, 1);
    st_tileset_destroy(id);
    return 0;
}

static int lua_st_tileset_get_tile_count(lua_State* L) {
    STTilesetID id = (STTilesetID)luaL_checkinteger(L, 1);
    int32_t count = st_tileset_get_tile_count(id);
    lua_pushinteger(L, count);
    return 1;
}

static int lua_st_tileset_get_dimensions(lua_State* L) {
    STTilesetID id = (STTilesetID)luaL_checkinteger(L, 1);
    int32_t columns, rows;
    st_tileset_get_dimensions(id, &columns, &rows);
    lua_pushinteger(L, columns);
    lua_pushinteger(L, rows);
    return 2;
}

static int lua_st_tilemap_layer_set_tileset(lua_State* L) {
    STLayerID layer = (STLayerID)luaL_checkinteger(L, 1);
    STTilesetID tileset = (STTilesetID)luaL_checkinteger(L, 2);
    st_tilemap_layer_set_tileset(layer, tileset);
    return 0;
}

// =============================================================================
// DisplayText API Bindings
// =============================================================================

static int lua_st_text_display_at(lua_State* L) {
    float x = (float)luaL_checknumber(L, 1);
    float y = (float)luaL_checknumber(L, 2);
    const char* text = luaL_checkstring(L, 3);
    float scale_x = (float)luaL_optnumber(L, 4, 1.0);
    float scale_y = (float)luaL_optnumber(L, 5, 1.0);
    float rotation = (float)luaL_optnumber(L, 6, 0.0);
    uint32_t color = (uint32_t)luaL_optinteger(L, 7, 0xFFFFFFFF);
    STTextAlignment alignment = (STTextAlignment)luaL_optinteger(L, 8, 0); // ST_ALIGN_LEFT
    int layer = (int)luaL_optinteger(L, 9, 0);

    int item_id = st_text_display_at(x, y, text, scale_x, scale_y, rotation, color, alignment, layer);
    lua_pushinteger(L, item_id);
    return 1;
}

static int lua_st_text_display_shear(lua_State* L) {
    float x = (float)luaL_checknumber(L, 1);
    float y = (float)luaL_checknumber(L, 2);
    const char* text = luaL_checkstring(L, 3);
    float scale_x = (float)luaL_optnumber(L, 4, 1.0);
    float scale_y = (float)luaL_optnumber(L, 5, 1.0);
    float rotation = (float)luaL_optnumber(L, 6, 0.0);
    float shear_x = (float)luaL_optnumber(L, 7, 0.0);
    float shear_y = (float)luaL_optnumber(L, 8, 0.0);
    uint32_t color = (uint32_t)luaL_optinteger(L, 9, 0xFFFFFFFF);
    STTextAlignment alignment = (STTextAlignment)luaL_optinteger(L, 10, 0); // ST_ALIGN_LEFT
    int layer = (int)luaL_optinteger(L, 11, 0);

    int item_id = st_text_display_shear(x, y, text, scale_x, scale_y, rotation, shear_x, shear_y, color, alignment, layer);
    lua_pushinteger(L, item_id);
    return 1;
}

static int lua_st_text_display_update(lua_State* L) {
    int item_id = (int)luaL_checkinteger(L, 1);
    const char* text = luaL_checkstring(L, 2);
    float x = (float)luaL_optnumber(L, 3, -1.0);
    float y = (float)luaL_optnumber(L, 4, -1.0);
    int result = st_text_update_item(item_id, text, x, y);
    lua_pushboolean(L, result);
    return 1;
}

static int lua_st_text_display_set_visible(lua_State* L) {
    int item_id = (int)luaL_checkinteger(L, 1);
    int visible = lua_toboolean(L, 2);
    int result = st_text_set_item_visible(item_id, visible);
    lua_pushboolean(L, result);
    return 1;
}

static int lua_st_text_display_clear(lua_State* L) {
    (void)L;
    st_text_clear_displayed();
    return 0;
}

static int lua_st_text_update_item(lua_State* L) {
    int item_id = (int)luaL_checkinteger(L, 1);
    const char* text = luaL_optstring(L, 2, nullptr);
    float x = (float)luaL_optnumber(L, 3, -1.0);
    float y = (float)luaL_optnumber(L, 4, -1.0);

    st_text_update_item(item_id, text, x, y);
    return 0;
}

static int lua_st_text_remove_item(lua_State* L) {
    int item_id = (int)luaL_checkinteger(L, 1);
    st_text_remove_item(item_id);
    return 0;
}

static int lua_st_text_clear_displayed(lua_State* L) {
    (void)L;
    st_text_clear_displayed();
    return 0;
}

static int lua_st_text_set_item_visible(lua_State* L) {
    int item_id = (int)luaL_checkinteger(L, 1);
    bool visible = lua_toboolean(L, 2);
    st_text_set_item_visible(item_id, visible);
    return 0;
}

static int lua_st_text_set_item_layer(lua_State* L) {
    int item_id = (int)luaL_checkinteger(L, 1);
    int layer = (int)luaL_checkinteger(L, 2);
    st_text_set_item_layer(item_id, layer);
    return 0;
}

static int lua_st_text_set_item_color(lua_State* L) {
    int item_id = (int)luaL_checkinteger(L, 1);
    uint32_t color = (uint32_t)luaL_checkinteger(L, 2);
    st_text_set_item_color(item_id, color);
    return 0;
}

static int lua_st_text_get_item_count(lua_State* L) {
    int count = st_text_get_item_count();
    lua_pushinteger(L, count);
    return 1;
}

static int lua_st_text_get_visible_count(lua_State* L) {
    int count = st_text_get_visible_count();
    lua_pushinteger(L, count);
    return 1;
}

// =============================================================================
// Collision Detection API Bindings
// =============================================================================

static int lua_collision_circle_circle(lua_State* L) {
    float x1 = (float)luaL_checknumber(L, 1);
    float y1 = (float)luaL_checknumber(L, 2);
    float r1 = (float)luaL_checknumber(L, 3);
    float x2 = (float)luaL_checknumber(L, 4);
    float y2 = (float)luaL_checknumber(L, 5);
    float r2 = (float)luaL_checknumber(L, 6);

    int result = st_collision_circle_circle(x1, y1, r1, x2, y2, r2);
    lua_pushboolean(L, result);
    return 1;
}

static int lua_collision_circle_rect(lua_State* L) {
    float cx = (float)luaL_checknumber(L, 1);
    float cy = (float)luaL_checknumber(L, 2);
    float radius = (float)luaL_checknumber(L, 3);
    float rx = (float)luaL_checknumber(L, 4);
    float ry = (float)luaL_checknumber(L, 5);
    float rw = (float)luaL_checknumber(L, 6);
    float rh = (float)luaL_checknumber(L, 7);

    int result = st_collision_circle_rect(cx, cy, radius, rx, ry, rw, rh);
    lua_pushboolean(L, result);
    return 1;
}

static int lua_collision_circle_rect_bottom(lua_State* L) {
    float cx = (float)luaL_checknumber(L, 1);
    float cy = (float)luaL_checknumber(L, 2);
    float radius = (float)luaL_checknumber(L, 3);
    float rx = (float)luaL_checknumber(L, 4);
    float ry = (float)luaL_checknumber(L, 5);
    float rw = (float)luaL_checknumber(L, 6);
    float rh = (float)luaL_checknumber(L, 7);

    int result = st_collision_circle_rect_bottom(cx, cy, radius, rx, ry, rw, rh);
    lua_pushboolean(L, result);
    return 1;
}

static int lua_collision_rect_rect(lua_State* L) {
    float x1 = (float)luaL_checknumber(L, 1);
    float y1 = (float)luaL_checknumber(L, 2);
    float w1 = (float)luaL_checknumber(L, 3);
    float h1 = (float)luaL_checknumber(L, 4);
    float x2 = (float)luaL_checknumber(L, 5);
    float y2 = (float)luaL_checknumber(L, 6);
    float w2 = (float)luaL_checknumber(L, 7);
    float h2 = (float)luaL_checknumber(L, 8);

    int result = st_collision_rect_rect(x1, y1, w1, h1, x2, y2, w2, h2);
    lua_pushboolean(L, result);
    return 1;
}

static int lua_collision_point_in_circle(lua_State* L) {
    float px = (float)luaL_checknumber(L, 1);
    float py = (float)luaL_checknumber(L, 2);
    float cx = (float)luaL_checknumber(L, 3);
    float cy = (float)luaL_checknumber(L, 4);
    float radius = (float)luaL_checknumber(L, 5);

    int result = st_collision_point_in_circle(px, py, cx, cy, radius);
    lua_pushboolean(L, result);
    return 1;
}

static int lua_collision_point_in_rect(lua_State* L) {
    float px = (float)luaL_checknumber(L, 1);
    float py = (float)luaL_checknumber(L, 2);
    float rx = (float)luaL_checknumber(L, 3);
    float ry = (float)luaL_checknumber(L, 4);
    float rw = (float)luaL_checknumber(L, 5);
    float rh = (float)luaL_checknumber(L, 6);

    int result = st_collision_point_in_rect(px, py, rx, ry, rw, rh);
    lua_pushboolean(L, result);
    return 1;
}

static int lua_collision_circle_rect_info(lua_State* L) {
    float cx = (float)luaL_checknumber(L, 1);
    float cy = (float)luaL_checknumber(L, 2);
    float radius = (float)luaL_checknumber(L, 3);
    float rx = (float)luaL_checknumber(L, 4);
    float ry = (float)luaL_checknumber(L, 5);
    float rw = (float)luaL_checknumber(L, 6);
    float rh = (float)luaL_checknumber(L, 7);

    st_collision_info info;
    st_collision_circle_rect_info(cx, cy, radius, rx, ry, rw, rh, &info);

    lua_newtable(L);
    lua_pushboolean(L, info.colliding);
    lua_setfield(L, -2, "colliding");
    lua_pushnumber(L, info.penetrationDepth);
    lua_setfield(L, -2, "penetrationDepth");
    lua_pushnumber(L, info.normalX);
    lua_setfield(L, -2, "normalX");
    lua_pushnumber(L, info.normalY);
    lua_setfield(L, -2, "normalY");

    return 1;
}

static int lua_collision_circle_circle_penetration(lua_State* L) {
    float x1 = (float)luaL_checknumber(L, 1);
    float y1 = (float)luaL_checknumber(L, 2);
    float r1 = (float)luaL_checknumber(L, 3);
    float x2 = (float)luaL_checknumber(L, 4);
    float y2 = (float)luaL_checknumber(L, 5);
    float r2 = (float)luaL_checknumber(L, 6);

    float penetration = st_collision_circle_circle_penetration(x1, y1, r1, x2, y2, r2);
    lua_pushnumber(L, penetration);
    return 1;
}

static int lua_collision_rect_rect_overlap(lua_State* L) {
    float x1 = (float)luaL_checknumber(L, 1);
    float y1 = (float)luaL_checknumber(L, 2);
    float w1 = (float)luaL_checknumber(L, 3);
    float h1 = (float)luaL_checknumber(L, 4);
    float x2 = (float)luaL_checknumber(L, 5);
    float y2 = (float)luaL_checknumber(L, 6);
    float w2 = (float)luaL_checknumber(L, 7);
    float h2 = (float)luaL_checknumber(L, 8);

    float overlapX, overlapY;
    st_collision_rect_rect_overlap(x1, y1, w1, h1, x2, y2, w2, h2, &overlapX, &overlapY);

    lua_pushnumber(L, overlapX);
    lua_pushnumber(L, overlapY);
    return 2;
}

static int lua_collision_swept_circle_rect(lua_State* L) {
    float cx = (float)luaL_checknumber(L, 1);
    float cy = (float)luaL_checknumber(L, 2);
    float radius = (float)luaL_checknumber(L, 3);
    float vx = (float)luaL_checknumber(L, 4);
    float vy = (float)luaL_checknumber(L, 5);
    float rx = (float)luaL_checknumber(L, 6);
    float ry = (float)luaL_checknumber(L, 7);
    float rw = (float)luaL_checknumber(L, 8);
    float rh = (float)luaL_checknumber(L, 9);

    int result = st_collision_swept_circle_rect(cx, cy, radius, vx, vy, rx, ry, rw, rh);
    lua_pushboolean(L, result);
    return 1;
}

// =============================================================================

void registerBindings(lua_State* L) {
    // Text API
    luaL_setglobalfunction(L, "text_putchar", lua_st_text_putchar);
    luaL_setglobalfunction(L, "poke_text", lua_poke_text);
    luaL_setglobalfunction(L, "text_put", lua_st_text_put);
    luaL_setglobalfunction(L, "text_clear", lua_st_text_clear);
    luaL_setglobalfunction(L, "cls", lua_st_text_clear);  // Alias for text_clear
    luaL_setglobalfunction(L, "text_clear_region", lua_st_text_clear_region);
    luaL_setglobalfunction(L, "text_set_size", lua_st_text_set_size);
    luaL_setglobalfunction(L, "text_get_size", lua_st_text_get_size);
    luaL_setglobalfunction(L, "text_scroll", lua_st_text_scroll);

    // Text Display API (GPU-accelerated positioned text)
    luaL_setglobalfunction(L, "text_display_at", lua_st_text_display_at);
    luaL_setglobalfunction(L, "text_display_shear", lua_st_text_display_shear);
    luaL_setglobalfunction(L, "text_display_update", lua_st_text_display_update);
    luaL_setglobalfunction(L, "text_display_set_visible", lua_st_text_display_set_visible);
    luaL_setglobalfunction(L, "text_display_set_color", lua_st_text_set_item_color);
    luaL_setglobalfunction(L, "text_display_clear", lua_st_text_display_clear);

    // Text Display alignment constants
    luaL_setglobalnumber(L, "ST_ALIGN_LEFT", 0);
    luaL_setglobalnumber(L, "ST_ALIGN_CENTER", 1);
    luaL_setglobalnumber(L, "ST_ALIGN_RIGHT", 2);

    // Sixel API
    luaL_setglobalfunction(L, "text_putsixel", lua_st_text_putsixel);
    luaL_setglobalfunction(L, "text_putsixel_packed", lua_st_text_putsixel_packed);
    luaL_setglobalfunction(L, "sixel_pack_colors", lua_st_sixel_pack_colors);
    luaL_setglobalfunction(L, "sixel_set_stripe", lua_st_sixel_set_stripe);
    luaL_setglobalfunction(L, "sixel_get_stripe", lua_st_sixel_get_stripe);
    luaL_setglobalfunction(L, "sixel_gradient", lua_st_sixel_gradient);
    luaL_setglobalfunction(L, "sixel_hline", lua_st_sixel_hline);
    luaL_setglobalfunction(L, "sixel_fill_rect", lua_st_sixel_fill_rect);

    // Chunky Pixel Graphics API (TODO: implement these functions)
    // luaL_setglobalfunction(L, "pset", lua_st_chunky_pset);
    // luaL_setglobalfunction(L, "line", lua_st_chunky_line);
    // luaL_setglobalfunction(L, "rect", lua_st_chunky_rect);
    // luaL_setglobalfunction(L, "fillrect", lua_st_chunky_fillrect);
    // luaL_setglobalfunction(L, "hline", lua_st_chunky_hline);
    // luaL_setglobalfunction(L, "vline", lua_st_chunky_vline);
    // luaL_setglobalfunction(L, "chunky_clear", lua_st_chunky_clear);
    // luaL_setglobalfunction(L, "chunky_resolution", lua_st_chunky_get_resolution);

    // Graphics Mode Switching API
    luaL_setglobalfunction(L, "st_mode", lua_st_mode);
    luaL_setglobalfunction(L, "text_mode", lua_st_text_mode);
    luaL_setglobalfunction(L, "lores", lua_st_lores);
    luaL_setglobalfunction(L, "mediumres", lua_st_mediumres);
    luaL_setglobalfunction(L, "highres", lua_st_highres);
    luaL_setglobalfunction(L, "ultrares", lua_st_ultrares);
    luaL_setglobalfunction(L, "xres", lua_st_xres);
    luaL_setglobalfunction(L, "wres", lua_st_wres);

    // LORES Pixel Buffer API (works in LORES/MEDIUMRES/HIRES modes)
    luaL_setglobalfunction(L, "lores_pset", lua_st_lores_pset);
    luaL_setglobalfunction(L, "lores_line", lua_st_lores_line);
    luaL_setglobalfunction(L, "lores_rect", lua_st_lores_rect);
    luaL_setglobalfunction(L, "lores_fillrect", lua_st_lores_fillrect);
    luaL_setglobalfunction(L, "lores_hline", lua_st_lores_hline);
    luaL_setglobalfunction(L, "lores_vline", lua_st_lores_vline);
    luaL_setglobalfunction(L, "lores_clear", lua_st_lores_clear);
    luaL_setglobalfunction(L, "lores_resolution", lua_st_lores_resolution);
    luaL_setglobalfunction(L, "lores_buffer", lua_st_lores_buffer);
    luaL_setglobalfunction(L, "lores_buffer_get", lua_st_lores_buffer_get);
    luaL_setglobalfunction(L, "lores_flip", lua_st_lores_flip);
    luaL_setglobalfunction(L, "lores_blit", lua_st_lores_blit);
    luaL_setglobalfunction(L, "lores_blit_trans", lua_st_lores_blit_trans);

    // LORES Palette API
    luaL_setglobalfunction(L, "lores_palette_set", lua_st_lores_palette_set);
    luaL_setglobalfunction(L, "lores_palette_poke", lua_st_lores_palette_poke);
    luaL_setglobalfunction(L, "lores_palette_peek", lua_st_lores_palette_peek);

    // URES (Ultra Resolution) API (1280×720 direct color)
    luaL_setglobalfunction(L, "ures_pset", lua_st_ures_pset);
    luaL_setglobalfunction(L, "ures_pget", lua_st_ures_pget);
    luaL_setglobalfunction(L, "ures_clear", lua_st_ures_clear);
    luaL_setglobalfunction(L, "ures_fillrect", lua_st_ures_fillrect);
    luaL_setglobalfunction(L, "ures_hline", lua_st_ures_hline);
    luaL_setglobalfunction(L, "ures_vline", lua_st_ures_vline);
    luaL_setglobalfunction(L, "ures_buffer", lua_st_ures_buffer);
    luaL_setglobalfunction(L, "ures_buffer_get", lua_st_ures_buffer_get);
    luaL_setglobalfunction(L, "ures_flip", lua_st_ures_flip);
    luaL_setglobalfunction(L, "ures_gpu_flip", lua_st_ures_gpu_flip);
    luaL_setglobalfunction(L, "ures_sync", lua_st_ures_sync);
    luaL_setglobalfunction(L, "ures_swap", lua_st_ures_swap);
    luaL_setglobalfunction(L, "ures_blit_from", lua_st_ures_blit_from);
    luaL_setglobalfunction(L, "ures_blit_from_trans", lua_st_ures_blit_from_trans);
    luaL_setglobalfunction(L, "urgb", lua_st_urgb);
    luaL_setglobalfunction(L, "urgba", lua_st_urgba);

    // XRES Buffer API (320×240, 256-color palette)
    luaL_setglobalfunction(L, "xres_pset", lua_st_xres_pset);
    luaL_setglobalfunction(L, "xres_pget", lua_st_xres_pget);
    luaL_setglobalfunction(L, "xres_clear", lua_st_xres_clear);
    luaL_setglobalfunction(L, "xres_fillrect", lua_st_xres_fillrect);
    luaL_setglobalfunction(L, "xres_hline", lua_st_xres_hline);
    luaL_setglobalfunction(L, "xres_vline", lua_st_xres_vline);
    luaL_setglobalfunction(L, "xres_buffer", lua_st_xres_buffer);
    luaL_setglobalfunction(L, "xres_flip", lua_st_xres_flip);
    luaL_setglobalfunction(L, "xres_blit", lua_st_xres_blit);
    luaL_setglobalfunction(L, "xres_blit_trans", lua_st_xres_blit_trans);
    luaL_setglobalfunction(L, "xres_blit_from", lua_st_xres_blit_from);
    luaL_setglobalfunction(L, "xres_blit_from_trans", lua_st_xres_blit_from_trans);
    
    // LORES GPU functions
    luaL_setglobalfunction(L, "lores_blit_gpu", lua_st_lores_blit_gpu);
    luaL_setglobalfunction(L, "lores_blit_trans_gpu", lua_st_lores_blit_trans_gpu);
    luaL_setglobalfunction(L, "lores_clear_gpu", lua_st_lores_clear_gpu);
    luaL_setglobalfunction(L, "lores_rect_fill_gpu", lua_st_lores_rect_fill_gpu);
    luaL_setglobalfunction(L, "lores_circle_fill_gpu", lua_st_lores_circle_fill_gpu);
    luaL_setglobalfunction(L, "lores_line_gpu", lua_st_lores_line_gpu);
    
    // XRES GPU functions
    luaL_setglobalfunction(L, "xres_blit_gpu", lua_st_xres_blit_gpu);
    luaL_setglobalfunction(L, "xres_blit_trans_gpu", lua_st_xres_blit_trans_gpu);
    luaL_setglobalfunction(L, "gpu_sync", lua_st_gpu_sync);
    luaL_setglobalfunction(L, "xres_clear_gpu", lua_st_xres_clear_gpu);
    luaL_setglobalfunction(L, "wres_clear_gpu", lua_st_wres_clear_gpu);
    luaL_setglobalfunction(L, "xres_rect_fill_gpu", lua_st_xres_rect_fill_gpu);
    luaL_setglobalfunction(L, "wres_rect_fill_gpu", lua_st_wres_rect_fill_gpu);
    luaL_setglobalfunction(L, "xres_circle_fill_gpu", lua_st_xres_circle_fill_gpu);
    luaL_setglobalfunction(L, "wres_circle_fill_gpu", lua_st_wres_circle_fill_gpu);
    luaL_setglobalfunction(L, "xres_line_gpu", lua_st_xres_line_gpu);
    luaL_setglobalfunction(L, "wres_line_gpu", lua_st_wres_line_gpu);
    luaL_setglobalfunction(L, "xres_circle_fill_aa", lua_st_xres_circle_fill_aa);
    luaL_setglobalfunction(L, "wres_circle_fill_aa", lua_st_wres_circle_fill_aa);
    luaL_setglobalfunction(L, "xres_line_aa", lua_st_xres_line_aa);
    luaL_setglobalfunction(L, "wres_line_aa", lua_st_wres_line_aa);

    // URES GPU Blitter API
    luaL_setglobalfunction(L, "ures_blit_copy_gpu", lua_st_ures_blit_copy_gpu);
    luaL_setglobalfunction(L, "ures_blit_transparent_gpu", lua_st_ures_blit_transparent_gpu);
    luaL_setglobalfunction(L, "ures_blit_alpha_composite_gpu", lua_st_ures_blit_alpha_composite_gpu);
    luaL_setglobalfunction(L, "ures_clear_gpu", lua_st_ures_clear_gpu);

    // URES GPU Primitive Drawing API
    luaL_setglobalfunction(L, "ures_rect_fill_gpu", lua_st_ures_rect_fill_gpu);
    luaL_setglobalfunction(L, "ures_circle_fill_gpu", lua_st_ures_circle_fill_gpu);
    luaL_setglobalfunction(L, "ures_line_gpu", lua_st_ures_line_gpu);

    // URES GPU Anti-Aliased Primitive Drawing API
    luaL_setglobalfunction(L, "ures_circle_fill_aa", lua_st_ures_circle_fill_aa);
    luaL_setglobalfunction(L, "ures_line_aa", lua_st_ures_line_aa);

    // URES GPU Gradient Primitive Drawing API
    luaL_setglobalfunction(L, "ures_rect_fill_gradient_gpu", lua_st_ures_rect_fill_gradient_gpu);
    luaL_setglobalfunction(L, "ures_circle_fill_gradient_gpu", lua_st_ures_circle_fill_gradient_gpu);
    luaL_setglobalfunction(L, "ures_circle_fill_gradient_aa", lua_st_ures_circle_fill_gradient_aa);

    // URES Color Utility API
    luaL_setglobalfunction(L, "ures_pack_argb4", lua_st_ures_pack_argb4);
    luaL_setglobalfunction(L, "ures_pack_argb8", lua_st_ures_pack_argb8);
    luaL_setglobalfunction(L, "ures_unpack_argb4", lua_st_ures_unpack_argb4);
    luaL_setglobalfunction(L, "ures_unpack_argb8", lua_st_ures_unpack_argb8);
    luaL_setglobalfunction(L, "ures_blend_colors", lua_st_ures_blend_colors);
    luaL_setglobalfunction(L, "ures_lerp_colors", lua_st_ures_lerp_colors);
    luaL_setglobalfunction(L, "ures_color_from_hsv", lua_st_ures_color_from_hsv);
    luaL_setglobalfunction(L, "ures_adjust_brightness", lua_st_ures_adjust_brightness);
    luaL_setglobalfunction(L, "ures_set_alpha", lua_st_ures_set_alpha);
    luaL_setglobalfunction(L, "ures_get_alpha", lua_st_ures_get_alpha);

    luaL_setglobalfunction(L, "begin_blit_batch", lua_st_begin_blit_batch);
    luaL_setglobalfunction(L, "end_blit_batch", lua_st_end_blit_batch);
    luaL_setglobalfunction(L, "xres_palette_row", lua_st_xres_palette_row);
    luaL_setglobalfunction(L, "xres_palette_global", lua_st_xres_palette_global);
    luaL_setglobalfunction(L, "xres_palette_rotate_row", lua_st_xres_palette_rotate_row);
    luaL_setglobalfunction(L, "xres_palette_rotate_global", lua_st_xres_palette_rotate_global);
    luaL_setglobalfunction(L, "xres_palette_copy_row", lua_st_xres_palette_copy_row);
    luaL_setglobalfunction(L, "xres_palette_lerp_row", lua_st_xres_palette_lerp_row);
    luaL_setglobalfunction(L, "xres_palette_lerp_global", lua_st_xres_palette_lerp_global);
    luaL_setglobalfunction(L, "xres_palette_make_ramp", lua_st_xres_palette_make_ramp);
    luaL_setglobalfunction(L, "xres_gradient_h", lua_st_xres_gradient_h);
    luaL_setglobalfunction(L, "xres_gradient_v", lua_st_xres_gradient_v);
    luaL_setglobalfunction(L, "xres_gradient_radial", lua_st_xres_gradient_radial);
    luaL_setglobalfunction(L, "xres_gradient_corners", lua_st_xres_gradient_corners);
    luaL_setglobalfunction(L, "xrgb", lua_st_xrgb);

    // WRES Buffer API (432×240, 256-color palette)
    luaL_setglobalfunction(L, "wres_pset", lua_st_wres_pset);
    luaL_setglobalfunction(L, "wres_pget", lua_st_wres_pget);
    luaL_setglobalfunction(L, "wres_clear", lua_st_wres_clear);
    luaL_setglobalfunction(L, "wres_fillrect", lua_st_wres_fillrect);
    luaL_setglobalfunction(L, "wres_hline", lua_st_wres_hline);
    luaL_setglobalfunction(L, "wres_vline", lua_st_wres_vline);
    luaL_setglobalfunction(L, "wres_buffer", lua_st_wres_buffer);
    luaL_setglobalfunction(L, "wres_flip", lua_st_wres_flip);
    luaL_setglobalfunction(L, "wres_blit", lua_st_wres_blit);
    luaL_setglobalfunction(L, "wres_blit_trans", lua_st_wres_blit_trans);
    luaL_setglobalfunction(L, "wres_blit_from", lua_st_wres_blit_from);
    luaL_setglobalfunction(L, "wres_blit_from_trans", lua_st_wres_blit_from_trans);
    luaL_setglobalfunction(L, "wres_blit_gpu", lua_st_wres_blit_gpu);
    luaL_setglobalfunction(L, "wres_blit_trans_gpu", lua_st_wres_blit_trans_gpu);
    luaL_setglobalfunction(L, "wres_palette_row", lua_st_wres_palette_row);
    luaL_setglobalfunction(L, "wres_palette_global", lua_st_wres_palette_global);
    luaL_setglobalfunction(L, "wres_palette_rotate_row", lua_st_wres_palette_rotate_row);
    luaL_setglobalfunction(L, "wres_palette_rotate_global", lua_st_wres_palette_rotate_global);
    luaL_setglobalfunction(L, "wres_palette_copy_row", lua_st_wres_palette_copy_row);
    luaL_setglobalfunction(L, "wres_palette_lerp_row", lua_st_wres_palette_lerp_row);
    luaL_setglobalfunction(L, "wres_palette_lerp_global", lua_st_wres_palette_lerp_global);
    luaL_setglobalfunction(L, "wres_palette_make_ramp", lua_st_wres_palette_make_ramp);
    luaL_setglobalfunction(L, "wres_gradient_h", lua_st_wres_gradient_h);
    luaL_setglobalfunction(L, "wres_gradient_v", lua_st_wres_gradient_v);
    luaL_setglobalfunction(L, "wres_gradient_radial", lua_st_wres_gradient_radial);
    luaL_setglobalfunction(L, "wres_gradient_corners", lua_st_wres_gradient_corners);
    luaL_setglobalfunction(L, "wrgb", lua_st_wrgb);
    
    // PRES Buffer API (1280×720, 256-color palette)
    luaL_setglobalfunction(L, "pres_pset", lua_st_pres_pset);
    luaL_setglobalfunction(L, "pres_pget", lua_st_pres_pget);
    luaL_setglobalfunction(L, "pres_clear", lua_st_pres_clear);
    luaL_setglobalfunction(L, "pres_fillrect", lua_st_pres_fillrect);
    luaL_setglobalfunction(L, "pres_hline", lua_st_pres_hline);
    luaL_setglobalfunction(L, "pres_vline", lua_st_pres_vline);
    luaL_setglobalfunction(L, "pres_buffer", lua_st_pres_buffer);
    luaL_setglobalfunction(L, "pres_flip", lua_st_pres_flip);
    luaL_setglobalfunction(L, "pres_blit", lua_st_pres_blit);
    luaL_setglobalfunction(L, "pres_blit_trans", lua_st_pres_blit_trans);
    luaL_setglobalfunction(L, "pres_blit_from", lua_st_pres_blit_from);
    luaL_setglobalfunction(L, "pres_blit_from_trans", lua_st_pres_blit_from_trans);
    luaL_setglobalfunction(L, "pres_blit_gpu", lua_st_pres_blit_gpu);
    luaL_setglobalfunction(L, "pres_blit_trans_gpu", lua_st_pres_blit_trans_gpu);
    luaL_setglobalfunction(L, "pres_clear_gpu", lua_st_pres_clear_gpu);
    luaL_setglobalfunction(L, "pres_rect_fill_gpu", lua_st_pres_rect_fill_gpu);
    luaL_setglobalfunction(L, "pres_circle_fill_gpu", lua_st_pres_circle_fill_gpu);
    luaL_setglobalfunction(L, "pres_line_gpu", lua_st_pres_line_gpu);
    luaL_setglobalfunction(L, "pres_circle_fill_aa", lua_st_pres_circle_fill_aa);
    luaL_setglobalfunction(L, "pres_line_aa", lua_st_pres_line_aa);
    luaL_setglobalfunction(L, "pres_palette_row", lua_st_pres_palette_row);
    luaL_setglobalfunction(L, "pres_palette_global", lua_st_pres_palette_global);
    luaL_setglobalfunction(L, "pres_palette_rotate_row", lua_st_pres_palette_rotate_row);
    luaL_setglobalfunction(L, "pres_palette_rotate_global", lua_st_pres_palette_rotate_global);
    luaL_setglobalfunction(L, "pres_palette_copy_row", lua_st_pres_palette_copy_row);
    luaL_setglobalfunction(L, "pres_palette_lerp_row", lua_st_pres_palette_lerp_row);
    luaL_setglobalfunction(L, "pres_palette_lerp_global", lua_st_pres_palette_lerp_global);
    luaL_setglobalfunction(L, "pres_palette_make_ramp", lua_st_pres_palette_make_ramp);
    luaL_setglobalfunction(L, "pres_gradient_h", lua_st_pres_gradient_h);
    luaL_setglobalfunction(L, "pres_gradient_v", lua_st_pres_gradient_v);
    luaL_setglobalfunction(L, "pres_gradient_radial", lua_st_pres_gradient_radial);
    luaL_setglobalfunction(L, "pres_gradient_corners", lua_st_pres_gradient_corners);
    luaL_setglobalfunction(L, "prgb", lua_st_prgb);
    
    // Unified Video Palette API
    luaL_setglobalfunction(L, "video_get_color_depth", lua_video_get_color_depth);
    luaL_setglobalfunction(L, "video_has_palette", lua_video_has_palette);
    luaL_setglobalfunction(L, "video_has_per_row_palette", lua_video_has_per_row_palette);
    luaL_setglobalfunction(L, "video_get_palette_info", lua_video_get_palette_info);
    luaL_setglobalfunction(L, "video_set_palette", lua_video_set_palette);
    luaL_setglobalfunction(L, "video_set_palette_row", lua_video_set_palette_row);
    luaL_setglobalfunction(L, "video_get_palette", lua_video_get_palette);
    luaL_setglobalfunction(L, "video_get_palette_row", lua_video_get_palette_row);
    luaL_setglobalfunction(L, "video_load_palette", lua_video_load_palette);
    luaL_setglobalfunction(L, "video_save_palette", lua_video_save_palette);
    luaL_setglobalfunction(L, "video_load_preset_palette", lua_video_load_preset_palette);
    luaL_setglobalfunction(L, "video_load_preset_palette_rows", lua_video_load_preset_palette_rows);
    luaL_setglobalfunction(L, "video_pack_rgb", lua_video_pack_rgb);
    luaL_setglobalfunction(L, "video_unpack_rgb", lua_video_unpack_rgb);
    
    // Convenient aliases for unified palette functions
    luaL_setglobalfunction(L, "palette_global", lua_video_set_palette);
    luaL_setglobalfunction(L, "palette_row", lua_video_set_palette_row);
    
    // Palette preset constants
    luaL_setglobalnumber(L, "PALETTE_IBM_RGBI", ST_PALETTE_IBM_RGBI);
    luaL_setglobalnumber(L, "PALETTE_C64", ST_PALETTE_C64);
    luaL_setglobalnumber(L, "PALETTE_GRAYSCALE", ST_PALETTE_GRAYSCALE);
    luaL_setglobalnumber(L, "PALETTE_RGB_CUBE_6x8x5", ST_PALETTE_RGB_CUBE_6x8x5);

    // Unified Video Mode API
    luaL_setglobalfunction(L, "video_mode", lua_video_mode);
    luaL_setglobalfunction(L, "video_mode_name", lua_video_mode_name);
    luaL_setglobalfunction(L, "video_mode_get", lua_video_mode_get);
    luaL_setglobalfunction(L, "video_mode_disable", lua_video_mode_disable);
    luaL_setglobalfunction(L, "video_pset", lua_video_pset);
    luaL_setglobalfunction(L, "video_pget", lua_video_pget);
    luaL_setglobalfunction(L, "video_clear", lua_video_clear);
    luaL_setglobalfunction(L, "video_clear_gpu", lua_video_clear_gpu);
    luaL_setglobalfunction(L, "video_rect", lua_video_rect);
    luaL_setglobalfunction(L, "video_rect_gpu", lua_video_rect_gpu);
    luaL_setglobalfunction(L, "video_circle", lua_video_circle);
    luaL_setglobalfunction(L, "video_circle_gpu", lua_video_circle_gpu);
    luaL_setglobalfunction(L, "video_circle_aa", lua_video_circle_aa);
    luaL_setglobalfunction(L, "video_line", lua_video_line);
    luaL_setglobalfunction(L, "video_line_gpu", lua_video_line_gpu);
    luaL_setglobalfunction(L, "video_line_aa", lua_video_line_aa);
    luaL_setglobalfunction(L, "video_rect_gradient_gpu", lua_video_rect_gradient_gpu);
    luaL_setglobalfunction(L, "video_circle_gradient_gpu", lua_video_circle_gradient_gpu);
    luaL_setglobalfunction(L, "video_supports_gradients", lua_video_supports_gradients);
    luaL_setglobalfunction(L, "video_enable_antialias", lua_video_enable_antialias);
    luaL_setglobalfunction(L, "video_supports_antialias", lua_video_supports_antialias);
    luaL_setglobalfunction(L, "video_set_line_width", lua_video_set_line_width);
    luaL_setglobalfunction(L, "video_get_line_width", lua_video_get_line_width);
    luaL_setglobalfunction(L, "video_blit", lua_video_blit);
    luaL_setglobalfunction(L, "video_blit_trans", lua_video_blit_trans);
    luaL_setglobalfunction(L, "video_blit_gpu", lua_video_blit_gpu);
    luaL_setglobalfunction(L, "video_blit_trans_gpu", lua_video_blit_trans_gpu);
    luaL_setglobalfunction(L, "video_buffer", lua_video_buffer);
    luaL_setglobalfunction(L, "video_buffer_get", lua_video_buffer_get);
    luaL_setglobalfunction(L, "video_get_back_buffer", lua_video_get_back_buffer);
    luaL_setglobalfunction(L, "video_get_front_buffer", lua_video_get_front_buffer);
    luaL_setglobalfunction(L, "video_gpu_flip", lua_video_gpu_flip);
    luaL_setglobalfunction(L, "load_image", lua_video_load_image);
    luaL_setglobalfunction(L, "save_image", lua_video_save_image);
    luaL_setglobalfunction(L, "load_palette", lua_video_load_palette);
    luaL_setglobalfunction(L, "save_palette", lua_video_save_palette);
    luaL_setglobalfunction(L, "video_resolution", lua_video_resolution);
    
    // Unified API - Buffer Management (Phase 1)
    luaL_setglobalfunction(L, "video_get_max_buffers", lua_video_get_max_buffers);
    luaL_setglobalfunction(L, "video_is_valid_buffer", lua_video_is_valid_buffer);
    luaL_setglobalfunction(L, "video_get_current_buffer", lua_video_get_current_buffer);
    
    // Unified API - Feature Detection (Phase 1)
    luaL_setglobalfunction(L, "video_get_feature_flags", lua_video_get_feature_flags);
    luaL_setglobalfunction(L, "video_uses_palette", lua_video_uses_palette);
    luaL_setglobalfunction(L, "video_has_gpu", lua_video_has_gpu);
    
    // Unified API - Memory Queries (Phase 2)
    luaL_setglobalfunction(L, "video_get_memory_per_buffer", lua_video_get_memory_per_buffer);
    luaL_setglobalfunction(L, "video_get_memory_usage", lua_video_get_memory_usage);
    luaL_setglobalfunction(L, "video_get_pixel_count", lua_video_get_pixel_count);
    
    // Unified API - Palette Management (Phase 2)
    luaL_setglobalfunction(L, "video_reset_palette_to_default", lua_video_reset_palette_to_default);
    
    // Feature flag constants
    luaL_setglobalnumber(L, "VIDEO_FEATURE_PALETTE", ST_VIDEO_FEATURE_PALETTE);
    luaL_setglobalnumber(L, "VIDEO_FEATURE_PER_ROW_PALETTE", ST_VIDEO_FEATURE_PER_ROW_PALETTE);
    luaL_setglobalnumber(L, "VIDEO_FEATURE_GPU_ACCEL", ST_VIDEO_FEATURE_GPU_ACCEL);
    luaL_setglobalnumber(L, "VIDEO_FEATURE_ANTIALIASING", ST_VIDEO_FEATURE_ANTIALIASING);
    luaL_setglobalnumber(L, "VIDEO_FEATURE_GRADIENTS", ST_VIDEO_FEATURE_GRADIENTS);
    luaL_setglobalnumber(L, "VIDEO_FEATURE_ALPHA_BLEND", ST_VIDEO_FEATURE_ALPHA_BLEND);
    luaL_setglobalnumber(L, "VIDEO_FEATURE_DIRECT_COLOR", ST_VIDEO_FEATURE_DIRECT_COLOR);
    
    // Unified API - Other functions
    luaL_setglobalfunction(L, "video_flip", lua_video_flip);
    luaL_setglobalfunction(L, "video_sync", lua_video_sync);
    luaL_setglobalfunction(L, "video_swap", lua_video_swap);
    luaL_setglobalfunction(L, "video_begin_batch", lua_video_begin_batch);
    luaL_setglobalfunction(L, "video_end_batch", lua_video_end_batch);
    luaL_setglobalfunction(L, "video_resolution", lua_video_resolution);

    // Graphics API
    luaL_setglobalfunction(L, "gfx_clear", lua_st_gfx_clear);
    luaL_setglobalfunction(L, "gfx_rect", lua_st_gfx_rect);
    luaL_setglobalfunction(L, "gfx_rect_outline", lua_st_gfx_rect_outline);
    luaL_setglobalfunction(L, "gfx_circle", lua_st_gfx_circle);
    luaL_setglobalfunction(L, "gfx_circle_outline", lua_st_gfx_circle_outline);
    luaL_setglobalfunction(L, "gfx_line", lua_st_gfx_line);
    luaL_setglobalfunction(L, "gfx_point", lua_st_gfx_point);

    // Rectangle API - ID-Based Management
    luaL_setglobalfunction(L, "rect_create", lua_st_rect_create);
    luaL_setglobalfunction(L, "rect_create_gradient", lua_st_rect_create_gradient);
    luaL_setglobalfunction(L, "rect_create_three_point", lua_st_rect_create_three_point);
    luaL_setglobalfunction(L, "rect_create_four_corner", lua_st_rect_create_four_corner);
    luaL_setglobalfunction(L, "rect_set_position", lua_st_rect_set_position);
    luaL_setglobalfunction(L, "rect_set_size", lua_st_rect_set_size);
    luaL_setglobalfunction(L, "rect_set_color", lua_st_rect_set_color);
    luaL_setglobalfunction(L, "rect_set_colors", lua_st_rect_set_colors);
    luaL_setglobalfunction(L, "rect_set_mode", lua_st_rect_set_mode);
    luaL_setglobalfunction(L, "rect_set_rotation", lua_st_rect_set_rotation);
    luaL_setglobalfunction(L, "rect_set_visible", lua_st_rect_set_visible);
    luaL_setglobalfunction(L, "rect_exists", lua_st_rect_exists);
    luaL_setglobalfunction(L, "rect_is_visible", lua_st_rect_is_visible);
    luaL_setglobalfunction(L, "rect_delete", lua_st_rect_delete);
    luaL_setglobalfunction(L, "rect_delete_all", lua_st_rect_delete_all);

    // Rectangle API - Pattern Functions
    luaL_setglobalfunction(L, "rect_create_outline", lua_st_rect_create_outline);
    luaL_setglobalfunction(L, "rect_create_horizontal_stripes", lua_st_rect_create_horizontal_stripes);
    luaL_setglobalfunction(L, "rect_create_vertical_stripes", lua_st_rect_create_vertical_stripes);
    luaL_setglobalfunction(L, "rect_create_diagonal_stripes", lua_st_rect_create_diagonal_stripes);
    luaL_setglobalfunction(L, "rect_create_checkerboard", lua_st_rect_create_checkerboard);
    luaL_setglobalfunction(L, "rect_create_dots", lua_st_rect_create_dots);
    luaL_setglobalfunction(L, "rect_create_grid", lua_st_rect_create_grid);

    // Circle API - ID-Based Management
    luaL_setglobalfunction(L, "circle_create", lua_st_circle_create);
    luaL_setglobalfunction(L, "circle_create_radial", lua_st_circle_create_radial);
    luaL_setglobalfunction(L, "circle_create_radial_3", lua_st_circle_create_radial_3);
    luaL_setglobalfunction(L, "circle_create_radial_4", lua_st_circle_create_radial_4);
    luaL_setglobalfunction(L, "circle_create_outline", lua_st_circle_create_outline);
    luaL_setglobalfunction(L, "circle_create_dashed_outline", lua_st_circle_create_dashed_outline);
    luaL_setglobalfunction(L, "circle_create_ring", lua_st_circle_create_ring);
    luaL_setglobalfunction(L, "circle_create_pie_slice", lua_st_circle_create_pie_slice);
    luaL_setglobalfunction(L, "circle_create_arc", lua_st_circle_create_arc);
    luaL_setglobalfunction(L, "circle_create_dots_ring", lua_st_circle_create_dots_ring);
    luaL_setglobalfunction(L, "circle_create_star_burst", lua_st_circle_create_star_burst);
    luaL_setglobalfunction(L, "circle_set_position", lua_st_circle_set_position);
    luaL_setglobalfunction(L, "circle_set_radius", lua_st_circle_set_radius);
    luaL_setglobalfunction(L, "circle_set_color", lua_st_circle_set_color);
    luaL_setglobalfunction(L, "circle_set_colors", lua_st_circle_set_colors);
    luaL_setglobalfunction(L, "circle_set_parameters", lua_st_circle_set_parameters);
    luaL_setglobalfunction(L, "circle_set_visible", lua_st_circle_set_visible);
    luaL_setglobalfunction(L, "circle_exists", lua_st_circle_exists);
    luaL_setglobalfunction(L, "circle_is_visible", lua_st_circle_is_visible);
    luaL_setglobalfunction(L, "circle_delete", lua_st_circle_delete);
    luaL_setglobalfunction(L, "circle_delete_all", lua_st_circle_delete_all);
    luaL_setglobalfunction(L, "circle_count", lua_st_circle_count);
    luaL_setglobalfunction(L, "circle_is_empty", lua_st_circle_is_empty);
    luaL_setglobalfunction(L, "circle_set_max", lua_st_circle_set_max);
    luaL_setglobalfunction(L, "circle_get_max", lua_st_circle_get_max);

    // Line API - ID-Based Management
    luaL_setglobalfunction(L, "line_create", lua_st_line_create);
    luaL_setglobalfunction(L, "line_create_gradient", lua_st_line_create_gradient);
    luaL_setglobalfunction(L, "line_create_dashed", lua_st_line_create_dashed);
    luaL_setglobalfunction(L, "line_create_dotted", lua_st_line_create_dotted);
    luaL_setglobalfunction(L, "line_set_endpoints", lua_st_line_set_endpoints);
    luaL_setglobalfunction(L, "line_set_thickness", lua_st_line_set_thickness);
    luaL_setglobalfunction(L, "line_set_color", lua_st_line_set_color);
    luaL_setglobalfunction(L, "line_set_colors", lua_st_line_set_colors);
    luaL_setglobalfunction(L, "line_set_dash_pattern", lua_st_line_set_dash_pattern);
    luaL_setglobalfunction(L, "line_set_visible", lua_st_line_set_visible);
    luaL_setglobalfunction(L, "line_exists", lua_st_line_exists);
    luaL_setglobalfunction(L, "line_is_visible", lua_st_line_is_visible);
    luaL_setglobalfunction(L, "line_delete", lua_st_line_delete);
    luaL_setglobalfunction(L, "line_delete_all", lua_st_line_delete_all);
    luaL_setglobalfunction(L, "line_count", lua_st_line_count);
    luaL_setglobalfunction(L, "line_is_empty", lua_st_line_is_empty);
    luaL_setglobalfunction(L, "line_set_max", lua_st_line_set_max);
    luaL_setglobalfunction(L, "line_get_max", lua_st_line_get_max);

    // Polygon API - ID-Based Management
    luaL_setglobalfunction(L, "polygon_create", lua_st_polygon_create);
    luaL_setglobalfunction(L, "polygon_create_gradient", lua_st_polygon_create_gradient);
    luaL_setglobalfunction(L, "polygon_set_position", lua_st_polygon_set_position);
    luaL_setglobalfunction(L, "polygon_set_radius", lua_st_polygon_set_radius);
    luaL_setglobalfunction(L, "polygon_set_sides", lua_st_polygon_set_sides);
    luaL_setglobalfunction(L, "polygon_set_color", lua_st_polygon_set_color);
    luaL_setglobalfunction(L, "polygon_set_rotation", lua_st_polygon_set_rotation);
    luaL_setglobalfunction(L, "polygon_set_visible", lua_st_polygon_set_visible);
    luaL_setglobalfunction(L, "polygon_delete", lua_st_polygon_delete);
    luaL_setglobalfunction(L, "polygon_delete_all", lua_st_polygon_delete_all);
    luaL_setglobalfunction(L, "polygon_count", lua_st_polygon_count);

    // Star API - ID-Based Management
    luaL_setglobalfunction(L, "star_create", lua_st_star_create);
    luaL_setglobalfunction(L, "star_create_custom", lua_st_star_create_custom);
    luaL_setglobalfunction(L, "star_create_gradient", lua_st_star_create_gradient);
    luaL_setglobalfunction(L, "star_create_outline", lua_st_star_create_outline);
    luaL_setglobalfunction(L, "star_set_position", lua_st_star_set_position);
    luaL_setglobalfunction(L, "star_set_radius", lua_st_star_set_radius);
    luaL_setglobalfunction(L, "star_set_radii", lua_st_star_set_radii);
    luaL_setglobalfunction(L, "star_set_points", lua_st_star_set_points);
    luaL_setglobalfunction(L, "star_set_color", lua_st_star_set_color);
    luaL_setglobalfunction(L, "star_set_colors", lua_st_star_set_colors);
    luaL_setglobalfunction(L, "star_set_rotation", lua_st_star_set_rotation);
    luaL_setglobalfunction(L, "star_set_visible", lua_st_star_set_visible);
    luaL_setglobalfunction(L, "star_exists", lua_st_star_exists);
    luaL_setglobalfunction(L, "star_is_visible", lua_st_star_is_visible);
    luaL_setglobalfunction(L, "star_delete", lua_st_star_delete);
    luaL_setglobalfunction(L, "star_delete_all", lua_st_star_delete_all);
    luaL_setglobalfunction(L, "star_count", lua_st_star_count);
    luaL_setglobalfunction(L, "star_is_empty", lua_st_star_is_empty);

    // Star gradient mode constants
    luaL_setglobalnumber(L, "STAR_SOLID", 0);
    luaL_setglobalnumber(L, "STAR_RADIAL", 1);
    luaL_setglobalnumber(L, "STAR_ALTERNATING", 2);
    luaL_setglobalnumber(L, "STAR_OUTLINE", 100);
    luaL_setglobalnumber(L, "STAR_DASHED_OUTLINE", 101);

    // Audio API
    // Rectangle gradient mode constants
    luaL_setglobalnumber(L, "GRADIENT_HORIZONTAL", ST_GRADIENT_HORIZONTAL);
    luaL_setglobalnumber(L, "GRADIENT_VERTICAL", ST_GRADIENT_VERTICAL);
    luaL_setglobalnumber(L, "GRADIENT_RADIAL", ST_GRADIENT_RADIAL);
    luaL_setglobalnumber(L, "GRADIENT_DIAGONAL_TL_BR", ST_GRADIENT_DIAGONAL_TL_BR);
    luaL_setglobalnumber(L, "GRADIENT_DIAGONAL_TR_BL", ST_GRADIENT_DIAGONAL_TR_BL);

    // Rectangle pattern mode constants
    luaL_setglobalnumber(L, "PATTERN_OUTLINE", ST_PATTERN_OUTLINE);
    luaL_setglobalnumber(L, "PATTERN_HORIZONTAL_STRIPES", ST_PATTERN_HORIZONTAL_STRIPES);
    luaL_setglobalnumber(L, "PATTERN_VERTICAL_STRIPES", ST_PATTERN_VERTICAL_STRIPES);
    luaL_setglobalnumber(L, "PATTERN_DIAGONAL_STRIPES", ST_PATTERN_DIAGONAL_STRIPES);
    luaL_setglobalnumber(L, "PATTERN_CHECKERBOARD", ST_PATTERN_CHECKERBOARD);
    luaL_setglobalnumber(L, "PATTERN_DOTS", ST_PATTERN_DOTS);
    luaL_setglobalnumber(L, "PATTERN_GRID", ST_PATTERN_GRID);



    // Audio API
    luaL_setglobalfunction(L, "music_play", lua_st_music_play);
    luaL_setglobalfunction(L, "music_play_file", lua_st_music_play_file);
    luaL_setglobalfunction(L, "music_stop", lua_st_music_stop);
    luaL_setglobalfunction(L, "music_pause", lua_st_music_pause);
    luaL_setglobalfunction(L, "music_resume", lua_st_music_resume);
    luaL_setglobalfunction(L, "music_is_playing", lua_st_music_is_playing);
    luaL_setglobalfunction(L, "music_set_volume", lua_st_music_set_volume);
    luaL_setglobalfunction(L, "synth_note", lua_st_synth_note);
    luaL_setglobalfunction(L, "synth_set_instrument", lua_st_synth_set_instrument);

    // Sound Bank API
    luaL_setglobalfunction(L, "sound_create_beep", lua_st_sound_create_beep);
    luaL_setglobalfunction(L, "sound_create_blip", lua_st_sound_create_blip);
    luaL_setglobalfunction(L, "sound_create_click", lua_st_sound_create_click);
    luaL_setglobalfunction(L, "sound_create_zap", lua_st_sound_create_zap);
    luaL_setglobalfunction(L, "sound_create_explode", lua_st_sound_create_explode);
    luaL_setglobalfunction(L, "sound_create_pickup", lua_st_sound_create_pickup);
    luaL_setglobalfunction(L, "sound_create_hurt", lua_st_sound_create_hurt);
    luaL_setglobalfunction(L, "sound_create_sweep_down", lua_st_sound_create_sweep_down);
    luaL_setglobalfunction(L, "sound_create_coin", lua_st_sound_create_coin);
    luaL_setglobalfunction(L, "sound_create_powerup", lua_st_sound_create_powerup);
    luaL_setglobalfunction(L, "sound_play_id", lua_st_sound_play_id);
    luaL_setglobalfunction(L, "sound_play", lua_st_sound_play_id);  // Alias for convenience
    luaL_setglobalfunction(L, "sound_exists", lua_st_sound_exists);
    luaL_setglobalfunction(L, "sound_delete", lua_st_sound_delete);
    luaL_setglobalfunction(L, "synth_frequency", lua_st_synth_frequency);

    // Voice Controller API
    luaL_setglobalfunction(L, "voice_set_waveform", lua_st_voice_set_waveform);
    luaL_setglobalfunction(L, "voice_set_frequency", lua_st_voice_set_frequency);
    luaL_setglobalfunction(L, "voice_set_note", lua_st_voice_set_note);
    luaL_setglobalfunction(L, "voice_set_note_name", lua_st_voice_set_note_name);
    luaL_setglobalfunction(L, "voice_set_envelope", lua_st_voice_set_envelope);
    luaL_setglobalfunction(L, "voice_set_gate", lua_st_voice_set_gate);
    luaL_setglobalfunction(L, "voice_set_volume", lua_st_voice_set_volume);
    luaL_setglobalfunction(L, "voice_set_pulse_width", lua_st_voice_set_pulse_width);
    luaL_setglobalfunction(L, "voice_set_pan", lua_st_voice_set_pan);
    luaL_setglobalfunction(L, "voice_set_filter_routing", lua_st_voice_set_filter_routing);
    luaL_setglobalfunction(L, "voice_set_filter_type", lua_st_voice_set_filter_type);
    luaL_setglobalfunction(L, "voice_set_filter_cutoff", lua_st_voice_set_filter_cutoff);
    luaL_setglobalfunction(L, "voice_set_filter_resonance", lua_st_voice_set_filter_resonance);
    luaL_setglobalfunction(L, "voice_set_filter_enabled", lua_st_voice_set_filter_enabled);
    luaL_setglobalfunction(L, "voice_set_master_volume", lua_st_voice_set_master_volume);
    luaL_setglobalfunction(L, "voice_get_master_volume", lua_st_voice_get_master_volume);
    luaL_setglobalfunction(L, "voice_reset_all", lua_st_voice_reset_all);
    luaL_setglobalfunction(L, "voice_wait", lua_st_voice_wait);

    // Physical Modeling API
    luaL_setglobalfunction(L, "voice_set_physical_model", lua_st_voice_set_physical_model);
    luaL_setglobalfunction(L, "voice_set_physical_damping", lua_st_voice_set_physical_damping);
    luaL_setglobalfunction(L, "voice_set_physical_brightness", lua_st_voice_set_physical_brightness);
    luaL_setglobalfunction(L, "voice_set_physical_excitation", lua_st_voice_set_physical_excitation);
    luaL_setglobalfunction(L, "voice_set_physical_resonance", lua_st_voice_set_physical_resonance);
    luaL_setglobalfunction(L, "voice_set_physical_tension", lua_st_voice_set_physical_tension);
    luaL_setglobalfunction(L, "voice_set_physical_pressure", lua_st_voice_set_physical_pressure);
    luaL_setglobalfunction(L, "voice_physical_trigger", lua_st_voice_physical_trigger);

    // SID-style Modulation API
    luaL_setglobalfunction(L, "voice_set_ring_mod", lua_st_voice_set_ring_mod);
    luaL_setglobalfunction(L, "voice_set_sync", lua_st_voice_set_sync);
    luaL_setglobalfunction(L, "voice_set_portamento", lua_st_voice_set_portamento);
    luaL_setglobalfunction(L, "voice_set_detune", lua_st_voice_set_detune);

    // Delay Effects API
    luaL_setglobalfunction(L, "voice_set_delay_enable", lua_st_voice_set_delay_enable);
    luaL_setglobalfunction(L, "voice_set_delay_time", lua_st_voice_set_delay_time);
    luaL_setglobalfunction(L, "voice_set_delay_feedback", lua_st_voice_set_delay_feedback);
    luaL_setglobalfunction(L, "voice_set_delay_mix", lua_st_voice_set_delay_mix);

    // LFO Controls API
    luaL_setglobalfunction(L, "lfo_set_waveform", lua_st_lfo_set_waveform);
    luaL_setglobalfunction(L, "lfo_set_rate", lua_st_lfo_set_rate);
    luaL_setglobalfunction(L, "lfo_reset", lua_st_lfo_reset);
    luaL_setglobalfunction(L, "lfo_to_pitch", lua_st_lfo_to_pitch);
    luaL_setglobalfunction(L, "lfo_to_volume", lua_st_lfo_to_volume);
    luaL_setglobalfunction(L, "lfo_to_filter", lua_st_lfo_to_filter);
    luaL_setglobalfunction(L, "lfo_to_pulsewidth", lua_st_lfo_to_pulsewidth);

    luaL_setglobalfunction(L, "voices_start", lua_st_voices_start);
    luaL_setglobalfunction(L, "voices_set_tempo", lua_st_voices_set_tempo);
    luaL_setglobalfunction(L, "voices_end_slot", lua_st_voices_end_slot);
    luaL_setglobalfunction(L, "voices_next_slot", lua_st_voices_next_slot);
    luaL_setglobalfunction(L, "voices_end_play", lua_st_voices_end_play);
    luaL_setglobalfunction(L, "voices_end_save", lua_st_voices_end_save);
    luaL_setglobalfunction(L, "voices_are_playing", lua_st_voices_are_playing);
    luaL_setglobalfunction(L, "voice_direct", lua_st_voice_direct);
    luaL_setglobalfunction(L, "voice_direct_slot", lua_st_voice_direct_slot);
    luaL_setglobalfunction(L, "vscript_save_to_bank", lua_st_vscript_save_to_bank);

    // Voice waveform constants
    luaL_setglobalnumber(L, "WAVE_SILENCE", 0);
    luaL_setglobalnumber(L, "WAVE_SINE", 1);
    luaL_setglobalnumber(L, "WAVE_SQUARE", 2);
    luaL_setglobalnumber(L, "WAVE_SAW", 3);
    luaL_setglobalnumber(L, "WAVE_SAWTOOTH", 3);
    luaL_setglobalnumber(L, "WAVE_TRIANGLE", 4);
    luaL_setglobalnumber(L, "WAVE_NOISE", 5);
    luaL_setglobalnumber(L, "WAVE_PULSE", 6);
    luaL_setglobalnumber(L, "WAVE_PHYSICAL", 7);

    // Filter type constants
    luaL_setglobalnumber(L, "FILTER_NONE", 0);
    luaL_setglobalnumber(L, "FILTER_LOWPASS", 1);
    luaL_setglobalnumber(L, "FILTER_HIGHPASS", 2);
    luaL_setglobalnumber(L, "FILTER_BANDPASS", 3);

    // Input API
    luaL_setglobalfunction(L, "key_pressed", lua_st_key_pressed);
    luaL_setglobalfunction(L, "key_just_pressed", lua_st_key_just_pressed);
    luaL_setglobalfunction(L, "key_just_released", lua_st_key_just_released);
    luaL_setglobalfunction(L, "key_get_char", lua_st_key_get_char);
    luaL_setglobalfunction(L, "key_clear_buffer", lua_st_key_clear_buffer);
    luaL_setglobalfunction(L, "mouse_position", lua_st_mouse_position);
    luaL_setglobalfunction(L, "mouse_grid_position", lua_st_mouse_grid_position);
    luaL_setglobalfunction(L, "mouse_button", lua_st_mouse_button);
    luaL_setglobalfunction(L, "mouse_button_just_pressed", lua_st_mouse_button_just_pressed);
    luaL_setglobalfunction(L, "mouse_button_just_released", lua_st_mouse_button_just_released);

    // Frame Control API
    luaL_setglobalfunction(L, "wait_frame", lua_st_wait_frame);
    luaL_setglobalfunction(L, "wait_frames", lua_st_wait_frames);
    luaL_setglobalfunction(L, "wait_key", lua_st_wait_key);
    luaL_setglobalfunction(L, "wait", lua_st_wait);  // Wait for N seconds
    luaL_setglobalfunction(L, "sleep", lua_st_sleep);  // Sleep for N milliseconds
    luaL_setglobalfunction(L, "frame_count", lua_st_frame_count);
    luaL_setglobalfunction(L, "time", lua_st_time);
    luaL_setglobalfunction(L, "delta_time", lua_st_delta_time);

    // Utility API
    luaL_setglobalfunction(L, "rgb", lua_st_rgb);
    luaL_setglobalfunction(L, "rgba", lua_st_rgba);
    luaL_setglobalfunction(L, "hsv", lua_st_hsv);
    luaL_setglobalfunction(L, "debug_print", lua_st_debug_print);

    // Display API
    luaL_setglobalfunction(L, "display_size", lua_st_display_size);
    luaL_setglobalfunction(L, "cell_size", lua_st_cell_size);

    // Error handling API
    luaL_setglobalfunction(L, "st_get_error", lua_st_get_error);
    luaL_setglobalfunction(L, "st_clear_error", lua_st_clear_error);

    // Key codes as constants
    luaL_setglobalnumber(L, "KEY_ESCAPE", ST_KEY_ESCAPE);
    luaL_setglobalnumber(L, "KEY_ENTER", ST_KEY_ENTER);
    luaL_setglobalnumber(L, "KEY_SPACE", ST_KEY_SPACE);
    luaL_setglobalnumber(L, "KEY_BACKSPACE", ST_KEY_BACKSPACE);
    luaL_setglobalnumber(L, "KEY_TAB", ST_KEY_TAB);
    luaL_setglobalnumber(L, "KEY_UP", ST_KEY_UP);
    luaL_setglobalnumber(L, "KEY_DOWN", ST_KEY_DOWN);
    luaL_setglobalnumber(L, "KEY_LEFT", ST_KEY_LEFT);
    luaL_setglobalnumber(L, "KEY_RIGHT", ST_KEY_RIGHT);

    // Letters
    luaL_setglobalnumber(L, "KEY_A", ST_KEY_A);
    luaL_setglobalnumber(L, "KEY_B", ST_KEY_B);
    luaL_setglobalnumber(L, "KEY_C", ST_KEY_C);
    luaL_setglobalnumber(L, "KEY_D", ST_KEY_D);
    luaL_setglobalnumber(L, "KEY_E", ST_KEY_E);
    luaL_setglobalnumber(L, "KEY_F", ST_KEY_F);
    luaL_setglobalnumber(L, "KEY_G", ST_KEY_G);
    luaL_setglobalnumber(L, "KEY_H", ST_KEY_H);
    luaL_setglobalnumber(L, "KEY_I", ST_KEY_I);
    luaL_setglobalnumber(L, "KEY_J", ST_KEY_J);
    luaL_setglobalnumber(L, "KEY_K", ST_KEY_K);
    luaL_setglobalnumber(L, "KEY_L", ST_KEY_L);
    luaL_setglobalnumber(L, "KEY_M", ST_KEY_M);
    luaL_setglobalnumber(L, "KEY_N", ST_KEY_N);
    luaL_setglobalnumber(L, "KEY_O", ST_KEY_O);
    luaL_setglobalnumber(L, "KEY_P", ST_KEY_P);
    luaL_setglobalnumber(L, "KEY_Q", ST_KEY_Q);
    luaL_setglobalnumber(L, "KEY_R", ST_KEY_R);
    luaL_setglobalnumber(L, "KEY_S", ST_KEY_S);
    luaL_setglobalnumber(L, "KEY_T", ST_KEY_T);
    luaL_setglobalnumber(L, "KEY_U", ST_KEY_U);
    luaL_setglobalnumber(L, "KEY_V", ST_KEY_V);
    luaL_setglobalnumber(L, "KEY_W", ST_KEY_W);
    luaL_setglobalnumber(L, "KEY_X", ST_KEY_X);
    luaL_setglobalnumber(L, "KEY_Y", ST_KEY_Y);
    luaL_setglobalnumber(L, "KEY_Z", ST_KEY_Z);

    // Function keys
    luaL_setglobalnumber(L, "KEY_F1", ST_KEY_F1);
    luaL_setglobalnumber(L, "KEY_F2", ST_KEY_F2);
    luaL_setglobalnumber(L, "KEY_F3", ST_KEY_F3);
    luaL_setglobalnumber(L, "KEY_F4", ST_KEY_F4);
    luaL_setglobalnumber(L, "KEY_F5", ST_KEY_F5);
    luaL_setglobalnumber(L, "KEY_F6", ST_KEY_F6);
    luaL_setglobalnumber(L, "KEY_F7", ST_KEY_F7);
    luaL_setglobalnumber(L, "KEY_F8", ST_KEY_F8);
    luaL_setglobalnumber(L, "KEY_F9", ST_KEY_F9);
    luaL_setglobalnumber(L, "KEY_F10", ST_KEY_F10);
    luaL_setglobalnumber(L, "KEY_F11", ST_KEY_F11);
    luaL_setglobalnumber(L, "KEY_F12", ST_KEY_F12);

    // Mouse buttons
    luaL_setglobalnumber(L, "MOUSE_LEFT", ST_MOUSE_LEFT);
    luaL_setglobalnumber(L, "MOUSE_RIGHT", ST_MOUSE_RIGHT);
    luaL_setglobalnumber(L, "MOUSE_MIDDLE", ST_MOUSE_MIDDLE);

    // Particle System API
    luaL_setglobalfunction(L, "st_sprite_explode", lua_st_sprite_explode);
    luaL_setglobalfunction(L, "st_sprite_explode_advanced", lua_st_sprite_explode_advanced);
    luaL_setglobalfunction(L, "st_sprite_explode_directional", lua_st_sprite_explode_directional);
    luaL_setglobalfunction(L, "st_particle_clear", lua_st_particle_clear);
    luaL_setglobalfunction(L, "st_particle_pause", lua_st_particle_pause);
    luaL_setglobalfunction(L, "st_particle_resume", lua_st_particle_resume);
    luaL_setglobalfunction(L, "st_particle_set_time_scale", lua_st_particle_set_time_scale);
    luaL_setglobalfunction(L, "st_particle_set_world_bounds", lua_st_particle_set_world_bounds);
    luaL_setglobalfunction(L, "st_particle_set_enabled", lua_st_particle_set_enabled);
    luaL_setglobalfunction(L, "st_particle_get_active_count", lua_st_particle_get_active_count);
    luaL_setglobalfunction(L, "st_particle_get_total_created", lua_st_particle_get_total_created);
    luaL_setglobalfunction(L, "st_particle_dump_stats", lua_st_particle_dump_stats);

    // Sprite Management API
    luaL_setglobalfunction(L, "sprite_load", lua_st_sprite_load);
    luaL_setglobalfunction(L, "sprite_load_builtin", lua_st_sprite_load_builtin);
    luaL_setglobalfunction(L, "sprite_load_sprtz", lua_st_sprite_load_sprtz);
    luaL_setglobalfunction(L, "sprite_show", lua_st_sprite_show);
    luaL_setglobalfunction(L, "sprite_hide", lua_st_sprite_hide);
    luaL_setglobalfunction(L, "sprite_transform", lua_st_sprite_transform);
    luaL_setglobalfunction(L, "sprite_tint", lua_st_sprite_tint);
    luaL_setglobalfunction(L, "sprite_unload", lua_st_sprite_unload);

    // Indexed Sprite API
    luaL_setglobalfunction(L, "sprite_load_indexed_from_rgba", lua_st_sprite_load_indexed_from_rgba);
    luaL_setglobalfunction(L, "sprite_is_indexed", lua_st_sprite_is_indexed);
    luaL_setglobalfunction(L, "sprite_set_palette", lua_st_sprite_set_palette);
    luaL_setglobalfunction(L, "sprite_get_palette", lua_st_sprite_get_palette);
    luaL_setglobalfunction(L, "sprite_set_palette_color", lua_st_sprite_set_palette_color);
    luaL_setglobalfunction(L, "sprite_lerp_palette", lua_st_sprite_lerp_palette);
    luaL_setglobalfunction(L, "sprite_rotate_palette", lua_st_sprite_rotate_palette);
    luaL_setglobalfunction(L, "sprite_adjust_brightness", lua_st_sprite_adjust_brightness);
    luaL_setglobalfunction(L, "sprite_copy_palette", lua_st_sprite_copy_palette);
    luaL_setglobalfunction(L, "sprite_set_standard_palette", lua_st_sprite_set_standard_palette);

    // Sprite-based Particle Explosion API (v1 compatible)
    luaL_setglobalfunction(L, "sprite_explode", lua_sprite_explode);
    luaL_setglobalfunction(L, "sprite_explode_advanced", lua_sprite_explode_advanced);
    luaL_setglobalfunction(L, "sprite_explode_directional", lua_sprite_explode_directional);
    luaL_setglobalfunction(L, "sprite_explode_mode", lua_sprite_explode_mode);

    // Explosion mode constants
    luaL_setglobalnumber(L, "BASIC_EXPLOSION", 1);
    luaL_setglobalnumber(L, "MASSIVE_BLAST", 2);
    luaL_setglobalnumber(L, "GENTLE_DISPERSAL", 3);
    luaL_setglobalnumber(L, "RIGHTWARD_BLAST", 4);
    luaL_setglobalnumber(L, "UPWARD_ERUPTION", 5);
    luaL_setglobalnumber(L, "RAPID_BURST", 6);

    // Particle rendering mode constants
    luaL_setglobalnumber(L, "PARTICLE_MODE_POINT_SPRITE", 0);
    luaL_setglobalnumber(L, "PARTICLE_MODE_SPRITE_FRAGMENT", 1);

    // Asset type constants
    luaL_setglobalnumber(L, "ASSET_IMAGE", ST_ASSET_IMAGE);
    luaL_setglobalnumber(L, "ASSET_SOUND", ST_ASSET_SOUND);
    luaL_setglobalnumber(L, "ASSET_MUSIC", ST_ASSET_MUSIC);
    luaL_setglobalnumber(L, "ASSET_FONT", ST_ASSET_FONT);
    luaL_setglobalnumber(L, "ASSET_SPRITE", ST_ASSET_SPRITE);
    luaL_setglobalnumber(L, "ASSET_DATA", ST_ASSET_DATA);

    // Asset Management API - Create 'asset' namespace table
    lua_newtable(L);

    // Initialization functions
    lua_pushcfunction(L, lua_st_asset_init);
    lua_setfield(L, -2, "init");

    lua_pushcfunction(L, lua_st_asset_shutdown);
    lua_setfield(L, -2, "shutdown");

    lua_pushcfunction(L, lua_st_asset_is_initialized);
    lua_setfield(L, -2, "isInitialized");

    // Loading/Unloading functions
    lua_pushcfunction(L, lua_st_asset_load);
    lua_setfield(L, -2, "load");

    lua_pushcfunction(L, lua_st_asset_load_file);
    lua_setfield(L, -2, "loadFile");

    lua_pushcfunction(L, lua_st_asset_unload);
    lua_setfield(L, -2, "unload");

    lua_pushcfunction(L, lua_st_asset_is_loaded);
    lua_setfield(L, -2, "isLoaded");

    // Import/Export functions
    lua_pushcfunction(L, lua_st_asset_import);
    lua_setfield(L, -2, "import");

    lua_pushcfunction(L, lua_st_asset_import_directory);
    lua_setfield(L, -2, "importDirectory");

    lua_pushcfunction(L, lua_st_asset_export);
    lua_setfield(L, -2, "export");

    lua_pushcfunction(L, lua_st_asset_delete);
    lua_setfield(L, -2, "delete");

    // Data access functions
    lua_pushcfunction(L, lua_st_asset_get_data);
    lua_setfield(L, -2, "getData");

    lua_pushcfunction(L, lua_st_asset_get_size);
    lua_setfield(L, -2, "getSize");

    lua_pushcfunction(L, lua_st_asset_get_type);
    lua_setfield(L, -2, "getType");

    lua_pushcfunction(L, lua_st_asset_get_name);
    lua_setfield(L, -2, "getName");

    // Query functions
    lua_pushcfunction(L, lua_st_asset_exists);
    lua_setfield(L, -2, "exists");

    lua_pushcfunction(L, lua_st_asset_list);
    lua_setfield(L, -2, "list");

    lua_pushcfunction(L, lua_st_asset_search);
    lua_setfield(L, -2, "search");

    lua_pushcfunction(L, lua_st_asset_get_count);
    lua_setfield(L, -2, "getCount");

    // Cache management functions
    lua_pushcfunction(L, lua_st_asset_clear_cache);
    lua_setfield(L, -2, "clearCache");

    lua_pushcfunction(L, lua_st_asset_get_cache_size);
    lua_setfield(L, -2, "getCacheSize");

    lua_pushcfunction(L, lua_st_asset_get_cached_count);
    lua_setfield(L, -2, "getCachedCount");

    lua_pushcfunction(L, lua_st_asset_set_max_cache_size);
    lua_setfield(L, -2, "setMaxCacheSize");

    // Statistics functions
    lua_pushcfunction(L, lua_st_asset_get_hit_rate);
    lua_setfield(L, -2, "getHitRate");

    lua_pushcfunction(L, lua_st_asset_get_database_size);
    lua_setfield(L, -2, "getDatabaseSize");

    // Error handling functions
    lua_pushcfunction(L, lua_st_asset_get_error);
    lua_setfield(L, -2, "getError");

    lua_pushcfunction(L, lua_st_asset_clear_error);
    lua_setfield(L, -2, "clearError");

    // Set the 'asset' global table
    lua_setglobal(L, "asset");

    // Tilemap API - Create 'tilemap' namespace table
    lua_newtable(L);

    // System management
    lua_pushcfunction(L, lua_st_tilemap_init);
    lua_setfield(L, -2, "init");

    lua_pushcfunction(L, lua_st_tilemap_shutdown);
    lua_setfield(L, -2, "shutdown");

    // Tilemap management
    lua_pushcfunction(L, lua_st_tilemap_create);
    lua_setfield(L, -2, "create");

    lua_pushcfunction(L, lua_st_tilemap_destroy);
    lua_setfield(L, -2, "destroy");

    lua_pushcfunction(L, lua_st_tilemap_get_size);
    lua_setfield(L, -2, "getSize");

    // Layer management
    lua_pushcfunction(L, lua_st_tilemap_create_layer);
    lua_setfield(L, -2, "createLayer");

    lua_pushcfunction(L, lua_st_tilemap_destroy_layer);
    lua_setfield(L, -2, "destroyLayer");

    lua_pushcfunction(L, lua_st_tilemap_layer_set_tilemap);
    lua_setfield(L, -2, "layerSetTilemap");

    lua_pushcfunction(L, lua_st_tilemap_layer_set_tileset);
    lua_setfield(L, -2, "layerSetTileset");

    lua_pushcfunction(L, lua_st_tilemap_layer_set_parallax);
    lua_setfield(L, -2, "layerSetParallax");

    lua_pushcfunction(L, lua_st_tilemap_layer_set_opacity);
    lua_setfield(L, -2, "layerSetOpacity");

    lua_pushcfunction(L, lua_st_tilemap_layer_set_visible);
    lua_setfield(L, -2, "layerSetVisible");

    lua_pushcfunction(L, lua_st_tilemap_layer_set_z_order);
    lua_setfield(L, -2, "layerSetZOrder");

    lua_pushcfunction(L, lua_st_tilemap_layer_set_auto_scroll);
    lua_setfield(L, -2, "layerSetAutoScroll");

    // Tile manipulation
    lua_pushcfunction(L, lua_st_tilemap_set_tile);
    lua_setfield(L, -2, "setTile");

    lua_pushcfunction(L, lua_st_tilemap_get_tile);
    lua_setfield(L, -2, "getTile");

    lua_pushcfunction(L, lua_st_tilemap_fill_rect);
    lua_setfield(L, -2, "fillRect");

    lua_pushcfunction(L, lua_st_tilemap_clear);
    lua_setfield(L, -2, "clear");

    // Camera control
    lua_pushcfunction(L, lua_st_tilemap_set_camera);
    lua_setfield(L, -2, "setCamera");

    lua_pushcfunction(L, lua_st_tilemap_move_camera);
    lua_setfield(L, -2, "moveCamera");

    lua_pushcfunction(L, lua_st_tilemap_get_camera);
    lua_setfield(L, -2, "getCamera");

    lua_pushcfunction(L, lua_st_tilemap_set_zoom);
    lua_setfield(L, -2, "setZoom");

    lua_pushcfunction(L, lua_st_tilemap_camera_follow);
    lua_setfield(L, -2, "cameraFollow");

    lua_pushcfunction(L, lua_st_tilemap_set_camera_bounds);
    lua_setfield(L, -2, "setCameraBounds");

    lua_pushcfunction(L, lua_st_tilemap_camera_shake);
    lua_setfield(L, -2, "cameraShake");

    // Update
    lua_pushcfunction(L, lua_st_tilemap_update);
    lua_setfield(L, -2, "update");

    // Coordinate conversion
    lua_pushcfunction(L, lua_st_tilemap_world_to_tile);
    lua_setfield(L, -2, "worldToTile");

    lua_pushcfunction(L, lua_st_tilemap_tile_to_world);
    lua_setfield(L, -2, "tileToWorld");

    // Tileset management
    lua_pushcfunction(L, lua_st_tileset_load);
    lua_setfield(L, -2, "loadTileset");

    lua_pushcfunction(L, lua_st_tileset_load_asset);
    lua_setfield(L, -2, "loadTilesetAsset");

    lua_pushcfunction(L, lua_st_tileset_destroy);
    lua_setfield(L, -2, "destroyTileset");

    lua_pushcfunction(L, lua_st_tileset_get_tile_count);
    lua_setfield(L, -2, "getTilesetTileCount");

    lua_pushcfunction(L, lua_st_tileset_get_dimensions);
    lua_setfield(L, -2, "getTilesetDimensions");

    // Set the 'tilemap' global table
    lua_setglobal(L, "tilemap");

    // Collision Detection API
    luaL_setglobalfunction(L, "collision_circle_circle", lua_collision_circle_circle);
    luaL_setglobalfunction(L, "collision_circle_rect", lua_collision_circle_rect);
    luaL_setglobalfunction(L, "collision_circle_rect_bottom", lua_collision_circle_rect_bottom);
    luaL_setglobalfunction(L, "collision_rect_rect", lua_collision_rect_rect);
    luaL_setglobalfunction(L, "collision_point_in_circle", lua_collision_point_in_circle);
    luaL_setglobalfunction(L, "collision_point_in_rect", lua_collision_point_in_rect);
    luaL_setglobalfunction(L, "collision_circle_rect_info", lua_collision_circle_rect_info);
    luaL_setglobalfunction(L, "collision_circle_circle_penetration", lua_collision_circle_circle_penetration);
    luaL_setglobalfunction(L, "collision_rect_rect_overlap", lua_collision_rect_rect_overlap);
    luaL_setglobalfunction(L, "collision_swept_circle_rect", lua_collision_swept_circle_rect);

    // Indexed Tile Rendering API
    SuperTerminal::IndexedTileBindings::registerBindings(L);
}

} // namespace LuaRunner2
