// File: A1-Task4-All-20231246-20230545-20227032

// student 1:
// Name: Mohamed Abd El Aziz
// ID: 20231246
// Section: #All

// Student 2:
// Name: Basil Hossam
// ID: 20230545
// Section: #All

//  Student 3:
// Name: Hossam Anwar El sayed 
// ID: 20227032
// Section: #All 

#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <sstream>
#include <iomanip>
#include <algorithm>

class RegisterBank {
private:
    int cells[16];

public:
    RegisterBank() {
        std::fill(std::begin(cells), std::end(cells), 0);
    }

    int readCell(int index) const {
        if (index >= 0 && index < 16) {
            return cells[index];
        }
        std::cerr << "Error: Invalid register index." << std::endl;
        return -1;
    }

    void writeCell(int index, int value) {
        if (index >= 0 && index < 16) {
            cells[index] = value;
        }
        else {
            std::cerr << "Error: Invalid register index." << std::endl;
        }
    }

    void debugPrint() const {
        std::cout << "Register Values: ";
        for (int i = 0; i < 16; ++i) {
            std::cout << cells[i] << " ";
        }
        std::cout << std::endl;
    }
};

class RAM {
public:
    std::string slots[256];

    RAM() {
        for (std::string& slot : slots) {
            slot = "0000";
        }
    }

    std::string readSlot(int index) const {
        if (index >= 0 && index < 256) {
            return slots[index];
        }
        std::cerr << "Error: Memory read out of bounds." << std::endl;
        return "0000";
    }

    void writeSlot(int index, const std::string& data) {
        if (index >= 0 && index < 256) {
            slots[index] = data;
        }
        else {
            std::cerr << "Error: Memory write out of bounds." << std::endl;
        }
    }

    void debugPrint() const {
        for (int i = 0; i < 16; ++i) {
            std::cout << slots[i] << " ";
        }
        std::cout << std::endl;
    }
};

class OutputDevice {
public:
    void output(const std::string& message) {
        std::cout << "Output: " << message << std::endl;
    }
};

class ArithmeticUnit {
public:
    void addInt(int dest, int src1, int src2, RegisterBank& regs) {
        int result = regs.readCell(src1) + regs.readCell(src2);
        regs.writeCell(dest, result);
    }

    void addFloat(int dest, int src1, int src2, RegisterBank& regs) {
        float val1 = static_cast<float>(regs.readCell(src1));
        float val2 = static_cast<float>(regs.readCell(src2));
        float result = val1 + val2;
        regs.writeCell(dest, static_cast<int>(result));
    }
};

class ControlModule {
public:
    void loadFromRAM(int regIndex, int memIndex, RegisterBank& regs, RAM& memory) {
        int val = std::stoi(memory.readSlot(memIndex), nullptr, 16);
        regs.writeCell(regIndex, val);
    }

    void loadImmediate(int regIndex, int value, RegisterBank& regs) {
        regs.writeCell(regIndex, value);
    }

    void storeToRAM(int regIndex, int memIndex, RegisterBank& regs, RAM& memory) {
        std::stringstream ss;
        ss << std::setw(4) << std::setfill('0') << std::hex << regs.readCell(regIndex);
        memory.writeSlot(memIndex, ss.str());
    }

    void transfer(int destIndex, int srcIndex, RegisterBank& regs) {
        int val = regs.readCell(srcIndex);
        regs.writeCell(destIndex, val);
    }

    void jumpIfZero(int regIndex, int target, RegisterBank& regs, int& pc) {
        if (regs.readCell(regIndex) == regs.readCell(0)) {
            pc = target;
        }
    }

    void halt() {
        std::cout << "Execution halted." << std::endl;
        exit(0);
    }

    std::vector<int> parseInstruction(const std::string& instr) {
        std::vector<int> parsed;
        for (char ch : instr) {
            if (isxdigit(ch)) {
                parsed.push_back(ch >= '0' && ch <= '9' ? ch - '0' : ch - 'A' + 10);
            }
        }
        return parsed;
    }
};

class Simulator {
private:
    RegisterBank regs;
    ArithmeticUnit arith;
    ControlModule control;
    RAM memory;
    int pc;
    std::string instruction;

public:
    Simulator() : pc(0) {}

