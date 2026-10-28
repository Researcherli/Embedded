#ifndef _VALDEFINE_H_
#define _VALDEFINE_H_

#ifdef  OS_GLOBALS
#define OS_EXT
#else
#define OS_EXT extern
#endif

#include <stdint.h>
#include <stdbool.h>

#ifndef NEW_UI
#define NEW_UI
#endif

#define IIC_RD_NUM 4

#define TICK_VALUE_5MS		(5)
#define TICK_VALUE_10MS     (10)
#define TICK_VALUE_20MS     (20)
#define TICK_VALUE_30MS     (30)
#define TICK_VALUE_50MS     (50)
#define TICK_VALUE_100MS    (100)
#define TICK_VALUE_200MS    (200)
#define TICK_VALUE_500MS    (500)
#define TICK_VALUE_1S  		(1000)
#define TICK_VALUE_10S  	(10000)

#define P32_ADDR(X)                 ((uint32_t *)(X))
#define P32_ADDR_GET(X)             (*P32_ADDR(X))
#define P16_ADDR(X)                 ((uint16_t *)(X))
#define P16_ADDR_GET(X)             (*P16_ADDR(X))
#define P8_ADDR(X)                  ((uint8_t *)(X))
#define P8_ADDR_GET(X)              (*P8_ADDR(X))

#define BYTE_1ST_DATA(__DATA__)     ((uint32_t)((__DATA__) << 24U))
#define BYTE_2ND_DATA(__DATA__)     ((uint32_t)((__DATA__) << 16U))
#define BYTE_3RD_DATA(__DATA__)     ((uint32_t)((__DATA__) <<  8U))
#define BYTE_4TH_DATA(__DATA__)     ((uint32_t)((__DATA__) <<  0U))

#define DATA_1ST_BYTE(__DATA__)     ((uint8_t)((__DATA__) >> 24U))
#define DATA_2ND_BYTE(__DATA__)     ((uint8_t)((__DATA__) >> 16U))
#define DATA_3RD_BYTE(__DATA__)     ((uint8_t)((__DATA__) >>  8U))
#define DATA_4TH_BYTE(__DATA__)     ((uint8_t)((__DATA__) >>  0U))

#define countof(Obj) (sizeof(Obj)/sizeof(Obj[0]))

typedef enum {
	MAIN_PAGE,
	MULTI_METER_PAGE,
	CURRENT_CURVE_PAGE,
	CAN_BUS_PAGE,
	LIN_BUS_PAGE,
	SETTINGS_PAGE
} PageType;

typedef enum {
	DEEP_THEME,
	LIGHT_THEME,
	THEME_NUM
} ThemeStyleIdx;

typedef enum {
	BOOTUP_COLOR,
	TITLE_BAR_BK_COLOR,
	TITLE_BAR_BATTERY_COLOR,
	MAIN_BK_COLOR,
	MAIN_TEXT_COLOR,
	MAIN_BTN_SELECTED_BK_COLOR,
	MAIN_BTN_SELECTED_TEXT_COLOR,

	BUTTON_BK_COLOR,
	SELECTED_BUTTON_BK_COLOR,
	RECORD_TEXT_COLOR,
	SELECTED_BTN_TEXT_COLOR,
	BTN_TEXT_COLOR,

	// 电流曲线界面
	VOLTAGE_WIN_COLOR,
	CURRENT_WIN_COLOR,
	POWER_WIN_COLOR,
	CURVE_WIN_COLOR,
	GRID_COLOR,

	// 设置
	MENU_BK_COLOR,
	MENU_TEXT_COLOR,
	MENU_SEL_TEXT_COLOR,
	WIFI_STATE_OPEN_COLOR,
	WIFI_STATE_CLOSE_COLOR,

	COLOR_NUMS
} ColorIdx;

typedef enum {
	ZH_CN,
	EN_US,
	LANG_NUM
} LangIdx;

