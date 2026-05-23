#include <unistd.h>
#include <iostream>
#include <string>
#include "Game.h"
#include "Item.h"
#include "EffectManager.h"

namespace {
void printUsage(const char* prog) {
    std::cout
        << "Usage: " << prog << " [options]\n"
        << "  -h, --help           Show this help and exit\n"
        << "  --seed, -seed N      Seed the PRNG with N (deterministic replay)\n"
        << "  --file, -file PATH   Load floor layout from PATH\n"
        << "  --enableWeather, -enableWeather   Enable random weather effects\n"
        << "  --enableQuest,   -enableQuest     Enable side quests\n";
}
} // namespace

int main(int argc, char* argv[]) {
    bool weatherEnabled = false;
    bool questEnabled = false;
    std::string filename = "./files/default.txt";
    int seed = 1;

    auto matches = [](const std::string& arg, const char* a, const char* b) {
        return arg == a || arg == b;
    };

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (matches(arg, "-h", "--help")) {
            printUsage(argv[0]);
            return 0;
        } else if (matches(arg, "-enableWeather", "--enableWeather")) {
            weatherEnabled = true;
        } else if (matches(arg, "-enableQuest", "--enableQuest")) {
            questEnabled = true;
        } else if (matches(arg, "-file", "--file")) {
            if (i + 1 < argc) {
                filename = argv[++i];
            } else {
                std::cerr << "Error: Missing filename after " << arg << " option.\n";
                return 1;
            }
        } else if (matches(arg, "-seed", "--seed")) {
            if (i + 1 < argc) {
                try {
                    seed = std::stoi(argv[++i]);
                } catch (const std::exception&) {
                    std::cerr << "Error: Invalid seed value '" << argv[i] << "'.\n";
                    return 1;
                }
            } else {
                std::cerr << "Error: Missing seed value after " << arg << " option.\n";
                return 1;
            }
        } else {
            std::cerr << "Error: Unknown option '" << arg << "'. Try --help.\n";
            return 1;
        }
    }

    // Echo the resolved seed so a player can reproduce a run later.
    std::cerr << "[cc3k] seed=" << seed << " file=" << filename
              << " weather=" << (weatherEnabled ? "on" : "off")
              << " quest=" << (questEnabled ? "on" : "off") << '\n';

    Game game(seed, filename, weatherEnabled, questEnabled);
    game.run();
    return 0;
}
