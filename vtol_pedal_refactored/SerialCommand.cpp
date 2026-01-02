#include "SerialCommand.h"

// Register commands: Name | Handler Function | Requires Parameter
const CommandEntry SerialCommand::commands[] = {
    {"RESET",       &SerialCommand::cmdReset,   false},
    {"BLE",         &SerialCommand::cmdBle,     false},
    {"HID",         &SerialCommand::cmdHid,     false},
    {"Debug",       &SerialCommand::cmdDebug,   false},
    {"Curve",       &SerialCommand::cmdCurve,   true},
    {"Filter",      &SerialCommand::cmdFilter,  true},
    {"minRudder_L", &SerialCommand::cmdMinL,    true},
    {"minRudder_R", &SerialCommand::cmdMinR,    true},
    {"maxRudder_L", &SerialCommand::cmdMaxL,    true},
    {"maxRudder_R", &SerialCommand::cmdMaxR,    true},
    {"ERange",      &SerialCommand::cmdERange,  true},
    {"Test",        &SerialCommand::cmdTest,    false}
};

const int SerialCommand::numCommands = sizeof(commands) / sizeof(commands[0]);

SerialCommand::SerialCommand() {}

void SerialCommand::begin(RudderSettings* settings) {
    _settings = settings;
    loadPreferences();
}

void SerialCommand::checkSerial() {
    while (Serial.available()) {
        char ch = Serial.read();
        if (ch == '\n' || ch == '\r') {
            inputBuffer[inputIndex] = '\0';
            if (inputIndex > 0) { // Ignore empty lines
                handleInput(inputBuffer);
            }
            inputIndex = 0;
        } else {
            if (inputIndex < sizeof(inputBuffer) - 1) {
                inputBuffer[inputIndex++] = ch;
            }
        }
    }
}

void SerialCommand::handleInput(const char* input) {
    // State 1: Waiting for parameter value
    if (waitingForValue && pendingCommand != nullptr) {
        int value = atoi(input);
        // Call the stored handler
        (this->*(pendingCommand->handler))(value);
        
        // Reset state
        waitingForValue = false;
        pendingCommand = nullptr;
        return;
    }

    // State 2: Parsing new command
    const CommandEntry* cmd = findCommand(input);
    if (cmd) {
        if (cmd->requiresValue) {
            Serial.printf("Please input value for %s:\n", cmd->name);
            waitingForValue = true;
            pendingCommand = cmd;
        } else {
            // No parameter needed, execute immediately with dummy value
            (this->*(cmd->handler))(0);
        }
    } else {
        Serial.println("Match Failed: ERROR");
    }
}

const CommandEntry* SerialCommand::findCommand(const char* name) {
    for (int i = 0; i < numCommands; i++) {
        if (strcmp(name, commands[i].name) == 0) {
            Serial.print("Match Success: ");
            Serial.println(commands[i].name);
            return &commands[i];
        }
    }
    return nullptr;
}

// --- Command Implementations ---

void SerialCommand::cmdReset(int val) {
    prefs.begin("presets", false);
    prefs.putInt("bootCount", 0);
    prefs.end();
    Serial.println("Boot count reset");
}

void SerialCommand::cmdBle(int val) {
    _settings->MODE = 0;
    prefs.begin("presets", false);
    prefs.putInt("MODE", 0);
    prefs.end();
    Serial.println("Switched to BLE Mode");
}

void SerialCommand::cmdHid(int val) {
    _settings->MODE = 1;
    prefs.begin("presets", false);
    prefs.putInt("MODE", 1);
    prefs.end();
    Serial.println("Switched to HID Mode");
}

void SerialCommand::cmdDebug(int val) {
    _settings->printControl = !_settings->printControl;
    Serial.println(_settings->printControl ? "Debug ON" : "Debug OFF");
}

void SerialCommand::cmdTest(int val) {
    _settings->Test = !_settings->Test;
    Serial.println(_settings->Test ? "Test Mode ON" : "Test Mode OFF");
}

void SerialCommand::cmdCurve(int val) {
    if (val > 0 && val < 4096) {
        _settings->Curve = val;
        prefs.begin("presets", false);
        prefs.putInt("Curve", val);
        prefs.end();
        Serial.printf("Curve set to %d\n", val);
    }
}

void SerialCommand::cmdFilter(int val) {
    if (val > 0 && val < 4096) {
        _settings->Filter = val;
        prefs.begin("presets", false);
        prefs.putInt("Filter", val);
        prefs.end();
        Serial.printf("Filter set to %d\n", val);
    }
}

void SerialCommand::cmdMinL(int val) {
    if (val > 0 && val < 4096) {
        _settings->minRudder_L = val;
        prefs.begin("presets", false);
        prefs.putInt("minRudder_L", val);
        prefs.end();
        Serial.printf("minRudder_L set to %d\n", val);
    }
}

void SerialCommand::cmdMinR(int val) {
    if (val > 0 && val < 4096) {
        _settings->minRudder_R = val;
        prefs.begin("presets", false);
        prefs.putInt("minRudder_R", val);
        prefs.end();
        Serial.printf("minRudder_R set to %d\n", val);
    }
}

void SerialCommand::cmdMaxL(int val) {
    if (val > 0 && val < 4096) {
        _settings->maxRudder_L = val;
        prefs.begin("presets", false);
        prefs.putInt("maxRudder_L", val);
        prefs.end();
        Serial.printf("maxRudder_L set to %d\n", val);
    }
}

void SerialCommand::cmdMaxR(int val) {
    if (val > 0 && val < 4096) {
        _settings->maxRudder_R = val;
        prefs.begin("presets", false);
        prefs.putInt("maxRudder_R", val);
        prefs.end();
        Serial.printf("maxRudder_R set to %d\n", val);
    }
}

void SerialCommand::cmdERange(int val) {
    if (val > 0 && val < 4096) {
        _settings->ERange = val;
        prefs.begin("presets", false);
        prefs.putInt("ERange", val);
        prefs.end();
        Serial.printf("ERange set to %d\n", val);
    }
}

void SerialCommand::cmdHelp(int val) {
    Serial.println("\n--- Available Commands ---");
    for (int i = 0; i < numCommands; i++) {
        Serial.printf("%-15s", commands[i].name);
        if (commands[i].requiresValue) {
            Serial.println(" [value]");
        } else {
            Serial.println("");
        }
    }
    Serial.println("--------------------------");
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
