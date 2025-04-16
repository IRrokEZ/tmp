#include <algorithm>
#include <iterator>
#include <utility>

#include "geo.h"
#include "input_reader.h"

#include <iostream>

using std::literals::string_literals::operator""s;

namespace input_reader {

Coordinates ParseCoordinates (std::string_view str) {
    std::size_t comma = str.find(',');
    
    if (comma == str.npos) {
        return {std::nan(""), std::nan("")};
    }

    return {std::stod(std::string(Trim(str.substr(0, comma)))),
            std::stod(std::string(Trim(str.substr(comma + 1))))};
}

std::string_view Trim (std::string_view str) {
    const std::size_t start = str.find_first_not_of(' ');
    if (start == str.npos) {
        return {};
    }
    return str.substr(start, str.find_last_not_of(' ') + 1 - start);
}

std::vector<std::string_view> Split (std::string_view str, char delim) {
    std::vector<std::string_view> result;

    std::size_t pos = 0;
    while ((pos = str.find_first_not_of(' ', pos)) < str.length()) {
        std::size_t delim_pos = str.find(delim, pos);
        if (delim_pos == str.npos) {
            delim_pos = str.size();
        }

        std::string_view substr = Trim(str.substr(pos, delim_pos - pos));
        if (!substr.empty()) {
            result.push_back(substr);
        }
        pos = delim_pos + 1;
    }

    return result;
}

std::vector<std::string_view> ParseRoute (std::string_view route) {
    if (route.find('>') != route.npos) {
        return Split(route, '>');
    }

    std::vector<std::string_view> stops = Split(route, '-');
    std::vector<std::string_view> result(stops.begin(), stops.end());
    result.insert(result.end(), std::next(stops.rbegin()), stops.rend());

    return result;
}

InputCommand ParseInputCommand (std::string_view line) {
    std::size_t colon_pos = line.find(':');

    if (colon_pos == line.npos) {
        return {};
    }

    std::size_t space_pos = line.find(' ');
    if (space_pos >= colon_pos) {
        return {};
    }

    std::size_t not_space = line.find_first_not_of(' ', space_pos);
    if (not_space >= colon_pos) {
        return {};
    }

    return {std::string(Trim(line.substr(0, space_pos))),
            std::string(Trim(line.substr(not_space, colon_pos - not_space))),
            std::string(Trim(line.substr(colon_pos + 1)))};
}

void InputReader::ParseLine(std::string_view line) {
    InputCommand input_command = ParseInputCommand(line);
    if (input_command) {
        if (input_command.command == "Bus"s) {
            input_buses_.push_back(std::move(input_command));
        }
        if (input_command.command == "Stop"s) {
            input_stops_.push_back(std::move(input_command));
        }
    }
}

void InputReader::ApplyCommands(TransportCatalogue& catalogue) const {
    for (const InputCommand &command : input_stops_) {
        std::vector<std::string_view> tokens = input_reader::Split(command.body, ',');
        if (tokens.size() < 2) {
            throw std::logic_error("Incorrect data in command: ["s + command.body + "]"s);
        }

        std::string coord_str = std::string(tokens[0]) + ","s + std::string(tokens[1]);
        Coordinates coord = input_reader::ParseCoordinates(coord_str);
        catalogue.AddStop(command.object_name, coord);

        for (std::size_t i = 2; i < tokens.size(); ++ i) {
            std::string_view distance_token = input_reader::Trim(tokens[i]);
            std::size_t distance_end_pos = distance_token.find("m to ");
            if (distance_end_pos == std::string::npos) {
                throw std::logic_error("Incorrect distance data: ["s + std::string(tokens[i]) + "]"s);
            }

            double distance = std::stod(std::string(distance_token.substr(0, distance_end_pos)));
            std::string to_stop = std::string(distance_token.substr(distance_end_pos + 5));

            catalogue.SetDistance(command.object_name, to_stop, distance);
        }
    }
    for (const InputCommand &command : input_buses_) {
        std::vector<std::string_view> parsed_route = input_reader::ParseRoute(command.body);
        bool is_circular = command.body.find('>') != std::string::npos;
        catalogue.AddBus(command.object_name, std::move(parsed_route), is_circular);
    }
}

} // namespace input_reader