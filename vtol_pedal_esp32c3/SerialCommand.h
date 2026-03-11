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
  RudderSettings* _settings;
  Preferences prefs;

  static const int INPUT_BUFFER_SIZE = 128;
  static const int MAX_TOKENS = 8;

  char inputBuffer[INPUT_BUFFER_SIZE];
  int inputIndex = 0;

  static const CommandEntry commands[];
  static const int numCommands;

  void handleLine(char* input);
  const CommandEntry* findCommand(const char* name);
  int tokenize(char* input, char** argv, int maxTokens);
  void toLowerInPlace(char* text);

  void cmdHelp(int argc, char** argv);
  void cmdStatus(int argc, char** argv);
  void cmdMode(int argc, char** argv);
  void cmdDebug(int argc, char** argv);
  void cmdTest(int argc, char** argv);
  void cmdSet(int argc, char** argv);
  void cmdSave(int argc, char** argv);
  void cmdFactoryReset(int argc, char** argv);

  void loadPreferences();
  void persistInt(const char* key, int value);
  bool parseIntStrict(const char* text, int minValue, int maxValue, int& out);
  bool validateSettings(const RudderSettings& candidate, String& reason);
  bool expectArgCount(const char* cmd, int argc, int expected);
  void printOk(const String& message);
  void printErr(const String& message);
};

#endif
