/**
 * ,---------,       ____  _ __
 * |  ,-^-,  |      / __ )(_) /_______________ _____  ___
 * | (  O  ) |     / __  / / __/ ___/ ___/ __ `/_  / / _ \
 * | / ,--´  |    / /_/ / / /_/ /__/ /  / /_/ / / /_/  __/
 *    +------`   /_____/_/\__/\___/_/   \__,_/ /___/\___/
 *
 * Crazyflie control firmware
 *
 * Copyright (C) 2023 Bitcraze AB
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, in version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 *
 * App layer application that communicates with the GAP8 on an AI deck.
 */


#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>  

#include "app.h"
#include "cpx.h"
#include "cpx_internal_router.h"

#include "FreeRTOS.h"
#include "task.h"

#define DEBUG_MODULE "APP"
#include "debug.h"

#define MAX_COORD_VALUE 1000
#define MIN_COORD_VALUE (-1000)

#pragma pack(push, 1)
typedef struct {
    int16_t x;
    int16_t y;
} __attribute__((packed)) face_position_t;// 仅保留一种打包方式
#pragma pack(pop)


// Callback that is called when a CPX packet arrives
static void cpxPacketCallback(const CPXPacket_t* cpxRx);
//static CPXPacket_t txPacket;


void appMain() {
  DEBUG_PRINT("Coordinate receiver ready\n");

  // Register a callback for CPX packets.
  // Packets sent to destination=CPX_T_STM32 and function=CPX_F_APP will arrive here
  cpxRegisterAppMessageHandler(cpxPacketCallback);

  //uint8_t counter = 0;
  while(1) {
    vTaskDelay(M2T(2000));

    //cpxInitRoute(CPX_T_STM32, CPX_T_GAP8, CPX_F_APP, &txPacket.route);
    // txPacket.data[0] = counter;
    // txPacket.dataLength = 1;
    //cpxSendPacketBlocking(&txPacket);
    // DEBUG_PRINT("Sent packet to GAP8 (%u)\n", counter);
    //counter++;
  }
}

  static void cpxPacketCallback(const CPXPacket_t* cpxRx) {

  // 检查数据长度是否足够
  if(cpxRx->dataLength != sizeof(face_position_t)) {
    DEBUG_PRINT("Invalid coord packet! Length:%d (Expected:%d)\n", 
               cpxRx->dataLength, 
               sizeof(face_position_t));
    return;
  }

  // 解析数据结构
  const face_position_t* coord = (const face_position_t*)cpxRx->data;

  // 数据有效性检查
  if((coord->x < MIN_COORD_VALUE) || (coord->x > MAX_COORD_VALUE) ||
   (coord->y < MIN_COORD_VALUE) || (coord->y > MAX_COORD_VALUE)) {
    DEBUG_PRINT("Out of range! X:%d Y:%d\n", coord->x, coord->y);
    return;
}

  // 转换并显示实际值
  // 传输时保持整数，显示时添加小数点
  DEBUG_PRINT("Received:[%d bytes]:X:%d.%d | Y:%d.%d\n",
           cpxRx->dataLength,
           coord->x / 10, abs(coord->x % 10),
           coord->y / 10, abs(coord->y % 10));
  }



