#include "SerialCommand.h"

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

namespace {
const uint32_t SETTINGS_MAGIC = 0x56544F4CUL; // "VTOL"
const uint16_t SETTINGS_VERSION = 1;

struct StoredSettings {
    uint32_t magic;
    uint16_t version;
    uint16_t reserved;
    int minRudder_L;
    int minRudder_R;
    int maxRudder_L;
    int maxRudder_R;
    int ERange;
    int MODE;
    int Curve;
    int Filter;
};
} // namespace

const CommandEntry SerialCommand::commands[] = {
    {"help", &SerialCommand::cmdHelp},
    {"status", &SerialCommand::cmdStatus},
    {"mode", &SerialCommand::cmdMode},
    {"debug", &SerialCommand::cmdDebug},
    {"test", &SerialCommand::cmdTest},
    {"set", &SerialCommand::cmdSet},
    {"save", &SerialCommand::cmdSave},
    {"factory_reset", &SerialCommand::cmdFactoryReset},
};

const int SerialCommand::numCommands = sizeof(commands) / sizeof(commands[0]);

SerialCommand::SerialCommand()
    : _settings(nullptr), _inputIndex(0), _awaitingModeArg(false),
      _awaitingDebugArg(false), _awaitingTestArg(false), _awaitingSetKey(false),
      _awaitingSetValue(false) {
    _pendingSetKey[0] = '\0';
}

void SerialCommand::begin(RudderSettings* settings) {
    _settings = settings;
    loadDefaults();
    EEPROM.begin(sizeof(StoredSettings));
    loadFromStorage();
    Serial.println("OK: serial CLI ready. Type 'help'.");
}

void SerialCommand::loadDefaults() {
    _settings->minRudder_L = DEFAULT_MIN_L;
    _settings->minRudder_R = DEFAULT_MIN_R;
    _settings->maxRudder_L = DEFAULT_MAX_L;
    _settings->maxRudder_R = DEFAULT_MAX_R;
    _settings->ERange = DEFAULT_ERANGE;
    _settings->MODE = DEFAULT_MODE;
    _settings->Curve = DEFAULT_CURVE;
    _settings->Filter = DEFAULT_FILTER;
    _settings->printControl = false;
    _settings->Test = false;
}

void SerialCommand::loadFromStorage() {
    StoredSettings stored;
    EEPROM.get(0, stored);
    if (stored.magic != SETTINGS_MAGIC || stored.version != SETTINGS_VERSION) {
        printErr("no valid saved settings found; using defaults");
        return;
    }

    RudderSettings candidate = *_settings;
    candidate.minRudder_L = stored.minRudder_L;
    candidate.minRudder_R = stored.minRudder_R;
    candidate.maxRudder_L = stored.maxRudder_L;
    candidate.maxRudder_R = stored.maxRudder_R;
    candidate.ERange = stored.ERange;
    candidate.MODE = stored.MODE;
    candidate.Curve = stored.Curve;
    candidate.Filter = stored.Filter;
    candidate.printControl = false;
    candidate.Test = false;

    String reason;
    if (!validateSettings(candidate, reason)) {
        printErr(String("saved settings invalid: ") + reason + "; using defaults");
        return;
    }

    *_settings = candidate;
    printOk("loaded saved settings");
}

void SerialCommand::persistToStorage() {
    StoredSettings stored;
    stored.magic = SETTINGS_MAGIC;
    stored.version = SETTINGS_VERSION;
    stored.reserved = 0;
    stored.minRudder_L = _settings->minRudder_L;
    stored.minRudder_R = _settings->minRudder_R;
    stored.maxRudder_L = _settings->maxRudder_L;
    stored.maxRudder_R = _settings->maxRudder_R;
    stored.ERange = _settings->ERange;
    stored.MODE = _settings->MODE;
    stored.Curve = _settings->Curve;
    stored.Filter = _settings->Filter;

    EEPROM.put(0, stored);
    EEPROM.commit();
}

void SerialCommand::checkSerial() {
    while (Serial.available()) {
        char ch = Serial.read();
        if (ch == '\n' || ch == '\r') {
            if (_inputIndex > 0) {
                _inputBuffer[_inputIndex] = '\0';
                handleLine(_inputBuffer);
                _inputIndex = 0;
            }
        } else {
            if (_inputIndex < INPUT_BUFFER_SIZE - 1) {
                _inputBuffer[_inputIndex++] = ch;
            } else {
                _inputIndex = 0;
                printErr("input too long; max 127 chars");
            }
        }
    }
}

