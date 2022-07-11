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

# Init scripts
PRODUCT_PACKAGES += \
    init.s9863a1h10.rc

# Input
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/keylayout/gpio-keys.kl:system/usr/keylayout/sprd-gpio-keys.kl

# Soong namespaces
PRODUCT_SOONG_NAMESPACES += \
    $(LOCAL_PATH)
