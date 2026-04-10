#include "SerialCommand.h"

const CommandEntry SerialCommand::commands[] = {
    {"help", &SerialCommand::cmdHelp},
    {"status", &SerialCommand::cmdStatus},
    {"mode", &SerialCommand::cmdMode},
    {"debug", &SerialCommand::cmdDebug},
    {"test", &SerialCommand::cmdTest},
    {"set", &SerialCommand::cmdSet},
    {"save", &SerialCommand::cmdSave},
    {"factory_reset", &SerialCommand::cmdFactoryReset},
    {"cancel", &SerialCommand::cmdCancel},
    {"reset", &SerialCommand::cmdReset},
    {"ble", &SerialCommand::cmdBle},
    {"hid", &SerialCommand::cmdHid},
    {"curve", &SerialCommand::cmdCurve},
    {"filter", &SerialCommand::cmdFilter},
    {"minrudder_l", &SerialCommand::cmdMinL},
    {"minrudder_r", &SerialCommand::cmdMinR},
    {"maxrudder_l", &SerialCommand::cmdMaxL},
    {"maxrudder_r", &SerialCommand::cmdMaxR},
    {"erange", &SerialCommand::cmdERange}
};

const int SerialCommand::numCommands = sizeof(commands) / sizeof(commands[0]);

SerialCommand::SerialCommand() : _settings(nullptr) {
    pendingSetKey[0] = '\0';
}

void SerialCommand::begin(RudderSettings* settings) {
    _settings = settings;
    loadPreferences();
    Serial.println("OK: interactive CLI ready. Type 'help'.");
}

void SerialCommand::checkSerial() {
    while (Serial.available()) {
        char ch = Serial.read();
        if (discardUntilNewline) {
            if (ch == '\n' || ch == '\r') {
                discardUntilNewline = false;
                inputIndex = 0;
            }
            continue;
        }

        if (ch == '\n' || ch == '\r') {
            inputBuffer[inputIndex] = '\0';
            if (inputIndex > 0) {
                handleInput(inputBuffer);
            }
            inputIndex = 0;
        } else if (inputIndex < INPUT_BUFFER_SIZE - 1) {
            inputBuffer[inputIndex++] = ch;
        } else {
            inputIndex = 0;
            discardUntilNewline = true;
            printErr("input too long");
        }
    }
}

