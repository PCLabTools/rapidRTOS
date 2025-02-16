/**
 * @file rapidRTOS.h
 * @author Larry Colvin (PCLabTools@github)
 * @brief 
 * @version 0.1
 * @date 2023-10-22
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef rapidRTOS_h
#define rapidRTOS_h

#include <Arduino.h>

#ifdef BOARD_ESP32
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/timers.h"
#else
#include <FreeRTOS.h>
#include "task.h"
#include "queue.h"
#include "timers.h"
#endif

/**
 * @brief rapidDebug type enum for categorising debug
 * messages for printing to the debug console
 * 
 */
enum class rapidDebug
{
  ERROR,
  WARNING,
  INFO
};

/**
 * @brief rapidFunction type struct for messaging to
 * and from rapidPlugins
 * 
 */
struct rapidFunction
{
  char function[16];
  char parameters[48];
};

#include "rapidRTOS_manager.h"

#ifndef rapidRTOS_DEFAULT_STACK_SIZE
#define rapidRTOS_DEFAULT_STACK_SIZE 64         // Default stack size used by rapidPlugins if not specified during run
#endif

#ifndef rapidRTOS_DEFAULT_INTERFACE_SIZE
#define rapidRTOS_DEFAULT_INTERFACE_SIZE 256    // Default interface stack size if not specified during run
#endif

#ifndef rapidRTOS_DEFAULT_INTERFACE_BUFFER
#define rapidRTOS_DEFAULT_INTERFACE_BUFFER 64   // Default interface buffer size for rapidFunction commands
#endif

#ifndef rapidRTOS_DEFAULT_QUEUE_SIZE
#define rapidRTOS_DEFAULT_QUEUE_SIZE 1          // Default interface incoming queue size for rapidFunctions
#endif

#ifndef rapidRTOS_DEFAULT_PRIORITY
#define rapidRTOS_DEFAULT_PRIORITY 1            // Default task priority if not specified during run
#endif

/**
 * @brief rapidPlugin class provides a parent definition of a plugin
 * that can be used by derived classes to simplify setting up
 * and running a task in the rapidRTOS framework
 * 
 */
class rapidPlugin
{
  public:
    rapidPlugin();
    ~rapidPlugin();
    BaseType_t run(TaskFunction_t child, uint32_t stackDepth = rapidRTOS_DEFAULT_STACK_SIZE, uint32_t interfaceDepth = rapidRTOS_DEFAULT_INTERFACE_SIZE, int queueSize = rapidRTOS_DEFAULT_QUEUE_SIZE, UBaseType_t priority = rapidRTOS_DEFAULT_PRIORITY);
    BaseType_t runCore(UBaseType_t core, TaskFunction_t child, uint32_t stackDepth = rapidRTOS_DEFAULT_STACK_SIZE, uint32_t interfaceDepth = rapidRTOS_DEFAULT_INTERFACE_SIZE, int queueSize = rapidRTOS_DEFAULT_QUEUE_SIZE, UBaseType_t priority = rapidRTOS_DEFAULT_PRIORITY);
    void stop();
    const char* cmd(const char* command, TickType_t timeout = portMAX_DELAY);
    virtual uint8_t interface(rapidFunction incoming, char messageBuffer[]);

  protected:
    const char* _pID;                     // main task name
    char _iID[32];                        // interface task name buffer
    TaskHandle_t _taskHandle = NULL;      // main task handle reference
    TaskHandle_t _interfaceHandle = NULL; // interface task handle reference
    QueueHandle_t _taskQueue = NULL;      // interface task incoming queue
    QueueHandle_t _taskResponse = NULL;   // interface task outgoing queue
    static void interface_loop(void*);
};

/**
 * @brief Construct a new rapid Plugin::rapid Plugin object
 * 
 */
rapidPlugin::rapidPlugin()
{
  //
}

/**
 * @brief Destroy the rapid Plugin::rapid Plugin object
 * 
 */
rapidPlugin::~rapidPlugin()
{
  //
}

/**
 * @brief Runs a rapidPlugin task and registers the task with the rapidRTOS manager
 * 
 * @param child reference to task of plugin
 * @param stackDepth stack depth of the task (stack size is target dependent)
 * @param queueSize queue size for rapidFunction commands between plugins
 * @param priority FreeRTOS task priority
 * @return BaseType_t 1 = task started and registered | 0 = task failed to start
 */
BaseType_t rapidPlugin::run(TaskFunction_t child, uint32_t stackDepth, uint32_t interfaceDepth, int queueSize, UBaseType_t priority)
{
  #ifdef BOARD_ESP32
  stackDepth = stackDepth * 4;
  interfaceDepth = interfaceDepth * 4;
  #endif
  sprintf(_iID, "i_%s", _pID);
  if (!rapidRTOS.getTaskHandle(_pID))
  {
    _taskQueue = xQueueCreate(queueSize, sizeof(const char*));
    _taskResponse = xQueueCreate(1, sizeof(const char*));
    if(xTaskCreate(child, _pID, stackDepth, this, priority, &_taskHandle)\
    && xTaskCreate(&interface_loop, _iID, interfaceDepth, this, priority, &_interfaceHandle))\
    return (BaseType_t)rapidRTOS.reg(_taskHandle, _pID, &_taskQueue, &_taskResponse);
    else
    {
      vTaskDelete(_taskHandle);
      _taskHandle = NULL;
      vQueueDelete(_taskQueue);
      _taskQueue = NULL;
      vQueueDelete(_taskResponse);
      _taskResponse = NULL;
    return 0;
    }
  }
  return 0;
}

