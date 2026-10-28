#ifndef DEVICE_H
#define DEVICE_H

/* temp 骨架最小身份信息：仅保留 USB 枚举字符串所需宏（usbd_enum.h 引用） */

#define DEVICE_LOGO "JCID"
#define DEVICE_NAME "GD32F450"

#define DEVICE_MANUF       DEVICE_LOGO
#define DEVICE_PRODUCT     DEVICE_MANUF"_"DEVICE_NAME
#define DEVICE_SERIAL      DEVICE_PRODUCT"_"

#endif /* DEVICE_H */
