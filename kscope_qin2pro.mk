#
# Copyright (C) 2022 The KaleidoscopeOS Project
#
# SPDX-License-Identifier: Apache-2.0
#

# Inherit from those products. Most specific first.
$(call inherit-product, $(SRC_TARGET_DIR)/product/aosp_arm64.mk)
$(call inherit-product, $(SRC_TARGET_DIR)/product/aosp_base_telephony.mk)
$(call inherit-product, $(SRC_TARGET_DIR)/product/gsi_release.mk)

# Enable Android Go features.
$(call inherit-product, $(SRC_TARGET_DIR)/product/go_defaults.mk)

# Inherit from qin2pro device.
$(call inherit-product, device/duoqin/qin2pro/device.mk)

# Inherit some common Kaleidoscope stuff.
$(call inherit-product, vendor/kscope/target/product/mobile.mk)

# Device identifier. This must come after all inclusions.
PRODUCT_NAME := kscope_qin2pro
PRODUCT_DEVICE := qin2pro
PRODUCT_BRAND := Qin
PRODUCT_MODEL := Qin 2 Pro
PRODUCT_MANUFACTURER := DuoQin

PRODUCT_AAPT_CONFIG := normal
PRODUCT_AAPT_PREF_CONFIG := xxhdpi
PRODUCT_CHARACTERISTICS := nosdcard

# Boot animation
TARGET_BOOT_ANIMATION_RES := 720

# Face unlock
TARGET_FACE_UNLOCK_SUPPORTED := false

# Build info
PRODUCT_BUILD_PROP_OVERRIDES += \
    PRODUCT_DEVICE=s9863a1h10 \
    PRODUCT_NAME=s9863a1h10_Natv \
    PRIVATE_BUILD_DESC="s9863a1h10_Natv-user 9 PPR1.180610.011 120 release-keys"

BUILD_FINGERPRINT := "Qin/s9863a1h10_Natv/s9863a1h10:9/PPR1.180610.011/120:user/release-keys"
