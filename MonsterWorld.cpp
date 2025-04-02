#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#include <algorithm>
#include <sstream>
#include <map>

using namespace std;

enum WarriorType { DRAGON, NINJA, ICEMAN, LION, WOLF };
enum WeaponType { SWORD, BOMB, ARROW };
string warriorNames[] = { "dragon", "ninja", "iceman", "lion", "wolf" };
string weaponNames[] = { "sword", "bomb", "arrow" };

int initialLife[5], initialAttack[5];
int M, N, K, T;

struct Event {
    int time;
    int city;
    int type;
    string message;

    Event(int t, int c, int tp, const string& msg) : time(t), city(c), type(tp), message(msg) {}

    bool operator<(const Event& other) const {
        if (time != other.time) return time < other.time;
        if (type != other.type) return type < other.type;
        if (city != other.city) return city < other.city;
        return message > other.message;
    }
};

vector<Event> events;

class Weapon {
public:
    WeaponType type;
    int durability;

    Weapon(WeaponType type) : type(type) {
        if (type == ARROW) durability = 2;
        else durability = 1;
    }

    bool isUsable() const {
        return durability > 0;
    }

    int calculateAttack(int attack) const {
        switch (type) {
            case SWORD: return attack * 2 / 10;
            case BOMB: return attack * 4 / 10;
            case ARROW: return attack * 3 / 10;
            default: return 0;
        }
    }

    bool use() {
        if (type == BOMB) {
            durability = 0;
            return false;
        } else if (type == ARROW) {
            durability--;
            return durability > 0;
        }

        return true;
    }

};

class Warrior {
public:
    int id;
    int hp;
    int attackPower;
    int city;
    bool isRed;
    WarriorType type;
    vector<Weapon*> weapons;
    int nextweapon;
    int loyalty;

    Warrior(int id, int hp, int attackpower, int city, bool isRed, WarriorType type)
        : id(id), hp(hp), attackPower(attackpower), city(city), isRed(isRed), type(type), nextweapon(0) {};

    virtual ~Warrior() {
        for(auto w : weapons)
            delete w;
    }

    void sortWeapons() {
        sort(weapons.begin(), weapons.end(), [](Weapon* a, Weapon* b) {
            if (a->type != b->type) return a->type < b->type;
            if (a->type == ARROW) return a->durability < b->durability;
            return false;
        });
        nextweapon = 0;
    }

    bool hasWeapons() const {
        for (auto w : weapons) {
            if (w->isUsable()) return true;
        }
        return false;
    }

    virtual void move() {
        if (isRed) city++;
        else city--;
    }

    virtual void afterMove() {}

    virtual bool checkEscape() {
        if (type != LION) return false;
        if (city == 0 || city == N + 1) return false;
        return loyalty <= 0;
    }

    virtual void beforeBattle(Warrior* enemy, int time) {
        if (enemy != nullptr) {}
        if (time) {}
    }

    void sortWeaponsForLoot() {
        sort(weapons.begin(), weapons.end(), [](Weapon* a, Weapon* b) {
            if (a->type != b->type) return a->type < b->type;
            if (a->type == ARROW) return a->durability > b->durability;
            return false;
        });
    }

    void loot(Warrior* enemy) {
        vector<Weapon*> loot;
        enemy->sortWeaponsForLoot();
        for (auto w : enemy->weapons) {
            loot.push_back(w);
        }
        enemy->weapons.clear();

        for (auto w : loot) {
            if (weapons.size() >= 10) {
                delete w;
                continue;
            }
            weapons.push_back(w);
        }
        sortWeapons();
    }

    string reportWeapons() {
        map<WeaponType, int> counts;
        for (auto w : weapons) {
            if (w->isUsable())
                counts[w->type]++;
        }
        stringstream ss;
        ss << counts[SWORD] << " " << weaponNames[SWORD] << " ";
        ss << counts[BOMB] << " " << weaponNames[BOMB] << " ";
        ss << counts[ARROW] << " " << weaponNames[ARROW];
        return ss.str();
    }
};

