#include <string.h>
#include "errlist.h"

#define ERR_COUNT 5

static const char* errlist[ERR_COUNT] = {
  ERR_COMMAND_LINE,
  ERR_OPEN_SOCKET,
  ERR_CLOSE_SOCKET,
  ERR_BAD_STATUS,
  ERR_INVALID_PAR_LOG_FILE
};

const char *strerror_udpsplitter(
  int errcode
)
{
  if ((errcode <= -500) && (errcode >= -500 - ERR_COUNT)) {
    return errlist[-(errcode + 500)];
  }
  return strerror(errcode);
}
