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
#include "commander.h"

#include "FreeRTOS.h"
#include "task.h"

#define DEBUG_MODULE "CCOO"
#include "debug.h"

#include "log.h"
#include "param.h"


#define MAX_COORD_VALUE 1000
#define MIN_COORD_VALUE (-1000)


#define FLIGHT_DURATION_MS 10000

static TickType_t flightStartTime = 0;

typedef enum {
  unlocked,
  stopping,
  idle
} State;

static State state = idle;

static void setHoverSetpoint(setpoint_t *setpoint, float vx, float vy, float z, float yawrate)
{
  setpoint->mode.z = modeAbs;
  setpoint->position.z = z;
  setpoint->mode.yaw = modeVelocity;
  setpoint->attitudeRate.yaw = yawrate;
  setpoint->mode.x = modeVelocity;
  setpoint->mode.y = modeVelocity;
  setpoint->velocity.x = vx;
  setpoint->velocity.y = vy;
  setpoint->velocity_body = true;
}

//data packed
#pragma pack(push, 1)
typedef struct {
    int16_t x;
    int16_t y;
} __attribute__((packed)) face_position_t;
#pragma pack(pop)
static void cpxPacketCallback(const CPXPacket_t* cpxRx);



void appMain() {
  vTaskDelay(M2T(100));
  cpxRegisterAppMessageHandler(cpxPacketCallback);
  DEBUG_PRINT("Coordinate receiver ready\n");

  static setpoint_t setpoint;
  paramVarId_t idPositioningDeck = paramGetVarId("deck", "bcFlow2");
  paramVarId_t idAppMode= paramGetVarId("flightmode", "appmode");

  while(1) {

    uint8_t positioningInit = paramGetUint(idPositioningDeck);
    uint8_t appModeEnabled = paramGetUint(idAppMode);
    static bool Flag = false;
    static bool Flag2 = false;
    vTaskDelay(M2T(10));


    if (appModeEnabled) { 
      
      if (state == unlocked) {

        if (1) {
        setHoverSetpoint(&setpoint, 0, 0, 0.4f, 0);
        commanderSetSetpoint(&setpoint, 3);
        }

        if (xTaskGetTickCount() - flightStartTime > pdMS_TO_TICKS(FLIGHT_DURATION_MS)) {
          state = stopping;
          memset(&setpoint, 0, sizeof(setpoint_t));
          commanderSetSetpoint(&setpoint, 3);
          Flag2 = true;
          DEBUG_PRINT("Stopping!\n");
         }

      } else {

        if (state == stopping) {
          DEBUG_PRINT("Stopping!!!\n");
          vTaskDelay(M2T(1000));
        }

        if (state == idle && positioningInit) {
          DEBUG_PRINT("Unlocked!\n");
          state = unlocked;
          if(Flag2 == false){
          flightStartTime = xTaskGetTickCount(); }
        }

        

      }
      Flag = true;
      Flag2 = false;
    }else{ 
      if(Flag == true){
        commanderRelaxPriority();
        Flag=false;
        DEBUG_PRINT("Flag=FALSE (已释放优先级)\n");  
      }
    }
    vTaskDelay(M2T(10));
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



