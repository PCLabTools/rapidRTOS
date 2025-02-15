# rapidRTOS

FreeRTOS based core library for running "rapidPlugins" which enable rapid prototyping using the Arduino framework and PlatformIO to integrate common sensors and peripherals.

## Navitagation

- [rapidRTOS](#rapidrtos)
  - [Navitagation](#navitagation)
  - [Description](#description)
    - [Why Would I Need an Interface?](#why-would-i-need-an-interface)
  - [Installation](#installation)
    - [Arduino](#arduino)
    - [PlatformIO](#platformio)
  - [Usage](#usage)
    - [General Usage](#general-usage)
    - [rapidRTOS Manager Functions](#rapidrtos-manager-functions)
      - [rapidRTOS\_manager\& getInstance()](#rapidrtos_manager-getinstance)
      - [uint8\_t reg(TaskHandle\_t taskHandle, QueueHandle\_t\* command, QueueHandle\_t\* response)](#uint8_t-regtaskhandle_t-taskhandle-queuehandle_t-command-queuehandle_t-response)
      - [uint8\_t dereg(const char\* taskName)](#uint8_t-deregconst-char-taskname)
      - [const char\* cmd(const char\* taskName, const char\* command, TickType\_t timeout)](#const-char-cmdconst-char-taskname-const-char-command-ticktype_t-timeout)
      - [uint8\_t setDebugLevel(uint8\_t debugLevel)](#uint8_t-setdebugleveluint8_t-debuglevel)
      - [void setDebugStream(Stream\* streamRef)](#void-setdebugstreamstream-streamref)
      - [rapidRTOS.printDebug()](#rapidrtosprintdebug)
    - [rapidPlugin Functions](#rapidplugin-functions)
      - [rapidPlugin()](#rapidplugin)
      - [~rapidPlugin()](#rapidplugin-1)
      - [BaseType\_t run(TaskFunction\_t child, uint32\_t stackDepth, int queueSize, UBaseType\_t priority)](#basetype_t-runtaskfunction_t-child-uint32_t-stackdepth-int-queuesize-ubasetype_t-priority)
      - [BaseType\_t runCore(UBaseType\_t core, TaskFunction\_t child, uint32\_t stackDepth, int queueSize, UBaseType\_t priority)](#basetype_t-runcoreubasetype_t-core-taskfunction_t-child-uint32_t-stackdepth-int-queuesize-ubasetype_t-priority)
      - [void stop()](#void-stop)
      - [const char\* cmd(const char\* command, TickType\_t timeout)](#const-char-cmdconst-char-command-ticktype_t-timeout)
      - [virtual uint8\_t interface(rapidFunction incoming, char\* messageBuffer)](#virtual-uint8_t-interfacerapidfunction-incoming-char-messagebuffer)
  - [License](#license)

## Description

The purpose of this library is to provide a common architecture to a FreeRTOS application. The intent is to create plug and play modules called **plugins** which can be ran using simple calls from a higher level project. When running the plugins, a running rapidRTOS task handles registering the plugin to allow cross-plugin communication through their interfaces. 

### Why Would I Need an Interface?

To provide a generic set of plugins that allow for a flexible combination of parts to a project, an interface is crucial to allow automated control of the peripherals.

A good example would be an MQTT plugin needing to interface with the network via another plugin like a WiFi plugin or ENET plugin. To generalise the network the MQTT plugin could expect the network plugin to be named "network" which would allow abstraction (choice) of the type of network being used.

## Installation

### Arduino

To install this library follow the standard method of installing libraries, either using the library manager or a downloaded zip file of this repository.

For more information on how to install libraries please visit [Installing Additional Arduino Libraries](https://www.arduino.cc/en/guide/libraries "arduino.cc").

### PlatformIO

To include this library and its dependencies simply add the following to the "platformio.ini" file:
```
[env:my_build_env]
framework = arduino
lib_deps = 
  https://github.com/PCLabTools/rapidRTOS.git
```

## Usage

### General Usage

Lorem Ipsum

### rapidRTOS Manager Functions

#### rapidRTOS_manager& getInstance()

#### uint8_t reg(TaskHandle_t taskHandle, QueueHandle_t* command, QueueHandle_t* response)

#### uint8_t dereg(const char* taskName)

#### const char* cmd(const char* taskName, const char* command, TickType_t timeout)

#### uint8_t setDebugLevel(uint8_t debugLevel)

#### void setDebugStream(Stream* streamRef)

#### rapidRTOS.printDebug()

Lorem Ipsum

Return: none

### rapidPlugin Functions

#### rapidPlugin()

Lorem Ipsum

Return: none

#### ~rapidPlugin()

Lorem Ipsum

Return: none

#### BaseType_t run(TaskFunction_t child, uint32_t stackDepth, int queueSize, UBaseType_t priority)

Lorem Ipsum

Return: Task Object

#### BaseType_t runCore(UBaseType_t core, TaskFunction_t child, uint32_t stackDepth, int queueSize, UBaseType_t priority)

Lorem Ipsum

Return: Task Object

#### void stop()

Lorem Ipsum

Return: none

#### const char* cmd(const char* command, TickType_t timeout)

Lorem Ipsum

Return: none

#### virtual uint8_t interface(rapidFunction incoming, char* messageBuffer)

Lorem Ipsum

Return: 0 = pass | 1 = fail

## License

[GNU License](https://github.com/PCLabTools/rapidRTOS/blob/main/LICENSE)
