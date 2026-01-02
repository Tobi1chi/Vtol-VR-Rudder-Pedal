#ifndef SERIAL_COMMAND_H
#define SERIAL_COMMAND_H

#include <Arduino.h>
#include <Preferences.h>
#include "RudderConfig.h"

class SerialCommand; // Forward declaration

// Define member function pointer type for command handlers
typedef void (SerialCommand::*CmdHandler)(int value);

struct CommandEntry {
    const char* name;
    CmdHandler handler;
    bool requiresValue; // Does this command require a subsequent parameter input?
};

class SerialCommand {
public:
    SerialCommand();
    void begin(RudderSettings* settings);
    void checkSerial();

private:
    RudderSettings* _settings;
    Preferences prefs;
    
    char inputBuffer[64];
    int inputIndex = 0;
    
    // State machine
    bool waitingForValue = false;
    const CommandEntry* pendingCommand = nullptr;

    // Command Registry
    static const CommandEntry commands[];
    static const int numCommands;

    // Core logic
    void handleInput(const char* input);
    const CommandEntry* findCommand(const char* name);
    
    // Command Handlers
    void cmdReset(int val);
    void cmdBle(int val);
    void cmdHid(int val);
    void cmdDebug(int val);
    void cmdCurve(int val);
    void cmdFilter(int val);
    void cmdMinL(int val);
    void cmdMinR(int val);
    void cmdMaxL(int val);
    void cmdMaxR(int val);
    void cmdERange(int val);
    void cmdTest(int val);
    void cmdHelp(int val);

    // Helpers
    void loadPreferences();
};

#endif
