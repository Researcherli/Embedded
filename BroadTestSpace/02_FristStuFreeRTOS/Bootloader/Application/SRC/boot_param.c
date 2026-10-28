#include "boot_param.h"
#include "bsp_flash.h"
#include <string.h>

/* CRC 覆盖范围：从 magic 起，直到 param_crc32 字段之前的所有字节 */
#define PARAM_CRC_SPAN  (sizeof(boot_param_t) - sizeof(uint32_t))

/**
 * @brief CRC32 查表 (反射式，多项式 0xEDB88320)，结果与 zlib crc32 一致
 * @note  表由多项式逐位展开生成，标准向量 crc32("123456789") = 0xCBF43926 已验证。
 *        相对逐位运算，每字节从 8 次迭代降到 1 次查表；校验 768KB 镜像时差距明显。
 */
static const uint32_t g_crc32_table[256] = {
    0x00000000U, 0x77073096U, 0xEE0E612CU, 0x990951BAU,
    0x076DC419U, 0x706AF48FU, 0xE963A535U, 0x9E6495A3U,
    0x0EDB8832U, 0x79DCB8A4U, 0xE0D5E91EU, 0x97D2D988U,
    0x09B64C2BU, 0x7EB17CBDU, 0xE7B82D07U, 0x90BF1D91U,
    0x1DB71064U, 0x6AB020F2U, 0xF3B97148U, 0x84BE41DEU,
    0x1ADAD47DU, 0x6DDDE4EBU, 0xF4D4B551U, 0x83D385C7U,
    0x136C9856U, 0x646BA8C0U, 0xFD62F97AU, 0x8A65C9ECU,
    0x14015C4FU, 0x63066CD9U, 0xFA0F3D63U, 0x8D080DF5U,
    0x3B6E20C8U, 0x4C69105EU, 0xD56041E4U, 0xA2677172U,
    0x3C03E4D1U, 0x4B04D447U, 0xD20D85FDU, 0xA50AB56BU,
    0x35B5A8FAU, 0x42B2986CU, 0xDBBBC9D6U, 0xACBCF940U,
    0x32D86CE3U, 0x45DF5C75U, 0xDCD60DCFU, 0xABD13D59U,
    0x26D930ACU, 0x51DE003AU, 0xC8D75180U, 0xBFD06116U,
    0x21B4F4B5U, 0x56B3C423U, 0xCFBA9599U, 0xB8BDA50FU,
    0x2802B89EU, 0x5F058808U, 0xC60CD9B2U, 0xB10BE924U,
    0x2F6F7C87U, 0x58684C11U, 0xC1611DABU, 0xB6662D3DU,
    0x76DC4190U, 0x01DB7106U, 0x98D220BCU, 0xEFD5102AU,
    0x71B18589U, 0x06B6B51FU, 0x9FBFE4A5U, 0xE8B8D433U,
    0x7807C9A2U, 0x0F00F934U, 0x9609A88EU, 0xE10E9818U,
    0x7F6A0DBBU, 0x086D3D2DU, 0x91646C97U, 0xE6635C01U,
    0x6B6B51F4U, 0x1C6C6162U, 0x856530D8U, 0xF262004EU,
    0x6C0695EDU, 0x1B01A57BU, 0x8208F4C1U, 0xF50FC457U,
    0x65B0D9C6U, 0x12B7E950U, 0x8BBEB8EAU, 0xFCB9887CU,
    0x62DD1DDFU, 0x15DA2D49U, 0x8CD37CF3U, 0xFBD44C65U,
    0x4DB26158U, 0x3AB551CEU, 0xA3BC0074U, 0xD4BB30E2U,
    0x4ADFA541U, 0x3DD895D7U, 0xA4D1C46DU, 0xD3D6F4FBU,
    0x4369E96AU, 0x346ED9FCU, 0xAD678846U, 0xDA60B8D0U,
    0x44042D73U, 0x33031DE5U, 0xAA0A4C5FU, 0xDD0D7CC9U,
    0x5005713CU, 0x270241AAU, 0xBE0B1010U, 0xC90C2086U,
    0x5768B525U, 0x206F85B3U, 0xB966D409U, 0xCE61E49FU,
    0x5EDEF90EU, 0x29D9C998U, 0xB0D09822U, 0xC7D7A8B4U,
    0x59B33D17U, 0x2EB40D81U, 0xB7BD5C3BU, 0xC0BA6CADU,
    0xEDB88320U, 0x9ABFB3B6U, 0x03B6E20CU, 0x74B1D29AU,
    0xEAD54739U, 0x9DD277AFU, 0x04DB2615U, 0x73DC1683U,
    0xE3630B12U, 0x94643B84U, 0x0D6D6A3EU, 0x7A6A5AA8U,
    0xE40ECF0BU, 0x9309FF9DU, 0x0A00AE27U, 0x7D079EB1U,
    0xF00F9344U, 0x8708A3D2U, 0x1E01F268U, 0x6906C2FEU,
    0xF762575DU, 0x806567CBU, 0x196C3671U, 0x6E6B06E7U,
    0xFED41B76U, 0x89D32BE0U, 0x10DA7A5AU, 0x67DD4ACCU,
    0xF9B9DF6FU, 0x8EBEEFF9U, 0x17B7BE43U, 0x60B08ED5U,
    0xD6D6A3E8U, 0xA1D1937EU, 0x38D8C2C4U, 0x4FDFF252U,
    0xD1BB67F1U, 0xA6BC5767U, 0x3FB506DDU, 0x48B2364BU,
    0xD80D2BDAU, 0xAF0A1B4CU, 0x36034AF6U, 0x41047A60U,
    0xDF60EFC3U, 0xA867DF55U, 0x316E8EEFU, 0x4669BE79U,
    0xCB61B38CU, 0xBC66831AU, 0x256FD2A0U, 0x5268E236U,
    0xCC0C7795U, 0xBB0B4703U, 0x220216B9U, 0x5505262FU,
    0xC5BA3BBEU, 0xB2BD0B28U, 0x2BB45A92U, 0x5CB36A04U,
    0xC2D7FFA7U, 0xB5D0CF31U, 0x2CD99E8BU, 0x5BDEAE1DU,
    0x9B64C2B0U, 0xEC63F226U, 0x756AA39CU, 0x026D930AU,
    0x9C0906A9U, 0xEB0E363FU, 0x72076785U, 0x05005713U,
    0x95BF4A82U, 0xE2B87A14U, 0x7BB12BAEU, 0x0CB61B38U,
    0x92D28E9BU, 0xE5D5BE0DU, 0x7CDCEFB7U, 0x0BDBDF21U,
    0x86D3D2D4U, 0xF1D4E242U, 0x68DDB3F8U, 0x1FDA836EU,
    0x81BE16CDU, 0xF6B9265BU, 0x6FB077E1U, 0x18B74777U,
    0x88085AE6U, 0xFF0F6A70U, 0x66063BCAU, 0x11010B5CU,
    0x8F659EFFU, 0xF862AE69U, 0x616BFFD3U, 0x166CCF45U,
    0xA00AE278U, 0xD70DD2EEU, 0x4E048354U, 0x3903B3C2U,
    0xA7672661U, 0xD06016F7U, 0x4969474DU, 0x3E6E77DBU,
    0xAED16A4AU, 0xD9D65ADCU, 0x40DF0B66U, 0x37D83BF0U,
    0xA9BCAE53U, 0xDEBB9EC5U, 0x47B2CF7FU, 0x30B5FFE9U,
    0xBDBDF21CU, 0xCABAC28AU, 0x53B39330U, 0x24B4A3A6U,
    0xBAD03605U, 0xCDD70693U, 0x54DE5729U, 0x23D967BFU,
    0xB3667A2EU, 0xC4614AB8U, 0x5D681B02U, 0x2A6F2B94U,
    0xB40BBE37U, 0xC30C8EA1U, 0x5A05DF1BU, 0x2D02EF8DU,
};

