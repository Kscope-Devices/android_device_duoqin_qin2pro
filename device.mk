#
# Copyright (C) 2022 The KaleidoscopeOS Project
#
# SPDX-License-Identifier: Apache-2.0
#

# Force disable updating of APEXes
PRODUCT_PRODUCT_PROPERTIES += \
    ro.apex.updatable=false

# Overlays
PRODUCT_PACKAGES += \
    CarrierConfigOverlayQin2Pro \
    FrameworkResOverlayQin2Pro

# IMS
PRODUCT_PACKAGES += \
    android.hidl.manager-V1.0-java \
    ImsHelper

PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/interfaces.xml:system/etc/permissions/interfaces.xml

# Init scripts
PRODUCT_PACKAGES += \
    ueventd.s9863a1h10.rc \
    init.s9863a1h10.rc

# Input
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/keylayout/gpio-keys.kl:system/usr/keylayout/sprd-gpio-keys.kl

# Light
PRODUCT_PACKAGES += \
    lights.sp9863a

# Permissions
PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/android.hardware.telephony.gsm.xml:system/etc/permissions/android.hardware.telephony.gsm.xml \
    frameworks/native/data/etc/android.hardware.telephony.ims.xml:system/etc/permissions/android.hardware.telephony.ims.xml \
    frameworks/native/data/etc/android.software.secure_lock_screen.xml:system/etc/permissions/android.software.secure_lock_screen.xml

# PowerHAL
PRODUCT_PACKAGES += \
    power.sprd

POWERHINT_PRODUCT_CONFIG := sharkl3

# Soong namespaces
PRODUCT_SOONG_NAMESPACES += \
    $(LOCAL_PATH)
