#include <cassert>
#include <iostream>
#include <memory>
#include <string>
#include <utility>

using std::literals::string_literals::operator""s;

struct Cat {
    Cat (const std::string& name, int age)
        : name_(name),
        age_(age)  {}

    Cat (const Cat &other)
        : name_(other.name_),
        age_(other.age_) {}

    const std::string& GetName() const noexcept {
        return name_;
    }
    int GetAge() const noexcept {
        return age_;
    }

    ~Cat() {
    }

    void Speak() const {
        std::cout << "Meow!"s << std::endl;
    }

private:
    std::string name_;
    int age_;
};

std::unique_ptr<Cat> CreateCat (const std::string& name) {
    return std::make_unique<Cat>(name, 2);
}

class Witch {
public:
    explicit Witch (const std::string& name)
        : name_(name) {}

    Witch (const Witch& other) {
        name_ = other.name_;
        cat_ = std::make_unique<Cat>(*other.cat_.get());
    }

    Witch (Witch && other) = default;


    const std::string& GetName() const noexcept {
        return name_;
    }

    void SetCat (std::unique_ptr<Cat>&& cat) noexcept {
        cat_ = std::move(cat);
    }

    std::unique_ptr<Cat> ReleaseCat() noexcept {
        return std::move(cat_);
    }

    Witch& operator= (const Witch &other) {
        Witch tmp_copy(other);
        name_.swap(tmp_copy.name_);
        cat_.swap(tmp_copy.cat_);
        return *this;
    }

    Witch& operator= (Witch&& other) = default;

private:
    std::string name_;
    std::unique_ptr<Cat> cat_;
};

void Test() {
    {
        Witch witch("Hermione"s);
        auto cat = CreateCat("Crookshanks"s);
        Cat* raw_cat = cat.get();
        assert(raw_cat);
        witch.SetCat(std::move(cat));

        Witch moved_witch(std::move(witch));
        auto released_cat = moved_witch.ReleaseCat();
        assert(released_cat.get() == raw_cat);
    }

    {
        Witch witch("Hermione"s);
        auto cat = CreateCat("Crookshanks"s);
        Cat* raw_cat = cat.get();
        witch.SetCat(std::move(cat));

        Witch witch2("Minerva McGonagall"s);
        witch2 = std::move(witch);
        auto released_cat = witch.ReleaseCat();
        assert(!released_cat);
        released_cat = witch2.ReleaseCat();
        assert(released_cat.get() == raw_cat);
    }

    {
        Witch witch("Hermione"s);
        auto cat = CreateCat("Crookshanks"s);
        witch.SetCat(std::move(cat));

        Witch witch_copy(witch);
        assert(!cat);
        cat = witch.ReleaseCat();
        assert(cat);  

        auto cat_copy = witch_copy.ReleaseCat();
        assert(cat_copy != nullptr && cat_copy != cat);
        assert(cat_copy->GetName() == cat->GetName()); 
    }

    {
        Witch witch("Hermione"s);
        auto cat = CreateCat("Crookshanks"s);
        witch.SetCat(std::move(cat));

        Witch witch2("Minerva McGonagall"s);
        witch2 = witch;

        assert(!cat);
        cat = witch.ReleaseCat();
        assert(cat);  

        auto cat_copy = witch2.ReleaseCat();
        assert(cat_copy != nullptr && cat_copy != cat);
        assert(cat_copy->GetName() == cat->GetName());  
    }
}

int main() {
    Test();
    std::cout << std::endl << "OK"s << std::endl;
    return 0;
}