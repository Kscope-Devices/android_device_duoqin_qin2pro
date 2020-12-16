#
# Copyright (C) 2022 The KaleidoscopeOS Project
#
# SPDX-License-Identifier: Apache-2.0
#

include build/make/target/board/generic_arm64/BoardConfig.mk

BUILD_BROKEN_DUP_RULES := true
BUILD_BROKEN_ELF_PREBUILT_PRODUCT_COPY_FILES := true

DEVICE_PATH := device/duoqin/qin2pro

# Architecture
TARGET_ARCH := arm64
TARGET_ARCH_VARIANT := armv8-a
TARGET_CPU_ABI := arm64-v8a
TARGET_CPU_ABI2 :=
TARGET_CPU_VARIANT := generic

TARGET_2ND_ARCH := arm
TARGET_2ND_ARCH_VARIANT := armv7-a-neon
TARGET_2ND_CPU_ABI := armeabi-v7a
TARGET_2ND_CPU_ABI2 := armeabi
TARGET_2ND_CPU_VARIANT := cortex-a15

# Bootloader
TARGET_BOOTLOADER_BOARD_NAME := sp9863a_1h10
TARGET_NO_BOOTLOADER := true

# Kernel
TARGET_NO_KERNEL := true

# Platform
TARGET_BOARD_PLATFORM := sp9863a

# Properties
TARGET_SYSTEM_PROP += $(DEVICE_PATH)/system.prop

# Bluetooth
BOARD_BLUETOOTH_BDROID_BUILDCFG_INCLUDE_DIR := $(DEVICE_PATH)/bluetooth/include

# SELinux
SYSTEM_EXT_PRIVATE_SEPOLICY_DIRS += $(DEVICE_PATH)/sepolicy/private
