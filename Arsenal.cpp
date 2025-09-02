#include "Arsenal.h"

Arsenal::Arsenal(ArsenalType t, int c) : type(t), count(c) {
    if (type == ArsenalType::RADAR) {
        icon.loadFromFile("radar_icon.png");
    }
    else if (type == ArsenalType::AIRPLANE) {
        icon.loadFromFile("airplane.png");
    }
    else {
        icon.loadFromFile("helicopter_icon.png");
    }
}
