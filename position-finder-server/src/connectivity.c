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

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <glib.h>
#include <app_common.h>
#include <iotcon.h>

#include "log.h"
#include "connectivity.h"
#include "webutil.h"
#include "controller_util.h"
#include "connection_manager.h"

#define DEFAULT_RESOURCE_TYPE "org.tizen.door"
#define BUFSIZE 1024
#define URI_PATH_LEN 64
#define URI_PATH "/door/1"
#define PATH "path"
#define IOTCON_DB_FILENAME "iotcon-test-svr-db-server.dat"

struct _connectivity_resource {
	char *path;
	char *type;
	char *ip;
	connectivity_protocol_e protocol_type;
	GHashTable *value_hash;
	union {
		struct {
			iotcon_resource_h res;
			iotcon_observers_h observers;
		} iotcon_data;
		struct {
			/* Nothing */
			char *reserve;
		} http_data;
	} conn_data;
};

typedef enum {
	DATA_VAL_TYPE_BOOL = 0,
	DATA_VAL_TYPE_INT,
	DATA_VAL_TYPE_DOUBLE,
	DATA_VAL_TYPE_STRING
} conn_data_val_type_e;

typedef struct _conn_data_value_s {
	conn_data_val_type_e type;
	union {
		bool b_val;
		int i_val;
		double d_val;
		char *s_val;
	};
} conn_data_value_s;

static connectivity_protocol_e ProtocolType = CONNECTIVITY_PROTOCOL_DEFAULT;
static int connectivity_iotcon_intialized = 0;
static int connectivity_http_intialized = 0;

static void _print_iotcon_error(int err_no)
{
	switch (err_no) {
	case IOTCON_ERROR_NOT_SUPPORTED:
		_E("IOTCON_ERROR_NOT_SUPPORTED");
		break;
	case IOTCON_ERROR_PERMISSION_DENIED:
		_E("IOTCON_ERROR_PERMISSION_DENIED");
		break;
	case IOTCON_ERROR_INVALID_PARAMETER:
		_E("IOTCON_ERROR_INVALID_PARAMETER");
		break;
	default:
		_E("Error : [%d]", err_no);
		break;
	}

	return;
}

static void _copy_file(const char *in_filename, const char *out_filename)
{
	char buf[BUFSIZE] = { 0, };
	size_t nread = 0;
	FILE *in = NULL;
	FILE *out = NULL;

	ret_if(!in_filename);
	ret_if(!out_filename);

	in = fopen(in_filename, "r");
	ret_if(!in);

	out = fopen(out_filename, "w");
	goto_if(!out, error);

	rewind(in);
	while ((nread = fread(buf, 1, sizeof(buf), in)) > 0) {
		if (fwrite(buf, 1, nread, out) < nread) {
			_E("critical error to copy a file");
			break;
		}
	}

	fclose(in);
	fclose(out);

	return;

error:
	fclose(in);
	return;
}

static bool _query_cb(const char *key, const char *value, void *user_data)
{
	_D("Key : [%s], Value : [%s]", key, value);

	return IOTCON_FUNC_CONTINUE;
}

static int _handle_query(iotcon_request_h request)
{
	iotcon_query_h query = NULL;
	int ret = -1;

	ret = iotcon_request_get_query(request, &query);
	retv_if(IOTCON_ERROR_NONE != ret, -1);

	if (query) iotcon_query_foreach(query, _query_cb, NULL);

	return 0;
}

static int _handle_request_by_crud_type(iotcon_request_h request, connectivity_resource_s *resource_info)
{
	iotcon_request_type_e type;
	int ret = -1;

	ret = iotcon_request_get_request_type(request, &type);
	retv_if(IOTCON_ERROR_NONE != ret, -1);

	switch (type) {
	case IOTCON_REQUEST_GET:
		_I("Do not support 'get' query");
		break;
	case IOTCON_REQUEST_PUT:
		_I("Do not support 'put' query");
		break;
	case IOTCON_REQUEST_POST:
		_I("Do not support 'post' query");
		break;
	case IOTCON_REQUEST_DELETE:
		_I("Do not support 'delete' query");
		break;
	default:
		_E("Cannot reach here");
		ret = -1;
		break;
	}
	retv_if(0 != ret, -1);

	return 0;
}

static int _handle_observer(iotcon_request_h request, iotcon_observers_h observers)
{
	iotcon_observe_type_e observe_type;
	int ret = -1;
	int observe_id = -1;

	ret = iotcon_request_get_observe_type(request, &observe_type);
	retv_if(IOTCON_ERROR_NONE != ret, -1);

	if (IOTCON_OBSERVE_REGISTER == observe_type) {
		ret = iotcon_request_get_observe_id(request, &observe_id);
		retv_if(IOTCON_ERROR_NONE != ret, -1);

		_I("Add an observer : %d", observe_id);

		ret = iotcon_observers_add(observers, observe_id);
		retv_if(IOTCON_ERROR_NONE != ret, -1);
	} else if (IOTCON_OBSERVE_DEREGISTER == observe_type) {
		ret = iotcon_request_get_observe_id(request, &observe_id);
		retv_if(IOTCON_ERROR_NONE != ret, -1);

		_I("Remove an observer : %d", observe_id);

		ret = iotcon_observers_remove(observers, observe_id);
		retv_if(IOTCON_ERROR_NONE != ret, -1);
	}

	return 0;
}

