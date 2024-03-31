
/*
 *  WhatsAScript compiler + standard library
    (c) Anpang54 me.anpang.fun
    Licensed under GPL 3.0 (see LICENSE.md)
*/

//* setup

// headers

#ifdef _WIN32
    #define _USE_MATH_DEFINES
#endif

#include <algorithm>
#include <any>
#include <bitset>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <optional>
#include <thread>
#include <sstream>
#include <string>
#include <vector>

#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <cstring>

using std::any_cast, std::bitset, std::cout, std::ios, std::string, std::vector;

// version
const uint8_t  version[3]  = {0, 1, 0};
const string   versionDate = "31 Mar 2024";

// macros and typedefs
size_t ri;
#define repeat(n)    for(ri = 0; ri < n; ++ri)

#define C_ERROR      "\033[38;5;202m"
#define C_INFO       "\033[38;5;220m"
#define C_SUCCESS    "\033[38;5;40m"
#define C_CLEAR      "\033[0m"

#ifdef _WIN32
    #define clearTerm() system("cls")
#else
    #define clearTerm() system("clear")
#endif

typedef vector<uint8_t>    bytecode_t;

//* actions

// compiler
enum byteOperations: uint8_t{
    oPrint   = 0x20,
    oInput   = 0x21,
    oGetChar = 0x22,
    oPush    = 0x40
};
enum byteTypes: uint8_t{
    tNull    = 0x10,
    tString  = 0x20,
    tChar    = 0x22,
    tInt32   = 0x40,
    tUInt32  = 0x41,
    tFloat   = 0x48,
    tBool    = 0x50
};
#define End 0x00

struct lObject{    // language object
    byteTypes type;
    std::any  object;
};

enum tokenTypes: uint8_t{
    kName,
    kNull,
    kString,
    kChar,
    kInt32,
    kUInt32,
    kFloat,
    kBool,
    kLeftPar,
    kRightPar
};
struct token{
    tokenTypes type;
    std::any   value;
};

string     code;
bytecode_t compiled;

lObject parse(vector<token> tokens) {
    
    //! DO LATER
    return {tNull};

}

void compileCode() {

    // tokenize
    vector<token> tokens;
    string        nameCache, stringCache;
    bool          addedToNameCache;

    for(unsigned int i = 0; i < code.length(); ++i) {

        if(code[i] == ' ' || code[i] == '\n') {
            // do nothing
        } else if(code[i] == '(') {
            tokens.push_back({kLeftPar});
        } else if(code[i] == ')') {
            tokens.push_back({kRightPar});
        } else if(isdigit(code[i]) || code[i] == '.' || code[i] == '-') {
            while(isdigit(code[i]) || code[i] == '.' || code[i] == '-') {
                stringCache.push_back(code[i]);
                ++i;
            }
            if(stringCache.contains('.')) {
                tokens.push_back({kFloat, stod(stringCache)});
            } else if(stringCache[0] == '-') {
                tokens.push_back({kInt32, (int32_t)stoi(stringCache)});
            } else {
                tokens.push_back({kUInt32, (uint32_t)stoi(stringCache)});
            }
            stringCache.clear();
        } else if(code[i] == '"') {
            ++i;
            while(code[i] != '"') {
                if(code[i] == '\\') {
                    ++i;
                    switch(code[i]) {
                        case 'n':
                            stringCache.push_back('\n');
                    }
                } else {
                    stringCache.push_back(code[i]);
                }
                ++i;
            }
            tokens.push_back({kString, stringCache});
            stringCache.clear();
        } else if(code.substr(i, 4) == "true") {
            tokens.push_back({kBool, true});
        } else {
            nameCache.push_back(code[i]);
        }
    }

    // parse
    std::optional<string> title, description;
    bool                  showTitle, showDescription;
    bytecode_t            parsed;

    for(unsigned int i = 0; i < tokens.size(); ++i) {
        switch(tokens[i].type) {
            case kName:
                nameCache = any_cast<string>(tokens[i].value);
                break;
            case kString:
                parsed.push_back(tString);
                for(const char thisChar: any_cast<string>(tokens[i].value)) {
                    parsed.push_back(thisChar);
                }
                parsed.push_back(End);
                break;
            case kUInt32: {
                parsed.push_back(tUInt32);

                uint32_t number = any_cast<uint32_t>(tokens[i].value);
                parsed.insert(parsed.end(), {
                    (uint8_t)(number >> 24), (uint8_t)(number >> 16), (uint8_t)(number >> 8), (uint8_t)number
                });
                break;
            } case kLeftPar:
                if(nameCache == "print") {
                    parsed.push_back(oPrint);
                } else {
                    cout << C_ERROR << "Unknown function \"" << nameCache << "\".\n" << C_CLEAR;
                }
                break;
        }
    }

    // put it all together
    compiled = {
        'W', 'A', 'E',                         // magic number
        version[0], version[1], version[2],    // version number
        0, 0, 0, 0                             // configs, metadata configs
    };
    
    bitset<8> metadataConfigs(0);
    if(title.has_value()) {
        metadataConfigs[7] = 1;
        for(char letter: title.value()) {
            compiled.push_back(letter);
        }
        compiled.push_back(End);
        if(showTitle) {
            metadataConfigs[4] = 1;
        }
    }
    if(description.has_value()) {
        metadataConfigs[3] = 1;
        for(char letter: description.value()) {
            compiled.push_back(letter);
        }
        compiled.push_back(End);
        if(showDescription) {
            metadataConfigs[0] = 1;
        }
    }
    compiled[9] = metadataConfigs.to_ulong();

    compiled.insert(compiled.end(), parsed.begin(), parsed.end());

}

