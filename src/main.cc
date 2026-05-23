#include <unistd.h>
#include <iostream>
#include <string>
#include "Game.h"
#include "Item.h"
#include "EffectManager.h"
#include "SaveGame.h"

namespace {
void printUsage(const char* prog) {
    std::cout
        << "Usage: " << prog << " [options]\n"
        << "  -h, --help           Show this help and exit\n"
        << "  --seed, -seed N      Seed the PRNG with N (deterministic replay)\n"
        << "  --file, -file PATH   Load floor layout from PATH\n"
        << "  --enableWeather, -enableWeather   Enable random weather effects\n"
        << "  --enableQuest,   -enableQuest     Enable side quests\n"
        << "  --save PATH          Write a save file at clean quit\n"
        << "  --load PATH          Resume from save file at startup\n";
}
} // namespace

int main(int argc, char* argv[]) {
    bool weatherEnabled = false;
    bool questEnabled = false;
    std::string filename = "./files/default.txt";
    int seed = 1;
    std::string savePath;
    std::string loadPath;

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
        } else if (arg == "--save") {
            if (i + 1 < argc) {
                savePath = argv[++i];
            } else {
                std::cerr << "Error: Missing path after --save.\n";
                return 1;
            }
        } else if (arg == "--load") {
            if (i + 1 < argc) {
                loadPath = argv[++i];
            } else {
                std::cerr << "Error: Missing path after --load.\n";
                return 1;
            }
        } else {
            std::cerr << "Error: Unknown option '" << arg << "'. Try --help.\n";
            return 1;
        }
    }

    std::optional<cc3k::SaveData> pendingLoad;
    if (!loadPath.empty()) {
        pendingLoad = cc3k::load(loadPath);
        if (!pendingLoad) {
            std::cerr << "Error: Failed to load save file '" << loadPath << "'.\n";
            return 1;
        }
        // Loaded settings override CLI defaults so the resumed run matches
        // the saved one exactly.
        seed = static_cast<int>(pendingLoad->seed);
        filename = pendingLoad->filename;
        weatherEnabled = pendingLoad->weatherEnabled;
        questEnabled = pendingLoad->questEnabled;
    }

    // Echo the resolved seed so a player can reproduce a run later.
    std::cerr << "[cc3k] seed=" << seed << " file=" << filename
              << " weather=" << (weatherEnabled ? "on" : "off")
              << " quest=" << (questEnabled ? "on" : "off");
    if (!savePath.empty()) std::cerr << " save=" << savePath;
    if (!loadPath.empty()) std::cerr << " load=" << loadPath;
    std::cerr << '\n';

    Game game(seed, filename, weatherEnabled, questEnabled, savePath, std::move(pendingLoad));
    game.run();
    return 0;
}