static int _send_response(iotcon_request_h request, iotcon_representation_h representation, iotcon_response_result_e result)
{
	int ret = -1;
	iotcon_response_h response;

	ret = iotcon_response_create(request, &response);
	retv_if(IOTCON_ERROR_NONE != ret, -1);

	ret = iotcon_response_set_result(response, result);
	goto_if(IOTCON_ERROR_NONE != ret, error);

	ret = iotcon_response_set_representation(response, representation);
	goto_if(IOTCON_ERROR_NONE != ret, error);

	ret = iotcon_response_send(response);
	goto_if(IOTCON_ERROR_NONE != ret, error);

	iotcon_response_destroy(response);

	return 0;

error:
	iotcon_response_destroy(response);
	return -1;
}

static void _request_resource_handler(iotcon_resource_h resource, iotcon_request_h request, void *user_data)
{
	connectivity_resource_s *resource_info = user_data;
	int ret = -1;
	char *host_address = NULL;

	ret_if(!request);

	ret = iotcon_request_get_host_address(request, &host_address);
	goto_if(IOTCON_ERROR_NONE != ret, error);

	_D("Host address : %s", host_address);

	ret = _handle_query(request);
	goto_if(IOTCON_ERROR_NONE != ret, error);

	ret = _handle_request_by_crud_type(request, resource_info);
	goto_if(0 != ret, error);

	ret = _handle_observer(request, resource_info->conn_data.iotcon_data.observers);
	goto_if(0 != ret, error);

	return;

error:
	_send_response(request, NULL, IOTCON_RESPONSE_ERROR);
	return;
}

static int __init_iotcon(connectivity_resource_s *resource_info)
{
	int ret = -1;
	iotcon_resource_types_h resource_types = NULL;
	iotcon_resource_interfaces_h ifaces = NULL;
	uint8_t policies = IOTCON_RESOURCE_NO_POLICY;
	char res_path[PATH_MAX] = {0,};
	char data_path[PATH_MAX] = {0,};
	char *prefix = NULL;

	retv_if(!resource_info, -1);
	retv_if(resource_info->protocol_type != CONNECTIVITY_PROTOCOL_IOTIVITY, -1);

	prefix = app_get_resource_path();
	retv_if(!prefix, -1);
	snprintf(res_path, sizeof(res_path)-1, "%s%s", prefix, IOTCON_DB_FILENAME);
	free(prefix);
	prefix = NULL;

	prefix = app_get_data_path();
	retv_if(!prefix, -1);
	snprintf(data_path, sizeof(data_path)-1, "%s%s", prefix, IOTCON_DB_FILENAME);
	free(prefix);

	_copy_file(res_path, data_path);

	ret = iotcon_initialize(data_path);
	retv_if(IOTCON_ERROR_NONE != ret, -1);

	/* TODO : If we have to set device name, naming it more gorgeous one */
	ret = iotcon_set_device_name(DEFAULT_RESOURCE_TYPE);
	goto_if(IOTCON_ERROR_NONE != ret, error);

	ret = iotcon_resource_types_create(&resource_types);
	goto_if(IOTCON_ERROR_NONE != ret, error);

	ret = iotcon_resource_types_add(resource_types, resource_info->type);
	goto_if(IOTCON_ERROR_NONE != ret, error);

	ret = iotcon_resource_interfaces_create(&ifaces);
	goto_if(IOTCON_ERROR_NONE != ret, error);

	ret = iotcon_resource_interfaces_add(ifaces, IOTCON_INTERFACE_DEFAULT);
	goto_if(IOTCON_ERROR_NONE != ret, error);

	ret = iotcon_resource_interfaces_add(ifaces, IOTCON_INTERFACE_BATCH);
	goto_if(IOTCON_ERROR_NONE != ret, error);

	policies =
		IOTCON_RESOURCE_DISCOVERABLE |
		IOTCON_RESOURCE_OBSERVABLE |
		IOTCON_RESOURCE_SECURE;

	ret = iotcon_resource_create(URI_PATH,
			resource_types,
			ifaces,
			policies,
			_request_resource_handler,
			resource_info,
			&resource_info->conn_data.iotcon_data.res);
	goto_if(IOTCON_ERROR_NONE != ret, error);

	ret = iotcon_observers_create(&resource_info->conn_data.iotcon_data.observers);
	goto_if(IOTCON_ERROR_NONE != ret, error);

	iotcon_resource_types_destroy(resource_types);
	iotcon_resource_interfaces_destroy(ifaces);
	connectivity_iotcon_intialized = 1;

	return 0;

error:
	if (resource_types) iotcon_resource_types_destroy(resource_types);
	if (ifaces) iotcon_resource_interfaces_destroy(ifaces);
	if (resource_info->conn_data.iotcon_data.res) iotcon_resource_destroy(resource_info->conn_data.iotcon_data.res);
	iotcon_deinitialize();
	return -1;
}

