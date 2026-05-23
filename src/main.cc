#include <unistd.h>
#include <string>
#include "Game.h"
#include "Item.h"
#include "EffectManager.h"


int main(int argc, char* argv[]) {
    bool weatherEnabled = false;
    bool questEnabled = false;
    std::string filename = "./files/default.txt"; // 默认文件名
    int seed = 1;

    // 解析命令行参数
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg == "-enableWeather") {
            weatherEnabled = true;
        } else if (arg == "-enableQuest") {
            questEnabled = true;
        } else if (arg == "-file") {
            // 检查是否有下一个参数作为文件名
            if (i + 1 < argc) {
                filename = argv[++i]; // 获取下一个参数作为文件名
            } else {
                std::cerr << "Error: Missing filename after -file option.\n";
                return 1; // 返回错误码
            }
        } else if (arg == "-seed") {
            // 检查是否有下一个参数作为种子值
            if (i + 1 < argc) {
                try {
                    seed = std::stoi(argv[++i]); // 将字符串转换为整数
                } catch (const std::exception& e) {
                    std::cerr << "Error: Invalid seed value.\n";
                    return 1; // 返回错误码
                }
            } else {
                std::cerr << "Error: Missing seed value after -seed option.\n";
                return 1; // 返回错误码
            }
        } else {
            std::cerr << "Error: Unknown option '" << arg << "'.\n";
            return 1; // 返回错误码
        }
    }

    // 初始化游戏
    Game game(seed, filename, weatherEnabled, questEnabled);

    // 运行游戏
    game.run();

    return 0;
}