typedef enum {
	// 主界面
	MULTIMETER_TEXT,
	CURRENT_CURVE_TEXT,
	CAN_BUS_TEXT,
	LIN_BUS_TEXT,
	SETTINGS_TEXT, 

	// 万用表界面
	MEASUREMENT_MODE_TEXT,
	RECORD_MODE_TEXT,
	RECORD_MODE_HIT_TEXT,
	RECORD_TEXT,
	MULTIMETER_TIP_TEXT,

	// 电流曲线界面
	VOLTAGE_TEXT,
	CURRENT_TEXT,
	POWER_TEXT,

	// 设置
	WIFI_SETTING_TEXT,
	OTA_UPGRADE_TEXT,
	THEME_STYLE_TEXT,
	LANG_SETTING_TEXT,
	VOICE_SETTING_TEXT,
	BKLIGHT_SETTING_TEXT,
	DEVICE_INFO_TEXT,
	FACTORY_RESET_TEXT,

	// 设置 - 二级菜单标题
	WIFI_TITLE_TEXT,
	OTA_TITLE_TEXT,
	THEME_TITLE_TEXT,
	LANG_TITLE_TEXT,
	VOICE_TITLE_TEXT,
	BKLIGHT_TITLE_TEXT,
	DEVICE_TITLE_TEXT,
	FACTORY_TITLE_TEXT,

	// 通用操作/状态词汇
	UPGRADE_TEXT,
	CANCEL_TEXT,
	CONFIRM_TEXT,
	DOWNLOAD_TEXT,
	CHECK_UPGRADE_TEXT,
	OPENED_TEXT,
	CLOSED_TEXT,
	SHORT_CIRCUIT_TEXT,

	// 主题与语言选项
	LIGHT_TEXT,
	DEEP_TEXT,
	ZH_TEXT,
	EN_TEXT,

	// 设备详情
	DEVICE_NAME_TEXT,
	SOFTWARE_VERSION_TEXT,
	SERIAL_NO_TEXT,

	// WIFI 详情与设置
	WIFI_STATE_TEXT,
	WIFI_SETTING_TEXT2,
	WIFI_SETTING_TEXT3,
	WIFI_CONNECTING_TEXT,
	SIGNAL_STRENGTH_TEXT,
	WIFI_PASSWORD_TEXT,
	OPEN_STA_TEXT,
	CLOSE_STA_TEXT,
	OPEN_AP_TEXT,
	CLOSE_AP_TEXT,
	WIFI_SETTING_TIP,

	// OTA 升级详情
	SERVER_APP_VERSION_TEXT,
	LOCAL_APP_VERSION_TEXT,
	LATEST_VERSION_TIP_TEXT,

	LANG_TEXT_NUMS
} LangTextIdx;

typedef enum {
	MEASUREMENT_MODE,
	RECORD_MODE
} MultimeterMode;

typedef enum {
	VOLTAGE_STATE,
	DIODE_STATE
} MultimeterSTATE;

typedef enum {
	BTN_ID_LEFT,    // 左侧按键
	BTN_ID_MID,     // 中间按键
	BTN_ID_RIGHT    // 右侧按键
} ButtonID_t;

OS_EXT uint16_t recordDataSwitchDelay;
OS_EXT bool recordDataSwitchFlag;

OS_EXT bool upload_flag;
OS_EXT bool wifi_upload_flag;

OS_EXT uint8_t sramBuff[0x8000];
OS_EXT uint32_t sramBuffLen;

OS_EXT uint8_t rebootDelay;
OS_EXT bool rebootFlag;


#ifdef OS_GLOBALS
PageType pageType = MAIN_PAGE;
ThemeStyleIdx themeStyleIdx = DEEP_THEME;
LangIdx langIdx = ZH_CN;
uint16_t requestVersionDelay;
bool requestVersionFlag = false;
uint16_t downloadAppDelay;
bool downloadAppFlag = false;
uint16_t saveDelay;
bool saveFlag = false;
bool bootup_flag = false;
uint8_t hh = 0;
uint8_t mm = 0;
uint8_t ss = 0;
float temperature = 0;
uint8_t soundLevel = 3;
uint16_t sound = 24;
uint8_t bkLevel = 5;
uint16_t bkLight = 400;
char device_buffer[50];
char id_buffer[32];
volatile bool g_bLeftBtnPressed = false;	// 左键
volatile bool g_bMidBtnPressed = false;		// 中键
volatile bool g_bRightBtnPressed = false;	// 右键
volatile bool g_bAboveBtnPressed = false;	// 上键
volatile bool g_bPenBtnPressed = false;		// 表笔键
MultimeterMode multimeterMode = MEASUREMENT_MODE;
MultimeterSTATE multimeterState = VOLTAGE_STATE;
uint8_t requireUpdate = 0; // 0: 需要检查更新   1: 需要更新   2: 已经是最新程序
#else
extern PageType pageType;
extern ThemeStyleIdx themeStyleIdx;
extern LangIdx langIdx;
extern uint16_t requestVersionDelay;
extern bool requestVersionFlag;
extern uint16_t downloadAppDelay;
extern bool downloadAppFlag;
extern uint16_t saveDelay;
extern bool saveFlag;
extern bool bootup_flag;
extern uint8_t hh;
extern uint8_t mm;
extern uint8_t ss;
extern float temperature;
extern uint8_t soundLevel;
extern uint16_t sound;
extern uint8_t bkLevel;
extern uint16_t bkLight;
extern char device_buffer[50];
extern char id_buffer[32];
extern volatile bool g_bLeftBtnPressed;
extern volatile bool g_bMidBtnPressed;
extern volatile bool g_bRightBtnPressed; // 全局标志位
extern volatile bool g_bAboveBtnPressed;
extern volatile bool g_bPenBtnPressed;
extern MultimeterMode multimeterMode;
extern MultimeterSTATE multimeterState;
extern uint8_t requireUpdate;
#endif

#endif // _VALDEFINE_H_