static int __init_http(connectivity_resource_s *resource_info)
{
	int ret = 0;
	ret = web_util_noti_init();
	if (!ret)
		connectivity_http_intialized = 1;

	return ret;
}

#ifdef PRINT_DEBUG_DETAIL
static bool __print_attributes_cb(iotcon_attributes_h attributes, const char *key, void *user_data)
{
	iotcon_type_e type = IOTCON_TYPE_NONE;

	iotcon_attributes_get_type(attributes, key, &type);

	switch (type) {
	case IOTCON_TYPE_INT: {
		int value = 0;
		iotcon_attributes_get_int(attributes, key, &value);
		_D("key[%s] - int value [%d]", key, value);
		}
		break;
	case IOTCON_TYPE_BOOL: {
		bool value = 0;
		iotcon_attributes_get_bool(attributes, key, &value);
		_D("key[%s] - bool value [%d]", key, value);
		}
		break;
	case IOTCON_TYPE_DOUBLE: {
		double value = 0;
		iotcon_attributes_get_double(attributes, key, &value);
		_D("key[%s] - double value [%lf]", key, value);
		}
	break;
	case IOTCON_TYPE_STR: {
		char *value = 0;
		iotcon_attributes_get_str(attributes, key, &value);
		_D("key[%s] - string value [%s]", key, value);
		}
	break;
	case IOTCON_TYPE_NONE:
	case IOTCON_TYPE_BYTE_STR:
	case IOTCON_TYPE_NULL:
	case IOTCON_TYPE_LIST:
	case IOTCON_TYPE_ATTRIBUTES:
	default:
		_W("unhandled key[%s] type[%d]", key, type);
		break;
	}

	return IOTCON_FUNC_CONTINUE;
}
#endif

static void __print_attribute(iotcon_attributes_h attributes)
{
#ifdef PRINT_DEBUG_DETAIL
	ret_if(!attributes);

	iotcon_attributes_foreach(attributes, __print_attributes_cb, NULL);
#endif
	return;
}

static void _destroy_representation(iotcon_representation_h representation)
{
	ret_if(!representation);
	iotcon_representation_destroy(representation);
	return;
}

static iotcon_representation_h _create_representation_with_bool(connectivity_resource_s *resource_info, const char *key, bool value)
{
	iotcon_attributes_h attributes = NULL;
	iotcon_representation_h representation = NULL;
	char *uri_path = NULL;
	int ret = -1;

	ret = iotcon_resource_get_uri_path(resource_info->conn_data.iotcon_data.res, &uri_path);
	retv_if(IOTCON_ERROR_NONE != ret, NULL);

	ret = iotcon_representation_create(&representation);
	retv_if(IOTCON_ERROR_NONE != ret, NULL);

	ret = iotcon_attributes_create(&attributes);
	goto_if(IOTCON_ERROR_NONE != ret, error);

	ret = iotcon_representation_set_uri_path(representation, uri_path);
	goto_if(IOTCON_ERROR_NONE != ret, error);

	ret = iotcon_attributes_add_str(attributes, PATH, resource_info->path);
	goto_if(IOTCON_ERROR_NONE != ret, error);

	ret = iotcon_attributes_add_bool(attributes, key, value);
	goto_if(IOTCON_ERROR_NONE != ret, error);

	ret = iotcon_representation_set_attributes(representation, attributes);
	goto_if(IOTCON_ERROR_NONE != ret, error);

	iotcon_attributes_destroy(attributes);

	return representation;

error:
	if (attributes) iotcon_attributes_destroy(attributes);
	if (representation) iotcon_representation_destroy(representation);

	return NULL;
}

static iotcon_representation_h _create_representation_with_int(connectivity_resource_s *resource_info, const char *key, int value)
{
	iotcon_attributes_h attributes = NULL;
	iotcon_representation_h representation = NULL;
	char *uri_path = NULL;
	int ret = -1;

	ret = iotcon_resource_get_uri_path(resource_info->conn_data.iotcon_data.res, &uri_path);
	retv_if(IOTCON_ERROR_NONE != ret, NULL);

	ret = iotcon_representation_create(&representation);
	retv_if(IOTCON_ERROR_NONE != ret, NULL);

	ret = iotcon_attributes_create(&attributes);
	goto_if(IOTCON_ERROR_NONE != ret, error);

	ret = iotcon_representation_set_uri_path(representation, uri_path);
	goto_if(IOTCON_ERROR_NONE != ret, error);

	ret = iotcon_attributes_add_str(attributes, PATH, resource_info->path);
	goto_if(IOTCON_ERROR_NONE != ret, error);

	ret = iotcon_attributes_add_int(attributes, key, value);
	goto_if(IOTCON_ERROR_NONE != ret, error);

	ret = iotcon_representation_set_attributes(representation, attributes);
	goto_if(IOTCON_ERROR_NONE != ret, error);

	iotcon_attributes_destroy(attributes);

	return representation;

error:
	if (attributes) iotcon_attributes_destroy(attributes);
	if (representation) iotcon_representation_destroy(representation);

	return NULL;
}

