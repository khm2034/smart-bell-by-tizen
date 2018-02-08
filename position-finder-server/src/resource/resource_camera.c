///*
// * Copyright (c) 2017 Samsung Electronics Co., Ltd.
// *
// * Contact: Jin Yoon <jinny.yoon@samsung.com>
// *          Geunsun Lee <gs86.lee@samsung.com>
// *          Eunyoung Lee <ey928.lee@samsung.com>
// *          Junkyu Han <junkyu.han@samsung.com>
// *
// * Licensed under the Flora License, Version 1.1 (the License);
// * you may not use this file except in compliance with the License.
// * You may obtain a copy of the License at
// *
// * http://floralicense.org/license/
// *
// * Unless required by applicable law or agreed to in writing, software
// * distributed under the License is distributed on an AS IS BASIS,
// * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// * See the License for the specific language governing permissions and
// * limitations under the License.
// */
//
//#include <camera.h>
//#include <glib.h>
//#include <stdio.h>
//#include <stdlib.h>
//#include <tizen.h>
//
//#include "log.h"
//#include "resource/resource_camera.h"
//
//#define RESOLUTION_W 320
//#define RESOLUTION_H 240
//
//static int __init(void);
//static void __completed_cb(void *user_data);
//static bool __resolution_list_cb(int width, int height, void *user_data);
//static void __capturing_cb(camera_image_data_s *image, camera_image_data_s *postview,
//		camera_image_data_s *thumbnail, void *user_data);
//
//struct __camera_data {
//	camera_h cam_handle;
//	int resolution_w;
//	int resolution_h;
//	void *captured_file;
//	unsigned int image_size;
//	capture_completed_cb completed_cb;
//	void *completed_cb_data;
//};
//
//static struct __camera_data *camera_data = NULL;
//
//int resource_capture_camera(capture_completed_cb capture_completed, void *user_data)
//{
//	camera_state_e state;
//	int ret = CAMERA_ERROR_NONE;
//
//	if (camera_data == NULL) {
//		_I("Camera is not initialized");
//		ret = __init();
//		if (ret < 0) {
//			_E("Failed to initialize camera");
//			return -1;
//		}
//	}
//
//	ret = camera_get_state(camera_data->cam_handle, &state);
//	if (ret != CAMERA_ERROR_NONE) {
//		_E("Failed to get camera state");
//		return -1;
//	}
//
//	if (state >= CAMERA_STATE_CAPTURING) {
//		_D("Camera is now capturing");
//		return 0;
//	}
//
//	if (state != CAMERA_STATE_PREVIEW) {
//		_I("Preview is not started");
//		ret = camera_start_preview(camera_data->cam_handle);
//		if (ret != CAMERA_ERROR_NONE) {
//			_E("Failed to start preview");
//			return -1;
//		}
//	}
//
//	ret = camera_start_capture(camera_data->cam_handle, __capturing_cb, __completed_cb, camera_data);
//	if (ret != CAMERA_ERROR_NONE) {
//		_E("Failed to start capturing");
//		return -1;
//	}
//
//	camera_data->completed_cb = capture_completed;
//	camera_data->completed_cb_data = user_data;
//
//	return 0;
//}
//
//void resource_close_camera(void)
//{
//	if (camera_data == NULL)
//		return;
//
//	camera_stop_preview(camera_data->cam_handle);
//
//	camera_destroy(camera_data->cam_handle);
//	camera_data->cam_handle = NULL;
//
//	free(camera_data);
//	camera_data = NULL;
//}
//
//static void __capturing_cb(camera_image_data_s *image, camera_image_data_s *postview,
//		camera_image_data_s *thumbnail, void *user_data)
//{
//	struct __camera_data *camera_data = user_data;
//	if (image == NULL) {
//		_E("Image is NULL");
//		return;
//	}
//
//	camera_data->captured_file = malloc(image->size);
//	if (camera_data->captured_file == NULL)
//		return;
//
//	_D("Now is on Capturing: Image size[%d x %d]", image->width, image->height);
//
//	memcpy(camera_data->captured_file, image->data, image->size);
//	camera_data->image_size = image->size;
//
//	return;
//}
//
//static void __completed_cb(void *user_data)
//{
//	struct __camera_data *camera_data = user_data;
//	int ret = CAMERA_ERROR_NONE;
//
//	if (camera_data->completed_cb)
//		camera_data->completed_cb(camera_data->captured_file, camera_data->image_size, camera_data->completed_cb_data);
//
//	free(camera_data->captured_file);
//	camera_data->captured_file = NULL;
//
//	if (!camera_data->cam_handle) {
//		_E("Camera is NULL");
//		return;
//	}
//	_D("Capture is completed");
//
//	ret = camera_start_preview(camera_data->cam_handle);
//	if (ret != CAMERA_ERROR_NONE) {
//		_E("Failed to start preview");
//		return;
//	}
//
//	ret = camera_stop_preview(camera_data->cam_handle);
//	if (ret != CAMERA_ERROR_NONE) {
//		_E("Failed to stop preview");
//		return;
//	}
//
//	return;
//}
//
//static bool __resolution_list_cb(int width, int height, void *user_data)
//{
//	_D("Supported resolution - Width[%d], Height[%d]", width, height);
//
//	if (width > camera_data->resolution_w && width <= RESOLUTION_W &&
//			height > camera_data->resolution_h && height <= RESOLUTION_H) {
//		camera_data->resolution_w = width;
//		camera_data->resolution_h = height;
//	}
//	_D("Fixed Resolution is Width[%d], Height[%d]", camera_data->resolution_w, camera_data->resolution_h);
//
//	return true;
//}
//
//static int __init(void)
//{
//	int ret = CAMERA_ERROR_NONE;
//
//	camera_data = malloc(sizeof(struct __camera_data));
//	if (camera_data == NULL) {
//		_E("Failed to allocate Camera data");
//		return -1;
//	}
//	memset(camera_data, 0, sizeof(struct __camera_data));
//
//	ret = camera_create(CAMERA_DEVICE_CAMERA0, &(camera_data->cam_handle));
//	if (ret != CAMERA_ERROR_NONE) {
//		_E("Failed to create camera");
//		goto ERROR;
//	}
//
//	ret = camera_foreach_supported_capture_resolution(camera_data->cam_handle, __resolution_list_cb, NULL);
//	if (ret != CAMERA_ERROR_NONE) {
//		_E("Failed to foreach supported capture resolution");
//		goto ERROR;
//	}
//
//	ret = camera_set_preview_resolution(camera_data->cam_handle, camera_data->resolution_w, camera_data->resolution_h);
//	if (ret != CAMERA_ERROR_NONE) {
//		_E("Failed to set preview resolution");
//		goto ERROR;
//	}
//
//	ret = camera_set_capture_resolution(camera_data->cam_handle, camera_data->resolution_w, camera_data->resolution_h);
//	if (ret != CAMERA_ERROR_NONE) {
//		_E("Failed to set capture resolution");
//		goto ERROR;
//	}
//
//	ret = camera_set_capture_format(camera_data->cam_handle, CAMERA_PIXEL_FORMAT_JPEG);
//	if (ret != CAMERA_ERROR_NONE) {
//		_E("Failed to set capture resolution");
//		goto ERROR;
//	}
//
//	ret = camera_start_preview(camera_data->cam_handle);
//	if (ret != CAMERA_ERROR_NONE) {
//		_E("Failed to start preview[%d]", ret);
//		goto ERROR;
//	}
//
//	return 0;
//
//ERROR:
//	camera_destroy(camera_data->cam_handle);
//	free(camera_data);
//	return -1;
//}
//
