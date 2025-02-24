/*
 *    ||          ____  _ __
 * +------+      / __ )(_) /_______________ _____  ___
 * | 0xBC |     / __  / / __/ ___/ ___/ __ `/_  / / _ \
 * +------+    / /_/ / / /_/ /__/ /  / /_/ / / /_/  __/
 *  ||  ||    /_____/_/\__/\___/_/   \__,_/ /___/\___/
 *
 * Crazyflie control firmware
 *
 * Copyright (C) 2011-2012 Bitcraze AB
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
 * transferdata.c - Top level module implementation
 */

#include "config.h"
#include "debug.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "static_mem.h"
#include "task.h"

static xQueueHandle inputQueue;
STATIC_MEM_QUEUE_ALLOC(inputQueue, 1, sizeof(int));

static void exampleTask(void*);
STATIC_MEM_TASK_ALLOC(exampleTask, TRANSFERDATA_TASK_STACKSIZE);

static bool isInit = false;

void exampleTaskInit() {
  inputQueue = STATIC_MEM_QUEUE_CREATE(inputQueue);
  // TODO
  STATIC_MEM_TASK_CREATE(exampleTask, exampleTask, TRANSFERDATA_TASK_NAME, NULL, TRANSFERDATA_TASK_PRI);
  isInit = true;
}

bool exampleTaskTest() {
  return isInit;
}

static void exampleTask(void* parameters) {
  DEBUG_PRINT("Example task main function is running!");
  while (true) {
    int input;
    if (pdTRUE == xQueueReceive(inputQueue, &input, portMAX_DELAY)) {
      // Respond to input here!
    }
  }
}

void exampleTaskEnqueueInput(int value) {
  xQueueOverwrite(inputQueue, &value);
}