static iotcon_representation_h _create_representation_with_double(connectivity_resource_s *resource_info, const char *key, double value)
{
	iotcon_attributes_h attributes = NULL;
	iotcon_representation_h representation = NULL;
	char *uri_path = NULL;
	int ret = -1;

	ret = iotcon_resource_get_uri_path(resource_info->conn_data.iotcon_data.res, &uri_path);
	retv_if(IOTCON_ERROR_NONE != ret, NULL);

	ret = iotcon_representation_create(&representation);
	retv_if(IOTCON_ERROR_NONE != ret, NULL);

	ret = iotcon_attributes_create(&attributes);
	goto_if(IOTCON_ERROR_NONE != ret, error);

	ret = iotcon_representation_set_uri_path(representation, uri_path);
	goto_if(IOTCON_ERROR_NONE != ret, error);

	ret = iotcon_attributes_add_str(attributes, PATH, resource_info->path);
	goto_if(IOTCON_ERROR_NONE != ret, error);

	ret = iotcon_attributes_add_double(attributes, key, value);
	goto_if(IOTCON_ERROR_NONE != ret, error);

	ret = iotcon_representation_set_attributes(representation, attributes);
	goto_if(IOTCON_ERROR_NONE != ret, error);

	iotcon_attributes_destroy(attributes);

	return representation;

error:
	if (attributes) iotcon_attributes_destroy(attributes);
	if (representation) iotcon_representation_destroy(representation);

	return NULL;
}

static iotcon_representation_h _create_representation_with_string(connectivity_resource_s *resource_info, const char *key, const char *value)
{
	iotcon_attributes_h attributes = NULL;
	iotcon_representation_h representation = NULL;
	char *uri_path = NULL;
	int ret = -1;

	ret = iotcon_resource_get_uri_path(resource_info->conn_data.iotcon_data.res, &uri_path);
	retv_if(IOTCON_ERROR_NONE != ret, NULL);

	ret = iotcon_representation_create(&representation);
	retv_if(IOTCON_ERROR_NONE != ret, NULL);

	ret = iotcon_attributes_create(&attributes);
	goto_if(IOTCON_ERROR_NONE != ret, error);

	ret = iotcon_representation_set_uri_path(representation, uri_path);
	goto_if(IOTCON_ERROR_NONE != ret, error);

	ret = iotcon_attributes_add_str(attributes, PATH, resource_info->path);
	goto_if(IOTCON_ERROR_NONE != ret, error);

	ret = iotcon_attributes_add_str(attributes, key, (char *)value);
	goto_if(IOTCON_ERROR_NONE != ret, error);

	ret = iotcon_representation_set_attributes(representation, attributes);
	goto_if(IOTCON_ERROR_NONE != ret, error);

	iotcon_attributes_destroy(attributes);

	return representation;

error:
	if (attributes) iotcon_attributes_destroy(attributes);
	if (representation) iotcon_representation_destroy(representation);

	return NULL;
}

static inline void __noti_by_http(void)
{
	char *json_data = NULL;

	json_data = web_util_get_json_string();
	if (json_data) {
		const char *url = NULL;
		controller_util_get_address(&url);
		if (url)
			web_util_noti_post(url, json_data);
		else
			_E("fail to get url");
		free(json_data);
	} else
		_E("fail to get json_data");

	return;
}

int connectivity_notify_bool(connectivity_resource_s *resource_info, const char *key, bool value)
{
	int ret = -1;

	retv_if(!resource_info, -1);
	retv_if(!key, -1);

	_D("Notify key[%s], value[%d]", key, value);

	switch (resource_info->protocol_type) {
	case CONNECTIVITY_PROTOCOL_IOTIVITY:
		retv_if(!resource_info->conn_data.iotcon_data.res, -1);
		retv_if(!resource_info->conn_data.iotcon_data.observers, -1);
		{
			iotcon_representation_h representation;
			representation = _create_representation_with_bool(resource_info, key, value);
			retv_if(!representation, -1);
			ret = iotcon_resource_notify(resource_info->conn_data.iotcon_data.res,
					representation, resource_info->conn_data.iotcon_data.observers, IOTCON_QOS_LOW);
			if (IOTCON_ERROR_NONE != ret) {
				_W("There are some troubles for notifying value[%d]", ret);
				_print_iotcon_error(ret);
				return -1;
			}
			_destroy_representation(representation);
		}
		break;
	case CONNECTIVITY_PROTOCOL_HTTP:
		ret = web_util_json_init();
		retv_if(ret, -1);

		ret = web_util_json_begin();
		retv_if(ret, -1);

		web_util_json_add_string("SensorPiID", resource_info->path);
		web_util_json_add_string("SensorPiType", resource_info->type);
		web_util_json_add_string("SensorPiIP", resource_info->ip);
		web_util_json_add_boolean(key, value);
		web_util_json_end();

		__noti_by_http();

		web_util_json_fini();
		break;
	default:
		_E("Unknown protocol type[%d]", resource_info->protocol_type);
		return -1;
		break;
	}

	return 0;
}

