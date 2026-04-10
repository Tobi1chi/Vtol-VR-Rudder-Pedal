#ifndef SERIAL_COMMAND_H
#define SERIAL_COMMAND_H

#include <Arduino.h>
#include <Preferences.h>

#include "RudderConfig.h"

class SerialCommand;
typedef void (SerialCommand::*CmdHandler)(int argc, char** argv);

struct CommandEntry {
    const char* name;
    CmdHandler handler;
};

class SerialCommand {
public:
    SerialCommand();
    void begin(RudderSettings* settings);
    void checkSerial();

private:
    static const int INPUT_BUFFER_SIZE = 96;
    static const int MAX_TOKENS = 4;

    enum PendingInput {
        PendingNone,
        PendingMode,
        PendingDebug,
        PendingTest,
        PendingSetKey,
        PendingSetValue
    };

    RudderSettings* _settings;
    Preferences prefs;

    char inputBuffer[INPUT_BUFFER_SIZE];
    int inputIndex = 0;
    bool discardUntilNewline = false;
    PendingInput pendingInput = PendingNone;
    char pendingSetKey[16];

    static const CommandEntry commands[];
    static const int numCommands;

    void handleInput(char* input);
    int tokenize(char* input, char** argv, int maxTokens);
    void toLowerInPlace(char* text);
    const CommandEntry* findCommand(const char* name);
    bool handlePendingInput(int argc, char** argv);
    void clearPendingInput();

    bool parseIntStrict(const char* text, int minValue, int maxValue, int& out);
    bool validateSettings(const RudderSettings& candidate, String& reason);
    void persistSetting(const char* key, int value);
    void persistRuntimeSettings();
    void printOk(const String& message);
    void printErr(const String& message);

    bool applyModeValue(const char* modeToken);
    bool applyDebugValue(const char* debugToken);
    bool applyTestValue(const char* testToken);
    bool applySetValue(const char* key, const char* valueToken);
    void printStatus();

    void cmdHelp(int argc, char** argv);
    void cmdStatus(int argc, char** argv);
    void cmdMode(int argc, char** argv);
    void cmdDebug(int argc, char** argv);
    void cmdTest(int argc, char** argv);
    void cmdSet(int argc, char** argv);
    void cmdSave(int argc, char** argv);
    void cmdFactoryReset(int argc, char** argv);
    void cmdCancel(int argc, char** argv);

    void cmdReset(int argc, char** argv);
    void cmdBle(int argc, char** argv);
    void cmdHid(int argc, char** argv);
    void cmdCurve(int argc, char** argv);
    void cmdFilter(int argc, char** argv);
    void cmdMinL(int argc, char** argv);
    void cmdMinR(int argc, char** argv);
    void cmdMaxL(int argc, char** argv);
    void cmdMaxR(int argc, char** argv);
    void cmdERange(int argc, char** argv);

    void loadPreferences();
};

#endif
