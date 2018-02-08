/*
 * Copyright (c) 2017 Samsung Electronics Co., Ltd.
 *
 * Contact: Jin Yoon <jinny.yoon@samsung.com>
 *          Geunsun Lee <gs86.lee@samsung.com>
 *          Eunyoung Lee <ey928.lee@samsung.com>
 *          Junkyu Han <junkyu.han@samsung.com>
 *
 * Licensed under the Flora License, Version 1.1 (the License);
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://floralicense.org/license/
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __POSITION_FINDER_SERVER_H__
#define __POSITION_FINDER_SERVER_H__

#include <dlog.h>

#ifdef  LOG_TAG
#undef  LOG_TAG
#endif
#define LOG_TAG "POSITION_FINDER_SERVER"

#if !defined(_D)
#define _D(fmt, arg...) dlog_print(DLOG_DEBUG, LOG_TAG, "[%s:%d] " fmt "\n", __func__, __LINE__, ##arg)
#endif

#if !defined(_I)
#define _I(fmt, arg...) dlog_print(DLOG_INFO, LOG_TAG, "[%s:%d] " fmt "\n", __func__, __LINE__, ##arg)
#endif

#if !defined(_W)
#define _W(fmt, arg...) dlog_print(DLOG_WARN, LOG_TAG, "[%s:%d] " fmt "\n", __func__, __LINE__, ##arg)
#endif

#if !defined(_E)
#define _E(fmt, arg...) dlog_print(DLOG_ERROR, LOG_TAG, "[%s:%d] " fmt "\n", __func__, __LINE__, ##arg)
#endif

#define retvm_if(expr, val, fmt, arg...) do { \
	if (expr) { \
		_E(fmt, ##arg); \
		_E("(%s) -> %s() return", #expr, __FUNCTION__); \
		return val; \
	} \
} while (0)

#define retv_if(expr, val) do { \
	if (expr) { \
		_E("(%s) -> %s() return", #expr, __FUNCTION__); \
		return (val); \
	} \
} while (0)

#define retm_if(expr, fmt, arg...) do { \
	if (expr) { \
		_E(fmt, ##arg); \
		_E("(%s) -> %s() return", #expr, __FUNCTION__); \
		return; \
	} \
} while (0)

#define ret_if(expr) do { \
	if (expr) { \
		_E("(%s) -> %s() return", #expr, __FUNCTION__); \
		return; \
	} \
} while (0)

#define goto_if(expr, val) do { \
	if (expr) { \
		_E("(%s) -> goto", #expr); \
		goto val; \
	} \
} while (0)

#define break_if(expr) { \
	if (expr) { \
		_E("(%s) -> break", #expr); \
		break; \
	} \
}

#define continue_if(expr) { \
	if (expr) { \
		_E("(%s) -> continue", #expr); \
		continue; \
	} \
}



#endif /* __POSITION_FINDER_SERVER_H__ */