int connectivity_notify_int(connectivity_resource_s *resource_info, const char *key, int value)
{
	int ret = -1;

	retv_if(!resource_info, -1);
	retv_if(!key, -1);

	_D("Notify key[%s], value[%d]", key, value);

	switch (resource_info->protocol_type) {
	case CONNECTIVITY_PROTOCOL_IOTIVITY:
		retv_if(!resource_info->conn_data.iotcon_data.res, -1);
		retv_if(!resource_info->conn_data.iotcon_data.observers, -1);
		{
			iotcon_representation_h representation;
			representation = _create_representation_with_int(resource_info, key, value);
			retv_if(!representation, -1);
			ret = iotcon_resource_notify(resource_info->conn_data.iotcon_data.res,
					representation, resource_info->conn_data.iotcon_data.observers, IOTCON_QOS_LOW);
			if (IOTCON_ERROR_NONE != ret) {
				_W("There are some troubles for notifying value[%d]", ret);
				_print_iotcon_error(ret);
				return -1;
			}
			_destroy_representation(representation);
		}
		break;
	case CONNECTIVITY_PROTOCOL_HTTP:
		ret = web_util_json_init();
		retv_if(ret, -1);

		ret = web_util_json_begin();
		retv_if(ret, -1);

		web_util_json_add_string("SensorPiID", resource_info->path);
		web_util_json_add_string("SensorPiType", resource_info->type);
		web_util_json_add_string("SensorPiIP", resource_info->ip);
		web_util_json_add_int(key, value);
		web_util_json_end();

		__noti_by_http();

		web_util_json_fini();
		break;
	default:
		_E("Unknown protocol type[%d]", resource_info->protocol_type);
		return -1;
		break;
	}
	return 0;
}

int connectivity_notify_double(connectivity_resource_s *resource_info, const char *key, double value)
{
	int ret = -1;

	retv_if(!resource_info, -1);
	retv_if(!key, -1);

	_D("Notify key[%s], value[%lf]", key, value);

	switch (resource_info->protocol_type) {
	case CONNECTIVITY_PROTOCOL_IOTIVITY:
		retv_if(!resource_info->conn_data.iotcon_data.res, -1);
		retv_if(!resource_info->conn_data.iotcon_data.observers, -1);
		{
			iotcon_representation_h representation;
			representation = _create_representation_with_double(resource_info, key, value);
			retv_if(!representation, -1);
			ret = iotcon_resource_notify(resource_info->conn_data.iotcon_data.res,
					representation, resource_info->conn_data.iotcon_data.observers, IOTCON_QOS_LOW);
			if (IOTCON_ERROR_NONE != ret) {
				_W("There are some troubles for notifying value[%d]", ret);
				_print_iotcon_error(ret);
				return -1;
			}
			_destroy_representation(representation);
		}
		break;
	case CONNECTIVITY_PROTOCOL_HTTP:
		ret = web_util_json_init();
		retv_if(ret, -1);

		ret = web_util_json_begin();
		retv_if(ret, -1);

		web_util_json_add_string("SensorPiID", resource_info->path);
		web_util_json_add_string("SensorPiType", resource_info->type);
		web_util_json_add_string("SensorPiIP", resource_info->ip);
		web_util_json_add_double(key, value);
		web_util_json_end();

		__noti_by_http();

		web_util_json_fini();
		break;
	default:
		_E("Unknown protocol type[%d]", resource_info->protocol_type);
		return -1;
		break;
	}
	return 0;
}

int connectivity_notify_string(connectivity_resource_s *resource_info, const char *key, const char *value)
{
	int ret = -1;

	retv_if(!resource_info, -1);
	retv_if(!key, -1);
	retv_if(!value, -1);

	_D("Notify key[%s], value[%s]", key, value);

	switch (resource_info->protocol_type) {
	case CONNECTIVITY_PROTOCOL_IOTIVITY:
		retv_if(!resource_info->conn_data.iotcon_data.res, -1);
		retv_if(!resource_info->conn_data.iotcon_data.observers, -1);
		{
			iotcon_representation_h representation;
			representation = _create_representation_with_string(resource_info, key, value);
			retv_if(!representation, -1);
			ret = iotcon_resource_notify(resource_info->conn_data.iotcon_data.res,
					representation, resource_info->conn_data.iotcon_data.observers, IOTCON_QOS_LOW);
			if (IOTCON_ERROR_NONE != ret) {
				_W("There are some troubles for notifying value[%d]", ret);
				_print_iotcon_error(ret);
				return -1;
			}
			_destroy_representation(representation);
		}
		break;
	case CONNECTIVITY_PROTOCOL_HTTP:
		ret = web_util_json_init();
		retv_if(ret, -1);

		ret = web_util_json_begin();
		retv_if(ret, -1);

		web_util_json_add_string("SensorPiID", resource_info->path);
		web_util_json_add_string("SensorPiType", resource_info->type);
		web_util_json_add_string("SensorPiIP", resource_info->ip);
		web_util_json_add_string(key, value);
		web_util_json_end();

		__noti_by_http();

		web_util_json_fini();
		break;
	default:
		_E("Unknown protocol type[%d]", resource_info->protocol_type);
		return -1;
		break;
	}
	return 0;
}

