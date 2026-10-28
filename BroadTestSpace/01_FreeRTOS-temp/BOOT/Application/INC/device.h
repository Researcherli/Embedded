#ifndef DEVICE_H
#define DEVICE_H


#define DEVICE_RUN_STATE            "BOOT"

#define DEVICE_SOFT_VERSION         "1.00"
#define DEVICE_BOOT_VERSION         "1.00"
#define DEVICE_SAFE_SOFT_VERSION    "1.00"
#define DEVICE_HARD_VERSION         "1.00"
#define DEVICE_LOGO                 "JCID"
#define DEVICE_NAME                 "GD32F450"

#define JCID_DEVICE_LOGO            DEVICE_LOGO
#define JCID_DEVICE_TYPE            JCID_DEVICE_LOGO"_"DEVICE_NAME
#define JCID_DEVICE_VERS            JCID_DEVICE_TYPE"_Version"DEVICE_SOFT_VERSION
#define JCID_DEVICE_BOOT_VERS       JCID_DEVICE_TYPE"_Version"DEVICE_BOOT_VERSION

#define JCXX_IDENTITY               "JC_identity"
#define JCXX_USER                   "JC_User"
#define JCXX_BOOT                   "JC_boot"
#define JCXX_VERSION                "JC_version"
#define JCID_DEVICE_MID             "00160000"
#define JCXX_REBOOT                 "JC_reset"
#define TRANS_UPDATE                "update_jcxx"
#define TRANS_ACK                   "ack_jcxx"
#define TRANS_FAIL                  "fail_jcxx"
#define JCXX_CPU_ID         	    "jcxxstid"

#define JC_TO_ESP32                 "JC_to_esp32:"
#define JC_FROM_ESP32               "JC_from_esp32:"

#define COMMAND_OK          		"JC_OK"
#define JC_CV_UPLOAD_OPEN	        "JC_cv_upload_open"
#define JC_CV_UPLOAD_CLOSE	        "JC_cv_upload_close"

#define JC_BOOT_UPGRADE_START       "JC_boot_upgrade_start"
#define JC_BOOT_UPGRADE_END         "JC_boot_upgrade_end"
#define JC_BOOT_UPGRADE_OK          "JC_boot_ok"
#define JC_BOOT_UPGRADE_FAIL        "JC_boot_fail"

#define JC_BAUD_START               "{Baud:"
#define JC_BAUD_END                 "}"

#define JC_VOLTAGE                  "jc_bus_voltage"
#define JC_RESIS                    "jc_bus_resis"
#define JC_USART                    "jc_bus_usart"
#define JC_CLOSE_USART              "jc_bus_close_usart"

#endif // DEVICE_H
