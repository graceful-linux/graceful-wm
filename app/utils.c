//
// Created by dingjing on 23-11-23.
//

#include "utils.h"

#include <err.h>
#include <glob.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <glib.h>
#include <glib/gi18n.h>
#include <ev.h>

#include "log.h"
#include "val.h"


xcb_visualtype_t* util_get_visual_type(xcb_screen_t *screen)
{
    for (xcb_depth_iterator_t it = xcb_screen_allowed_depths_iterator(screen); it.rem; xcb_depth_next(&it)) {
        for (struct xcb_visualtype_iterator_t vit = xcb_depth_visuals_iterator(it.data); vit.rem; xcb_visualtype_next(&vit)) {
            if (screen->root_visual == vit.data->visual_id) {
                return vit.data;
            }
        }
    }

    return NULL;
}

uint32_t util_aio_get_mod_mask_for(uint32_t keySym, xcb_key_symbols_t *symbols)
{
    xcb_get_modifier_mapping_cookie_t   cookie;
    xcb_get_modifier_mapping_reply_t*   modMapR = NULL;

    xcb_flush(gConn);

    cookie = xcb_get_modifier_mapping(gConn);
    if (!(modMapR = xcb_get_modifier_mapping_reply(gConn, cookie, NULL))) {
        return 0;
    }

    uint32_t result = util_get_mod_mask_for(keySym, symbols, modMapR);

    FREE(modMapR);

    return result;
}

uint32_t util_get_mod_mask_for(uint32_t keySym, xcb_key_symbols_t *symbols, xcb_get_modifier_mapping_reply_t *modMapReply) {
    xcb_keycode_t modCode;
    xcb_keycode_t *codes, *modMap;

    modMap = xcb_get_modifier_mapping_keycodes (modMapReply);

    if (!(codes = xcb_key_symbols_get_keycode (symbols, keySym))) {
        return 0;
    }

    for (int mod = 0; mod < 8; mod++) {
        for (int j = 0; j < modMapReply->keycodes_per_modifier; j++) {
            modCode = modMap[(mod * modMapReply->keycodes_per_modifier) + j];
            for (xcb_keycode_t *code = codes; *code; code++) {
                if (*code != modCode) {
                    continue;
                }
                FREE(codes);
                return (1 << mod);
            }
        }
    }

    return 0;
}

bool util_parse_long(const char *str, long *out, int base)
{
    char *end = NULL;
    long result = strtol(str, &end, base);
    if (result == LONG_MIN || result == LONG_MAX || result < 0 || (end != NULL && *end != '\0')) {
        *out = result;
        return false;
    }

    *out = result;

    return true;
}

char* util_resolve_tilde(const char *path)
{
    static glob_t globBuf;
    char *head, *tail, *result;

    tail = strchr(path, '/');
    head = g_strndup(path, tail ? (size_t)(tail - path) : strlen(path));

    int res = glob(head, GLOB_TILDE, NULL, &globBuf);
    FREE(head);
    /* no match, or many wildcard matches are bad */
    if (res == GLOB_NOMATCH || globBuf.gl_pathc != 1) {
        result = g_strdup(path);
    }
    else if (res != 0) {
        err(EXIT_FAILURE, "glob() failed");
    }
    else {
        head = globBuf.gl_pathv[0];
        result = g_malloc0(strlen(head) + (tail ? strlen(tail) : 0) + 1);
        strcpy(result, head);
        if (tail) {
            strcat(result, tail);
        }
    }

    globfree(&globBuf);

    return result;
}

bool util_path_exists(const char *path)
{
    struct stat buf;
    return (stat(path, &buf) == 0);
}

ssize_t util_slurp(const char *path, char **buf)
{
    FILE *f;
    if ((f = fopen(path, "r")) == NULL) {
        ERROR(_("Cannot open file \"%s\": %s"), path, strerror(errno));
        return -1;
    }
    struct stat stbuf;
    if (fstat(fileno(f), &stbuf) != 0) {
        ERROR(_("Cannot fstat() \"%s\": %s"), path, strerror(errno));
        fclose(f);
        return -1;
    }
    /* Allocate one extra NUL byte to make the buffer usable with C string
     * functions. yajl doesn’t need this, but this makes slurp safer. */
    *buf = g_malloc0 (stbuf.st_size + 1);
    size_t n = fread(*buf, 1, stbuf.st_size, f);
    fclose(f);
    if ((ssize_t)n != stbuf.st_size) {
        ERROR("File \"%s\" could not be read entirely: got %zd, want %" PRIi64 "", path, n, (int64_t)stbuf.st_size);
        {
            FREE(*buf);
            *buf = NULL;
        }
        return -1;
    }

    return (ssize_t)n;
}

