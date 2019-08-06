/* Copyright JS Foundation and other contributors, http://js.foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "jerryscript.h"
#include "jerryscript-port-default.h"
#line 15 "/mnt/work/projects/jerryscript_main_repo/jerry-port/default/default-io.c"

#if !defined (WIN32)
#include <libgen.h>
#endif /* !defined (WIN32) */
#include <limits.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#line 27 "/mnt/work/projects/jerryscript_main_repo/jerry-port/default/default-io.c"

#ifndef DISABLE_EXTRA_API

/**
 * Actual log level
 */
static jerry_log_level_t jerry_port_default_log_level = JERRY_LOG_LEVEL_ERROR;

#define JERRY_PORT_DEFAULT_LOG_LEVEL jerry_port_default_log_level

/**
 * Get the log level
 *
 * @return current log level
 *
 * Note:
 *      This function is only available if the port implementation library is
 *      compiled without the DISABLE_EXTRA_API macro.
 */
jerry_log_level_t
jerry_port_default_get_log_level (void)
{
  return jerry_port_default_log_level;
} /* jerry_port_default_get_log_level */

/**
 * Set the log level
 *
 * Note:
 *      This function is only available if the port implementation library is
 *      compiled without the DISABLE_EXTRA_API macro.
 */
void
jerry_port_default_set_log_level (jerry_log_level_t level) /**< log level */
{
  jerry_port_default_log_level = level;
} /* jerry_port_default_set_log_level */

#else /* DISABLE_EXTRA_API */
#define JERRY_PORT_DEFAULT_LOG_LEVEL JERRY_LOG_LEVEL_ERROR
#endif /* !DISABLE_EXTRA_API */

/**
 * Default implementation of jerry_port_log. Prints log message to the standard
 * error with 'vfprintf' if message log level is less than or equal to the
 * current log level.
 *
 * If debugger support is enabled, printing happens first to an in-memory buffer,
 * which is then sent both to the standard error and to the debugger client.
 *
 * Note:
 *      Changing the log level from JERRY_LOG_LEVEL_ERROR is only possible if
 *      the port implementation library is compiled without the
 *      DISABLE_EXTRA_API macro.
 */
void
jerry_port_log (jerry_log_level_t level, /**< message log level */
                const char *format, /**< format string */
                ...)  /**< parameters */
{
  if (level <= JERRY_PORT_DEFAULT_LOG_LEVEL)
  {
    va_list args;
    va_start (args, format);
#if defined (JERRY_DEBUGGER) && (JERRY_DEBUGGER == 1)
    int length = vsnprintf (NULL, 0, format, args);
    va_end (args);
    va_start (args, format);

    JERRY_VLA (char, buffer, length + 1);
    vsnprintf (buffer, (size_t) length + 1, format, args);

    fprintf (stderr, "%s", buffer);
    jerry_debugger_send_log (level, (jerry_char_t *) buffer, (jerry_size_t) length);
#else /* If jerry-debugger isn't defined, libc is turned on */
    vfprintf (stderr, format, args);
#endif /* defined (JERRY_DEBUGGER) && (JERRY_DEBUGGER == 1) */
    va_end (args);
  }
} /* jerry_port_log */

#if defined (JERRY_DEBUGGER) && (JERRY_DEBUGGER == 1)

#define DEBUG_BUFFER_SIZE (256)
static char debug_buffer[DEBUG_BUFFER_SIZE];
static int debug_buffer_index = 0;

#endif /* defined (JERRY_DEBUGGER) && (JERRY_DEBUGGER == 1) */

/**
 * Default implementation of jerry_port_print_char. Uses 'putchar' to
 * print a single character to standard output.
 */
void
jerry_port_print_char (char c) /**< the character to print */
{
  putchar (c);

#if defined (JERRY_DEBUGGER) && (JERRY_DEBUGGER == 1)
  debug_buffer[debug_buffer_index++] = c;

  if ((debug_buffer_index == DEBUG_BUFFER_SIZE) || (c == '\n'))
  {
    jerry_debugger_send_output ((jerry_char_t *) debug_buffer, (jerry_size_t) debug_buffer_index);
    debug_buffer_index = 0;
  }
#endif /* defined (JERRY_DEBUGGER) && (JERRY_DEBUGGER == 1) */
} /* jerry_port_print_char */

/**
 * Determines the size of the given file.
 * @return size of the file
 */
static size_t
jerry_port_get_file_size (FILE *file_p) /**< opened file */
{
  fseek (file_p, 0, SEEK_END);
  long size = ftell (file_p);
  fseek (file_p, 0, SEEK_SET);

  return (size_t) size;
} /* jerry_port_get_file_size */

/**
 * Opens file with the given path and reads its source.
 * @return the source of the file
 */
