#pragma once
#include "number.hpp"

struct Stats {
    Number health;
    Number attack;
};

struct Progress {
    Number floor = 1;
    Number level = 1;
    Number xp, gold, sword, armor;
    int farmTurns = 0;
};

class Game {
public:
    Progress progress;

    Stats hero() const {
        const auto& p = progress;
        return {p.level * 5 + p.armor * 8 + 18,
                p.level * 2 + p.sword * 3 + 4};
    }

    static Stats enemy(const Number& floor) {
        const unsigned boss = floor.isBossFloor() ? 2 : 1;
        return {(floor * 4 + 12) * boss, (floor + 2) * boss};
    }

    Number xpNeeded() const { return progress.level * 5 + 10; }

    // The hero strikes first. Fights draw after eight rounds.
    static bool wins(Stats player, Stats monster) {
        for (unsigned round = 1; round <= 8; ++round) {
            if (!(player.attack * round < monster.health)) return true;
            if (!(monster.attack * round < player.health)) return false;
        }
        return false;
    }

    void tick() {
        auto& p = progress;
        const bool farming = p.farmTurns > 0;
        const Number target = farming ? p.floor - 1 : p.floor;
        if (farming) --p.farmTurns;

        if (!wins(hero(), enemy(target))) {
            if (!farming && Number(1) < p.floor) p.farmTurns = 3;
            return;
        }

        const unsigned reward = target.isBossFloor() ? 2 : 1;
        p.gold = p.gold + (target * 2 + 4) * reward;
        p.xp = p.xp + (target + 4) * reward;
        for (Number cost = xpNeeded(); !(p.xp < cost); cost = xpNeeded()) {
            p.xp = p.xp - cost;
            p.level = p.level + 1;
        }
        if (!farming) p.floor = p.floor + 1;
    }

    static Number price(const Number& rank) { return rank * 10 + 12; }

    void buy(bool sword, std::ostream& out) {
        Number& rank = sword ? progress.sword : progress.armor;
        const Number cost = price(rank);
        if (progress.gold < cost) {
            out << "Upgrade costs " << cost << " gold.\n";
            return;
        }
        progress.gold = progress.gold - cost;
        rank = rank + 1;
        out << (sword ? "Sword" : "Armor") << " upgraded to +" << rank << ".\n";
    }

    void status(std::ostream& out) const {
        const auto& p = progress;
        const Stats player = hero();
        out << "Next floor " << p.floor
            << (p.floor.isBossFloor() ? " (boss)" : "")
            << " | Level " << p.level << " | XP " << p.xp << '/' << xpNeeded()
            << " | Gold " << p.gold << '\n'
            << "HP " << player.health << " | Attack " << player.attack
            << " | Sword +" << p.sword << " (" << price(p.sword) << "g)"
            << " | Armor +" << p.armor << " (" << price(p.armor) << "g)\n";
        if (p.farmTurns)
            out << "Farming floor " << p.floor - 1 << " for "
                << p.farmTurns << " more fights before retrying.\n";
    }
};