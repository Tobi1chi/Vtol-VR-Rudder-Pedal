#include "SerialCommand.h"

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

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

SerialCommand::SerialCommand() {}

void SerialCommand::begin(RudderSettings* settings) {
    _settings = settings;
    loadPreferences();
    Serial.println("OK: serial CLI ready. Type 'help'.");
}

void SerialCommand::checkSerial() {
    while (Serial.available()) {
        char ch = Serial.read();
        if (ch == '\n' || ch == '\r') {
            if (inputIndex > 0) {
                inputBuffer[inputIndex] = '\0';
                handleLine(inputBuffer);
                inputIndex = 0;
            }
        } else {
            if (inputIndex < INPUT_BUFFER_SIZE - 1) {
                inputBuffer[inputIndex++] = ch;
            } else {
                inputIndex = 0;
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
    const CommandEntry* cmd = findCommand(argv[0]);
    if (cmd == nullptr) {
        printErr("unknown command; type 'help'");
        return;
    }
    (this->*(cmd->handler))(argc, argv);
}

const CommandEntry* SerialCommand::findCommand(const char* name) {
    for (int i = 0; i < numCommands; i++) {
        if (strcmp(name, commands[i].name) == 0) {
            return &commands[i];
        }
    }
    return nullptr;
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

void SerialCommand::cmdHelp(int argc, char** argv) {
    (void)argv;
    if (!expectArgCount("help", argc, 1)) {
        return;
    }

    Serial.println("OK: commands");
    Serial.println("  help");
    Serial.println("  status");
    Serial.println("  mode hid|ble");
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
}

void SerialCommand::cmdStatus(int argc, char** argv) {
    (void)argv;
    if (!expectArgCount("status", argc, 1)) {
        return;
    }

    Serial.printf("STATUS: MODE=%s\n", _settings->MODE == MODE_HID ? "HID" : "BLE");
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
    if (!expectArgCount("mode", argc, 2)) {
        return;
    }

    toLowerInPlace(argv[1]);
    int targetMode = -1;
    if (strcmp(argv[1], "hid") == 0) {
        targetMode = MODE_HID;
    } else if (strcmp(argv[1], "ble") == 0) {
        targetMode = MODE_BLE;
    } else {
        printErr("mode must be hid|ble");
        return;
    }

    _settings->MODE = targetMode;
    persistInt("MODE", targetMode);
    printOk(String("MODE=") + (targetMode == MODE_HID ? "HID" : "BLE") + " persisted");
}

void SerialCommand::cmdDebug(int argc, char** argv) {
    if (!expectArgCount("debug", argc, 2)) {
        return;
    }

    toLowerInPlace(argv[1]);
    if (strcmp(argv[1], "on") == 0) {
        _settings->printControl = true;
    } else if (strcmp(argv[1], "off") == 0) {
        _settings->printControl = false;
    } else {
        printErr("debug must be on|off");
        return;
    }

    printOk(String("Debug=") + (_settings->printControl ? "ON" : "OFF"));
}

void SerialCommand::cmdTest(int argc, char** argv) {
    if (!expectArgCount("test", argc, 2)) {
        return;
    }

    toLowerInPlace(argv[1]);
    if (strcmp(argv[1], "on") == 0) {
        _settings->Test = true;
    } else if (strcmp(argv[1], "off") == 0) {
        _settings->Test = false;
    } else {
        printErr("test must be on|off");
        return;
    }

    printOk(String("Test=") + (_settings->Test ? "ON" : "OFF"));
}

void SerialCommand::cmdSet(int argc, char** argv) {
    if (!expectArgCount("set", argc, 3)) {
        return;
    }

    toLowerInPlace(argv[1]);
    int value = 0;
    RudderSettings candidate = *_settings;
    const char* prefKey = nullptr;
    String successKey;

    if (strcmp(argv[1], "curve") == 0) {
        if (!parseIntStrict(argv[2], 0, 1, value)) {
            printErr("curve must be 0|1");
            return;
        }
        candidate.Curve = value;
        prefKey = "Curve";
        successKey = "Curve";
    } else if (strcmp(argv[1], "filter") == 0) {
        if (!parseIntStrict(argv[2], 0, 1, value)) {
            printErr("filter must be 0|1");
            return;
        }
        candidate.Filter = value;
        prefKey = "Filter";
        successKey = "Filter";
    } else if (strcmp(argv[1], "min_l") == 0) {
        if (!parseIntStrict(argv[2], SERIAL_PARAM_MIN, SERIAL_PARAM_MAX, value)) {
            printErr("min_l out of range (1-4095)");
            return;
        }
        candidate.minRudder_L = value;
        prefKey = "minRudder_L";
        successKey = "minRudder_L";
    } else if (strcmp(argv[1], "min_r") == 0) {
        if (!parseIntStrict(argv[2], SERIAL_PARAM_MIN, SERIAL_PARAM_MAX, value)) {
            printErr("min_r out of range (1-4095)");
            return;
        }
        candidate.minRudder_R = value;
        prefKey = "minRudder_R";
        successKey = "minRudder_R";
    } else if (strcmp(argv[1], "max_l") == 0) {
        if (!parseIntStrict(argv[2], SERIAL_PARAM_MIN, SERIAL_PARAM_MAX, value)) {
            printErr("max_l out of range (1-4095)");
            return;
        }
        candidate.maxRudder_L = value;
        prefKey = "maxRudder_L";
        successKey = "maxRudder_L";
    } else if (strcmp(argv[1], "max_r") == 0) {
        if (!parseIntStrict(argv[2], SERIAL_PARAM_MIN, SERIAL_PARAM_MAX, value)) {
            printErr("max_r out of range (1-4095)");
            return;
        }
        candidate.maxRudder_R = value;
        prefKey = "maxRudder_R";
        successKey = "maxRudder_R";
    } else if (strcmp(argv[1], "erange") == 0) {
        if (!parseIntStrict(argv[2], SERIAL_PARAM_MIN, SERIAL_PARAM_MAX, value)) {
            printErr("erange out of range (1-4095)");
            return;
        }
        candidate.ERange = value;
        prefKey = "ERange";
        successKey = "ERange";
    } else {
        printErr("unknown set key; use curve|filter|min_l|min_r|max_l|max_r|erange");
        return;
    }

    String reason;
    if (!validateSettings(candidate, reason)) {
        printErr(reason);
        return;
    }

    *_settings = candidate;
    persistInt(prefKey, value);
    printOk(successKey + "=" + String(value) + " persisted");
}

void SerialCommand::cmdSave(int argc, char** argv) {
    (void)argv;
    if (!expectArgCount("save", argc, 1)) {
        return;
    }
    printOk("save is not required; values persist immediately");
}

void SerialCommand::cmdFactoryReset(int argc, char** argv) {
    (void)argv;
    if (!expectArgCount("factory_reset", argc, 1)) {
        return;
    }

    prefs.begin("presets", false);
    prefs.putInt("minRudder_L", DEFAULT_MIN_L);
    prefs.putInt("minRudder_R", DEFAULT_MIN_R);
    prefs.putInt("maxRudder_L", DEFAULT_MAX_L);
    prefs.putInt("maxRudder_R", DEFAULT_MAX_R);
    prefs.putInt("ERange", DEFAULT_ERANGE);
    prefs.putInt("MODE", DEFAULT_MODE);
    prefs.putInt("Curve", DEFAULT_CURVE);
    prefs.putInt("Filter", DEFAULT_FILTER);
    prefs.end();

    loadPreferences();
    printOk("factory defaults restored and persisted");
}

void SerialCommand::loadPreferences() {
    prefs.begin("presets", false);

    int bootCount = prefs.getInt("bootCount", 0);
    if (bootCount == 0) {
        prefs.putInt("minRudder_L", DEFAULT_MIN_L);
        prefs.putInt("minRudder_R", DEFAULT_MIN_R);
        prefs.putInt("maxRudder_L", DEFAULT_MAX_L);
        prefs.putInt("maxRudder_R", DEFAULT_MAX_R);
        prefs.putInt("ERange", DEFAULT_ERANGE);
        prefs.putInt("MODE", DEFAULT_MODE);
        prefs.putInt("Curve", DEFAULT_CURVE);
        prefs.putInt("Filter", DEFAULT_FILTER);
    }
    prefs.putInt("bootCount", bootCount + 1);

    _settings->minRudder_L = prefs.getInt("minRudder_L", DEFAULT_MIN_L);
    _settings->minRudder_R = prefs.getInt("minRudder_R", DEFAULT_MIN_R);
    _settings->maxRudder_L = prefs.getInt("maxRudder_L", DEFAULT_MAX_L);
    _settings->maxRudder_R = prefs.getInt("maxRudder_R", DEFAULT_MAX_R);
    _settings->ERange = prefs.getInt("ERange", DEFAULT_ERANGE);
    _settings->MODE = prefs.getInt("MODE", DEFAULT_MODE);
    _settings->Curve = prefs.getInt("Curve", DEFAULT_CURVE);
    _settings->Filter = prefs.getInt("Filter", DEFAULT_FILTER);
    _settings->printControl = false;
    _settings->Test = false;

    String reason;
    if (!validateSettings(*_settings, reason)) {
        _settings->minRudder_L = DEFAULT_MIN_L;
        _settings->minRudder_R = DEFAULT_MIN_R;
        _settings->maxRudder_L = DEFAULT_MAX_L;
        _settings->maxRudder_R = DEFAULT_MAX_R;
        _settings->ERange = DEFAULT_ERANGE;
        _settings->MODE = DEFAULT_MODE;
        _settings->Curve = DEFAULT_CURVE;
        _settings->Filter = DEFAULT_FILTER;

        prefs.putInt("minRudder_L", _settings->minRudder_L);
        prefs.putInt("minRudder_R", _settings->minRudder_R);
        prefs.putInt("maxRudder_L", _settings->maxRudder_L);
        prefs.putInt("maxRudder_R", _settings->maxRudder_R);
        prefs.putInt("ERange", _settings->ERange);
        prefs.putInt("MODE", _settings->MODE);
        prefs.putInt("Curve", _settings->Curve);
        prefs.putInt("Filter", _settings->Filter);
        printErr(String("stored settings invalid: ") + reason + "; restored defaults");
    }

    if (_settings->MODE != MODE_BLE && _settings->MODE != MODE_HID) {
        _settings->MODE = DEFAULT_MODE;
        prefs.putInt("MODE", _settings->MODE);
    }
    if (_settings->Curve < 0 || _settings->Curve > 1) {
        _settings->Curve = DEFAULT_CURVE;
        prefs.putInt("Curve", _settings->Curve);
    }
    if (_settings->Filter < 0 || _settings->Filter > 1) {
        _settings->Filter = DEFAULT_FILTER;
        prefs.putInt("Filter", _settings->Filter);
    }

    prefs.end();
}

void SerialCommand::persistInt(const char* key, int value) {
    prefs.begin("presets", false);
    prefs.putInt(key, value);
    prefs.end();
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
    if (candidate.MODE != MODE_BLE && candidate.MODE != MODE_HID) {
        reason = "mode must be 0 or 1";
        return false;
    }
    if ((candidate.Curve != 0 && candidate.Curve != 1) ||
        (candidate.Filter != 0 && candidate.Filter != 1)) {
        reason = "curve/filter must be 0 or 1";
        return false;
    }
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
