#include "save.hpp"
#include <chrono>
#include <ctime>

int main() {
    Game game;
    if (!Save::load(game)) {
        std::cerr << "Could not load " << Save::path << ".\n";
        return 1;
    }

    using Clock = std::chrono::steady_clock;
    const auto fightInterval = std::chrono::seconds(Save::tickSeconds);
    auto lastFight = Clock::now();

    const auto catchUp = [&] {
        const auto fights = (Clock::now() - lastFight) / fightInterval;
        for (auto remaining = fights; remaining > 0; --remaining) {
            game.tick();
        }
        lastFight += fightInterval * fights;
        return fights;
    };

    const auto help = [] {
        std::cout << "watch | status | sword | armor | save | help | quit\n";
    };

    std::cout << "=== Endless Delve ===\n"
              << "One fight per " << Save::tickSeconds
              << " elapsed seconds. Type watch for live updates.\n";
    help();
    game.status(std::cout);

    std::string command;
    while (std::cout << "> " << std::flush, std::getline(std::cin, command)) {
        if (!command.empty() && command.back() == '\r') command.pop_back();

        const auto fights = catchUp();
        if (command == "quit") break;

        const bool upgrade = command == "sword" || command == "armor";
        if (upgrade) {
            game.buy(command == "sword", std::cout);
        } else if (command == "watch") {
            std::cout << "Live updates for 30 seconds. Commands resume afterward.\n"
                      << std::flush;

            for (int i = 0; i < 30 / Save::tickSeconds; ++i) {
                const std::timespec delay{Save::tickSeconds, 0};
                if (::nanosleep(&delay, nullptr) != 0) {
                    std::cerr << "Live timer unavailable or interrupted.\n";
                    break;
                }

                catchUp();
                if (!Save::write(game)) {
                    std::cerr << "Save failed.\n";
                    break;
                }

                game.status(std::cout);
                std::cout << '\n' << std::flush;
            }
        } else if (!command.empty() && command != "status" && command != "save") {
            help();
        }

        if (fights > 0 || upgrade || command == "save") {
            if (!Save::write(game)) {
                std::cerr << "Save failed.\n";
            } else if (command == "save") {
                std::cout << "Saved.\n";
            }
        }
        game.status(std::cout);
    }

    catchUp();
    if (!Save::write(game)) {
        std::cerr << "Save failed.\n";
        return 1;
    }
    std::cout << "Progress saved.\n";
}