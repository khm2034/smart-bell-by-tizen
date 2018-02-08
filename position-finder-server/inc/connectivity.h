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

#ifndef __POSITION_FINDER_CONNECTIVITY_H__
#define __POSITION_FINDER_CONNECTIVITY_H__

typedef struct _connectivity_resource connectivity_resource_s;

typedef enum {
	CONNECTIVITY_PROTOCOL_DEFAULT = 0, /* default protocol */
	CONNECTIVITY_PROTOCOL_IOTIVITY, /* IoTvity protocol */
	CONNECTIVITY_PROTOCOL_HTTP, /* HTTP protocol */
	CONNECTIVITY_PROTOCOL_MAX
} connectivity_protocol_e;

/**
 * @brief Set connectivity protocol to communicate with other devices.
 * @param[in] protocol_type protocol type to use
 * @return 0 on success, otherwise a negative error value
 * @see You should set protocol before call connectivity_set_resource(),
 * otherwise IoTvitiy protocol will be set as default.
 */
extern int connectivity_set_protocol(connectivity_protocol_e protocol_type);

/**
 * @brief Create connectivity resource.
 * @param[in] path The path of the resource
 * @param[in] type The string data to insert into the resource types (e.g. "org.tizen.light")
 * @param[out] out_resource_info A structure containing information about connectivity resource
 * @return 0 on success, otherwise a negative error value
 * @see uri_path length must be less than 128.
 * @see You must destroy resource by calling connectivity_unset_resource() if resource is no longer needed.
 */
extern int connectivity_set_resource(const char *path, const char *type, connectivity_resource_s **out_resource_info);

/**
 * @brief Releases all resource about connectivity.
 * @param[in] resource_info A structure containing information about connectivity resource
 */
extern void connectivity_unset_resource(connectivity_resource_s *resource);

/**
 * @brief Notifies a resource's value to observed devices or clouds.
 * @param[in] resource_info A structure containing information about connectivity resource
 * @param[in] key A key to be sended.
 * @param[in] value A value to be sended.
 * @return 0 on success, otherwise a negative error value
 * @see If key is already exists, current value will be replaced with new value.
 */
extern int connectivity_notify_bool(connectivity_resource_s *resource_info, const char *key, bool value);

/**
 * @brief Notifies a resource's value to observed devices or clouds.
 * @param[in] resource_info A structure containing information about connectivity resource
 * @param[in] key A key to be sended.
 * @param[in] value A value to be sended.
 * @return 0 on success, otherwise a negative error value
 * @see If key is already exists, current value will be replaced with new value.
 */
extern int connectivity_notify_int(connectivity_resource_s *resource_info, const char *key, int value);

/**
 * @brief Notifies a resource's value to observed devices or clouds.
 * @param[in] resource_info A structure containing information about connectivity resource
 * @param[in] key A key to be sended.
 * @param[in] value A value to be sended.
 * @return 0 on success, otherwise a negative error value
 * @see If key is already exists, current value will be replaced with new value.
 */
extern int connectivity_notify_double(connectivity_resource_s *resource_info, const char *key, double value);

/**
 * @brief Notifies a resource's value to observed devices or clouds.
 * @param[in] resource_info A structure containing information about connectivity resource
 * @param[in] key A key to be sended.
 * @param[in] value A value to be sended.
 * @return 0 on success, otherwise a negative error value
 * @see If key is already exists, current value will be replaced with new value.
 */
extern int connectivity_notify_string(connectivity_resource_s *resource_info, const char *key, const char *value);

/* TODO : add comments for these functions */
/**
 * @brief Add a boolean type value to attributes for notifying to observed devices or clouds.
 * @param[in] resource_info A structure containing information about connectivity resource
 * @param[in] key A key to be sended.
 * @param[in] value A value to be added.
 * @return 0 on success, otherwise a negative error value
 * @see If key is already exists, current value will be replaced with new value.
 */
extern int connectivity_attributes_add_bool(connectivity_resource_s *resource_info, const char *key, bool value);

/**
 * @brief Add a integer type value to attributes for notifying to observed devices or clouds.
 * @param[in] resource_info A structure containing information about connectivity resource
 * @param[in] key A key to be sended.
 * @param[in] value A value to be added.
 * @return 0 on success, otherwise a negative error value
 * @see If key is already exists, current value will be replaced with new value.
 */
extern int connectivity_attributes_add_int(connectivity_resource_s *resource_info, const char *key, int value);

/**
 * @brief Add a double type value to attributes for notifying to observed devices or clouds.
 * @param[in] resource_info A structure containing information about connectivity resource
 * @param[in] key A key to be sended.
 * @param[in] value A value to be added.
 * @return 0 on success, otherwise a negative error value
 * @see If key is already exists, current value will be replaced with new value.
 */
extern int connectivity_attributes_add_double(connectivity_resource_s *resource_info, const char *key, double value);

/**
 * @brief Add a string value to attributes for notifying to observed devices or clouds.
 * @param[in] resource_info A structure containing information about connectivity resource
 * @param[in] key A key to be sended.
 * @param[in] value A value to be added.
 * @return 0 on success, otherwise a negative error value
 * @see If key is already exists, current value will be replaced with new value.
 */
extern int connectivity_attributes_add_string(connectivity_resource_s *resource_info, const char *key, const char *value);

/**
 * @brief Notifies values in the attributs to observed devices or clouds.
 * @param[in] resource_info A structure containing information about connectivity resource
 * @return 0 on success, otherwise a negative error value
 */
extern int connectivity_attributes_notify_all(connectivity_resource_s *resource_info);

/**
 * @brief Remove a value from attributes for notifying to observed devices or clouds.
 * @param[in] resource_info A structure containing information about connectivity resource
 * @param[in] key A key to be sended.
 * @return 0 on success, otherwise a negative error value
 * @see If key is already exists, current value will be replaced with new value.
 */
extern int connectivity_attributes_remove_value_by_key(connectivity_resource_s *resource_info, const char *key);

/**
 * @brief Remove all values from attributes for notifying to observed devices or clouds.
 * @param[in] resource_info A structure containing information about connectivity resource
 * @return 0 on success, otherwise a negative error value
 */
extern int connectivity_attributes_remove_all(connectivity_resource_s *resource_info);

#endif /* __POSITION_FINDER_CONNECTIVITY_H__ */