static void __hash_key_free(gpointer data)
{
	free(data);
	return;
}

static void __hash_value_free(gpointer data)
{
	conn_data_value_s *value = data;
	if (value && (value->type == DATA_VAL_TYPE_STRING))
		free(value->s_val);
	free(value);

	return;
}

static int __add_value_to_hash(connectivity_resource_s *resource_info, const char *key, conn_data_value_s *value)
{
	retv_if(!resource_info, -1);
	retv_if(!key, -1);
	retv_if(!value, -1);

	if (!resource_info->value_hash) {
		resource_info->value_hash = g_hash_table_new_full(g_str_hash, g_str_equal,
										__hash_key_free, __hash_value_free);
		retv_if(!resource_info->value_hash, -1);
	}

	g_hash_table_insert(resource_info->value_hash, strdup(key), value);

	return 0;
}

int connectivity_attributes_add_bool(connectivity_resource_s *resource_info, const char *key, bool value)
{
	int ret = 0;
	conn_data_value_s *data_value = NULL;

	retv_if(!resource_info, -1);
	retv_if(!key, -1);

	_D("adding key[%s] - value[%d]", key, value);

	data_value = malloc(sizeof(conn_data_value_s));
	retv_if(!data_value, -1);

	data_value->type = DATA_VAL_TYPE_BOOL;
	data_value->b_val = value;

	ret = __add_value_to_hash(resource_info, key, data_value);
	if (ret) {
		free(data_value);
		return -1;
	}

	return 0;
}

int connectivity_attributes_add_int(connectivity_resource_s *resource_info, const char *key, int value)
{
	int ret = 0;
	conn_data_value_s *data_value = NULL;

	retv_if(!resource_info, -1);
	retv_if(!key, -1);

	_D("adding key[%s] - value[%d]", key, value);

	data_value = malloc(sizeof(conn_data_value_s));
	retv_if(!data_value, -1);

	data_value->type = DATA_VAL_TYPE_INT;
	data_value->i_val = value;

	ret = __add_value_to_hash(resource_info, key, data_value);
	if (ret) {
		free(data_value);
		return -1;
	}

	return 0;
}

int connectivity_attributes_add_double(connectivity_resource_s *resource_info, const char *key, double value)
{
	int ret = 0;
	conn_data_value_s *data_value = NULL;

	retv_if(!resource_info, -1);
	retv_if(!key, -1);

	_D("adding key[%s] - value[%lf]", key, value);

	data_value = malloc(sizeof(conn_data_value_s));
	retv_if(!data_value, -1);

	data_value->type = DATA_VAL_TYPE_DOUBLE;
	data_value->d_val = value;

	ret = __add_value_to_hash(resource_info, key, data_value);
	if (ret) {
		free(data_value);
		return -1;
	}

	return 0;
}

int connectivity_attributes_add_string(connectivity_resource_s *resource_info, const char *key, const char *value)
{
	int ret = 0;
	conn_data_value_s *data_value = NULL;

	retv_if(!resource_info, -1);
	retv_if(!key, -1);
	retv_if(!value, -1);

	_D("adding key[%s] - value[%s]", key, value);

	data_value = malloc(sizeof(conn_data_value_s));
	retv_if(!data_value, -1);

	data_value->type = DATA_VAL_TYPE_STRING;
	data_value->s_val = strdup(value);

	ret = __add_value_to_hash(resource_info, key, data_value);
	if (ret) {
		free(data_value->s_val);
		free(data_value);
		return -1;
	}

	return 0;
}

int connectivity_attributes_remove_value_by_key(connectivity_resource_s *resource_info, const char *key)
{
	retv_if(!resource_info, -1);
	retv_if(!key, -1);

	if (resource_info->value_hash)
		g_hash_table_remove(resource_info->value_hash, key);

	if (g_hash_table_size(resource_info->value_hash) == 0) {
		g_hash_table_unref(resource_info->value_hash);
		resource_info->value_hash = NULL;
	}

	return 0;
}

int connectivity_attributes_remove_all(connectivity_resource_s *resource_info)
{
	retv_if(!resource_info, -1);

	if (resource_info->value_hash) {
		g_hash_table_destroy(resource_info->value_hash);
		resource_info->value_hash = NULL;
	}

	return 0;
}

