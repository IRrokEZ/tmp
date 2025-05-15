#pragma once

#include <optional>
#include <string_view>
#include <memory>

class BiMap {
public:
    BiMap ();
    BiMap (const BiMap& other);
    BiMap (BiMap&& other) noexcept;
    
    BiMap& operator= (const BiMap& other);
    BiMap& operator= (BiMap&& other) noexcept;

    bool Add(std::string_view key, std::string_view value);

    std::optional<std::string_view> FindValue(std::string_view key) const noexcept;
    std::optional<std::string_view> FindKey(std::string_view value) const noexcept;

    ~BiMap();

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};