void SerialCommand::handleLine(char* input) {
    char* argv[MAX_TOKENS];
    int argc = tokenize(input, argv, MAX_TOKENS);
    if (argc == 0) {
        return;
    }

    toLowerInPlace(argv[0]);
    if (strcmp(argv[0], "cancel") == 0) {
        clearInteractiveState();
        printOk("interactive input canceled");
        return;
    }

    if (handlePendingInput(argc, argv)) {
        return;
    }

    const CommandEntry* cmd = findCommand(argv[0]);
    if (cmd == nullptr) {
        printErr("unknown command; type 'help'");
        return;
    }
    (this->*(cmd->handler))(argc, argv);
}

bool SerialCommand::handlePendingInput(int argc, char** argv) {
    if (_awaitingModeArg) {
        _awaitingModeArg = false;
        applyModeValue(argv[0]);
        return true;
    }
    if (_awaitingDebugArg) {
        _awaitingDebugArg = false;
        applyDebugValue(argv[0]);
        return true;
    }
    if (_awaitingTestArg) {
        _awaitingTestArg = false;
        applyTestValue(argv[0]);
        return true;
    }
    if (_awaitingSetKey) {
        _awaitingSetKey = false;
        if (argc != 1) {
            printErr("set key must be a single token");
            return true;
        }
        if (strlen(argv[0]) >= sizeof(_pendingSetKey)) {
            printErr("set key too long");
            return true;
        }
        strcpy(_pendingSetKey, argv[0]);
        _awaitingSetValue = true;
        Serial.printf("OK: enter value for %s:\n", _pendingSetKey);
        return true;
    }
    if (_awaitingSetValue) {
        _awaitingSetValue = false;
        applySetValue(_pendingSetKey, argv[0]);
        _pendingSetKey[0] = '\0';
        return true;
    }
    return false;
}

void SerialCommand::clearInteractiveState() {
    _awaitingModeArg = false;
    _awaitingDebugArg = false;
    _awaitingTestArg = false;
    _awaitingSetKey = false;
    _awaitingSetValue = false;
    _pendingSetKey[0] = '\0';
}

int SerialCommand::tokenize(char* input, char** argv, int maxTokens) {
    int argc = 0;
    char* token = strtok(input, " \t");
    while (token != nullptr && argc < maxTokens) {
        argv[argc++] = token;
        token = strtok(nullptr, " \t");
    }
    return argc;
}

void SerialCommand::toLowerInPlace(char* text) {
    for (int i = 0; text[i] != '\0'; i++) {
        text[i] = (char)tolower((unsigned char)text[i]);
    }
}

const CommandEntry* SerialCommand::findCommand(const char* name) {
    for (int i = 0; i < numCommands; i++) {
        if (strcmp(name, commands[i].name) == 0) {
            return &commands[i];
        }
    }
    return nullptr;
}

bool SerialCommand::parseIntStrict(const char* text, int minValue, int maxValue, int& out) {
    if (text == nullptr || *text == '\0') {
        return false;
    }
    errno = 0;
    char* endPtr = nullptr;
    long parsed = strtol(text, &endPtr, 10);
    if (errno != 0 || endPtr == text || *endPtr != '\0') {
        return false;
    }
    if (parsed < (long)minValue || parsed > (long)maxValue) {
        return false;
    }
    if (parsed < INT_MIN || parsed > INT_MAX) {
        return false;
    }
    out = (int)parsed;
    return true;
}

bool SerialCommand::validateSettings(const RudderSettings& candidate, String& reason) {
    if (candidate.maxRudder_L <= candidate.minRudder_L + candidate.ERange) {
        reason = "max_l must be > min_l + erange";
        return false;
    }
    if (candidate.maxRudder_R <= candidate.minRudder_R + candidate.ERange) {
        reason = "max_r must be > min_r + erange";
        return false;
    }
    if ((candidate.Curve != 0 && candidate.Curve != 1) ||
        (candidate.Filter != 0 && candidate.Filter != 1)) {
        reason = "curve/filter must be 0 or 1";
        return false;
    }
    if (candidate.MODE != MODE_HID) {
        reason = "rp2040 firmware currently supports mode hid only";
        return false;
    }
    return true;
}