uint8_t *
jerry_port_read_source (const char *file_name_p, /**< file name */
                        size_t *out_size_p) /**< [out] read bytes */
{
  FILE *file_p = fopen (file_name_p, "rb");

  if (file_p == NULL)
  {
    jerry_port_log (JERRY_LOG_LEVEL_ERROR, "Error: failed to open file: %s\n", file_name_p);
    return NULL;
  }

  size_t file_size = jerry_port_get_file_size (file_p);
  uint8_t *buffer_p = (uint8_t *) malloc (file_size);

  if (buffer_p == NULL)
  {
    fclose (file_p);

    jerry_port_log (JERRY_LOG_LEVEL_ERROR, "Error: failed to allocate memory for module");
    return NULL;
  }

  size_t bytes_read = fread (buffer_p, 1u, file_size, file_p);

  if (!bytes_read)
  {
    fclose (file_p);
    free (buffer_p);

    jerry_port_log (JERRY_LOG_LEVEL_ERROR, "Error: failed to read file: %s\n", file_name_p);
    return NULL;
  }

  fclose (file_p);
  *out_size_p = bytes_read;

  return buffer_p;
} /* jerry_port_read_source */

/**
 * Release the previously opened file's content.
 */
void
jerry_port_release_source (uint8_t *buffer_p) /**< buffer to free */
{
  free (buffer_p);
} /* jerry_port_release_source */

/**
 * Normalize a file path
 *
 * @return length of the path written to the output buffer
 */
size_t
jerry_port_normalize_path (const char *in_path_p,   /**< input file path */
                           char *out_buf_p,         /**< output buffer */
                           size_t out_buf_size,     /**< size of output buffer */
                           char *base_file_p)       /**< base file path */
{
  size_t ret = 0;

#if defined (WIN32)
  char drive[_MAX_DRIVE];
  char *dir_p = (char *) malloc (_MAX_DIR);

  char *path_p = (char *) malloc (_MAX_PATH * 2);
  *path_p = '\0';

  if (base_file_p != NULL)
  {
    _splitpath_s (base_file_p,
                  &drive,
                  _MAX_DRIVE,
                  dir_p,
                  _MAX_DIR,
                  NULL,
                  0,
                  NULL,
                  0);
    strncat (path_p, &drive, _MAX_DRIVE);
    strncat (path_p, dir_p, _MAX_DIR);
  }

  strncat (path_p, in_path_p, _MAX_PATH);

  char *norm_p = _fullpath (out_buf_p, path_p, out_buf_size);

  free (path_p);
  free (dir_p);

  if (norm_p != NULL)
  {
    ret = strnlen (norm_p, out_buf_size);
  }
#elif defined (__unix__) || defined (__APPLE__)
#define MAX_JERRY_PATH_SIZE 256
  char *buffer_p = (char *) malloc (PATH_MAX);
  char *path_p = (char *) malloc (PATH_MAX);

  char *base_p = dirname (base_file_p);
  strncpy (path_p, base_p, MAX_JERRY_PATH_SIZE);
  strncat (path_p, "/", 1);
  strncat (path_p, in_path_p, MAX_JERRY_PATH_SIZE);

  char *norm_p = realpath (path_p, buffer_p);
  free (path_p);

  if (norm_p != NULL)
  {
    const size_t len = strnlen (norm_p, out_buf_size);
    if (len < out_buf_size)
    {
      strncpy (out_buf_p, norm_p, out_buf_size);
      ret = len;
    }
  }

  free (buffer_p);
#undef MAX_JERRY_PATH_SIZE
#else
  (void) base_file_p;

  /* Do nothing, just copy the input. */
  const size_t len = strnlen (in_path_p, out_buf_size);
  if (len < out_buf_size)
  {
    strncpy (out_buf_p, in_path_p, out_buf_size);
    ret = len;
  }
#endif

  return ret;
} /* jerry_port_normalize_path */
#line 15 "/mnt/work/projects/jerryscript_main_repo/jerry-port/default/default-fatal.c"

#include <stdlib.h>

#line 20 "/mnt/work/projects/jerryscript_main_repo/jerry-port/default/default-fatal.c"

#ifndef DISABLE_EXTRA_API

static bool abort_on_fail = false;

/**
 * Sets whether 'abort' should be called instead of 'exit' upon exiting with
 * non-zero exit code in the default implementation of jerry_port_fatal.
 *
 * Note:
 *      This function is only available if the port implementation library is
 *      compiled without the DISABLE_EXTRA_API macro.
 */
void jerry_port_default_set_abort_on_fail (bool flag) /**< new value of 'abort on fail' flag */
{
  abort_on_fail = flag;
} /* jerry_port_default_set_abort_on_fail */

/**
 * Check whether 'abort' should be called instead of 'exit' upon exiting with
 * non-zero exit code in the default implementation of jerry_port_fatal.
 *
 * @return true - if 'abort on fail' flag is set,
 *         false - otherwise
 *
 * Note:
 *      This function is only available if the port implementation library is
 *      compiled without the DISABLE_EXTRA_API macro.
 */
bool jerry_port_default_is_abort_on_fail (void)
{
  return abort_on_fail;
} /* jerry_port_default_is_abort_on_fail */

#endif /* !DISABLE_EXTRA_API */

/**
 * Default implementation of jerry_port_fatal. Calls 'abort' if exit code is
 * non-zero and "abort-on-fail" behaviour is enabled, 'exit' otherwise.
 *
 * Note:
 *      The "abort-on-fail" behaviour is only available if the port
 *      implementation library is compiled without the DISABLE_EXTRA_API macro.
 */
