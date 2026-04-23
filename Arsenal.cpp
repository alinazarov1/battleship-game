#include "Arsenal.h"

Arsenal::Arsenal(ArsenalType t, int c) : type(t), count(c) {
    if (type == ArsenalType::RADAR) {
        icon.loadFromFile("assets/radar_icon.png");
    }
    else if (type == ArsenalType::AIRPLANE) {
        icon.loadFromFile("assets/airplane.png");
    }
    else {
        icon.loadFromFile("assets/helicopter_icon.png");
    }
}
