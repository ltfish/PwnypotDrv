#include "Error.h"

#define PDC_PIPE_PING				'\x01'
#define PDC_PIPE_PONG				'\x02'
#define PDC_PIPE_ADD_PID			'\x03'
#define PDC_PIPE_ADD_PID_RESP		'\x04'
#define PDC_PIPE_REMOVE_PID			'\x05'
#define PDC_PIPE_REMOVE_PID_RESP	'\x06'

STATUS CreatePipeServer();