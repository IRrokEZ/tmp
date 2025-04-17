#pragma once

#include <deque>
#include <string>
#include <string_view>
#include <vector>

#include "transport_catalogue.h"

struct InputCommand {
    explicit operator bool() const {
        return !command.empty();
    }

    bool operator!() const {
        return !operator bool();
    }

    std::string command;       // Название команды
    std::string object_name;   // Номер маршрута или название остановки
    std::string body;          // Параметры команды
};

namespace input_reader {

std::string_view Trim (std::string_view str);
std::vector<std::string_view> Split (std::string_view str, char delim);
Coordinates ParseCoordinates (std::string_view str);
std::vector<std::string_view> ParseRoute (std::string_view route);
InputCommand ParseInputCommand (std::string_view line);

class InputReader {
public:
    void ParseLine(std::string_view line);
    void ApplyCommands(TransportCatalogue& catalogue) const;

private:
    std::deque<InputCommand> input_stops_;
    std::deque<InputCommand> input_buses_;
};

} // namespace input_reader