/*
 * Copyright (C) 2012 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_TAG "PowerHAL"

#include "sprd_power.h"
#include "utils.h"
#include "common.h"
#include "hint_id.h"

extern int scene_name_to_scene_id(char *scene_name);
extern struct sprd_power_module power_impl;

static int power_hint_enable = 1;

static bool is_in_interactive = false;
static bool sceenoff_configed = false;
static bool boost_for_benchmark = false;

// System_server ready get prop and only one time
static bool has_get_prop = false;
int DEBUG_D = 0;

// if Screenoff boost when Charging
static int is_screenoff_ign_charge = 0;

static void power_set_interactive(struct sprd_power_module __unused *module, int on)
{
    struct sprd_power_module *pm = (struct sprd_power_module *)module;

    // get prop
    if (CC_UNLIKELY(!has_get_prop)) {
        power_hint_enable = property_get_int32(POWER_HINT_ENABLE_PROP, 1);
        is_screenoff_ign_charge = property_get_int32(POWER_HINT_IGNORE_CHARGE, 0);
        DEBUG_D = property_get_int32(POWER_HINT_DEBUG_D, 0);
        has_get_prop = true;
    }

    if (CC_UNLIKELY(power_hint_enable == 0)) return;

    if (CC_UNLIKELY(!pm->init_done)) {
        ALOGE("%s: power hint is not inited", __func__);
        return;
    }

    ENTER("%d", on);

    if (is_in_interactive != !!on) {
        is_in_interactive = !!on;

        pthread_mutex_lock(&pm->lock);
        if (power_mode == POWER_HINT_VENDOR_MODE_NORMAL) {
            if (is_in_interactive)  {
                boost(POWER_HINT_VENDOR_SCREEN_ON_PULSE, 0, 1, BOOST_DURATION_DEFAULT);
            } else {
                boost(POWER_HINT_VENDOR_SCREEN_OFF_PULSE, 0, 1, BOOST_DURATION_DEFAULT);
            }

            if (is_in_interactive && sceenoff_configed) {
                boost(POWER_HINT_VENDOR_SCREEN_OFF, 0, 0, 0);
                sceenoff_configed = false;
            } else if (!is_in_interactive) {
                if (is_screenoff_ign_charge || !(pm->isCharging)) {
                    boost(POWER_HINT_VENDOR_SCREEN_OFF, 0, 1, 0);
                    sceenoff_configed = true;
                }
            }
        } else if (is_in_interactive) {
            boost(POWER_HINT_VENDOR_SCREEN_OFF, 0, 0, 0);
            usleep(60000);
            boost(POWER_HINT_VENDOR_SCREEN_ON, 0, 1, 0);
        } else {
            boost(POWER_HINT_VENDOR_SCREEN_ON, 0, 0, 0);
            usleep(60000);
            boost(POWER_HINT_VENDOR_SCREEN_OFF, 0, 1, 0);
        }
        pthread_mutex_unlock(&pm->lock);
    }
    EXIT("%d", on);
}

static void handle_power_mode_switch(int mode, int enable, int value)
{
    int ret = -1;

    if (CC_UNLIKELY((power_mode == mode && enable == 1) || (power_mode != mode && enable == 0)))
        return;

    if (enable) {
        ALOGD("switch mode: 0x%08x -> 0x%08x", power_mode, mode);
    } else {
        ALOGD("switch mode: 0x%08x -> 0x%08x", power_mode, POWER_HINT_VENDOR_MODE_NORMAL);
    }

    if (update_mode(mode, enable)) {
        if (power_mode == POWER_HINT_VENDOR_MODE_NORMAL)
            sceenoff_configed = false;

        if (is_in_interactive) {
            boost(POWER_HINT_VENDOR_SCREEN_ON, 0, 1, 0);
        } else {
            if (power_mode == POWER_HINT_VENDOR_MODE_NORMAL) {
                if (is_screenoff_ign_charge || !(power_impl.isCharging)) {
                    boost(POWER_HINT_VENDOR_SCREEN_OFF, 0, 1, 0);
                    sceenoff_configed = true;
                }
            } else {
                boost(POWER_HINT_VENDOR_SCREEN_OFF, 0, 1, 0);
            }
        }
    }
}

static void sprd_power_hint(struct sprd_power_module *module, power_hint_t hint,
                             void *data)
{
    struct sprd_power_module *pm = (struct sprd_power_module *)module;
    static bool is_launching = false;

    if (CC_UNLIKELY(power_hint_enable == 0)) return;

    ALOGD_IF(DEBUG_V, "Enter %s:(%d:%d)", __func__, hint, ((data!=NULL)?(*(int*)data):0));
    pthread_mutex_lock(&pm->lock);
    if (CC_UNLIKELY(!pm->init_done)) {
        pthread_mutex_unlock(&pm->lock);
        ALOGE("%s: power hint is not inited", __func__);
        return;
    }

#if 0
    // TODO: delete to support boost for all modes
    // Do not support boost in non-normal mode
    if ((power_mode != POWER_HINT_VENDOR_MODE_NORMAL)
        && (hint < POWER_HINT_VENDOR_MODE_NORMAL || hint > POWER_HINT_VENDOR_SCREEN_ON)) {
        pthread_mutex_unlock(&pm->lock);
        return;
    }
#endif

    switch (hint) {
        case POWER_HINT_INTERACTION:
        {
            /*
             * 23       15          0
             * +---------+----------+
             * |  subtype|  duration|
             * +---------+----------+
             */
            int duration = 0;

            if (data != NULL) {
                duration = *((int*)data) & 0xffff;
            }

            if (duration < BOOST_DURATION_DEFAULT || duration > BOOST_DURATION_MAX)
                duration = BOOST_DURATION_DEFAULT;

            boost(POWER_HINT_INTERACTION, 0, 1, duration);
            break;
        }
        case POWER_HINT_LAUNCH:
        {
            bool launch = (data != NULL)? true: false;

            if (launch) {
                if (!is_in_interactive) {
                    ALOGD_IF(DEBUG_V, "Screenoff don't do LAUNCH boost!!!");
                    break;
                }
            }

            if (is_launching == launch) break;

            is_launching = launch;
            boost(POWER_HINT_LAUNCH, 0, (is_launching? 1: 0), 0);
            break;
        }
        case POWER_HINT_VSYNC:
            break;
        case POWER_HINT_SUSTAINED_PERFORMANCE:
            break;
        case POWER_HINT_VR_MODE:
            break;
        case POWER_HINT_VIDEO_DECODE:
            break;
        case POWER_HINT_VIDEO_ENCODE:
            if (data != NULL) {
                int state =  *((int*)data) & 0xffff;
                ALOGE("POWER_HINT_VIDEO_ENCODE: %d", state);

                if (state == 1) {
                    /* Video encode started */
                    boost(hint, 0, 1, 0);
                } else if (state == 0) {
                    /* Video encode stopped */
                    boost(hint, 0, 0, 0);
                }
            }
            break;
        case POWER_HINT_LOW_POWER:
        case POWER_HINT_VENDOR_MODE_NORMAL:
        case POWER_HINT_VENDOR_MODE_LOW_POWER:
        case POWER_HINT_VENDOR_MODE_POWER_SAVE:
        case POWER_HINT_VENDOR_MODE_ULTRA_POWER_SAVE:
        case POWER_HINT_VENDOR_MODE_PERFORMANCE:
            if (((power_mode == hint) && (data != NULL)) || ((power_mode != hint) && (data == NULL)))
                break;

            handle_power_mode_switch(hint, (data != NULL)? 1: 0, 0);
            break;
        default: {
            int duration = 0;

            if (data != NULL) {
                duration = *((int*)data) & 0xffff;
                if (duration == 1 || duration == 0) {
                    duration = 0;
                } else if (duration < 0) {
                    break;
                } else if (duration < BOOST_DURATION_DEFAULT || duration > BOOST_DURATION_MAX) {
                    duration = BOOST_DURATION_DEFAULT;
                }
            }

            boost(hint, 0, ((data != NULL)? 1: 0), duration);
            break;
        }

    }

    pthread_mutex_unlock(&pm->lock);
    ALOGD_IF(DEBUG_V, "Exit %s:(%d:%d)", __func__, hint, ((data!=NULL)?(*(int*)data):0));
}

