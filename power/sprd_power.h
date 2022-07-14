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

#ifndef INCLUDE_SPRD_POWER_H
#define INCLUDE_SPRD_POWER_H

#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <linux/time.h>
#include <stdbool.h>
#include <cutils/properties.h>
#include <cutils/compiler.h>
#include <math.h>
#include <sys/prctl.h>
#include <sys/resource.h>

#include <utils/Log.h>

#include <hardware/hardware.h>
#include <hardware/power.h>

#define BOOST_DURATION_DEFAULT               500
#define BOOST_DURATION_MAX                   5000

#define POWER_HINT_ENABLE_PROP               "persist.vendor.power.hint"
#define POWER_HINT_IGNORE_CHARGE             "persist.vendor.power.ign_charge"
#define POWER_HINT_DEBUG_D                   "persist.vendor.power.debug_d"
#define PATH_POWER_HINT_DISABLE              "/vendor/etc/power_hint_disable"

// For POWER_HINT_VIDEO_ENCODE
#define STATE_ON                             "state=1"
#define STATE_OFF                            "state=0"
#define STATE_HDR_ON                         "state=2"
#define STATE_HDR_OFF                        "state=3"

#define POWER_STATE_SUBSYSTEM_NAME_MAX_LENGTH          100
#define LEN_SCENE_NAME_MAX                             40

/**
 * Subsytem-level sleep state stats:
 * power_state_subsystem_sleep_state_t represents the sleep states
 * a subsystem (e.g. wifi, bt) is capable of getting into.
 *
 * SoCs like wifi, bt usually have more than one subsystem level sleep state.
 */
typedef struct {
    /**
     * Subsystem-level Sleep state name.
     */
    char name[POWER_STATE_NAME_MAX_LENGTH];

    /**
     * Time spent in msec at this subsystem-level sleep state since boot.
     */
    uint64_t residency_in_msec_since_boot;

    /**
     * Total number of times sub-system entered this state.
     */
    uint64_t total_transitions;

    /**
     * Timestamp of last entry of this state measured in MSec
     */
    uint64_t last_entry_timestamp_ms;

    /**
     * This subsystem-level sleep state can only be reached during system suspend
     */
    bool supported_only_in_suspend;
} power_state_subsystem_sleep_state_t;

/**
 * Subsytem-level sleep state stats:
 * power_state_subsystem_t represents a subsystem (e.g. wifi, bt)
 * and all the sleep states this susbsystem is capable of getting into.
 *
 * SoCs like wifi, bt usually have more than one subsystem level sleep state.
 */
typedef struct {
    /**
     * Subsystem name (e.g. wifi, bt etc.)
     */
    char name[POWER_STATE_SUBSYSTEM_NAME_MAX_LENGTH];

    /**
     * states represents the list of sleep states supported by this susbsystem.
     * Higher the index in the returned <states> vector deeper the state is
     * i.e. lesser steady-state power is consumed by the subsystem to
     * to be resident in that state.
     *
     */
    power_state_subsystem_sleep_state_t *states;
} power_state_subsystem_t;

struct sprd_power_module {

    /*
     * (*init)() performs power management setup actions at runtime
     * startup, such as to set default cpufreq parameters.  This is
     * called only by the Power HAL instance loaded by
     * PowerManagerService.
     *
     * Platform-level sleep state stats:
     * Can Also be used to initiate device specific Platform-level
     * Sleep state nodes from version 0.5 onwards.
     */
    void (*init)(struct sprd_power_module *module);

