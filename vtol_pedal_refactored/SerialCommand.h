#ifndef SERIAL_COMMAND_H
#define SERIAL_COMMAND_H

#include <Arduino.h>

#include "BoardCliConfig.h"
#include "RudderConfig.h"

#if CLI_STORAGE_BACKEND_PREFERENCES
#include <Preferences.h>
#elif CLI_STORAGE_BACKEND_EEPROM
#include <EEPROM.h>
#endif

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
  static const int INPUT_BUFFER_SIZE = 128;
  static const int MAX_TOKENS = 8;

  RudderSettings* _settings;
#if CLI_STORAGE_BACKEND_PREFERENCES
  Preferences _prefs;
#endif
  char _inputBuffer[INPUT_BUFFER_SIZE];
  int _inputIndex;
  bool _discardUntilNewline;
  bool _awaitingModeArg;
  bool _awaitingDebugArg;
  bool _awaitingTestArg;
  bool _awaitingSetKey;
  bool _awaitingSetValue;
  char _pendingSetKey[16];

  static const CommandEntry commands[];
  static const int numCommands;

  void loadDefaults();
  void loadFromStorage();
  void persistRuntimeSettings();
  void persistSetting(const char* key, int value);
  void restoreFactoryDefaults();

  void handleLine(char* input);
  bool handlePendingInput(int argc, char** argv);
  void clearInteractiveState();
  int tokenize(char* input, char** argv, int maxTokens);
  void toLowerInPlace(char* text);
  const CommandEntry* findCommand(const char* name);
  bool parseIntStrict(const char* text, int minValue, int maxValue, int& out);
  bool validateSettings(const RudderSettings& candidate, String& reason);
  bool applyModeValue(const char* modeToken);
  bool applyDebugValue(const char* debugToken);
  bool applyTestValue(const char* testToken);
  bool applySetValue(const char* key, const char* valueToken);
  bool expectArgCount(const char* cmd, int argc, int expected);
  bool isModeSupported(int mode) const;
  const char* modeLabel(int mode) const;
  const char* disabledModeReason(int mode) const;
  void printBoardStatusNotices() const;
  void printOk(const String& message);
  void printErr(const String& message);

  void cmdHelp(int argc, char** argv);
  void cmdStatus(int argc, char** argv);
  void cmdMode(int argc, char** argv);
  void cmdDebug(int argc, char** argv);
  void cmdTest(int argc, char** argv);
  void cmdSet(int argc, char** argv);
  void cmdSave(int argc, char** argv);
  void cmdFactoryReset(int argc, char** argv);
};

#endif
