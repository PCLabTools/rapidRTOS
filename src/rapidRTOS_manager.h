/**
 * @file rapidRTOS_manager.h
 * @author Larry Colvin (PCLabTools@github)
 * @brief 
 * @version 0.1
 * @date 2023-10-28
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef rapidRTOS_manager_h
#define rapidRTOS_manager_h

#ifndef rapidRTOS_MAX_MODULES
/**
 * @brief Max number of modules the manager can store.
 * Increasing this number will increase the overall memory
 * used by the manager.
 * 
 */
#define rapidRTOS_MAX_MODULES 10
#endif

#ifndef rapidRTOS_MANAGER_STACK_SIZE
/**
 * @brief Stack size used by the manager task. This can be increased
 * to accomodate increasing numbers of plugins and tasks.
 * 
 */
#define rapidRTOS_MANAGER_STACK_SIZE 256
#endif

/**
 * @brief rapidRTOS manager class provides a 'singleton' class for a rapidRTOS
 * task management object. Using this class will allow for
 * centralised and communicable paralell tasks to be managed.
 * 
 */
class rapidRTOS_manager
{
  public:
    static rapidRTOS_manager& getInstance() 
    {
      // Allocate with `new` in case Singleton is not trivially destructible.
      static rapidRTOS_manager* singleton = new rapidRTOS_manager();
      return *singleton;
    }
    uint8_t reg(TaskHandle_t taskHandle, const char* taskName, QueueHandle_t* command, QueueHandle_t* response);
    uint8_t dereg(const char* taskName);
    const char* cmd(const char* taskName, const char* command, TickType_t timeout = portMAX_DELAY);
    uint8_t setDebugLevel(uint8_t debugLevel);
    uint8_t getDebugLevel();
    void setDebugStream(Stream* streamRef);
    template<typename... Args> void printDebug(uint8_t debugLevel, const char* message, Args... args);
    template<typename... Args> void printDebug(uint8_t debugLevel, rapidDebug messageType, const char* message, Args... args);
    TaskHandle_t getTaskHandle(const char* taskName);
    TaskStatus_t getTaskStatus(const char* taskName);
    uint8_t getNumTasks();
    void printTaskStatus(const char* taskName);
    rapidFunction parse(const char* message);

  private:
    rapidRTOS_manager();
    rapidRTOS_manager(const rapidRTOS_manager&) = delete;
    rapidRTOS_manager& operator=(const rapidRTOS_manager&) = delete;
    rapidRTOS_manager(rapidRTOS_manager&&) = delete;
    rapidRTOS_manager& operator=(rapidRTOS_manager&&) = delete;
    static void managerTask(void* pParameters);
    const char* _taskNames[rapidRTOS_MAX_MODULES];
    TaskHandle_t _taskHandles[rapidRTOS_MAX_MODULES];
    QueueHandle_t* _taskQueues[rapidRTOS_MAX_MODULES];
    QueueHandle_t* _responseQueues[rapidRTOS_MAX_MODULES];
    uint8_t _debugLevel = 0;
    Stream* _debugStream = &Serial;
    TaskHandle_t _managerHandle;
};

/**
 * @brief Construct a new rapidRTOS manager::rapidRTOS manager object and
 * initialises _taskNames as blank tasks
 * 
 */
rapidRTOS_manager::rapidRTOS_manager()
{
  for (size_t i = 0; i < rapidRTOS_MAX_MODULES; i++)
  {
    _taskNames[i] = "";
  }
  #ifndef rapidRTOS_DISABLE_MANAGER
  xTaskCreate(managerTask, "rapidRTOS_manager", rapidRTOS_MANAGER_STACK_SIZE, this, 1, &_managerHandle);
  #endif
}

/**
 * @brief Registers a task with the manager which allows for searching
 * and interacting with tasks through the manager interface
 * 
 * @param taskHandle task reference
 * @param command command queue reference
 * @param response response queue reference
 * @return uint8_t 1 = no space for task registration | 0 = pass
 */
uint8_t rapidRTOS_manager::reg(TaskHandle_t taskHandle, const char* taskName, QueueHandle_t* command, QueueHandle_t* response = NULL)
{
  for (size_t i = 0; i < rapidRTOS_MAX_MODULES; i++)
  {
    if(!strcmp(_taskNames[i],""))
    {
      _taskNames[i] = taskName;
      _taskHandles[i] = taskHandle;
      _taskQueues[i] = command;
      _responseQueues[i] = response;
      return 1;
    }
  }
  return 0;
}

/**
 * @brief Unregisters the task defined by task name
 * 
 * @param taskName string literal name of the task
 * @return uint8_t 1 = task unregistered | 0 = task not found
 */