class Dragon : public Warrior {
public:
    Dragon(int id, int hp, int attack, bool isRed, int city)
        : Warrior(id, hp, attack, city, isRed, DRAGON) {
        weapons.push_back(new Weapon(static_cast<WeaponType>(id % 3)));
    }
    
    virtual void afterMove() {}
};
    
class Ninja : public Warrior {
public:
    Ninja(int id, int hp, int attack, bool isRed, int city)
        : Warrior(id, hp, attack, city, isRed, NINJA) {
        weapons.push_back(new Weapon(static_cast<WeaponType>(id % 3)));
        weapons.push_back(new Weapon(static_cast<WeaponType>((id + 1) % 3)));
    }
    
    virtual void afterMove() {}
};
    
class Iceman : public Warrior {
public:
    Iceman(int id, int hp, int attack, bool isRed, int city)
        : Warrior(id, hp, attack, city, isRed, ICEMAN) {
        weapons.push_back(new Weapon(static_cast<WeaponType>(id % 3)));
    }

    virtual void afterMove() {
        hp -= hp / 10;
    }
};
    
class Lion : public Warrior {
public:
    Lion(int id, int hp, int attack, bool isRed, int city, int loyalty)
        : Warrior(id, hp, attack, city, isRed, LION){
            this->loyalty = loyalty;
        weapons.push_back(new Weapon(static_cast<WeaponType>(id % 3)));
    }
    
    virtual bool checkEscape() {
        if (city == 0 || city == N + 1) return false;
        return loyalty <= 0;
    }
    
    virtual void afterMove() {
        loyalty -= K;
    }
};
    
class Wolf : public Warrior {
public:
    Wolf(int id, int hp, int attack, bool isRed, int city)
        : Warrior(id, hp, attack, city, isRed, WOLF) {}
    
    virtual void beforeBattle(Warrior* enemy, int time) {
        if (enemy->type == WOLF) return;
        if (enemy->weapons.empty()) return;
    
        WeaponType minType = enemy->weapons[0]->type;
        for (auto w : enemy->weapons) {
            if (w->type < minType) minType = w->type;
        }
    
        vector<Weapon*> stolen;
        auto it = enemy->weapons.begin();
        while (it != enemy->weapons.end()) {
            if ((*it)->type == minType) {
                stolen.push_back(*it);
                it = enemy->weapons.erase(it);
            } else {
                ++it;
            }
        }
    
        sort(stolen.begin(), stolen.end(), [](Weapon* a, Weapon* b) {
            if (a->type == ARROW && b->type == ARROW) return a->durability > b->durability;
            return false;
        });
        
        int num = 0;
        for (auto w : stolen) {
            if (weapons.size() >= 10) {
                delete w;
            } else {
                weapons.push_back(w);
                num++;
            }
        }
        string msg = (isRed ? "red " : "blue ") + string("wolf ") + to_string(id) + " took " + to_string(num) + " " + string(weaponNames[minType]) + " from " + (isRed ? "blue " : "red ") + string(warriorNames[enemy->type]) + " " + to_string(enemy->id) + " in city " + to_string(city);
        events.emplace_back(time, city, 4, msg);
        sortWeapons();
    }
};

class Headquarters {
    public:
        int life;
        vector<WarriorType> productionOrder;
        bool isRed;
        int nextId;
        int totalId;
        bool stopped;
        bool istaken;
        vector<Warrior*> warriors;
    
        Headquarters(int life, const vector<WarriorType>& order, bool isRed)
            : life(life), productionOrder(order), isRed(isRed), nextId(0), totalId(1), stopped(false), istaken(false) {}
    
        ~Headquarters() {
            for (auto w : warriors) {
                if(!w) delete w;
            }
        }
    
