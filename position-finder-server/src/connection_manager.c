 /*
 * Copyright (c) 2017 Samsung Electronics Co., Ltd.
 *
 * Contact: Jin Yoon <jinny.yoon@samsung.com>
 *          Geunsun Lee <gs86.lee@samsung.com>
 *          Eunyoung Lee <ey928.lee@samsung.com>
 *          Junkyu Han <junkyu.han@samsung.com>
 *          Jeonghoon Park <jh1979.park@samsung.com>
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

#include <net_connection.h>
#include <stdlib.h>
#include <string.h>

#include "log.h"

struct conn_mgr_s {
	connection_h connection;
	connection_type_e net_state;
	connection_wifi_state_e wifi_state;
	char *ip_addr;
};

struct conn_mgr_s conn_mgr = { 0, };

static const char *__connection_error_to_string(connection_error_e error)
{
	switch (error) {
	case CONNECTION_ERROR_NONE:
		return "CONNECTION_ERROR_NONE";
	case CONNECTION_ERROR_INVALID_PARAMETER:
		return "CONNECTION_ERROR_INVALID_PARAMETER";
	case CONNECTION_ERROR_OUT_OF_MEMORY:
		return "CONNECTION_ERROR_OUT_OF_MEMORY";
	case CONNECTION_ERROR_INVALID_OPERATION:
		return "CONNECTION_ERROR_INVALID_OPERATION";
	case CONNECTION_ERROR_ADDRESS_FAMILY_NOT_SUPPORTED:
		return "CONNECTION_ERROR_ADDRESS_FAMILY_NOT_SUPPORTED";
	case CONNECTION_ERROR_OPERATION_FAILED:
		return "CONNECTION_ERROR_OPERATION_FAILED";
	case CONNECTION_ERROR_ITERATOR_END:
		return "CONNECTION_ERROR_ITERATOR_END";
	case CONNECTION_ERROR_NO_CONNECTION:
		return "CONNECTION_ERROR_NO_CONNECTION";
	case CONNECTION_ERROR_NOW_IN_PROGRESS:
		return "CONNECTION_ERROR_NOW_IN_PROGRESS";
	case CONNECTION_ERROR_ALREADY_EXISTS:
		return "CONNECTION_ERROR_ALREADY_EXISTS";
	case CONNECTION_ERROR_OPERATION_ABORTED:
		return "CONNECTION_ERROR_OPERATION_ABORTED";
	case CONNECTION_ERROR_DHCP_FAILED:
		return "CONNECTION_ERROR_DHCP_FAILED";
	case CONNECTION_ERROR_INVALID_KEY:
		return "CONNECTION_ERROR_INVALID_KEY";
	case CONNECTION_ERROR_NO_REPLY:
		return "CONNECTION_ERROR_NO_REPLY";
	case CONNECTION_ERROR_PERMISSION_DENIED:
		return "CONNECTION_ERROR_PERMISSION_DENIED";
	case CONNECTION_ERROR_NOT_SUPPORTED:
		return "CONNECTION_ERROR_NOT_SUPPORTED";
	default:
		return "CONNECTION_ERROR_UNKNOWN";
	}
}

static void __conn_mgr_ip_changed_cb(const char* ipv4_address,
		const char* ipv6_address, void* user_data)
{
	_D("ip changed from[%s] to[%s]", conn_mgr.ip_addr, ipv4_address);

	if (conn_mgr.ip_addr) {
		free(conn_mgr.ip_addr);
		conn_mgr.ip_addr = strdup(ipv4_address);
	}
	return;
}

static void __conn_mgr_connection_changed_cb(connection_type_e type, void* user_data)
{
	int ret = CONNECTION_ERROR_NONE;
	_D("connection changed from[%d] to[%d]", conn_mgr.net_state, type);

	conn_mgr.net_state = type;
	if (conn_mgr.net_state != CONNECTION_TYPE_WIFI) {
		ret = connection_get_wifi_state(conn_mgr.connection, &conn_mgr.wifi_state);
		if (CONNECTION_ERROR_NONE != ret)
			_E("fail connection_get_wifi_state - [%s]",
				__connection_error_to_string(ret));
		else
			_D("net_state[%d] - wifi_state[%d]",
				conn_mgr.net_state, conn_mgr.wifi_state);
	}

	return;
}

int connection_manager_get_ip(const char **ip)
{
	int ret = CONNECTION_ERROR_NONE;

	retv_if(conn_mgr.connection == NULL, -1);
	retv_if(ip == NULL, -1);

	if (conn_mgr.ip_addr) {
		*ip = conn_mgr.ip_addr;
		return 0;
	}

	if (conn_mgr.net_state == CONNECTION_TYPE_DISCONNECTED) {
		_W("disconnected now");
		if (conn_mgr.ip_addr) {
			free(conn_mgr.ip_addr);
			conn_mgr.ip_addr = NULL;
		}
		return -1;
	}

	ret = connection_get_ip_address(conn_mgr.connection,
			CONNECTION_ADDRESS_FAMILY_IPV4, &conn_mgr.ip_addr);
	if ((CONNECTION_ERROR_NONE != ret) || (conn_mgr.ip_addr == NULL)) {
		_E("fail to connection_get_ip_address() - [%s]",
			__connection_error_to_string(ret));
		return -1;
	}

	return 0;
}

int connection_manager_init(void)
{
	int ret = CONNECTION_ERROR_NONE;
	if (conn_mgr.connection) {
		_W("connection manager is already initialized");
		return 0;
	}

	ret = connection_create(&conn_mgr.connection);
	if (CONNECTION_ERROR_NONE != ret) {
		_E("fail to create connection - [%s]",
			__connection_error_to_string(ret));
		return -1;
	}

	ret = connection_get_type(conn_mgr.connection, &conn_mgr.net_state);
	if (CONNECTION_ERROR_NONE != ret)
		_E("fail connection_get_type - [%s]",
			__connection_error_to_string(ret));

	if (conn_mgr.net_state != CONNECTION_TYPE_DISCONNECTED) {
		ret = connection_get_ip_address(conn_mgr.connection,
			CONNECTION_ADDRESS_FAMILY_IPV4, &conn_mgr.ip_addr);
		if ((CONNECTION_ERROR_NONE != ret) || (conn_mgr.ip_addr == NULL))
			_E("fail to connection_get_ip_address() - [%s]",
				__connection_error_to_string(ret));
	}

	ret = connection_set_type_changed_cb(conn_mgr.connection,
			__conn_mgr_connection_changed_cb, NULL);
	if (CONNECTION_ERROR_NONE != ret)
		_E("fail connection_set_type_changed_cb - [%s]",
			__connection_error_to_string(ret));

	ret = connection_get_wifi_state(conn_mgr.connection, &conn_mgr.wifi_state);
	if (CONNECTION_ERROR_NONE != ret)
		_E("fail connection_get_wifi_state - [%s]",
			__connection_error_to_string(ret));

	ret = connection_set_ip_address_changed_cb(conn_mgr.connection,
			__conn_mgr_ip_changed_cb, NULL);
	if (CONNECTION_ERROR_NONE != ret)
		_E("fail toconnection_set_ip_address_changed_cb - [%s]",
			__connection_error_to_string(ret));

	_D("net_state[%d], wifi_state[%d], ip address[%s]",
		conn_mgr.net_state, conn_mgr.wifi_state, conn_mgr.ip_addr);

	return 0;
}

int connection_manager_fini(void)
{
	if (conn_mgr.connection) {
		int ret = 0;
		ret = connection_destroy(conn_mgr.connection);
		_D("connection_destroy - [%s]", __connection_error_to_string(ret));
		conn_mgr.connection = NULL;
	}

	if (conn_mgr.ip_addr) {
		free(conn_mgr.ip_addr);
		conn_mgr.ip_addr = NULL;
	}
	return 0;
}
