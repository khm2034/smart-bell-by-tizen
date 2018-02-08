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

#include <iotcon.h>

#include "log.h"
#include "connectivity.h"
#include "resource.h"
#include "controller_util.h"
#include "webutil.h"
#include "connection_manager.h"

void controller_init_internal_functions(void)
{
	connection_manager_init();
	web_util_noti_init();
	return;
}

void controller_fini_internal_functions(void)
{
	_I("Terminating...");
	resource_close_all();
	web_util_noti_fini();
	connection_manager_fini();
	controller_util_free();

	return;
}