        Warrior* produce() {
            if (stopped) return nullptr;
            WarriorType type = productionOrder[nextId];
            int cost = initialLife[type];
            if (life >= cost) {
                life -= cost;
                Warrior* w = createWarrior(totalId++, type);
                warriors.push_back(w);
                nextId = (nextId + 1) % 5;
                return w;
            } else {
                stopped = true;
            return nullptr;
            }
        }
    
private:
    Warrior* createWarrior(int id, WarriorType type) {
        int hp = initialLife[type];
        int attack = initialAttack[type];
        int city = isRed ? 0 : N + 1;
        switch (type) {
            case DRAGON:
                return new Dragon(id, hp, attack, isRed, city);
            case NINJA:
                return new Ninja(id, hp, attack, isRed, city);
            case ICEMAN:
                return new Iceman(id, hp, attack, isRed, city);
            case LION:
                return new Lion(id, hp, attack, isRed, city, life);
            case WOLF:
                return new Wolf(id, hp, attack, isRed, city);
            default:
                return nullptr;
        }
    }
};
    
Headquarters* redHQ = nullptr;
Headquarters* blueHQ = nullptr;
vector<Warrior*> redWarriors, blueWarriors;
map<int, pair<Warrior*, Warrior*>> cityWarriors;

void produceWarriors(int time) {
    Warrior* red = redHQ->produce();
    if (red) {
        stringstream ss;
        ss << (redHQ->isRed ? "red " : "blue ") << string(warriorNames[red->type]) << " " << to_string(red->id) << " born";
        if (red->type == LION) {
            ss << endl << "Its loyalty is " << to_string(red->loyalty);
        }
        events.emplace_back(time, 0, 0, ss.str());
        redWarriors.push_back(red);
    }

    Warrior* blue = blueHQ->produce();
    if (blue) {
        stringstream ss;
        ss << (blueHQ->isRed ? "red " : "blue ") << string(warriorNames[blue->type]) << " " << to_string(blue->id) << " born";
        if (blue->type == LION) {
            ss << endl << "Its loyalty is " << to_string(blue->loyalty);
        }
        events.emplace_back(time, 0, 0, ss.str());
        blueWarriors.push_back(blue);
    }
}

void clearWarrior(Warrior * w) {
    auto deleteWarrior = [](Warrior * w, bool isRed) {
        if (isRed) {
                redWarriors.erase(remove(redWarriors.begin(), redWarriors.end(), w), redWarriors.end());
                redHQ->warriors.erase(remove(redHQ->warriors.begin(), redHQ->warriors.end(), w), redHQ->warriors.end());
                delete w;
        } else {
                blueWarriors.erase(remove(blueWarriors.begin(), blueWarriors.end(), w), blueWarriors.end());
                blueHQ->warriors.erase(remove(blueHQ->warriors.begin(), blueHQ->warriors.end(), w), blueHQ->warriors.end());
                delete w;
        }
    };

    deleteWarrior(w, w->isRed);
}

void checkLionEscape(int time) {
    auto check = [time](vector<Warrior*>& warriors, bool isRed) {
        vector<Warrior*> toRemove;
        for (auto& w : warriors) {
            if (w->type == LION && w->checkEscape()) {
                string msg = (isRed ? "red " : "blue ") + string(warriorNames[w->type]) + " " + to_string(w->id) + " ran away";
                events.emplace_back(time, w->city, 1, msg);
                toRemove.push_back(w);
            }
        }
        for (auto w : toRemove) {
            warriors.erase(remove(warriors.begin(), warriors.end(), w), warriors.end());
            delete w;
        }
    };

    check(redWarriors, true);
    check(blueWarriors, false);
}