static void __json_add_data_iter_cb(gpointer key, gpointer value, gpointer user_data)
{
	char *name = key;
	conn_data_value_s *data = value;
	int ret = 0;

	ret_if(!name);
	ret_if(!data);

	switch (data->type) {
	case DATA_VAL_TYPE_BOOL:
		ret = web_util_json_add_boolean(name, data->b_val);
		if (IOTCON_ERROR_NONE != ret)
			_E("failed to add key[%s], value[%d]", name, data->b_val);
		break;
	case DATA_VAL_TYPE_INT:
		ret = web_util_json_add_int(name, data->i_val);
		if (IOTCON_ERROR_NONE != ret)
			_E("failed to add key[%s], value[%d]", name, data->i_val);
		break;
	case DATA_VAL_TYPE_DOUBLE:
		ret = web_util_json_add_double(name, data->d_val);
		if (IOTCON_ERROR_NONE != ret)
			_E("failed to add key[%s], value[%lf]", name, data->d_val);
		break;
	case DATA_VAL_TYPE_STRING:
		ret = web_util_json_add_string(name, data->s_val);
		if (IOTCON_ERROR_NONE != ret)
			_E("failed to add key[%s], value[%s]", name, data->s_val);
		break;
	default:
		_E("Unknown data type [%d]", data->type);
		break;
	}

	return;
}

static void __attr_add_data_iter_cb(gpointer key, gpointer value, gpointer user_data)
{
	char *name = key;
	conn_data_value_s *data = value;
	iotcon_attributes_h attr = user_data;
	int ret = 0;

	ret_if(!name);
	ret_if(!data);
	ret_if(!attr);

	switch (data->type) {
	case DATA_VAL_TYPE_BOOL:
		ret = iotcon_attributes_add_bool(attr, name, data->b_val);
		if (IOTCON_ERROR_NONE != ret)
			_E("failed to add key[%s], value[%d]", name, data->b_val);
		break;
	case DATA_VAL_TYPE_INT:
		ret = iotcon_attributes_add_int(attr, name, data->i_val);
		if (IOTCON_ERROR_NONE != ret)
			_E("failed to add key[%s], value[%d]", name, data->i_val);
		break;
	case DATA_VAL_TYPE_DOUBLE:
		ret = iotcon_attributes_add_double(attr, name, data->d_val);
		if (IOTCON_ERROR_NONE != ret)
			_E("failed to add key[%s], value[%lf]", name, data->d_val);
		break;
	case DATA_VAL_TYPE_STRING:
		ret = iotcon_attributes_add_str(attr, name, data->s_val);
		if (IOTCON_ERROR_NONE != ret)
			_E("failed to add key[%s], value[%s]", name, data->s_val);
		break;
	default:
		_E("Unknown data type [%d]", data->type);
		break;
	}

	return;
}

static int __create_attributes(connectivity_resource_s *resource_info, iotcon_attributes_h *attributes)
{
	int ret = 0;
	iotcon_attributes_h attr = NULL;

	ret = iotcon_attributes_create(&attr);
	if (IOTCON_ERROR_NONE != ret) {
		_print_iotcon_error(ret);
		return -1;
	}

	ret = iotcon_attributes_add_str(attr, PATH, resource_info->path);
	if (IOTCON_ERROR_NONE != ret) {
		_print_iotcon_error(ret);
		iotcon_attributes_destroy(attr);
		return -1;
	}
	if (resource_info->value_hash)
		g_hash_table_foreach(resource_info->value_hash, __attr_add_data_iter_cb, attr);

	*attributes = attr;

	return 0;
}

int connectivity_attributes_notify_all(connectivity_resource_s *resource_info)
{
	int ret = 0;

	retv_if(!resource_info, -1);

	if (resource_info->value_hash == NULL) {
		_W("You have nothing to notify now");
		return 0;
	}


	switch (resource_info->protocol_type) {
	case CONNECTIVITY_PROTOCOL_IOTIVITY:
		retv_if(!resource_info->conn_data.iotcon_data.res, -1);
		retv_if(!resource_info->conn_data.iotcon_data.observers, -1);
		{
			char *uri_path = NULL;
			iotcon_representation_h representation = NULL;
			iotcon_attributes_h attributes = NULL;
			int iotcon_ret = 0;

			iotcon_ret = iotcon_resource_get_uri_path(resource_info->conn_data.iotcon_data.res, &uri_path);
			retv_if(IOTCON_ERROR_NONE != iotcon_ret, -1);

			iotcon_ret = iotcon_representation_create(&representation);
			retv_if(IOTCON_ERROR_NONE != iotcon_ret, -1);

			iotcon_ret = iotcon_representation_set_uri_path(representation, uri_path);
			if (IOTCON_ERROR_NONE != iotcon_ret) {
				_print_iotcon_error(iotcon_ret);
				ret = -1;
			}

			iotcon_ret = __create_attributes(resource_info, &attributes);
			if (iotcon_ret) {
				_E("failed to create attributes");
				ret = -1;
			}
			__print_attribute(attributes);

			iotcon_ret = iotcon_representation_set_attributes(representation, attributes);
			if (IOTCON_ERROR_NONE != iotcon_ret) {
				_print_iotcon_error(iotcon_ret);
				ret = -1;
			}

			iotcon_ret = iotcon_resource_notify(resource_info->conn_data.iotcon_data.res, representation,
					resource_info->conn_data.iotcon_data.observers, IOTCON_QOS_LOW);
			if (IOTCON_ERROR_NONE != iotcon_ret) {
				_print_iotcon_error(iotcon_ret);
				ret = -1;
			}

			iotcon_representation_destroy(representation);
			iotcon_attributes_destroy(attributes);
		}
		break;
	case CONNECTIVITY_PROTOCOL_HTTP:
		ret = web_util_json_init();
		retv_if(ret, -1);

		ret = web_util_json_begin();
		retv_if(ret, -1);

		web_util_json_add_string("SensorPiID", resource_info->path);
		web_util_json_add_string("SensorPiType", resource_info->type);
		web_util_json_add_string("SensorPiIP", resource_info->ip);
		g_hash_table_foreach(resource_info->value_hash, __json_add_data_iter_cb, NULL);
		web_util_json_end();

		__noti_by_http();

		web_util_json_fini();
		break;
	default:
		break;
	}

	g_hash_table_destroy(resource_info->value_hash);
	resource_info->value_hash = NULL;

	return ret;
}

