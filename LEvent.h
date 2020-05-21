#pragma once

#include "Support/Utils.h"

/**
 * LEvent 表示一个网络锁步事件
 * 
 * +----+-----------+--------------+-----------+--------------+-----------+-------+
 * | ID | SessionID | TargetObject | EventEnum | ParamsLength | ParamData | Check |
 * +----+-----------+--------------+-----------+--------------+-----------+-------+
 *
 * Tick：第0个Tick时发生的事件
 *
 * +-----------------------+------+-------------+------+--------+-----------------------------------------+------+
 * | 0x0000 0000 0000 0001 | 0x00 | 0xFFFF FFFF | 0x00 | 0x0008 | 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 | 0x09 |
 * +-----------------------+------+-------------+------+--------+-----------------------------------------+------+
 *
 * ID[8]：事件的唯一ID，累加，由服务器指定，客户端发送时保持该参数为 0x0000 0000 0000 0000
 *        服务器填写后广播，如果不符合标准则丢弃。
 *
 * SessionID[1]：事件制造者的网络ID，0x00 保留，表示服务器产生的事件。
 *
 * TargetObject[4]：目标Object的唯一ID，0x0000 0000 表示客户端的锁步管理器负责处理
 *                  0xFFFF FFFF 表示广播给每一个 Object。
 *
 * EventEnum[1]：事件的具体名编号，只有具体的TargetObject知道编号所对应的函数，
 *               其中 0x00 到 0x0F 保留，0x00 为Tick事件。
 *
 * ParamsLength[2]：表示ParamData的长度，单位字节，vector的size。
 *
 * ParamData[-]：具体的数据，vector的data。
 *
 * Check[1]：校验位，前面所有数据的异或和。
 *
 */

struct LEvent
{
	uint64_t ID;
	uint8_t SessionID;
	uint32_t TargetObject;
	uint8_t EventEnum;
	std::vector<uint8_t> Params;
	uint8_t Check;
};