/**
 * @brief Runs a rapidPlugin task on the specified core and registers the task with the rapidRTOS manager
 * 
 * @param core core id
 * @param child reference to task of plugin
 * @param stackDepth stack depth of the task (stack size is target dependent)
 * @param queueSize queue size for rapidFunction commands between plugins
 * @param priority FreeRTOS task priority
 * @return BaseType_t BaseType_t 1 = task started and registered | 0 = task failed to start
 */
BaseType_t rapidPlugin::runCore(UBaseType_t core, TaskFunction_t child, uint32_t stackDepth, uint32_t interfaceDepth, int queueSize, UBaseType_t priority)
{
  #ifdef BOARD_ESP32
  stackDepth = stackDepth * 4;
  interfaceDetph = interfaceDepth * 4;
  #endif
  sprintf(_iID, "i_%s", _pID);
  if (!rapidRTOS.getTaskHandle(_pID))
  {
    _taskQueue = xQueueCreate(queueSize, sizeof(const char*));
    _taskResponse = xQueueCreate(1, sizeof(const char*));
    #ifdef BOARD_ESP32
    if(xTaskCreatePinnedToCore(child, _pID, stackDepth, this, priority, &_taskHandle, core)\
    && xTaskCreatePinnedToCore(&interface_loop, _iID, interfaceDepth, this, priority, &_interfaceHandle, core))\
    return (BaseType_t)rapidRTOS.reg(_taskHandle, _pID, &_taskQueue, &_taskResponse);
    #elif BOARD_TEENSY
    if(xTaskCreate(child, _pID, stackDepth, this, priority, &_taskHandle)\
    && xTaskCreate(&interface_loop, _iID, interfaceDepth, this, priority, &_interfaceHandle))\
    return (BaseType_t)rapidRTOS.reg(_taskHandle, _pID, &_taskQueue, &_taskResponse);
    #elif BOARD_STM32
    if(xTaskCreate(child, _pID, stackDepth, this, priority, &_taskHandle)\
    && xTaskCreate(&interface_loop, _iID, interfaceDepth, this, priority, &_interfaceHandle))\
    return (BaseType_t)rapidRTOS.reg(_taskHandle, _pID, &_taskQueue, &_taskResponse);
    #else
    if(xTaskCreateAffinitySet(child, _pID, stackDepth, this, priority, core, &_taskHandle)\
    && xTaskCreateAffinitySet(&interface_loop, _iID, interfaceDepth, this, priority, core, &_interfaceHandle))\
    return (BaseType_t)rapidRTOS.reg(_taskHandle, _pID, &_taskQueue, &_taskResponse);
    #endif
    else
    {
      vTaskDelete(_taskHandle);
      _taskHandle = NULL;
      vQueueDelete(_taskQueue);
      _taskQueue = NULL;
      vQueueDelete(_taskResponse);
      _taskResponse = NULL;
    return 0;
    }
  }
  return 0;
}

/**
 * @brief Stops a running task
 * 
 */
void rapidPlugin::stop()
{
  if (_taskHandle)
  {
    if (_interfaceHandle) { vTaskDelete(_interfaceHandle); }
    vTaskDelete(_taskHandle);
    _taskHandle = NULL;
    rapidRTOS.dereg(_pID);
    vQueueDelete(_taskQueue);
    _taskQueue = NULL;
    vQueueDelete(_taskResponse);
    _taskResponse = NULL;
  }
}

/**
 * @brief Sends a rapidFunction command to the plugin
 * 
 * @param command string literal containing the rapidFunction command
 * @param timeout maximum wait time for response from command
 * @return const char* response from command
 */
const char* rapidPlugin::cmd(const char* command, TickType_t timeout)
{
  xQueueSend(_taskQueue, &command, timeout);
  const char* response = "";
  xQueueReceive(_taskResponse, &response, timeout);
  return response;
}

/**
 * @brief Virtual function to be overriden in child implementations. 
 * This function is to be used for creating states that are called
 * when rapidFunction commands are received
 * 
 * @param incoming rapidFunction command received
 * @param messageBuffer response message buffer
 * @return uint8_t 1 = function run | 0 = no function
 */
uint8_t rapidPlugin::interface(rapidFunction incoming, char messageBuffer[])
{
  return 0;
}

/**
 * @brief Interface loop used to process incoming rapidFunction commands
 * 
 * @param pModule pointer to calling object
 */
void rapidPlugin::interface_loop(void* pModule)
{
  rapidPlugin* plugin = (rapidPlugin*)pModule;
  char messageBuffer[rapidRTOS_DEFAULT_INTERFACE_BUFFER] = "";
  const char* incomingMessage = messageBuffer;
  const char* outgoingMessage = messageBuffer;
  for ( ;; )
  {
    xQueueReceive(plugin->_taskQueue, &incomingMessage, portMAX_DELAY);
    sprintf(messageBuffer, ""); // clear message buffer for outgoing message
    rapidFunction incoming = rapidRTOS.parse(incomingMessage);
    do
    {
      if (!strcmp(incoming.function, "identity"))
      {
        sprintf(messageBuffer, "%s", plugin->_pID);
        continue;
      }
      if (!strcmp(incoming.function, "stop"))
      {
        plugin->stop();
        break;
      }
      if (plugin->interface(incoming, messageBuffer))
      {
        continue;
      }
      sprintf(messageBuffer, "unknown_function(%s)", incoming.function);
      rapidRTOS.printDebug(1, rapidDebug::ERROR, "%s: unknown_function(%s)\n", plugin->_pID, incoming.function);
    } while (false);
    outgoingMessage = messageBuffer;
    xQueueSend(plugin->_taskResponse, &outgoingMessage, portMAX_DELAY);
  }
}

#endif