/**
 * @brief CRC32 增量计算：可分段喂入，用于校验无法一次性缓冲的固件镜像
 * @param crc    上一次的返回值；首次调用传 0xFFFFFFFFU
 * @param data   本段数据起始地址
 * @param length 本段字节数
 * @return 中间结果；全部喂完后取反才得到最终 CRC
 */
static uint32_t crc32_update(uint32_t crc, const uint8_t *data, uint32_t length)
{
    while (length--) {
        crc = (crc >> 8) ^ g_crc32_table[(crc ^ *data++) & 0xFFU];
    }
    return crc;
}

/**
 * @brief 一次性计算 CRC32
 */
static uint32_t soft_crc32(const uint8_t *data, uint32_t length)
{
    return ~crc32_update(0xFFFFFFFFU, data, length);
}

/**
 * @brief 计算并更新结构体自身的 CRC32
 */
static void boot_param_update_crc(boot_param_t *param)
{
    param->param_crc32 = soft_crc32((const uint8_t *)param, PARAM_CRC_SPAN);
}

/**
 * @brief 校验单份参数是否有效 (魔数 + CRC 双重判断)
 */
static bool boot_param_is_valid(const boot_param_t *param)
{
    if (param->magic != BOOT_PARAM_MAGIC) {
        return false;
    }
    return (soft_crc32((const uint8_t *)param, PARAM_CRC_SPAN) == param->param_crc32);
}