void connectivity_unset_resource(connectivity_resource_s *resource_info)
{
	ret_if(!resource_info);

	switch (resource_info->protocol_type) {
	case CONNECTIVITY_PROTOCOL_IOTIVITY:
		if (resource_info->conn_data.iotcon_data.observers) iotcon_observers_destroy(resource_info->conn_data.iotcon_data.observers);
		if (resource_info->conn_data.iotcon_data.res) iotcon_resource_destroy(resource_info->conn_data.iotcon_data.res);
		iotcon_deinitialize();
		connectivity_iotcon_intialized = 0;
		break;
	case CONNECTIVITY_PROTOCOL_HTTP:
		web_util_noti_fini();
		connectivity_http_intialized = 0;
		break;
	default:
		break;
	}

	if (resource_info->path) free(resource_info->path);
	if (resource_info->type) free(resource_info->type);
	if (resource_info->ip) free(resource_info->ip);
	free(resource_info);

	return;
}

int connectivity_set_resource(const char *path, const char *type, connectivity_resource_s **out_resource_info)
{
	connectivity_resource_s *resource_info = NULL;
	const char *ip = NULL;
	int ret = -1;

	retv_if(!path, -1);
	retv_if(!type, -1);
	retv_if(!out_resource_info, -1);

	resource_info = calloc(1, sizeof(connectivity_resource_s));
	retv_if(!resource_info, -1);

	resource_info->path = strdup(path);
	goto_if(!resource_info->path, error);

	resource_info->type = strdup(type);
	goto_if(!resource_info->type, error);

	connection_manager_get_ip(&ip);
	resource_info->ip = strdup(ip);
	goto_if(!resource_info->ip, error);

	resource_info->protocol_type = ProtocolType;

	_D("Path[%s], Type[%s], protocol_type[%d]" , resource_info->path, resource_info->type, resource_info->protocol_type);

	switch (resource_info->protocol_type) {
	case CONNECTIVITY_PROTOCOL_DEFAULT:
		_D("default protocol type is iotivity\n \
			 To set connectivity use connectivity_set_protocol_type() function");
		resource_info->protocol_type = CONNECTIVITY_PROTOCOL_IOTIVITY;
	case CONNECTIVITY_PROTOCOL_IOTIVITY:
		ret = __init_iotcon(resource_info);
		goto_if(ret, error);
		break;
	case CONNECTIVITY_PROTOCOL_HTTP:
		ret = __init_http(resource_info);
		goto_if(ret, error);
		break;
	default:
		goto error;
		break;
	}

	*out_resource_info = resource_info;

	return 0;

error:
	if (resource_info->path) free(resource_info->path);
	if (resource_info->type) free(resource_info->type);
	if (resource_info->ip) free(resource_info->ip);
	if (resource_info) free(resource_info);

	return -1;
}

int connectivity_set_protocol(connectivity_protocol_e protocol_type)
{
	int ret = 0;
	retv_if(protocol_type >= CONNECTIVITY_PROTOCOL_MAX, -1);

	switch (protocol_type) {
	case CONNECTIVITY_PROTOCOL_DEFAULT:
	case CONNECTIVITY_PROTOCOL_IOTIVITY:
		if (connectivity_iotcon_intialized) {
			_E("protocol type[%d] aleady initialized", protocol_type);
			return -1;
		}
		ProtocolType = CONNECTIVITY_PROTOCOL_IOTIVITY;
		break;
	case CONNECTIVITY_PROTOCOL_HTTP:
		if (connectivity_http_intialized) {
			_E("protocol type[%d] aleady initialized", protocol_type);
			return -1;
		}
		ProtocolType = CONNECTIVITY_PROTOCOL_HTTP;
		break;
	default:
		_E("unknown protocol type[%d]", protocol_type);
		return -1;
		break;
	}

	return ret;
}