static int get_scene_id(struct sprd_power_module *module, char *scene_name)
{
    struct sprd_power_module *pm = (struct sprd_power_module *)module;

    ALOGD_IF(DEBUG_V, "Enter %s: scene_name:%s",  __func__, scene_name);
    if (CC_UNLIKELY(power_hint_enable == 0) || scene_name == NULL)
        return 0;

    pthread_mutex_lock(&pm->lock);
    if (CC_UNLIKELY(!pm->init_done)) {
        pthread_mutex_unlock(&pm->lock);
        ALOGE("%s: PowerHAL is not inited", __func__);
        return 0;
    }
    pthread_mutex_unlock(&pm->lock);

    return scene_name_to_scene_id(scene_name);
}

static void ctrl_power_hint(struct sprd_power_module *module, int enable) {
    struct sprd_power_module *pm = (struct sprd_power_module *)module;

    pthread_mutex_lock(&pm->lock);

    if (power_hint_enable != enable) {
        power_hint_enable = enable;
    } else {
        pthread_mutex_unlock(&pm->lock);
        return;
    }

    if (enable == 0) {
        clear_requests_for_all_file();
        ALOGD("%s: Power Hint disable!", __func__);
    } else {
        ALOGD("%s: Power Hint enable!", __func__);
    }

    pthread_mutex_unlock(&pm->lock);
}

static void power_init(struct sprd_power_module __unused *module) {

    struct sprd_power_module *pm = (struct sprd_power_module *)module;

    ALOGD_IF(DEBUG_V, "Delete get prop");
    //power_hint_enable = property_get_int32(POWER_HINT_ENABLE_PROP, 1);
    if (access(PATH_POWER_HINT_DISABLE, F_OK) == 0) {
        power_hint_enable = 0;
    }
    if (CC_UNLIKELY(power_hint_enable == 0)) return;

    if (CC_UNLIKELY(pm == NULL || pm->init_done)) return;

    pthread_mutex_lock(&pm->lock);
    // Read config file
    if (config_read() == 0) {
        pthread_mutex_unlock(&pm->lock);
        return;
    }

    // Must at the bottom
    start_thread_for_timing_request(module);
    pm->init_done = true;
    pthread_mutex_unlock(&pm->lock);
}

struct sprd_power_module power_impl = {
    .init = power_init,
    .setInteractive = power_set_interactive,
    .powerHint = sprd_power_hint,
    .get_scene_id = get_scene_id,
    .ctrl_power_hint = ctrl_power_hint,

    .init_done = false,
    .lock = PTHREAD_MUTEX_INITIALIZER,
    .isCharging = 0,
};
