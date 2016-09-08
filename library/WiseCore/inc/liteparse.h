/****************************************************************************/
/* Copyright(C) : Advantech Technologies, Inc.								*/
/* Create Date  : 2016/01/22 by Scott Chang								    */
/* Modified Date: 2016/01/22 by Scott Chang									*/
/* Abstract     : Lite Parser API definition								*/
/* Reference    : None														*/
/****************************************************************************/

#ifndef _LITE_PARSE_H_
#define _LITE_PARSE_H_

#include <stdbool.h>

bool lp_value_get(const char* strCmd, const char* strTag, char* value, int length);


#endif //_LITE_PARSE_H_