uint8_t rapidRTOS_manager::dereg(const char* taskName)
{
  for (size_t i = 0; i < rapidRTOS_MAX_MODULES; i++)
  {
    if(!strcmp(_taskNames[i],taskName))
    {
      _taskNames[i] = "";
      _taskHandles[i] = NULL;
      _taskQueues[i] = NULL;
      _responseQueues[i] = NULL;
      return 1;
    }
  }
  return 0;
}

/**
 * @brief Sends a command to the task defined by task name
 * 
 * @param taskName string literal name of task
 * @param command string literal containing the command
 * @param timeout timeout for waiting for response
 * @return const char* response to command
 */
const char* rapidRTOS_manager::cmd(const char* taskName, const char* command, TickType_t timeout)
{
  const char* response = "";
  for (size_t i = 0; i < rapidRTOS_MAX_MODULES; i++)
  {
    if(!strcmp(_taskNames[i],taskName))
    {
      xQueueSend(*_taskQueues[i], &command, timeout);
      xQueueReceive(*_responseQueues[i], &response, timeout);
      break;
    }
  }
  return response;
}

/**
 * @brief Sets the debug level which is used to filter debug messages for printing
 * 
 * @param debugLevel 0 = off | >0 = >debug detail
 * @return uint8_t current debug level
 */
uint8_t rapidRTOS_manager::setDebugLevel(uint8_t debugLevel)
{
  _debugLevel = debugLevel;
  return _debugLevel;
}

/**
 * @brief Gets the currently set debug level
 * 
 * @return uint8_t current debug level
 */
uint8_t rapidRTOS_manager::getDebugLevel()
{
  return _debugLevel;
}

/**
 * @brief Sets the stream to use for debug message printing
 * 
 * @param streamRef stream reference
 * @return uint8_t 
 */
void rapidRTOS_manager::setDebugStream(Stream* streamRef)
{
  _debugStream = streamRef;
}

/**
 * @brief If the input debug level is greater than the manager's debug level
 * a debug message is printed to the debug stream
 * 
 * @tparam Args any number of arguments of any type
 * @param debugLevel debug level of the message
 * @param message string literal containing message and escape characters for referencing a parameter argument
 * @param args message parameter arguments
 */
template<typename... Args>
void rapidRTOS_manager::printDebug(uint8_t debugLevel, const char* message, Args... args)
{
  printDebug(debugLevel, (rapidDebug)255, message, args...);
}

/**
 * @brief If the input debug level is greater than the manager's debug level
 * a debug message is printed to the debug stream
 * 
 * @tparam Args any number of arguments of any type
 * @param debugLevel debug level of the message
 * @param messageType type of debug message to categorise debug message
 * @param message string literal containing message and escape characters for referencing a parameter argument
 * @param args message parameter arguments
 */
template<typename... Args> void
rapidRTOS_manager::printDebug(uint8_t debugLevel, rapidDebug messageType, const char* message, Args... args)
{
  if (_debugLevel >= debugLevel)
  {
    switch (messageType)
    {
      case rapidDebug::ERROR:
        #ifdef ANSI_OUTPUT
        _debugStream->print("\033[41m[ERROR]");
        #else
        _debugStream->print("[ERROR]");
        #endif
        break;
      case rapidDebug::WARNING:
        #ifdef ANSI_OUTPUT
        _debugStream->print("\033[43m[WARNING]");
        #else
        _debugStream->print("[WARNING]");
        #endif
        break;
      case rapidDebug::INFO:
        #ifdef ANSI_OUTPUT
        _debugStream->print("\033[7m[INFO]");
        #else
        _debugStream->print("[INFO]");
        #endif
        break;
      default:
        #ifdef ANSI_OUTPUT
        _debugStream->print("\033[46m[DEBUG]");
        #else
        _debugStream->print("[DEBUG]");
        #endif
        break;
    }
    #ifdef ANSI_OUTPUT
    _debugStream->print("\033[0m");
    #endif
    _debugStream->printf(" (%d) ", millis());
    _debugStream->printf(message, args...);
  }
}

/**
 * @brief Requests the task handle of the task defined by task name
 * 
 * @param taskName string literal containing task name
 * @return TaskHandle_t task handle reference of task if found
 */
TaskHandle_t rapidRTOS_manager::getTaskHandle(const char* taskName)
{
  for (uint8_t i = 0 ; i < rapidRTOS_MAX_MODULES ; i++)
  {
    if(!strcmp(_taskNames[i],taskName))
    {
      return _taskHandles[i];
    }
  }
  return NULL;
}

/**
 * @brief Requests the task status of the task defined by the task name
 * 
 * @param taskName string literal containing task name
 * @return TaskStatus_t task status of the task if found
 */
