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

#include "app.h"
#include "cpx.h"
#include "cpx_internal_router.h"
#include "FreeRTOS.h"
#include "task.h"
#include "coordinate.h"

#define DEBUG_MODULE "APP"
#include "debug.h"

// 全局变量声明
static CoordinatePacket currentTarget = {0};
static QueueHandle_t coordQueue = NULL;

// 前向声明
static void cpxPacketCallback(const CPXPacket_t* cpxRx);
static void controlTask(void* params);

// 回调函数实现
static void cpxPacketCallback(const CPXPacket_t* cpxRx) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    
    if (cpxRx->dataLength == sizeof(CoordinatePacket)) {
        CoordinatePacket received;
        memcpy(&received, cpxRx->data, sizeof(CoordinatePacket));
        
        if(received.seq > currentTarget.seq) {
            xQueueSendFromISR(coordQueue, &received, &xHigherPriorityTaskWoken);
        }
    }
    
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    DEBUG_PRINT("Received seq: %lu\n", currentTarget.seq);
}

// 控制任务实现
static void controlTask(void* params) {
    TickType_t xLastWakeTime = xTaskGetTickCount();
    
    while(1) {
        CoordinatePacket newCoord;
        if(xQueueReceive(coordQueue, &newCoord, pdMS_TO_TICKS(10)) == pdTRUE) {
            currentTarget = newCoord;
            DEBUG_PRINT("New target: X=%.2f Y=%.2f Seq=%lu\n", 
                       currentTarget.x, currentTarget.y, currentTarget.seq);
        }
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(20));
    }
}

// 主函数
void appMain() {
    // 初始化队列
    coordQueue = xQueueCreate(5, sizeof(CoordinatePacket));
    if(coordQueue == NULL) {
        DEBUG_PRINT("Queue creation failed!\n");
        while(1);
    }
    
    // 创建控制任务
    if(xTaskCreate(controlTask, "CTRL", configMINIMAL_STACK_SIZE*2, 
                  NULL, configMAX_PRIORITIES-2, NULL) != pdPASS) {
        DEBUG_PRINT("Task creation failed!\n");
        while(1);
    }
    
    // 注册回调
    cpxRegisterAppMessageHandler(cpxPacketCallback);
    
    DEBUG_PRINT("Coordinate receiver initialized\n");
    
    while(1) {
        vTaskDelay(M2T(1000));
    }
}