/**
 * @brief 序列号比较：按 RFC1982 序列号算术判断 a 是否比 b 更新
 * @note  直接用 uint32 大小比较会在 0xFFFFFFFF -> 0 回绕时选错副本，
 *        这里用有符号差值规避（二补码平台上的标准做法）
 */
static bool boot_param_seq_is_newer(uint32_t a, uint32_t b)
{
    return ((int32_t)(a - b) > 0);
}

/* 单个日志区的扫描结果 */
typedef struct {
    int32_t  newest_slot;   /* 本区最新有效槽索引，-1 表示本区无有效数据 */
    uint32_t newest_seq;    /* 对应的 seq_num */
    int32_t  free_slot;     /* 第一个空槽索引，-1 表示本区已写满 */
} param_region_info_t;

static uint32_t param_region_base(uint32_t region)
{
    return (region == 0U) ? PARAM_REGION_A_ADDR : PARAM_REGION_B_ADDR;
}

static const boot_param_t *param_slot_ptr(uint32_t region, uint32_t slot)
{
    uint32_t addr = param_region_base(region) + (slot * PARAM_SLOT_SIZE);
    return (const boot_param_t *)(uintptr_t)addr;
}

/**
 * @brief 扫描单个日志区
 * @note  追加式日志保证：遇到第一个空槽（首字为全 0xFF）之后不会再有有效数据，
 *        因此可以提前结束扫描，避免每次启动都把 128 个槽全读一遍
 */
static void param_region_scan(uint32_t region, param_region_info_t *info)
{
    info->newest_slot = -1;
    info->newest_seq  = 0U;
    info->free_slot   = -1;

    for (uint32_t i = 0U; i < PARAM_SLOTS_PER_REGION; i++) {
        const boot_param_t *p = param_slot_ptr(region, i);

        if (p->magic == 0xFFFFFFFFU) {
            info->free_slot = (int32_t)i;
            break;
        }
        if (boot_param_is_valid(p)) {
            if ((info->newest_slot < 0) ||
                boot_param_seq_is_newer(p->seq_num, info->newest_seq)) {
                info->newest_slot = (int32_t)i;
                info->newest_seq  = p->seq_num;
            }
        }
    }
}

/**
 * @brief 从两个日志区中读取最新的一份有效参数
 */
bool boot_param_read(boot_param_t *param)
{
    if (param == NULL) {
        return false;
    }

    param_region_info_t info[2];
    param_region_scan(0U, &info[0]);
    param_region_scan(1U, &info[1]);

    int32_t  best_region = -1;
    uint32_t best_seq    = 0U;

    if (info[0].newest_slot >= 0) {
        best_region = 0;
        best_seq    = info[0].newest_seq;
    }
    if ((info[1].newest_slot >= 0) &&
        ((best_region < 0) || boot_param_seq_is_newer(info[1].newest_seq, best_seq))) {
        best_region = 1;
        best_seq    = info[1].newest_seq;
    }

    if (best_region < 0) {
        return false;   /* 两个区都还没有有效数据 */
    }

    memcpy(param,
           param_slot_ptr((uint32_t)best_region, (uint32_t)info[best_region].newest_slot),
           sizeof(boot_param_t));
    return true;
}

/**
 * @brief 追加写入一份新参数
 *
 * 策略：
 *   1. 优先在当前活动区（含最新参数的那个区）里追加到第一个空槽，不擦除任何旧槽；
 *   2. 活动区写满后切换另一区并从槽 0 开始，切换前擦除该区——此时它必然不含
 *      最新数据，所以擦除期间活动区始终完整可读；
 *   3. 两个区都空时，从 A 区槽 0 开始。
 *
 * 掉电安全性：任意时刻至少有一个区保存着完整的最新参数，最坏只丢最后一次写入。
 * 磨损：每 PARAM_SLOTS_PER_REGION 次写入才擦除一次扇区，而非每次写入都擦。
 *
 * 注意：seq_num 在函数内部基于 Flash 中的最新值自增，入参不会被修改。
 *       若调用方需要与 Flash 保持一致，请在写入成功后重新调用 boot_param_read()。
 */