TaskStatus_t rapidRTOS_manager::getTaskStatus(const char* taskName)
{
  TaskStatus_t taskstatus;
  uint8_t freestackspace;
  for (uint8_t i = 0 ; i < rapidRTOS_MAX_MODULES ; i++)
  {
    if(!strcmp(_taskNames[i],taskName))
    {
      #ifdef BOARD_ESP32
      taskstatus.pcTaskName = _taskNames[i];
      taskstatus.eCurrentState = eInvalid;
      taskstatus.pxStackBase = NULL;
      taskstatus.ulRunTimeCounter = 0;
      taskstatus.uxCurrentPriority = 0;
      taskstatus.xTaskNumber = 0;
      #else
      vTaskGetInfo(_taskHandles[i], &taskstatus, freestackspace, eInvalid);
      #endif
    }
  }
  return taskstatus;
}

/**
 * @brief Gets the number of registered tasks
 * 
 * @return uint8_t number of registered tasks
 */
uint8_t rapidRTOS_manager::getNumTasks()
{
  uint8_t numTasks = 0;
  for (uint8_t i = 0 ; i < rapidRTOS_MAX_MODULES ; i++)
  {
    if(strcmp(_taskNames[i],""))
    {
      numTasks++;
    }
  }
  return numTasks;
}

/**
 * @brief Prints the current task status to the debug stream
 * 
 * @param taskName string literal containing the task name
 */
void rapidRTOS_manager::printTaskStatus(const char* taskName)
{
  TaskStatus_t taskStatus = getTaskStatus(taskName);
  const char * state = "";
  switch (taskStatus.eCurrentState)
  {
    case eRunning:
      state = "Running";
      break;
    case eReady:
      state = "Ready";
      break;
    case eBlocked:
      state = "Blocked";
      break;
    case eSuspended:
      state = "Suspended";
      break;
    case eDeleted:
      state = "Deleted";
      break;
    case eInvalid:
      state = "Invalid";
      break;
    default:
      state = "Invalid";
      break;
  }
  if (!strcmp(taskStatus.pcTaskName,taskName))
  {
    _debugStream->printf ("***** TASK STATUS *****\n\
Name:     %s\n\
State:    %s[%d]\n\
Address:  %p\n\
Runtime:  %d\n\
Priority: %d\n\
Task ID:  %d\n\
***********************\n",\
    taskStatus.pcTaskName, state, taskStatus.eCurrentState, taskStatus.pxStackBase, taskStatus.ulRunTimeCounter, taskStatus.uxCurrentPriority, taskStatus.xTaskNumber);
  }
  else
  {
    _debugStream->printf ("***** TASK STATUS *****\n\
Name:     %s\n\
State:    %s[%d]\n\
Address:  %p\n\
Runtime:  %d\n\
Priority: %d\n\
Task ID:  %d\n\
***********************\n",\
    taskName, "Invalid", eInvalid, 0, 0, 0, 0);
  }
}

/**
 * @brief Function to be used to parse rapidFunction messages from a string literal
 * 
 * @param message string literal containing rapidFunction message
 * @return rapidFunction parsed rapidFunction
 */
rapidFunction rapidRTOS_manager::parse(const char* message)
{
  rapidFunction returnMessage;
  int functionIndex = 0;
  int parameterIndex = 0;
  bool functionComplete = false;
  bool messageComplete = false;
  while (message[0] != '\0' && !messageComplete)
  {
    if (!functionComplete)
    {
      returnMessage.function[functionIndex] = message[0];
      functionIndex++;
      message++;
      if (message[0] == ')')
      {
        messageComplete = true;
      }
      if (message[0] == '(')
      {
        returnMessage.function[functionIndex] = '\0';
        functionComplete = true;
      } 
    }
    else
    {
      if (message[0] == '(')
      {
        message++;
      }
      if(message[0] != ')')
      {
        returnMessage.parameters[parameterIndex] = message[0];
        parameterIndex++;
        message++;
      }
      if (message[0] == ')')
      {
        returnMessage.parameters[parameterIndex] = '\0';
        messageComplete = true;
        message++;
      }
    }
  }
  returnMessage.function[functionIndex] = '\0';
  returnMessage.parameters[parameterIndex] = '\0';
  return returnMessage;
}

/**
 * @brief This is the manager's own task used to monitor critical tasks
 * 
 * @param pParameters void* pointer to calling object
 */
void rapidRTOS_manager::managerTask(void* pParameters)
{
  rapidRTOS_manager* plugin = (rapidRTOS_manager*)pParameters;
  for (;;)
  {
    plugin->printDebug(255, rapidDebug::INFO, "Running in ULTRA debug mode");
    vTaskDelay(1000/portTICK_PERIOD_MS);
  }
}

/**
 * @brief External interface to the radpiRTOS_manager class singleton object
 * 
 */
rapidRTOS_manager& rapidRTOS = rapidRTOS_manager::getInstance();

#endif