bool SerialCommand::applyModeValue(const char* modeToken) {
    char token[8];
    strncpy(token, modeToken, sizeof(token) - 1);
    token[sizeof(token) - 1] = '\0';
    toLowerInPlace(token);
    if (strcmp(token, "hid") != 0) {
        printErr("rp2040 firmware currently supports only 'mode hid'");
        return false;
    }
    _settings->MODE = MODE_HID;
    printOk("MODE=HID");
    return true;
}

bool SerialCommand::applyDebugValue(const char* debugToken) {
    char token[8];
    strncpy(token, debugToken, sizeof(token) - 1);
    token[sizeof(token) - 1] = '\0';
    toLowerInPlace(token);
    if (strcmp(token, "on") == 0) {
        _settings->printControl = true;
    } else if (strcmp(token, "off") == 0) {
        _settings->printControl = false;
    } else {
        printErr("debug must be on|off");
        return false;
    }
    printOk(String("Debug=") + (_settings->printControl ? "ON" : "OFF"));
    return true;
}

bool SerialCommand::applyTestValue(const char* testToken) {
    char token[8];
    strncpy(token, testToken, sizeof(token) - 1);
    token[sizeof(token) - 1] = '\0';
    toLowerInPlace(token);
    if (strcmp(token, "on") == 0) {
        _settings->Test = true;
    } else if (strcmp(token, "off") == 0) {
        _settings->Test = false;
    } else {
        printErr("test must be on|off");
        return false;
    }
    printOk(String("Test=") + (_settings->Test ? "ON" : "OFF"));
    return true;
}

bool SerialCommand::applySetValue(const char* key, const char* valueToken) {
    int value = 0;
    RudderSettings candidate = *_settings;
    if (strcmp(key, "curve") == 0) {
        if (!parseIntStrict(valueToken, 0, 1, value)) {
            printErr("curve must be 0|1");
            return false;
        }
        candidate.Curve = value;
    } else if (strcmp(key, "filter") == 0) {
        if (!parseIntStrict(valueToken, 0, 1, value)) {
            printErr("filter must be 0|1");
            return false;
        }
        candidate.Filter = value;
    } else if (strcmp(key, "min_l") == 0) {
        if (!parseIntStrict(valueToken, SERIAL_PARAM_MIN, SERIAL_PARAM_MAX, value)) {
            printErr("min_l out of range (1-4095)");
            return false;
        }
        candidate.minRudder_L = value;
    } else if (strcmp(key, "min_r") == 0) {
        if (!parseIntStrict(valueToken, SERIAL_PARAM_MIN, SERIAL_PARAM_MAX, value)) {
            printErr("min_r out of range (1-4095)");
            return false;
        }
        candidate.minRudder_R = value;
    } else if (strcmp(key, "max_l") == 0) {
        if (!parseIntStrict(valueToken, SERIAL_PARAM_MIN, SERIAL_PARAM_MAX, value)) {
            printErr("max_l out of range (1-4095)");
            return false;
        }
        candidate.maxRudder_L = value;
    } else if (strcmp(key, "max_r") == 0) {
        if (!parseIntStrict(valueToken, SERIAL_PARAM_MIN, SERIAL_PARAM_MAX, value)) {
            printErr("max_r out of range (1-4095)");
            return false;
        }
        candidate.maxRudder_R = value;
    } else if (strcmp(key, "erange") == 0) {
        if (!parseIntStrict(valueToken, SERIAL_PARAM_MIN, SERIAL_PARAM_MAX, value)) {
            printErr("erange out of range (1-4095)");
            return false;
        }
        candidate.ERange = value;
    } else {
        printErr("unknown set key; use curve|filter|min_l|min_r|max_l|max_r|erange");
        return false;
    }

    String reason;
    if (!validateSettings(candidate, reason)) {
        printErr(reason);
        return false;
    }
    *_settings = candidate;
    printOk("settings updated");
    return true;
}

bool SerialCommand::expectArgCount(const char* cmd, int argc, int expected) {
    if (argc != expected) {
        printErr(String(cmd) + " expects " + String(expected - 1) + " argument(s)");
        return false;
    }
    return true;
}

void SerialCommand::printOk(const String& message) {
    Serial.println(String("OK: ") + message);
}

void SerialCommand::printErr(const String& message) {
    Serial.println(String("ERR: ") + message);
}