// runner
bytecode_t bytecode;

void runBytecode() {

    string title;
    string description;

    // check magic number
    if(bytecode[0] != 'W' || bytecode[1] != 'A' || bytecode[2] != 'E') {
        cout << C_ERROR << "Magic number not found. Are you sure this is a valid .wae file?\n" << C_CLEAR;
        exit(1);
    }

    // check version
    if(bytecode[3] != version[0] || bytecode[4] != version[1] || bytecode[5] != version[2]) {
        cout << C_INFO
             << "The file version is " << (int)bytecode[3] << '.' << (int)bytecode[4] << '.' << (int)bytecode[5]
             << ", while the WhatsAScript version is " << (int)version[0] << '.' << (int)version[1] << '.' << (int)version[2]
             << ".\nRun anyway? [ ]";

        char confirm = tolower(getchar());
        cout << "\b\b\b" << confirm << ']' << C_CLEAR;
        if(!(confirm == '1' || confirm == 'y' || confirm == 'r' || confirm == 'e')) {
            cout << C_ERROR << "Cancelled.\n" << C_CLEAR;
            exit(1);
        }
    }

    // check configs
    bitset<24> configs((bytecode[6] << 16) | (bytecode[7] << 8) | bytecode[8]);

    // get metadata
    bitset<8> metaConfigs(bytecode[9]);
    unsigned int i;

    if(metaConfigs[7]) {                         // has title
        if(metaConfigs[6]) {                     // unicode title
            cout << C_ERROR << "WhatsAScript currently doesn't support Unicode titles.\n" << C_CLEAR;
            exit(1);
        } else {
            for(i = 10; bytecode[i] != 0; ++i) {
                title.push_back(bytecode[i]);
            }
        }
        if(metaConfigs[4]) {                      // show title
            cout << "\033[1m\u2506 " << title << "\033[0m\n";
        }
    }
    if(metaConfigs[3]) {                         // has description
        if(metaConfigs[2]) {                     // unicode description
            cout << C_ERROR << "WhatsAScript currently doesn't support Unicode descriptions.\n" << C_CLEAR;
            exit(1);
        } else {
            for(i = 11 + title.length(); bytecode[i] != 0; ++i) {
                description.push_back(bytecode[i]);
            }
        }
        if(metaConfigs[0]) {                     // show description
            cout << "\u2506 " << description << '\n';
        }
    }

    // RUN
    i = 10 + (title.length() == 0? 0: title.length() + 1) + (description.length() == 0? 0: description.length() + 1);
    string cache;

    while(i < bytecode.size()) {
        switch(bytecode[i]) {
            case oPrint: {
                ++i;
                switch(bytecode[i]) {
                    case tChar:
                        cout << (char)i << '\n';
                        break;
                    case tString:
                        cache.clear();
                        while(bytecode[i] != 0) {
                            ++i;
                            cache.push_back((char)bytecode[i]);
                        }
                        cout << cache << '\n';
                        break;
                    case tUInt32: {
                        ++i;
                        uint32_t number = (bytecode[i] << 24) | (bytecode[i + 1] << 16) | (bytecode[i + 2] << 8) | bytecode[i + 3];
                        cout << number << '\n';
                        i += 4;
                        break;
                    } default:
                        cout << "Invalid type 0x" << std::hex << (int)bytecode[i] << '\n';
                        exit(1);
                }
                break;
            } default:
                cout << "Invalid instruction 0x" << std::hex << (int)bytecode[i] << '\n';
                exit(1);
        }
        ++i;
    }

    exit(0);

}

// get metadata
void getMetadata() {

    cout << C_ERROR << "Coming soon." << C_CLEAR;
    exit(1);

}

// packager
void package() {

    cout << C_ERROR << "Coming soon." << C_CLEAR;
    exit(1);

}

//* cli

