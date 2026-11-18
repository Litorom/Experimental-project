#pragma once
#include "game.hpp"
#include <ctime>
#include <filesystem>
#include <fstream>

namespace Save {
    constexpr auto path = "idle_rpg.save";
    constexpr auto temporary = "idle_rpg.save.tmp";
    constexpr auto format = "idle-rpg-v1";
    constexpr int tickSeconds = 2;
    constexpr int offlineLimitSeconds = 8 * 60 * 60;

    inline bool write(const Game& game) {
        const auto& p = game.progress;
        std::ofstream file(temporary);
        file << format << ' ' << std::time(nullptr) << '\n'
             << p.floor << ' ' << p.level << ' ' << p.xp << ' ' << p.gold
             << ' ' << p.sword << ' ' << p.armor << ' ' << p.farmTurns << '\n';
        file.close();
        if (!file) return false;
        std::error_code error;
        std::filesystem::rename(temporary, path, error);
        return !error;
    }

    inline bool load(Game& game) {
        std::error_code error;
        const bool exists = std::filesystem::exists(path, error);
        if (error) return false;
        if (!exists) return true;

        std::ifstream file(path);
        std::string version;
        std::time_t savedAt{};
        Progress p;
        if (!(file >> version >> savedAt >> p.floor >> p.level >> p.xp
                   >> p.gold >> p.sword >> p.armor >> p.farmTurns)
            || version != format || p.floor < 1 || p.level < 1
            || p.farmTurns < 0 || p.farmTurns > 3
            || (p.farmTurns && p.floor < 2))
            return false;

        game.progress = std::move(p);
        if (!(game.progress.xp < game.xpNeeded())) return false;

        const double elapsed = std::difftime(std::time(nullptr), savedAt);
        const int seconds = static_cast<int>(
            std::clamp(elapsed, 0.0, double(offlineLimitSeconds)));
        const int fights = seconds / tickSeconds;
        for (int i = 0; i < fights; ++i) game.tick();
        if (fights) std::cout << "Offline progress: " << fights << " fights.\n";
        return true;
    }
}