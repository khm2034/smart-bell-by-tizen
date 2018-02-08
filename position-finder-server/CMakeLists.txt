CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(${P_NAME} C)

SET(INSTALL_EXEC_PREFIX "${INSTALL_PREFIX}/bin")
SET(CMAKE_VERBOSE_MAKEFILE 0)

SET(PROJECT_ROOT_DIR "${CMAKE_SOURCE_DIR}")
SET(PROJECT_RESOURCES_DIR "${PROJECT_ROOT_DIR}/res")

INCLUDE(FindPkgConfig)
pkg_check_modules(APP_PKGS REQUIRED
	dlog
	aul
	capi-appfw-application
	capi-appfw-service-application
	capi-system-peripheral-io
	ecore
	eina
	iotcon
	gio-2.0
	libcurl
	glib-2.0
	json-glib-1.0
	capi-system-info
	capi-network-connection
	capi-media-camera
)

ADD_DEFINITIONS(-DCBOR_FILE_IN_RES="${INSTALL_RESDIR}/${CBOR_FILE}")
ADD_DEFINITIONS(-DCBOR_FILE_IN_DATA="${INSTALL_OWNER_DATADIR}/${CBOR_FILE}")
ADD_DEFINITIONS(-DCONF_FILE="${INSTALL_RESDIR}/${CONF_FILE}")

FOREACH (flag ${APP_PKGS_CFLAGS})
    SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${flag}")
ENDFOREACH(flag)

SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} -fvisibility=hidden -Wall -Winline -g -fno-builtin-malloc -fPIE")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_CFLAGS}")
SET(CMAKE_EXE_LINKER_FLAGS "-Wl,--as-needed -pie")

INCLUDE_DIRECTORIES(${PROJECT_ROOT_DIR}/inc)

ADD_EXECUTABLE(${PROJECT_NAME}
	${PROJECT_ROOT_DIR}/src/controller.c
	${PROJECT_ROOT_DIR}/src/controller_internal.c
	${PROJECT_ROOT_DIR}/src/controller_util.c
	${PROJECT_ROOT_DIR}/src/connectivity.c
	${PROJECT_ROOT_DIR}/src/connection_manager.c
	${PROJECT_ROOT_DIR}/src/webutil.c
	${PROJECT_ROOT_DIR}/src/resource.c
	${PROJECT_ROOT_DIR}/src/resource/resource_illuminance_sensor.c
	${PROJECT_ROOT_DIR}/src/resource/resource_infrared_motion_sensor.c
	${PROJECT_ROOT_DIR}/src/resource/resource_infrared_obstacle_avoidance_sensor.c
	${PROJECT_ROOT_DIR}/src/resource/resource_touch_sensor.c
	${PROJECT_ROOT_DIR}/src/resource/resource_ultrasonic_sensor.c
	${PROJECT_ROOT_DIR}/src/resource/resource_led.c
	${PROJECT_ROOT_DIR}/src/resource/resource_vibration_sensor.c
	${PROJECT_ROOT_DIR}/src/resource/resource_flame_sensor.c
	${PROJECT_ROOT_DIR}/src/resource/resource_rain_sensor.c
	${PROJECT_ROOT_DIR}/src/resource/resource_sound_detection_sensor.c
	${PROJECT_ROOT_DIR}/src/resource/resource_tilt_sensor.c
	${PROJECT_ROOT_DIR}/src/resource/resource_gas_detection_sensor.c
	${PROJECT_ROOT_DIR}/src/resource/resource_sound_level_sensor.c
	${PROJECT_ROOT_DIR}/src/resource/resource_adc_mcp3008.c
	${PROJECT_ROOT_DIR}/src/resource/resource_camera.c
)

TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${pkgs_LDFLAGS} -lm)
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${APP_PKGS_LDFLAGS})

Message("APP_LABEL : ${APP_LABEL}")

CONFIGURE_FILE(${PROJECT_ROOT_DIR}/tizen-manifest.xml.in ${ORG_PREFIX}.${PROJECT_NAME}.xml @ONLY)
# Install
INSTALL(TARGETS ${PROJECT_NAME} DESTINATION ${INSTALL_EXEC_PREFIX})
INSTALL(FILES ${ORG_PREFIX}.${PROJECT_NAME}.xml DESTINATION ${SYS_PACKAGES_DIR})
INSTALL(FILES ${PROJECT_ROOT_DIR}/shared/res/default_icon.png DESTINATION ${SYS_ICONS_DIR} RENAME ${PROJECT_NAME}.png)
INSTALL(FILES ${PROJECT_ROOT_DIR}/res/${CBOR_FILE} DESTINATION ${INSTALL_RESDIR})
INSTALL(FILES ${PROJECT_ROOT_DIR}/res/${CONF_FILE} DESTINATION ${INSTALL_RESDIR})

# End of a file