    /*
     * (*setInteractive)() performs power management actions upon the
     * system entering interactive state (that is, the system is awake
     * and ready for interaction, often with UI devices such as
     * display and touchscreen enabled) or non-interactive state (the
     * system appears asleep, display usually turned off).  The
     * non-interactive state is usually entered after a period of
     * inactivity, in order to conserve battery power during
     * such inactive periods.
     *
     * Typical actions are to turn on or off devices and adjust
     * cpufreq parameters.  This function may also call the
     * appropriate interfaces to allow the kernel to suspend the
     * system to low-power sleep state when entering non-interactive
     * state, and to disallow low-power suspend when the system is in
     * interactive state.  When low-power suspend state is allowed, the
     * kernel may suspend the system whenever no wakelocks are held.
     *
     * on is non-zero when the system is transitioning to an
     * interactive / awake state, and zero when transitioning to a
     * non-interactive / asleep state.
     *
     * This function is called to enter non-interactive state after
     * turning off the screen (if present), and called to enter
     * interactive state prior to turning on the screen.
     */
    void (*setInteractive)(struct sprd_power_module *module, int on);

    void (*powerHint)(struct sprd_power_module *module, power_hint_t hint,
                      void *data);

    /*
     * (*setFeature) is called to turn on or off a particular feature
     * depending on the state parameter. The possible features are:
     *
     * FEATURE_DOUBLE_TAP_TO_WAKE
     *
     *    Enabling/Disabling this feature will allow/disallow the system
     *    to wake up by tapping the screen twice.
     *
     * availability: version 0.3
     *
     */
    void (*setFeature)(struct sprd_power_module *module, feature_t feature, int state);

    /*
     * Platform-level sleep state stats:
     * Report cumulative info on the statistics on platform-level sleep states since boot.
     *
     * Caller of the function queries the get_number_of_sleep_states and allocates the
     * memory for the power_state_platform_sleep_state_t *list before calling this function.
     *
     * power_stats module is responsible to assign values to all the fields as
     * necessary.
     *
     * Higher the index deeper the state is i.e. lesser steady-state power is consumed
     * by the platform to be resident in that state.
     *
     * The function returns 0 on success or negative value -errno on error.
     * EINVAL - *list is NULL.
     * EIO - filesystem nodes access error.
     *
     * availability: version 0.5
     */
    int (*get_platform_low_power_stats)(struct sprd_power_module *module,
        power_state_platform_sleep_state_t *list);

    /*
     * Platform-level sleep state stats:
     * This function is called to determine the number of platform-level sleep states
     * for get_platform_low_power_stats.
     *
     * The value returned by this function is used to allocate memory for
     * power_state_platform_sleep_state_t *list for get_platform_low_power_stats.
     *
     * The number of parameters must not change for successive calls.
     *
     * Return number of parameters on success or negative value -errno on error.
     * EIO - filesystem nodes access error.
     *
     * availability: version 0.5
     */
    ssize_t (*get_number_of_platform_modes)(struct sprd_power_module *module);

    /*
     * Platform-level sleep state stats:
     * Provides the number of voters for each of the Platform-level sleep state.
     *
     * Caller uses this function to allocate memory for the power_state_voter_t list.
     *
     * Caller has to allocate the space for the *voter array which is
     * get_number_of_platform_modes() long.
     *
     * Return 0 on success or negative value -errno on error.
     * EINVAL - *voter is NULL.
     * EIO - filesystem nodes access error.
     *
     * availability: version 0.5
     */
    int (*get_voter_list)(struct sprd_power_module *module, size_t *voter);

    /*
     * Subsystem-level sleep state stats:
     * Report cumulative info on the statistics on subsystem-level sleep states
     * since boot.
     *
     * @return subsystems supported on this device and their sleep states
     * @return retval SUCCESS on success or FILESYSTEM_ERROR on filesystem
     * nodes access error.
     */
    int (*get_subsystem_low_power_stats)(struct sprd_power_module *module, power_state_subsystem_t *states);

    /*
     * Translate scene name to scene id.
     *
     * @return scene id if found, else 0.
     */
    int (*get_scene_id)(struct sprd_power_module *module, char *name);

    /*
     * (*ctrl_power_hint) is called to disabling/enabling power hint function
     * depending on the enable parameter.
     *
     * @param enable is zero power hint function will be disabled, Otherwise it is to enabled.
     */
    void (*ctrl_power_hint)(struct sprd_power_module *module, int enable);

    pthread_mutex_t lock;

    /* Indicate if has call init() */
    bool init_done;

    /* Is Charging */
    int isCharging;
};

#endif
