#pragma once

/* Something almost but not quite like entirely different from coroutines
  Use a static value to keep track of intermerdiate states
  (for example, the current index on a for loop)

 Example:

  static int index;
  bool counter() {
    CORO_INIT;

    while(index <= 10) {
      index++;
      CORO_YIELD(true);
    }

    CORO_FINISH(false);
  }

 */

#define _TOKEN_PASTE(x, y) x##y
#define _CAT(x, y) _TOKEN_PASTE(x, y)
#define UNIQUE_LABEL _CAT(step_, __LINE__)

#define CORO_INIT                                                              \
  static void *CORO_RESUME_LABEL = NULL;                                       \
  if (CORO_RESUME_LABEL != NULL) {                                             \
    goto *CORO_RESUME_LABEL;                                                   \
  }

#define CORO_RESET_WHEN(expr)                                                  \
  static void *CORO_RESUME_LABEL = NULL;                                       \
  if (expr) {                                                                  \
    CORO_RESUME_LABEL = NULL;                                                  \
  } else if (CORO_RESUME_LABEL != NULL) {                                      \
    goto *CORO_RESUME_LABEL;                                                   \
  }

#define CORO_YIELD(retval)                                                     \
  CORO_RESUME_LABEL = &&UNIQUE_LABEL;                                          \
  return retval;                                                               \
  UNIQUE_LABEL:
#define CORO_FINISH(retval)                                                    \
  CORO_RESUME_LABEL = NULL;                                                    \
  return retval
#define CORO_RESET CORO_RESUME_LABEL = NULL