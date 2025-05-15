#include <deque>
#include <stdexcept>
#include <unordered_map>
#include <utility>

#include "bimap.h"

struct BiMap::Impl {
    Impl () = default;
    
    Impl (const Impl& other) {
        auto temp_words = other.words_;
        auto temp_key_value = other.key_value_;
        auto temp_value_key = other.value_key_;
        std::swap(words_, temp_words);
        std::swap(key_value_, temp_key_value);
        std::swap(value_key_, temp_value_key);
    }

    Impl (Impl&& other) noexcept = default;

    Impl& operator= (const Impl& other) {
        auto temp_words = other.words_;
        auto temp_key_value = other.key_value_;
        auto temp_value_key = other.value_key_;
        std::swap(words_, temp_words);
        std::swap(key_value_, temp_key_value);
        std::swap(value_key_, temp_value_key);
        return *this;
    }

    Impl& operator= (Impl&& other) noexcept = default;

    bool Add(std::string_view key, std::string_view value) {
        if ((key_value_.find(key) != key_value_.end())
            || (value_key_.find(value) != value_key_.end())) {
            return false;
        }
        try {
            words_.push_back(std::string(value));
        } catch (std::exception &exception) {
            std::rethrow_exception(std::current_exception());
        }
        try {
            words_.push_front(std::string(key));
        } catch (std::exception &exception) {
            RollBack(true, false, false, "");
            std::rethrow_exception(std::current_exception());
        }
        try {
            key_value_[words_.front()] = words_.back();
        } catch (std::exception &exception) {
            RollBack(true, true, false, "");
            std::rethrow_exception(std::current_exception());
        }
        try {
            value_key_[words_.back()] = words_.front();
        } catch (std::exception &exception) {
            RollBack(true, true, false, "");
            std::rethrow_exception(std::current_exception());
        }
        return true;
    }
    
    std::optional<std::string_view> FindValue(std::string_view key) const noexcept {
        auto it = key_value_.find(key);
        if (it != key_value_.end()) {
            return it->second;
        }
        return std::nullopt;
    }
    
    std::optional<std::string_view> FindKey(std::string_view value) const noexcept {
        auto it = value_key_.find(value);
        if (it != value_key_.end()) {
            return it->second;
        }
        return std::nullopt;
    }

    ~Impl () = default;

private:
    std::unordered_map<std::string_view, std::string_view> key_value_;
    std::unordered_map<std::string_view, std::string_view> value_key_;
    std::deque<std::string> words_;

    void RollBack (bool rb_words_back, bool rb_words_front,
                   bool rb_key_val, std::string_view rb_key) {
        if (rb_words_back) {
            words_.pop_back();
        }
        if (rb_words_front) {
            words_.pop_front();
        }
        if (rb_key_val) {
            key_value_.erase(rb_key);
        }
    }
};

BiMap::BiMap ()
    : impl_(std::make_unique<Impl>()) {}

BiMap::BiMap (const BiMap& other)
    : impl_(other.impl_ ? std::make_unique<Impl>(*other.impl_) : nullptr) {}

BiMap::BiMap (BiMap&& other) noexcept = default;

BiMap& BiMap::operator= (const BiMap& other) {
    if (this != std::addressof(other)) {
        if (!other.impl_) {
            impl_.reset();
        } else if (impl_) {
            *impl_ = *other.impl_;
        } else {
            impl_ = std::make_unique<Impl>(*other.impl_);
        }
    }
    return *this;
}

BiMap& BiMap::operator= (BiMap&& other) noexcept = default;

bool BiMap::Add(std::string_view key, std::string_view value) {
    return impl_->Add(key, value);
}

std::optional<std::string_view> BiMap::FindValue(std::string_view key) const noexcept {
    return impl_->FindValue(key);
}

std::optional<std::string_view> BiMap::FindKey(std::string_view value) const noexcept {
    return impl_->FindKey(value);
}

BiMap::~BiMap () = default;