int main(int argc, char** argv) {

    // check for first argument
    if(argc == 1) {
        cout << C_ERROR << "You need to provide an option. Use --help to get a list of options.\n" << C_CLEAR;
        return 1;
    }
    if(argv[1][0] != '-') {
        cout << C_ERROR << "Invalid option " << argv[1] << ". Did you mean --" << argv[1] << "?\n" << C_CLEAR;
        return 1;
    }

    // check option
    switch(argv[1][1]) {

        case 'c': {
            std::ifstream inputFile(argv[2]);
            if(!inputFile.is_open()) {
                cout << C_ERROR << "Unable to open input file.\n" << C_CLEAR;
                return 1;
            }
            std::stringstream buffer;
            buffer << inputFile.rdbuf();
            code = buffer.str();

            compileCode();

            std::ofstream outputFile(
                string(argv[2]).substr(0, string(argv[2]).find_last_of('.')) + ".wae",
                ios::binary
            );
            if(!outputFile.is_open()) {
                cout << C_ERROR << "Unable to open output file.\n" << C_CLEAR;
                return 1;
            }
            outputFile << string(compiled.begin(), compiled.end());
            outputFile.close();

            return 0;
        } case 'r': {
            std::ifstream inputFile(argv[2], ios::binary);
            if(!inputFile.is_open()) {
                cout << C_ERROR << "Unable to open input file.\n" << C_CLEAR;
            }
            std::stringstream buffer;
            buffer << inputFile.rdbuf();
            
            bytecode.reserve(buffer.str().length());
            for(uint8_t byte: buffer.str()) {
                bytecode.push_back(byte);
            }

            runBytecode();
            return 0;
        } case 'f': {
            std::ifstream inputFile(argv[2]);
            if(!inputFile.is_open()) {
                cout << C_ERROR << "Unable to open input file.\n" << C_CLEAR;
                return 1;
            }
            std::stringstream buffer;
            buffer << inputFile.rdbuf();
            code = buffer.str();

            compileCode();

            bytecode = compiled;
            runBytecode();
            return 0;
        } case 'm': {
            cout << C_ERROR << "Coming soon.\n" << C_CLEAR;
            return 1;
        } case 'p': {
            cout << C_ERROR << "Coming soon.\n" << C_CLEAR;
            return 1;
        } case '-': { // second char is also a dash
            string option = string(argv[1]).substr(2);

            if(option == "help") {

                cout << "\nwas -c <file>    Compile a .was into a .wae\n"
                          "    -r <file>    Run a .was file\n"
                          "    -f <file>    Compile and run a .was file \033[1m(easiest)\033[0m\n"
                       // "    -m <file>    Get the metadata in a .wae file\n"
                       // "    -p <dir>     Package a folder of .wae's into a single package\n"
                          "    --help       Get a list of options (this)\n"
                          "    --version    Get the version information\n"
                       // "    --update     Check for updates and update WhatsAScript if needed\n"
                          "    --donut      Built-in spinning donut for calming purposes\n\n";
                return 0;

            } else if(option == "version") {

                cout << "\n    \033[1mWhatsAScript v" << (int)version[0] << '.' << (int)version[1] << '.' << (int)version[2]
                     << "\033[0m [" << versionDate
                     << "]\n    (c) 2024 anpang54 (GitHub)"
                        "\n    Code is licensed under GPL 3.0.\n\n";
                return 0;

            } else if(option == "update") {
                cout << C_ERROR << "Coming soon.\n" << C_CLEAR;
                return 1;
            } else if(option == "donut") {

                double   A = 0, B = 0, i, j;
                uint16_t k;
                double   z[1760];
                char     b[1760];

                clearTerm();

                while(true) {
                    memset(b, 32, 1760);
                    memset(z, 0, 1760 * 8);
                    for(j = 0; j < (2 * M_PI); j += 0.07) {
                        for(i = 0; i < (2 * M_PI); i += 0.02) {
                            double c = sin(i),
                                   d = cos(j),
                                   e = sin(A),
                                   f = sin(j),
                                   g = cos(A),
                                   h = d + 2,
                                   D = 1 / ((c * h * e) + (f * g) + 5),
                                   l = cos(i),
                                   m = cos(B),
                                   n = sin(B),
                                   t = (c * h * g) - (f * e);
                            int x = 40 + (30 * D * ((l * h * m) - (t * n))),
                                y = 12 + (15 * D * ((l * h * n) + (t * m))),
                                o = x + (80 * y),
                                N = 8 * ((((f * e) - (c * d * g)) * m) - (c * d * e) - (f * g) - (l * d * n));
                            if((22 > y) && (y > 0) && (x > 0) && (80 > x) && (D > z[o])) {
                                z[o] = D;
                                b[o] = "~~::;;==!#$@"[N > 0? N: 0];
                            }
                        }
                    }

                    clearTerm();
                    cout << "\n\n                 \033[1mBuilt-in spinning donut for calming purposes\033[0m\n";
                    for(k = 0; k < 1761; ++k) {
                        putchar((k % 80)? b[k]: 10);
                        A += 0.00004;
                        B += 0.00002;
                    }
                    cout << "\n\n                               ctrl + c to stop\n\n";

                    std::this_thread::sleep_for(std::chrono::milliseconds(30));
                }

                return 0;

            } else {
                cout << C_ERROR << "Invalid option " << argv[1] << ".\n" << C_CLEAR;
                return 1;
            }

        } default: {
            string option = string(argv[1]).substr(1);

            cout << C_ERROR << "Invalid option " << argv[1] << '.';
            if(option == "help" || option == "version" || option == "update" || option == "donut") {
                cout << " Did you mean -" << argv[1] << "?\n" << C_CLEAR;
            } else {
                cout << '\n' << C_CLEAR;
            }
            return 1;
        }
    }
}