void moveWarriors(int time) {
    cityWarriors.clear();
    auto moveSingle = [time](vector<Warrior*>& warriors, bool isRed) {

        for (auto w : warriors) {
            w->move();
            w->afterMove();

            if ((isRed && w->city == N + 1) || (!isRed && w->city == 0)) {
                if ((isRed && w->city == N + 1)) {
                    blueHQ->istaken = true;
                    stringstream ss;
                    ss << "red " + string(warriorNames[w->type]) + " " + to_string(w->id) + " reached blue headquarter with " + to_string(w->hp) + " elements and force " + to_string(w->attackPower);
                    ss << endl << setw(3) << setfill('0') << time / 60 << ":" << setw(2) << setfill('0') << time % 60 << " blue headquarter was taken";
                    events.emplace_back(time, w->city, 2, ss.str());
                }
                if ((!isRed && w->city == 0)) {
                    redHQ->istaken = true;
                    stringstream ss;
                    ss << "blue " + string(warriorNames[w->type]) + " " + to_string(w->id) + " reached red headquarter with " + to_string(w->hp) + " elements and force " + to_string(w->attackPower);
                    ss << endl << setw(3) << setfill('0') << time / 60 << ":" << setw(2) << setfill('0') << time % 60 << " red headquarter was taken";
                    events.emplace_back(time, w->city, 2, ss.str());
                }
                continue;
            }

            string msg = (isRed ? "red " : "blue ") + string(warriorNames[w->type]) + " " + to_string(w->id) + " marched to city " + to_string(w->city) + " with " + to_string(w->hp) + " elements and force " + to_string(w->attackPower);
            events.emplace_back(time, w->city, 2, msg);

            cityWarriors[w->city].first = isRed ? w : cityWarriors[w->city].first;
            cityWarriors[w->city].second = !isRed ? w : cityWarriors[w->city].second;
        }
    };

    moveSingle(redWarriors, true);
    moveSingle(blueWarriors, false);
}

void resolveWolfSteal(int time) {
    for (auto& entry : cityWarriors) {
        Warrior* red = entry.second.first;
        Warrior* blue = entry.second.second;

        if (red && blue) {
            if (red->type == WOLF) red->beforeBattle(blue, time);
            if (blue->type == WOLF) blue->beforeBattle(red, time);
        }
    }
}