GWMRect util_rect_add(GWMRect a, GWMRect b)
{
    return (GWMRect){a.x + b.x, a.y + b.y, a.width + b.width, a.height + b.height};
}

GWMRect util_rect_sub(GWMRect a, GWMRect b)
{
    return (GWMRect){a.x - b.x, a.y - b.y, a.width - b.width, a.height - b.height};
}

bool util_rect_equals(GWMRect a, GWMRect b)
{
    return a.x == b.x && a.y == b.y && a.width == b.width && a.height == b.height;
}

GWMRect util_rect_sanitize_dimensions(GWMRect rect)
{
    rect.width = (int32_t)rect.width <= 0 ? 1 : rect.width;
    rect.height = (int32_t)rect.height <= 0 ? 1 : rect.height;

    return rect;
}

bool util_rect_contains(GWMRect rect, uint32_t x, uint32_t y)
{
    return (x >= rect.x
        && x <= (rect.x + rect.width)
        && y >= rect.y
        && y <= (rect.y + rect.height));
}

bool util_name_is_digits(const char *name)
{
    for (size_t i = 0; i < strlen(name); i++) {
        if (!g_ascii_isdigit(name[i])) {
            return false;
        }
    }

    return true;
}

int util_ws_name_to_number(const char *name)
{
    char *endPtr = NULL;
    errno = 0;
    long long parsedNum = strtoll(name, &endPtr, 10);
    if (errno != 0 || parsedNum > INT32_MAX || parsedNum < 0 || endPtr == name) {
        parsedNum = -1;
    }

    return parsedNum;
}


char* util_parse_string(const char **walk, bool asWord)
{
    const char *beginning = *walk;

    if (**walk == '"') {
        beginning++;
        (*walk)++;
        for (; **walk != '\0' && **walk != '"'; (*walk)++) {
            if (**walk == '\\' && *(*walk + 1) != '\0') {
                (*walk)++;
            }
        }
    }
    else {
        if (!asWord) {
            while (**walk != ';'
                && **walk != ','
                && **walk != '\0'
                && **walk != '\r'
                && **walk != '\n') {
                (*walk)++;
            }
        }
        else {
            while (**walk != ' '
                && **walk != '\t'
                && **walk != ']'
                && **walk != ','
                && **walk != ';'
                && **walk != '\r'
                && **walk != '\n'
                && **walk != '\0') {
                (*walk)++;
            }
        }
    }
    if (*walk == beginning)
        return NULL;

    char *str = calloc(*walk - beginning + 1, 1);
    int inpos, outpos;
    for (inpos = 0, outpos = 0; inpos < (*walk - beginning); inpos++, outpos++) {
        if (beginning[inpos] == '\\' && (beginning[inpos + 1] == '"' || beginning[inpos + 1] == '\\')) {
            inpos++;
        }
        str[outpos] = beginning[inpos];
    }

    return str;
}

bool util_update_if_necessary(uint32_t *destination, uint32_t newValue)
{
    uint32_t oldValue = *destination;

    return ((*destination = newValue) != oldValue);
}

GWMOrientation util_orientation_from_direction(GWMDirection direction)
{
    return (direction == D_LEFT || direction == D_RIGHT) ? HORIZON : VERT;
}

const char* util_direction_to_string(GWMDirection direction)
{
    switch (direction) {
        case D_LEFT: {
            return "left";
        }
        case D_RIGHT: {
            return "right";
        }
        case D_UP: {
            return "up";
        }
        case D_DOWN: {
            return "down";
        }
    }
    return "invalid";
}

GWMPosition util_position_from_direction(GWMDirection direction)
{
    return (direction == D_LEFT || direction == D_UP) ? BEFORE : AFTER;
}

const char* util_position_to_string(GWMPosition position)
{
    switch (position) {
        case BEFORE: {
            return "before";
        }
        case AFTER: {
            return "after";
        }
    }

    return "invalid";
}