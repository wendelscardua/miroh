#pragma once

/* Something almost but not quite like entirely different from coroutines
  Use a static value to keep track of intermerdiate states
  (for example, the current index on a for loop)

 Example:

  static int index;
  bool counter() {
    CORO_INIT {
      index = 1;
    }

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
  static void *resume_label = NULL;                                            \
  if (resume_label != NULL) {                                                  \
    goto *resume_label;                                                        \
  } else
#define STR(value) #value
#define CORO_YIELD(retval)                                                     \
  resume_label = &&UNIQUE_LABEL;                                               \
  return retval;                                                               \
  UNIQUE_LABEL:
#define CORO_FINISH(retval)                                                    \
  resume_label = NULL;                                                         \
  return retval
