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

#include <stdlib.h>
#include <glib.h>
#include <stdio.h>
#include <app_common.h>
#include "log.h"

#define CONF_GROUP_DEFAULT_NAME "default"
#define CONF_KEY_PATH_NAME "path"
#define CONF_KEY_ADDRESS_NAME "address"
#define CONF_KEY_IMAGE_UPLOAD_NAME "image_address"
#define CONF_FILE_NAME "pi.conf"

struct controller_util_s {
	char *path;
	char *address;
	char *image_upload;
};

struct controller_util_s controller_util = { 0, };

static int _read_conf_file(void)
{
	GKeyFile *gkf = NULL;
	char conf_path[PATH_MAX] = {0,};
	char *prefix = NULL;

	gkf = g_key_file_new();
	retv_if(!gkf, -1);

	prefix = app_get_resource_path();
	retv_if(!prefix, -1);
	snprintf(conf_path, sizeof(conf_path)-1, "%s%s", prefix, CONF_FILE_NAME);
	free(prefix);
	prefix = NULL;

	if (!g_key_file_load_from_file(gkf, conf_path, G_KEY_FILE_NONE, NULL)) {
		_E("could not read config file %s", conf_path);
		return -1;
	}

	controller_util.path = g_key_file_get_string(gkf,
			CONF_GROUP_DEFAULT_NAME,
			CONF_KEY_PATH_NAME,
			NULL);
	if (!controller_util.path)
		_E("could not get the key string");

	controller_util.address = g_key_file_get_string(gkf,
			CONF_GROUP_DEFAULT_NAME,
			CONF_KEY_ADDRESS_NAME,
			NULL);
	if (!controller_util.address)
		_E("could not get the key string");

	controller_util.image_upload = g_key_file_get_string(gkf,
			CONF_GROUP_DEFAULT_NAME,
			CONF_KEY_IMAGE_UPLOAD_NAME,
			NULL);
	if (!controller_util.image_upload)
		_E("could not get the key string");

	g_key_file_free(gkf);

	return 0;
}

int controller_util_get_path(const char **path)
{
	retv_if(!path, -1);

	if (!controller_util.path) {
		int ret = -1;
		ret = _read_conf_file();
		retv_if(-1 == ret, -1);
	}

	*path = controller_util.path;

	return 0;
}

int controller_util_get_address(const char **address)
{
	retv_if(!address, -1);

	if (!controller_util.address) {
		int ret = -1;
		ret = _read_conf_file();
		retv_if(-1 == ret, -1);
	}

	*address = controller_util.address;

	return 0;
}

int controller_util_get_image_address(const char **image_upload)
{
	retv_if(!image_upload, -1);

	if (!controller_util.image_upload) {
		int ret = -1;
		ret = _read_conf_file();
		retv_if(-1 == ret, -1);
	}

	*image_upload = controller_util.image_upload;

	return 0;
}

void controller_util_free(void)
{
	if (controller_util.path) {
		free(controller_util.path);
		controller_util.path = NULL;
	}

	if (controller_util.address) {
		free(controller_util.address);
		controller_util.address = NULL;
	}
}
