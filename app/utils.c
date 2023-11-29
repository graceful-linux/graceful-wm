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

#include "log.h"
#include "val.h"


xcb_visualtype_t *get_visual_type(xcb_screen_t *screen)
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

    free(modMapR);

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
                free (codes);
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
    free(head);
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
        ERROR("File \"%s\" could not be read entirely: got %zd, want %" PRIi64 "\n", path, n, (int64_t)stbuf.st_size);
        {
            g_free (*buf);
            *buf = NULL;
        }
        return -1;
    }

    return (ssize_t)n;
}