void SerialCommand::cmdHelp(int argc, char** argv) {
    (void)argv;
    if (!expectArgCount("help", argc, 1)) {
        return;
    }
    Serial.println("OK: commands");
    Serial.println("  help");
    Serial.println("  status");
    Serial.println("  mode hid");
    Serial.println("  debug on|off");
    Serial.println("  test on|off");
    Serial.println("  set curve 0|1");
    Serial.println("  set filter 0|1");
    Serial.println("  set min_l 1..4095");
    Serial.println("  set min_r 1..4095");
    Serial.println("  set max_l 1..4095");
    Serial.println("  set max_r 1..4095");
    Serial.println("  set erange 1..4095");
    Serial.println("  save");
    Serial.println("  factory_reset");
    Serial.println("  cancel");
    Serial.println("OK: interactive tips");
    Serial.println("  mode        -> prompt for hid");
    Serial.println("  debug       -> prompt for on/off");
    Serial.println("  test        -> prompt for on/off");
    Serial.println("  set         -> prompt for key then value");
    Serial.println("  set min_l   -> prompt for value");
}

void SerialCommand::cmdStatus(int argc, char** argv) {
    (void)argv;
    if (!expectArgCount("status", argc, 1)) {
        return;
    }
    Serial.printf("STATUS: MODE=HID\n");
    Serial.printf("STATUS: Curve=%d\n", _settings->Curve);
    Serial.printf("STATUS: Filter=%d\n", _settings->Filter);
    Serial.printf("STATUS: minRudder_L=%d\n", _settings->minRudder_L);
    Serial.printf("STATUS: minRudder_R=%d\n", _settings->minRudder_R);
    Serial.printf("STATUS: maxRudder_L=%d\n", _settings->maxRudder_L);
    Serial.printf("STATUS: maxRudder_R=%d\n", _settings->maxRudder_R);
    Serial.printf("STATUS: ERange=%d\n", _settings->ERange);
    Serial.printf("STATUS: Test=%s\n", _settings->Test ? "ON" : "OFF");
    Serial.printf("STATUS: Debug=%s\n", _settings->printControl ? "ON" : "OFF");
}

void SerialCommand::cmdMode(int argc, char** argv) {
    if (argc == 1) {
        _awaitingModeArg = true;
        Serial.println("OK: enter mode (hid):");
        return;
    }
    if (argc != 2) {
        printErr("mode expects 0 or 1 argument");
        return;
    }
    applyModeValue(argv[1]);
}

void SerialCommand::cmdDebug(int argc, char** argv) {
    if (argc == 1) {
        _awaitingDebugArg = true;
        Serial.println("OK: enter debug (on/off):");
        return;
    }
    if (argc != 2) {
        printErr("debug expects 0 or 1 argument");
        return;
    }
    applyDebugValue(argv[1]);
}

void SerialCommand::cmdTest(int argc, char** argv) {
    if (argc == 1) {
        _awaitingTestArg = true;
        Serial.println("OK: enter test (on/off):");
        return;
    }
    if (argc != 2) {
        printErr("test expects 0 or 1 argument");
        return;
    }
    applyTestValue(argv[1]);
}

void SerialCommand::cmdSet(int argc, char** argv) {
    if (argc == 1) {
        _awaitingSetKey = true;
        Serial.println("OK: enter set key (curve/filter/min_l/min_r/max_l/max_r/erange):");
        return;
    }
    toLowerInPlace(argv[1]);
    if (argc == 2) {
        if (strlen(argv[1]) >= sizeof(_pendingSetKey)) {
            printErr("set key too long");
            return;
        }
        strcpy(_pendingSetKey, argv[1]);
        _awaitingSetValue = true;
        Serial.printf("OK: enter value for %s:\n", _pendingSetKey);
        return;
    }
    if (argc != 3) {
        printErr("set expects 0, 1, or 2 arguments");
        return;
    }
    applySetValue(argv[1], argv[2]);
}

void SerialCommand::cmdSave(int argc, char** argv) {
    (void)argv;
    if (!expectArgCount("save", argc, 1)) {
        return;
    }
    persistToStorage();
    printOk("settings persisted to EEPROM");
}

void SerialCommand::cmdFactoryReset(int argc, char** argv) {
    (void)argv;
    if (!expectArgCount("factory_reset", argc, 1)) {
        return;
    }
    loadDefaults();
    persistToStorage();
    printOk("factory defaults restored");
}