void fight(Warrior* a, Warrior* b, int city, int time) {
    a->sortWeapons();
    b->sortWeapons();

    bool aFirst = (city % 2 == 1);

    Warrior* attacker = aFirst ? a : b;
    Warrior* defender = aFirst ? b : a;

    bool aHasWeapons = a->hasWeapons();
    bool bHasWeapons = b->hasWeapons();

    if (!aHasWeapons && !bHasWeapons) {
        stringstream ss;
        ss << "both red " << warriorNames[a->type] << " " << a->id << " and blue " << warriorNames[b->type] << " " << b->id << " were alive in city " << city;
        if (a->type == DRAGON) {
            ss << endl << setw(3) << setfill('0') << time / 60 << ":" << setw(2) << setfill('0') << time % 60;
            ss << (a->isRed ? " red " : " blue ") << "dragon " << a->id << " yelled in city " << city;
        }
        if (b->type == DRAGON) {
            ss << endl << setw(3) << setfill('0') << time / 60 << ":" << setw(2) << setfill('0') << time % 60;
            ss << (b->isRed ? " red " : " blue ") << "dragon " << b->id << " yelled in city " << city;
        }
        events.emplace_back(time, city, 5, ss.str());
        return;
    }

    int count = 0;
    while (true) {
        bool aDead = (a->hp <= 0);
        bool bDead = (b->hp <= 0);

        if (aDead && bDead) {
            stringstream ss;
            ss << "both red " << warriorNames[a->type] << " " << a->id << " and blue " << warriorNames[b->type] << " " << b->id << " died in city " << city;
            events.emplace_back(time, city, 5, ss.str());
            clearWarrior(a);
            clearWarrior(b);
            break;
        } else if (aDead) {
            stringstream ss;
            ss << "blue " << warriorNames[b->type] << " " << b->id << " killed red " << warriorNames[a->type] << " " << a->id << " in city " << city << " remaining " << b->hp << " elements";
            b->loot(a);
            if (b->type == DRAGON) {
                ss << endl << setw(3) << setfill('0') << time / 60 << ":" << setw(2) << setfill('0') << time % 60;
                ss << (b->isRed ? " red " : " blue ") << "dragon " << b->id << " yelled in city " << city;
            }
            events.emplace_back(time, city, 5, ss.str());
            clearWarrior(a);
            break;
        } else if (bDead) {
            stringstream ss;
            ss << "red " << warriorNames[a->type] << " " << a->id << " killed blue " << warriorNames[b->type] << " " << b->id << " in city " << city << " remaining " << a->hp << " elements";
            a->loot(b);
            if (a->type == DRAGON) {
                ss << endl << setw(3) << setfill('0') << time / 60 << ":" << setw(2) << setfill('0') << time % 60;
                ss << (a->isRed ? " red " : " blue ") << "dragon " << a->id << " yelled in city " << city;
            }
            events.emplace_back(time, city, 5, ss.str());
            clearWarrior(b);
            break;
        }

        if (!attacker->hasWeapons() && !defender->hasWeapons()) {
            stringstream ss;
            ss << "both red " << warriorNames[a->type] << " " << a->id << " and blue " << warriorNames[b->type] << " " << b->id << " were alive in city " << city;
            if (a->type == DRAGON) {
                ss << endl << setw(3) << setfill('0') << time / 60 << ":" << setw(2) << setfill('0') << time % 60;
                ss << (a->isRed ? " red " : " blue ") << "dragon " << a->id << " yelled in city " << city;
            }
            if (b->type == DRAGON) {
                ss << endl << setw(3) << setfill('0') << time / 60 << ":" << setw(2) << setfill('0') << time % 60;
                ss << (b->isRed ? " red " : " blue ") << "dragon " << b->id << " yelled in city " << city;
            }
            events.emplace_back(time, city, 5, ss.str());
            break;
        }

        bool changed = false;
        int aInitialHp = a->hp;
        int bInitialHp = b->hp;

        if (attacker->hasWeapons()) {
            int k = attacker->weapons.size();
            for (int i = 0; i < k; i++) {
                int idx = (attacker->nextweapon + i) % k;
                Weapon* w = attacker->weapons[idx];
                if (w->isUsable()) {
                    int dmg = w->calculateAttack(attacker->attackPower);
                    defender->hp -= dmg;
                    defender->hp = max(defender->hp, 0);

                    if (w->type == BOMB && attacker->type != NINJA) {
                        int selfDmg = dmg / 2;
                        attacker->hp -= selfDmg;
                        attacker->hp = max(attacker->hp, 0);
                    }

                    w->use();
                    if (!w->isUsable()) {
                        attacker->weapons.erase(remove(attacker->weapons.begin(), attacker->weapons.end(), w), attacker->weapons.end());
                        delete w;
                    }

                    attacker->nextweapon = (idx + 1) % k;
                    changed = true;
                    break;
                }
            }
        }

        if (defender->hp <= 0) continue;

        if (defender->hasWeapons()) {
            int k = defender->weapons.size();
            for (int i = 0; i < k; i++) {
                int idx = (defender->nextweapon + i) % k;
                Weapon* w = defender->weapons[idx];
                if (w->isUsable()) {
                    int dmg = w->calculateAttack(defender->attackPower);
                    attacker->hp -= dmg;
                    attacker->hp = max(attacker->hp, 0);

                    if (w->type == BOMB && defender->type != NINJA) {
                        int selfDmg = dmg / 2;
                        defender->hp -= selfDmg;
                        defender->hp = max(defender->hp, 0);
                    }

                    w->use();
                    if (!w->isUsable()) {
                        defender->weapons.erase(remove(defender->weapons.begin(), defender->weapons.end(), w), defender->weapons.end());
                        delete w;
                    }

                    defender->nextweapon = (idx + 1) % k;
                    changed = true;
                    break;
                }
            }
        }
        
        if(a->hp == aInitialHp && b->hp == bInitialHp) count++;
        if(count >= 10) changed = false;
        if (!changed) {
            stringstream ss;
            ss << "both red " << warriorNames[a->type] << " " << a->id << " and blue " << warriorNames[b->type] << " " << b->id << " were alive in city " << city;
            if (a->type == DRAGON) {
                ss << endl << setw(3) << setfill('0') << time / 60 << ":" << setw(2) << setfill('0') << time % 60;
                ss << (a->isRed ? " red " : " blue ") << "dragon " << a->id << " yelled in city " << city;
            }
            if (b->type == DRAGON) {
                ss << endl << setw(3) << setfill('0') << time / 60 << ":" << setw(2) << setfill('0') << time % 60;
                ss << (b->isRed ? " red " : " blue ") << "dragon " << b->id << " yelled in city " << city;
            }
            events.emplace_back(time, city, 5, ss.str());
            break;
        }
    }
}

