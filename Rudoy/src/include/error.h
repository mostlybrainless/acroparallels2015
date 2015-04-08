///#include "../log/log.h"

#pragma once

#define RETURN_ERROR( msg, ret_value)\
do { ALOGF( msg); return ret_value;} while (0)

