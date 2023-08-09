/* xscreensaver-systemd-idle-hint, Copyright (c) 2023
 * Florian Schmaus <flo@geekplace.eu>
 *
 * ISC License
 *
 * Permission to use, copy, modify, and/or distribute this software
 * for any purpose with or without fee is hereby granted, provided
 * that the above copyright notice and this permission notice appear
 * in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
 * AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 *****************************************************************************
 * This utility sets the user's login session idle hint.
 *
 * If invoked with "active" as first argument, then the idle hint is set to
 * "false". If invoked with "idle" as first argument, then the idle hint is set
 * to "true".
 *****************************************************************************
 */

#include <stddef.h>
#include <stdlib.h>

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#if defined (HAVE_LIBSYSTEMD)
# include <systemd/sd-bus.h>
#elif defined (HAVE_LIBELOGIND)
# include <elogind/sd-bus.h>
#else
# error Neither HAVE_LIBSYSTEMD nor HAVE_LIBELOGIND is defined.
#endif

#include "blurb.h"

static int set_idle_hint(int idle) {
    sd_bus *bus = NULL;
    sd_bus_error error = SD_BUS_ERROR_NULL;
    int res;

    /* Connect to the system bus */
    res = sd_bus_open_system(&bus);
    if (res < 0) {
        fprintf(stderr, "%s: Failed to connect to system bus: %s\n",
                blurb(), strerror(-res));
        return -1;
    }

    /* Issue the method call and store the respons message in m */
    res = sd_bus_call_method(bus,
                             "org.freedesktop.login1",               /* service to contact */
                             "/org/freedesktop/login1/session/auto", /* object path */
                             "org.freedesktop.login1.Session",       /* interface name */
                             "SetIdleHint",                          /* method name */
                             &error,                                 /* object to return error in */
                             NULL,                                   /* return message on success */
                             "b",                                    /* input signature */
                             idle);                                  /* first argument */
    if (res < 0) {
        fprintf(stderr, "%s: Failed to issue method call: %s\n",
                blurb(), error.message);
        return -2;
    }

    sd_bus_error_free(&error);
    sd_bus_unref(bus);
    return 0;
}

int main(int argc, char *argv[]) {
    int idle, res;

    if (argc != 2) goto usage;

    if (!strcmp(argv[1], "active")) {
        idle = 0;
    } else if (!strcmp(argv[1], "active")) {
        idle = 1;
    } else {
        goto usage;
    }

    res = set_idle_hint(idle);
    if (res)
        return EXIT_FAILURE;
    return EXIT_SUCCESS;

 usage:
    fprintf(stderr, "Usage: %s <active|idle>\n", argv[0]);
    return EXIT_FAILURE;
}