void resolveBattles(int time) {
    for (auto& entry : cityWarriors) {
        int city = entry.first;
        Warrior* red = entry.second.first;
        Warrior* blue = entry.second.second;

        if (red && blue) {
            fight(red, blue, city, time);
            if (find(redWarriors.begin(), redWarriors.end(), red) == redWarriors.end()) {
                entry.second.first = nullptr;
            }
            if (find(blueWarriors.begin(), blueWarriors.end(), blue) == blueWarriors.end()) {
                entry.second.second = nullptr;
            }
        }
    }
}

void reportHeadquarters(int time) {
    events.emplace_back(time, 0, 8, to_string(redHQ->life) + " elements in red headquarter");
    events.emplace_back(time, N + 1, 8, to_string(blueHQ->life) + " elements in blue headquarter");
}

void reportWeapons(int time) {
    auto report = [time](const vector<Warrior*>& warriors, bool isRed) {
        for (auto w : warriors) {
            string msg = (isRed ? "red " : "blue ") + warriorNames[w->type] + " " + to_string(w->id) + " has " + w->reportWeapons() + " and " + to_string(w->hp) + " elements";
            events.emplace_back(time, w->city, 9, msg);
        }
    };

    report(redWarriors, true);
    report(blueWarriors, false);
}

void simulate(int cas) {
    cout << "Case " << cas << ":" << endl;
    for (int t = 0; t <= T; t+=5) {
        int minutes = t % 60;

        if (minutes == 0) {
            produceWarriors(t);
        } else if (minutes == 5) {
            checkLionEscape(t);
        } else if (minutes == 10) {
            moveWarriors(t);
            if (redHQ->istaken || blueHQ->istaken) {
                break;
            }
        } else if (minutes == 35) {
            resolveWolfSteal(t);
        } else if (minutes == 40) {
            resolveBattles(t);
        } else if (minutes == 50) {
            reportHeadquarters(t);
        } else if (minutes == 55) {
            reportWeapons(t);
        }
    }

    sort(events.begin(), events.end());
    for (auto& e : events) {
        if (e.time > T) continue;
        int hours = e.time / 60;
        int minutes = e.time % 60;
        cout << setw(3) << setfill('0') << hours << ":" << setw(2) << setfill('0') << minutes << " " << e.message << endl;
    }
}

int main() {
    int cases;
    cin >> cases;
    int cas = 1;
    while(cas <= cases) {
        events.clear();
        cin >> M >> N >> K >> T;
        for (int i = 0; i < 5; ++i) cin >> initialLife[i];
        for (int i = 0; i < 5; ++i) cin >> initialAttack[i];

        redHQ = new Headquarters(M, {ICEMAN, LION, WOLF, NINJA, DRAGON}, true);
        blueHQ = new Headquarters(M, {LION, DRAGON, NINJA, ICEMAN, WOLF}, false);

        simulate(cas);

        delete redHQ;
        delete blueHQ;
        redWarriors.clear();
        blueWarriors.clear();
        cityWarriors.clear();
        cas++;
    }
    return 0;
}