    void loadProgram(const std::string& filePath) {
        std::ifstream file(filePath);
        if (!file) {
            std::cerr << "Error: Unable to open file " << filePath << std::endl;
            return;
        }

        std::string line;
        int address = 0;
        while (std::getline(file, line)) {
            line.erase(0, line.find_first_not_of(" \t\r\n"));
            line.erase(line.find_last_not_of(" \t\r\n") + 1);

            if (line.empty() || line.substr(0, 2) == "//") {
                continue;
            }

            std::istringstream iss(line);
            std::string instruction;
            while (iss >> instruction) {
                if (instruction.size() == 4 && std::all_of(instruction.begin(), instruction.end(), ::isxdigit)) {
                    if (address < 256) {
                        memory.writeSlot(address++, instruction);
                    }
                    else {
                        std::cerr << "Error: Memory overflow." << std::endl;
                        break;
                    }
                }
                else {
                    std::cerr << "Error: Invalid instruction format for '" << instruction << "'" << std::endl;
                }
            }
        }
    }

    void fetchInstruction() {
        if (pc < 256) {
            instruction = memory.readSlot(pc++);
            std::cout << "Instruction fetched: " << instruction << " at PC: " << pc - 1 << std::endl;
        }
        else {
            std::cerr << "Error: PC out of bounds." << std::endl;
        }
    }

    bool executeInstruction();

    void displayState() const {
        std::cout << "PC: " << pc << "\nInstruction: " << instruction << "\nRegisters: ";
        regs.debugPrint();
        std::cout << "Memory: ";
        memory.debugPrint();
    }

    RAM& getMemory() {
        return memory;
    }

    int getPC() const {
        return pc;
    }

    void setPC(int value) {
        pc = value;
    }

    void runSimulation();
};

bool Simulator::executeInstruction() {
    if (instruction.empty()) {
        std::cerr << "Error: No instruction to execute." << std::endl;
        return false;
    }

    auto parsed = control.parseInstruction(instruction);
    if (!parsed.empty()) {
        std::cout << "Parsed Instruction: ";
        for (int val : parsed) {
            std::cout << val << " ";
        }
        std::cout << std::endl;

        switch (parsed[0]) {
        case 1:
            control.loadFromRAM(parsed[1], (parsed[2] << 4) | parsed[3], regs, memory);
            break;
        case 2:
            control.loadImmediate(parsed[1], (parsed[2] << 4) | parsed[3], regs);
            break;
        case 3:
            control.storeToRAM(parsed[1], (parsed[2] << 4) | parsed[3], regs, memory);
            break;
        case 4:
            control.transfer(parsed[1], parsed[2], regs);
            break;
        case 5:
            arith.addInt(parsed[1], parsed[2], parsed[3], regs);
            break;
        case 6:
            arith.addFloat(parsed[1], parsed[2], parsed[3], regs);
            break;
        case 11:
            control.jumpIfZero(parsed[1], (parsed[2] << 4) | parsed[3], regs, pc);
            break;
        case 12:
            control.halt();
            return false;
        default:
            std::cerr << "Error: Invalid opcode." << std::endl;
        }
        return true;
    }
    return false;
}

class SystemInterface {
private:
    Simulator& system;

public:
    SystemInterface(Simulator& sysRef) : system(sysRef) {}

    void displayMenu() const {
        std::cout << "\nOptions:\n"
            << "1. Load Program\n"
            << "2. Run Next Instruction\n"
            << "3. Show System State\n"
            << "4. Exit\n"
            << "Choose an option: ";
    }

    std::string getProgramFile() {
        std::cout << "Enter program file name: ";
        std::string fileName;
        std::cin >> fileName;
        return fileName;
    }

    char getOption() {
        char option;
        std::cin >> option;
        return option;
    }
};

void Simulator::runSimulation() {
    SystemInterface iface(*this);
    bool active = true;
    while (active) {
        iface.displayMenu();
        char option = iface.getOption();
        switch (option) {
        case '1':
            loadProgram(iface.getProgramFile());
            break;
        case '2':
            fetchInstruction();
            active = executeInstruction();
            break;
        case '3':
            displayState();
            break;
        case '4':
            active = false;
            break;
        default:
            std::cout << "Invalid option. Please try again." << std::endl;
        }
    }
}

int main() {
    Simulator simSystem;
    simSystem.runSimulation();
    return 0;
}