bool boot_param_write(const boot_param_t *param)
{
    if (param == NULL) {
        return false;
    }

    param_region_info_t info[2];
    param_region_scan(0U, &info[0]);
    param_region_scan(1U, &info[1]);

    int32_t  active   = -1;     /* 含最新参数的那个区 */
    uint32_t base_seq = 0U;

    if (info[0].newest_slot >= 0) {
        active   = 0;
        base_seq = info[0].newest_seq;
    }
    if ((info[1].newest_slot >= 0) &&
        ((active < 0) || boot_param_seq_is_newer(info[1].newest_seq, base_seq))) {
        active   = 1;
        base_seq = info[1].newest_seq;
    }

    uint32_t target_region;
    uint32_t target_slot;

    if (active < 0) {
        /* 两区皆空：从 A 区槽 0 开始 */
        target_region = 0U;
        target_slot   = 0U;
        base_seq      = 0U;
    } else if (info[active].free_slot >= 0) {
        /* 活动区还有空槽：直接追加，不擦除 */
        target_region = (uint32_t)active;
        target_slot   = (uint32_t)info[active].free_slot;
    } else {
        /* 活动区写满：切到另一区，先擦除（该区不含最新数据，擦除安全） */
        target_region = (uint32_t)(1 - active);
        if (!flash_erase_sector_by_addr(param_region_base(target_region))) {
            return false;
        }
        target_slot = 0U;
    }

    boot_param_t temp;
    memcpy(&temp, param, sizeof(boot_param_t));
    temp.magic   = BOOT_PARAM_MAGIC;
    temp.seq_num = base_seq + 1U;   /* 自然回绕，由 RFC1982 比较规则处理 */
    boot_param_update_crc(&temp);

    uint32_t addr = param_region_base(target_region) + (target_slot * PARAM_SLOT_SIZE);
    if (!flash_write_data(addr, (const uint8_t *)&temp, sizeof(boot_param_t))) {
        return false;
    }

    /* 写后回读校验，捕获编程失败 */
    if (!boot_param_is_valid((const boot_param_t *)(uintptr_t)addr)) {
        return false;
    }

    return true;
}

/**
 * @brief 分区表与 BSP 扇区表的交叉自检
 *
 * boot_param.h 里的槽地址是手工维护的常量，而 bsp_flash.c 的扇区表是另一份
 * 完全独立的定义。两处只要有一处改漏，擦写就会落到错误的物理扇区上，而且
 * 编译器不会报任何错。这里在启动时做一次交叉核对，把这种静默漂移变成明确的失败。
 *
 * @return true 表示槽地址与 BSP 扇区划分一致
 */
bool boot_param_selfcheck(void)
{
    /* 每个日志区必须正好覆盖一个完整的 16KB 扇区（地址越界时 BSP 返回 0） */
    if (flash_sector_size_get(PARAM_REGION_A_ADDR) != PARAM_REGION_SIZE) {
        return false;
    }
    if (flash_sector_size_get(PARAM_REGION_B_ADDR) != PARAM_REGION_SIZE) {
        return false;
    }

    /* 两区必须落在不同扇区，否则切换区时会擦掉自己刚写完的日志 */
    return (PARAM_REGION_B_ADDR != PARAM_REGION_A_ADDR);
}

/**
 * @brief 参数区初始化检测，若为空则格式化并写入出厂默认值
 */
bool boot_param_init(boot_param_t *param)
{
    if (param == NULL) {
        return false;
    }

    /* 分区表与 BSP 扇区表不一致时直接拒绝服务：继续读写会擦错扇区 */
    if (!boot_param_selfcheck()) {
        return false;
    }

    if (boot_param_read(param)) {
        return true;    /* 已有有效参数，直接加载 */
    }

    /* 两个日志区都没有有效数据，重置为出厂默认参数 */
    memset(param, 0, sizeof(boot_param_t));
    param->magic           = BOOT_PARAM_MAGIC;
    param->seq_num         = 0;                    /* 由 write 内部递增为 1 */
    param->active_slot     = SLOT_APP_A;
    param->upgrade_state   = UPGRADE_STATE_IDLE;
    param->boot_fail_count = 0;
    /* memset 已清零，只需给出 A 槽的默认版本；B 槽出厂为空 */
    PARAM_SLOT_INFO(param, SLOT_APP_A).fw_version = 0x010000;   /* v1.0.0 */

    if (!boot_param_write(param)) {
        return false;
    }

    /* seq_num 在 write 内部自增，回读一次让调用方拿到真正落盘的内容 */
    return boot_param_read(param);
}
