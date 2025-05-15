#include <iostream>
#include <string>
#include <string_view>
#include <vector>

using std::literals::string_view_literals::operator""sv;
using std::literals::string_literals::operator""s;

class Speakable {
public:
    virtual ~Speakable() = default;
    virtual void Speak(std::ostream& out) const = 0;
};

class Drawable {
public:
    virtual ~Drawable() = default;
    virtual void Draw(std::ostream& out) const = 0;
};

class Animal {
public:
    virtual ~Animal() = default;
    void Eat(std::string_view food) {
        std::cout << GetType() << " is eating "sv << food << std::endl;
        ++ energy_;
    }
    virtual std::string GetType() const = 0;

private:
    int energy_ = 100;
};

class Fish : public Animal, public Drawable {
public:
    std::string GetType() const override {
        return "fish"s;
    }
    void Draw(std::ostream& out) const override {
        out << "><(((*>"sv << std::endl;
    }
};

class Cat : public Animal, public Speakable, public Drawable {
public:
    void Speak(std::ostream& out) const override {
        out << "Meow-meow"sv << std::endl;
    }
    void Draw(std::ostream& out) const override {
        out << "(^w^)"sv << std::endl;
    }
    std::string GetType() const override {
        return "cat"s;
    }
};

void DrawAnimals(const std::vector<const Animal*>& animals, std::ostream& out) {
    for (const Animal* animal : animals) {
        if (const Drawable* drawable = dynamic_cast<const Drawable*>(animal)) {
            drawable->Draw(out);
        }
    }
}

void TalkToAnimals(const std::vector<const Animal*>& animals, std::ostream& out) {
    for (const Animal* animal : animals) {
        if (const Speakable* speakable = dynamic_cast<const Speakable*>(animal)) {
            speakable->Speak(out);
        }
    }
}


void PlayWithAnimals(const std::vector<const Animal*> animals, std::ostream& out) {
    TalkToAnimals(animals, out);
    DrawAnimals(animals, out);
}

int main() {
    Cat cat;
    Fish fish;
    std::vector<const Animal*> animals{&cat, &fish};
    PlayWithAnimals(animals, std::cerr);
    std::cout << std::endl << "ok"s << std::endl;
    return 0;
}