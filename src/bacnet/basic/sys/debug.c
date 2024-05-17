/*####COPYRIGHTBEGIN####
 -------------------------------------------
 Copyright (C) 2008 Steve Karg

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to:
 The Free Software Foundation, Inc.
 59 Temple Place - Suite 330
 Boston, MA  02111-1307, USA.

 As a special exception, if other files instantiate templates or
 use macros or inline functions from this file, or you compile
 this file and link it with other works to produce a work based
 on this file, this file does not by itself cause the resulting
 work to be covered by the GNU General Public License. However
 the source code for this file must still be made available in
 accordance with section (3) of the GNU General Public License.

 This exception does not invalidate any other reasons why a work
 based on this file might be covered by the GNU General Public
 License.
 -------------------------------------------
####COPYRIGHTEND####*/

#include <stdint.h> /* for standard integer types uint8_t etc. */
#include <stdbool.h> /* for the standard bool type. */
#include <stdio.h> /* Standard I/O */
#include <stdlib.h> /* Standard Library */
#include <stdarg.h>
#if DEBUG_ENABLED
#include <string.h>
#include <ctype.h>
#endif
#include "bacnet/basic/sys/debug.h"
#if DEBUG_PRINTF_WITH_TIMESTAMP
#include "bacnet/datetime.h"
#endif
/** @file debug.c  Debug print function */

#if DEBUG_PRINTF_WITH_TIMESTAMP
/**
 * @brief Print timestamp with a printf string
 * @param format - printf format string
 * @param ... - variable arguments
 * @note This function is only available if
 *  DEBUG_PRINTF_WITH_TIMESTAMP is non-zero
 *  and DEBUG_ENABLED is non-zero
 */
void debug_printf(const char *format, ...)
{
#if DEBUG_ENABLED
    va_list ap;
    char stamp_str[64];
    char buf[1024];
    BACNET_DATE date;
    BACNET_TIME time;
    datetime_local(&date, &time, NULL, NULL);
    snprintf(
        stamp_str, sizeof(stamp_str), "[%02d:%02d:%02d.%03d]: ", time.hour,
        time.min, time.sec, time.hundredths * 10);
    va_start(ap, format);
    vsprintf(buf, format, ap);
    va_end(ap);
    printf("%s%s", stamp_str, buf);
    fflush(stdout);
#else
    (void)format;
#endif
}
#else
/**
 * @brief Print with a printf string
 * @param format - printf format string
 * @param ... - variable arguments
 * @note This function is only available if
 * DEBUG_ENABLED is non-zero
 */
void debug_printf(const char *format, ...)
{
#if DEBUG_ENABLED
    va_list ap;

    va_start(ap, format);
    vfprintf(stdout, format, ap);
    va_end(ap);
    fflush(stdout);
#else
    (void)format;
#endif
}
#endif

/**
 * @brief print format with HEX dump of a buffer
 * @param offset - starting address to print to the left side
 * @param buffer - buffer from which to print hex from
 * @param buffer_length - number of bytes from the buffer to print
 * @param format - printf format string
 * @param ... - variable arguments
 * @note This function is only available if DEBUG_ENABLED is non-zero
 */
void debug_printf_hex(
    uint32_t offset,
    const uint8_t *buffer,
    size_t buffer_length,
    const char *format, ...)
{
#if DEBUG_ENABLED
    size_t i = 0;
    bool new_line = true;
    char line[16+1] = {0};
    size_t remainder = 0;
    va_list ap;

    va_start(ap, format);
    vfprintf(stdout, format, ap);
    va_end(ap);
    /* print the buffer after the formatted text */
    if (buffer && buffer_length) {
        for (i = 0; i < buffer_length; i++) {
            if (new_line) {
                new_line = false;
                printf("%08x  ", (unsigned int)(offset+i));
                memset(line, '.', sizeof(line)-1);
            }
            printf("%02x ", buffer[i]);
            if (isprint(buffer[i])) {
                line[i%16] = buffer[i];
            }
            if ((i != 0) && (!((i+1)%16))) {
                printf(" %s\n", line);
                new_line = true;
            }
        }
        remainder = buffer_length%16;
        if (remainder) {
            for (i = 0; i < (16-remainder); i++) {
                printf("   ");
            }
            printf(" %s\n", line);
        }
    }
    fflush(stdout);
#else
    (void)offset;
    (void)buffer;
    (void)buffer_length;
    (void)format;
#endif
}

/**
 * @brief Print with a printf string
 * @param format - printf format string
 * @param ... - variable arguments
 * @note This function is only available if
 * PRINT_ENABLED is non-zero
 * @return number of characters printed
 */
int debug_aprintf(const char *format, ...)
{
    int length = 0;
#if PRINT_ENABLED
    va_list ap;

    va_start(ap, format);
    length = vfprintf(stdout, format, ap);
    va_end(ap);
    fflush(stdout);
#else
    (void)format;
#endif
    return length;
}

/**
 * @brief Print with a printf string
 * @param stream - file stream to print to
 * @param format - printf format string
 * @param ... - variable arguments
 * @note This function is only available if
 * PRINT_ENABLED is non-zero
 * @return number of characters printed
 */
int debug_fprintf(FILE *stream, const char *format, ...)
{
    int length = 0;
#if PRINT_ENABLED
    va_list ap;

    va_start(ap, format);
    length = vfprintf(stream, format, ap);
    va_end(ap);
    fflush(stream);
#else
    (void)stream;
    (void)format;
#endif
    return length;
}

/**
 * @brief Print with a perror string
 * @param format - printf format string
 * @param ... - variable arguments
 * @note This function is only available if
 * PRINT_ENABLED is non-zero
*/
void debug_perror(const char *format, ...)
{
#if PRINT_ENABLED
    va_list ap;

    va_start(ap, format);
    vfprintf(stderr, format, ap);
    va_end(ap);
    fflush(stderr);
#else
    (void)format;
#endif
}

/**
 * @brief Print with a printf string that does nothing
 * @param format - printf format string
 * @param ... - variable arguments
 * @note useful when used with defines such as PRINTF
 */
void debug_printf_disabled(const char *format, ...)
{
    (void)format;
}