void SerialCommand::handleInput(char* input) {
    char* argv[MAX_TOKENS];
    int argc = tokenize(input, argv, MAX_TOKENS);
    if (argc == 0) {
        return;
    }

    toLowerInPlace(argv[0]);
    if (strcmp(argv[0], "cancel") == 0) {
        cmdCancel(argc, argv);
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

bool SerialCommand::handlePendingInput(int argc, char** argv) {
    if (pendingInput == PendingNone) {
        return false;
    }

    switch (pendingInput) {
        case PendingMode:
            clearPendingInput();
            applyModeValue(argv[0]);
            return true;
        case PendingDebug:
            clearPendingInput();
            applyDebugValue(argv[0]);
            return true;
        case PendingTest:
            clearPendingInput();
            applyTestValue(argv[0]);
            return true;
        case PendingSetKey:
            clearPendingInput();
            if (argc != 1) {
                printErr("set key must be a single token");
                return true;
            }
            toLowerInPlace(argv[0]);
            if (strlen(argv[0]) >= sizeof(pendingSetKey)) {
                printErr("set key too long");
                return true;
            }
            strcpy(pendingSetKey, argv[0]);
            pendingInput = PendingSetValue;
            Serial.printf("OK: enter value for %s:\n", pendingSetKey);
            return true;
        case PendingSetValue:
            clearPendingInput();
            applySetValue(pendingSetKey, argv[0]);
            pendingSetKey[0] = '\0';
            return true;
        default:
            return false;
    }
}

void SerialCommand::clearPendingInput() {
    pendingInput = PendingNone;
}

bool SerialCommand::parseIntStrict(const char* text, int minValue, int maxValue, int& out) {
    if (text == nullptr || *text == '\0') {
        return false;
    }

    char* endPtr = nullptr;
    long parsed = strtol(text, &endPtr, 10);
    if (endPtr == text || *endPtr != '\0') {
        return false;
    }
    if (parsed < minValue || parsed > maxValue) {
        return false;
    }

    out = (int)parsed;
    return true;
}

bool SerialCommand::validateSettings(const RudderSettings& candidate, String& reason) {
    if (candidate.maxRudder_L <= candidate.minRudder_L + candidate.ERange) {
        reason = "maxRudder_L must be > minRudder_L + ERange";
        return false;
    }
    if (candidate.maxRudder_R <= candidate.minRudder_R + candidate.ERange) {
        reason = "maxRudder_R must be > minRudder_R + ERange";
        return false;
    }
    if ((candidate.Curve != 0 && candidate.Curve != 1) ||
        (candidate.Filter != 0 && candidate.Filter != 1)) {
        reason = "Curve and Filter must be 0 or 1";
        return false;
    }
    return true;
}

void SerialCommand::persistSetting(const char* key, int value) {
    prefs.begin("presets", false);
    prefs.putInt(key, value);
    prefs.end();
}

void SerialCommand::persistRuntimeSettings() {
    prefs.begin("presets", false);
    prefs.putInt("minRudder_L", _settings->minRudder_L);
    prefs.putInt("minRudder_R", _settings->minRudder_R);
    prefs.putInt("maxRudder_L", _settings->maxRudder_L);
    prefs.putInt("maxRudder_R", _settings->maxRudder_R);
    prefs.putInt("ERange", _settings->ERange);
    prefs.putInt("MODE", _settings->MODE);
    prefs.putInt("Curve", _settings->Curve);
    prefs.putInt("Filter", _settings->Filter);
    prefs.end();
}

void SerialCommand::printOk(const String& message) {
    Serial.println(String("OK: ") + message);
}

void SerialCommand::printErr(const String& message) {
    Serial.println(String("ERR: ") + message);
}

bool SerialCommand::applyModeValue(const char* modeToken) {
    char token[8];
    strncpy(token, modeToken, sizeof(token) - 1);
    token[sizeof(token) - 1] = '\0';
    toLowerInPlace(token);

    if (strcmp(token, "hid") == 0) {
        _settings->MODE = 1;
        persistSetting("MODE", 1);
        printOk("MODE=HID");
        return true;
    }
    if (strcmp(token, "ble") == 0) {
        _settings->MODE = 0;
        persistSetting("MODE", 0);
        printOk("MODE=BLE");
        return true;
    }

    printErr("mode must be hid|ble");
    return false;
}

bool SerialCommand::applyDebugValue(const char* debugToken) {
    char token[8];
    strncpy(token, debugToken, sizeof(token) - 1);
    token[sizeof(token) - 1] = '\0';
    toLowerInPlace(token);

    if (strcmp(token, "on") == 0) {
        _settings->printControl = true;
        printOk("Debug=ON");
        return true;
    }
    if (strcmp(token, "off") == 0) {
        _settings->printControl = false;
        printOk("Debug=OFF");
        return true;
    }

    printErr("debug must be on|off");
    return false;
}

bool SerialCommand::applyTestValue(const char* testToken) {
    char token[8];
    strncpy(token, testToken, sizeof(token) - 1);
    token[sizeof(token) - 1] = '\0';
    toLowerInPlace(token);

    if (strcmp(token, "on") == 0) {
        _settings->Test = true;
        printOk("Test=ON");
        return true;
    }
    if (strcmp(token, "off") == 0) {
        _settings->Test = false;
        printOk("Test=OFF");
        return true;
    }

    printErr("test must be on|off");
    return false;
}

bool SerialCommand::applySetValue(const char* key, const char* valueToken) {
    int value = 0;
    RudderSettings candidate = *_settings;
    const char* prefKey = nullptr;
    String successKey;

    if (strcmp(key, "curve") == 0) {
        if (!parseIntStrict(valueToken, 0, 1, value)) {
            printErr("curve must be 0|1");
            return false;
        }
        candidate.Curve = value;
        prefKey = "Curve";
        successKey = "Curve";
    } else if (strcmp(key, "filter") == 0) {
        if (!parseIntStrict(valueToken, 0, 1, value)) {
            printErr("filter must be 0|1");
            return false;
        }
        candidate.Filter = value;
        prefKey = "Filter";
        successKey = "Filter";
    } else if (strcmp(key, "min_l") == 0) {
        if (!parseIntStrict(valueToken, 1, 4095, value)) {
            printErr("min_l out of range (1-4095)");
            return false;
        }
        candidate.minRudder_L = value;
        prefKey = "minRudder_L";
        successKey = "minRudder_L";
    } else if (strcmp(key, "min_r") == 0) {
        if (!parseIntStrict(valueToken, 1, 4095, value)) {
            printErr("min_r out of range (1-4095)");
            return false;
        }
        candidate.minRudder_R = value;
        prefKey = "minRudder_R";
        successKey = "minRudder_R";
    } else if (strcmp(key, "max_l") == 0) {
        if (!parseIntStrict(valueToken, 1, 4095, value)) {
            printErr("max_l out of range (1-4095)");
            return false;
        }
        candidate.maxRudder_L = value;
        prefKey = "maxRudder_L";
        successKey = "maxRudder_L";
    } else if (strcmp(key, "max_r") == 0) {
        if (!parseIntStrict(valueToken, 1, 4095, value)) {
            printErr("max_r out of range (1-4095)");
            return false;
        }
        candidate.maxRudder_R = value;
        prefKey = "maxRudder_R";
        successKey = "maxRudder_R";
    } else if (strcmp(key, "erange") == 0) {
        if (!parseIntStrict(valueToken, 1, 4095, value)) {
            printErr("erange out of range (1-4095)");
            return false;
        }
        candidate.ERange = value;
        prefKey = "ERange";
        successKey = "ERange";
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
    persistSetting(prefKey, value);
    printOk(successKey + "=" + String(value));
    return true;
}

void SerialCommand::printStatus() {
    Serial.printf("STATUS: MODE=%s\n", _settings->MODE == 1 ? "HID" : "BLE");
    Serial.printf("STATUS: Curve=%d\n", _settings->Curve);
    Serial.printf("STATUS: Filter=%d\n", _settings->Filter);
    Serial.printf("STATUS: minRudder_L=%d\n", _settings->minRudder_L);
    Serial.printf("STATUS: minRudder_R=%d\n", _settings->minRudder_R);
    Serial.printf("STATUS: maxRudder_L=%d\n", _settings->maxRudder_L);
    Serial.printf("STATUS: maxRudder_R=%d\n", _settings->maxRudder_R);
    Serial.printf("STATUS: ERange=%d\n", _settings->ERange);
    Serial.printf("STATUS: Debug=%s\n", _settings->printControl ? "ON" : "OFF");
    Serial.printf("STATUS: Test=%s\n", _settings->Test ? "ON" : "OFF");
}

void SerialCommand::cmdHelp(int argc, char** argv) {
    (void)argc;
    (void)argv;
    Serial.println("OK: commands");
    Serial.println("  help");
    Serial.println("  status");
    Serial.println("  mode [hid|ble]");
    Serial.println("  debug [on|off]");
    Serial.println("  test [on|off]");
    Serial.println("  set [curve|filter|min_l|min_r|max_l|max_r|erange] [value]");
    Serial.println("  save");
    Serial.println("  factory_reset");
    Serial.println("  cancel");
    Serial.println("OK: legacy aliases");
    Serial.println("  RESET BLE HID Debug Test Curve Filter minRudder_L minRudder_R maxRudder_L maxRudder_R ERange");
}

void SerialCommand::cmdStatus(int argc, char** argv) {
    (void)argc;
    (void)argv;
    printStatus();
}

void SerialCommand::cmdMode(int argc, char** argv) {
    if (argc == 1) {
        pendingInput = PendingMode;
        Serial.println("OK: enter mode (hid/ble):");
        return;
    }
    applyModeValue(argv[1]);
}

void SerialCommand::cmdDebug(int argc, char** argv) {
    if (argc == 1) {
        pendingInput = PendingDebug;
        Serial.println("OK: enter debug (on/off):");
        return;
    }
    applyDebugValue(argv[1]);
}

void SerialCommand::cmdTest(int argc, char** argv) {
    if (argc == 1) {
        pendingInput = PendingTest;
        Serial.println("OK: enter test (on/off):");
        return;
    }
    applyTestValue(argv[1]);
}

void SerialCommand::cmdSet(int argc, char** argv) {
    if (argc == 1) {
        pendingInput = PendingSetKey;
        Serial.println("OK: enter set key (curve/filter/min_l/min_r/max_l/max_r/erange):");
        return;
    }
    if (argc == 2) {
        toLowerInPlace(argv[1]);
        strncpy(pendingSetKey, argv[1], sizeof(pendingSetKey) - 1);
        pendingSetKey[sizeof(pendingSetKey) - 1] = '\0';
        pendingInput = PendingSetValue;
        Serial.printf("OK: enter value for %s:\n", pendingSetKey);
        return;
    }
    toLowerInPlace(argv[1]);
    applySetValue(argv[1], argv[2]);
}

void SerialCommand::cmdSave(int argc, char** argv) {
    (void)argc;
    (void)argv;
    persistRuntimeSettings();
    printOk("settings persisted");
}

void SerialCommand::cmdFactoryReset(int argc, char** argv) {
    (void)argc;
    (void)argv;
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
    prefs.begin("presets", false);
    prefs.putInt("bootCount", 0);
    prefs.end();
    persistRuntimeSettings();
    printOk("factory defaults restored");
}

void SerialCommand::cmdCancel(int argc, char** argv) {
    (void)argc;
    (void)argv;
    clearPendingInput();
    pendingSetKey[0] = '\0';
    printOk("interactive input canceled");
}

void SerialCommand::cmdReset(int argc, char** argv) {
    (void)argc;
    (void)argv;
    prefs.begin("presets", false);
    prefs.putInt("bootCount", 0);
    prefs.end();
    printOk("boot count reset");
}

void SerialCommand::cmdBle(int argc, char** argv) {
    (void)argc;
    (void)argv;
    applyModeValue("ble");
}

void SerialCommand::cmdHid(int argc, char** argv) {
    (void)argc;
    (void)argv;
    applyModeValue("hid");
}

void SerialCommand::cmdCurve(int argc, char** argv) {
    if (argc == 1) {
        pendingInput = PendingSetValue;
        strcpy(pendingSetKey, "curve");
        Serial.println("OK: enter value for curve:");
        return;
    }
    applySetValue("curve", argv[1]);
}

void SerialCommand::cmdFilter(int argc, char** argv) {
    if (argc == 1) {
        pendingInput = PendingSetValue;
        strcpy(pendingSetKey, "filter");
        Serial.println("OK: enter value for filter:");
        return;
    }
    applySetValue("filter", argv[1]);
}

void SerialCommand::cmdMinL(int argc, char** argv) {
    if (argc == 1) {
        pendingInput = PendingSetValue;
        strcpy(pendingSetKey, "min_l");
        Serial.println("OK: enter value for min_l:");
        return;
    }
    applySetValue("min_l", argv[1]);
}

void SerialCommand::cmdMinR(int argc, char** argv) {
    if (argc == 1) {
        pendingInput = PendingSetValue;
        strcpy(pendingSetKey, "min_r");
        Serial.println("OK: enter value for min_r:");
        return;
    }
    applySetValue("min_r", argv[1]);
}

void SerialCommand::cmdMaxL(int argc, char** argv) {
    if (argc == 1) {
        pendingInput = PendingSetValue;
        strcpy(pendingSetKey, "max_l");
        Serial.println("OK: enter value for max_l:");
        return;
    }
    applySetValue("max_l", argv[1]);
}

void SerialCommand::cmdMaxR(int argc, char** argv) {
    if (argc == 1) {
        pendingInput = PendingSetValue;
        strcpy(pendingSetKey, "max_r");
        Serial.println("OK: enter value for max_r:");
        return;
    }
    applySetValue("max_r", argv[1]);
}

void SerialCommand::cmdERange(int argc, char** argv) {
    if (argc == 1) {
        pendingInput = PendingSetValue;
        strcpy(pendingSetKey, "erange");
        Serial.println("OK: enter value for erange:");
        return;
    }
    applySetValue("erange", argv[1]);
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

    prefs.end();
}
