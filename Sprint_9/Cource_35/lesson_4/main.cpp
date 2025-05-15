#include <iostream>
#include <string_view>

using namespace std;

// clang-format off

const int NUM_PLANETS = 8; // Исправлено на 8
const std::string_view PLANETS[] = {
    "Mercury"sv, "Venus"sv, "Earth"sv,
    "Mars"sv, "Jupiter"sv, "Saturn"sv,
    "Uranus"sv, "Neptune"sv,
};

// clang-format on

bool IsPlanet(std::string_view name) {
    for (int i = 0; i < NUM_PLANETS; ++i) {
        if (PLANETS[i] == name) {
            return true;
        }
    }
    return false;
}

int main() {
    std::string bodyName;
    std::getline(std::cin, bodyName); // Считываем имя небесного тела

    // Проверяем, является ли введённое название планетой
    std::cout << bodyName << " is " << (IsPlanet(bodyName) ? "" : "NOT ")
         << "a planet" << std::endl;

    return 0;
}