void jerry_port_fatal (jerry_fatal_code_t code) /**< cause of error */
{
#ifndef DISABLE_EXTRA_API
  if (code != 0
      && code != ERR_OUT_OF_MEMORY
      && abort_on_fail)
  {
    abort ();
  }
#endif /* !DISABLE_EXTRA_API */

  exit ((int) code);
} /* jerry_port_fatal */
#line 15 "/mnt/work/projects/jerryscript_main_repo/jerry-port/default/default-external-context.c"

#line 18 "/mnt/work/projects/jerryscript_main_repo/jerry-port/default/default-external-context.c"

/**
 * Pointer to the current context.
 * Note that it is a global variable, and is not a thread safe implementation.
 */
static jerry_context_t *current_context_p = NULL;

/**
 * Set the current_context_p as the passed pointer.
 */
void
jerry_port_default_set_current_context (jerry_context_t *context_p) /**< points to the created context */
{
  current_context_p = context_p;
} /* jerry_port_default_set_current_context */

/**
 * Get the current context.
 *
 * @return the pointer to the current context
 */
jerry_context_t *
jerry_port_get_current_context (void)
{
  return current_context_p;
} /* jerry_port_get_current_context */
#line 15 "/mnt/work/projects/jerryscript_main_repo/jerry-port/default/default-debugger.c"

#if !defined (_XOPEN_SOURCE) || _XOPEN_SOURCE < 500
#undef _XOPEN_SOURCE
/* Required macro for sleep functions (nanosleep or usleep) */
#define _XOPEN_SOURCE 500
#endif

#ifdef WIN32
#include <windows.h>
#elif defined (HAVE_TIME_H)
#include <time.h>
#elif defined (HAVE_UNISTD_H)
#include <unistd.h>
#endif /* WIN32 */

#line 32 "/mnt/work/projects/jerryscript_main_repo/jerry-port/default/default-debugger.c"

/**
 * Default implementation of jerry_port_sleep. Uses 'nanosleep' or 'usleep' if
 * available on the system, does nothing otherwise.
 */
void jerry_port_sleep (uint32_t sleep_time) /**< milliseconds to sleep */
{
#ifdef WIN32
  Sleep (sleep_time);
#elif defined (HAVE_TIME_H)
  struct timespec sleep_timespec;
  sleep_timespec.tv_sec = (time_t) sleep_time / 1000;
  sleep_timespec.tv_nsec = ((long int) sleep_time % 1000) * 1000000L;

  nanosleep (&sleep_timespec, NULL);
#elif defined (HAVE_UNISTD_H)
  usleep ((useconds_t) sleep_time * 1000);
#else
  (void) sleep_time;
#endif /* HAVE_TIME_H */
} /* jerry_port_sleep */
#line 15 "/mnt/work/projects/jerryscript_main_repo/jerry-port/default/default-date.c"

#ifdef HAVE_TM_GMTOFF
#include <time.h>
#endif /* HAVE_TM_GMTOFF */
#ifdef __GNUC__
#include <sys/time.h>
#endif /* __GNUC__ */

#line 25 "/mnt/work/projects/jerryscript_main_repo/jerry-port/default/default-date.c"

/**
 * Default implementation of jerry_port_get_local_time_zone_adjustment. Uses the 'tm_gmtoff' field
 * of 'struct tm' (a GNU extension) filled by 'localtime_r' if available on the
 * system, does nothing otherwise.
 *
 * @return offset between UTC and local time at the given unix timestamp, if
 *         available. Otherwise, returns 0, assuming UTC time.
 */
double jerry_port_get_local_time_zone_adjustment (double unix_ms,  /**< ms since unix epoch */
                                                  bool is_utc)  /**< is the time above in UTC? */
{
#ifdef HAVE_TM_GMTOFF
  struct tm tm;
  time_t now = (time_t) (unix_ms / 1000);
  localtime_r (&now, &tm);
  if (!is_utc)
  {
    now -= tm.tm_gmtoff;
    localtime_r (&now, &tm);
  }
  return ((double) tm.tm_gmtoff) * 1000;
#else /* !HAVE_TM_GMTOFF */
  (void) unix_ms;
  (void) is_utc;
  return 0.0;
#endif /* HAVE_TM_GMTOFF */
} /* jerry_port_get_local_time_zone_adjustment */

/**
 * Default implementation of jerry_port_get_current_time. Uses 'gettimeofday' if
 * available on the system, does nothing otherwise.
 *
 * @return milliseconds since Unix epoch - if 'gettimeofday' is available and
 *                                         executed successfully,
 *         0 - otherwise.
 */
double jerry_port_get_current_time (void)
{
#ifdef __GNUC__
  struct timeval tv;

  if (gettimeofday (&tv, NULL) == 0)
  {
    return ((double) tv.tv_sec) * 1000.0 + ((double) tv.tv_usec) / 1000.0;
  }
#endif /* __GNUC__ */

  return 0.0;
} /* jerry_port_get_current_time */
