#pragma once

// The player's money. Persons 2 and 3 use it like this:
//     world.coins.get()            -> how many coins right now
//     world.coins.add(10)          -> a coin was collected
//     if (world.coins.spend(10))   -> true = paid; false = not enough (nothing changes)
class Coins {
public:
    void reset(int startAmount) { amount = startAmount; }
    int  get() const { return amount; }
    void add(int value) { if (value > 0) amount += value; }

    bool spend(int cost) {
        if (cost < 0 || cost > amount) return false;
        amount -= cost;
        return true;
    }

private:
    int amount = 0;
};
