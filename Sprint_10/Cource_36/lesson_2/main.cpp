#include <cassert>
#include <iostream>
#include <stdexcept>
#include <string>

using std::literals::string_literals::operator""s;

class House {
public:

    House(int length, int width, int height)
        : length_(length), width_(width), height_(height) {}

    int GetLength() const {
        return length_;
    }

    int GetWidth() const {
        return width_;
    }

    int GetHeight() const {
        return height_;
    }

private:
    int length_;
    int width_;
    int height_;
};

class Resources {
public:
    Resources (int brick_count)
        : brick_count_(brick_count) {}

    int GetBrickCount() const {
        return brick_count_;
    }

    void TakeBricks(int count) {
        if (count < 0 || count > brick_count_) {
            throw std::out_of_range("Кирпичей мало или такого количества нет"s);
        }
        brick_count_ -= count;
    }

private:
    int brick_count_;
};

struct HouseSpecification {
    int length = 0;
    int width = 0;
    int height = 0;

    int CalculateBricksNeeded () const {
        int perimetr = 2 * (length + width);
        return perimetr * height * 4 * 8;
    }
};

class Builder {
public:
    Builder(Resources& resources)
        : resources_(resources) {}

    House BuildHouse(const HouseSpecification& spec) {
        int bricks_needed = spec.CalculateBricksNeeded();
        if (bricks_needed > resources_.GetBrickCount()) {
            throw std::runtime_error("Не хватает кирпичей для постройки дома"s);
        }
        resources_.TakeBricks(bricks_needed);
        return House(spec.length, spec.width, spec.height);
    }

private:
    Resources& resources_;
};

int main() {
    Resources resources{10000};
    Builder builder1{resources};
    Builder builder2{resources};

    House house1 = builder1.BuildHouse(HouseSpecification{12, 9, 3});
    assert(house1.GetLength() == 12);
    assert(house1.GetWidth() == 9);
    assert(house1.GetHeight() == 3);
    std::cout << resources.GetBrickCount() << " bricks left"s << std::endl;

    House house2 = builder2.BuildHouse(HouseSpecification{8, 6, 3});
    assert(house2.GetLength() == 8);
    std::cout << resources.GetBrickCount() << " bricks left"s << std::endl;

    House banya = builder1.BuildHouse(HouseSpecification{4, 3, 2});
    assert(banya.GetHeight() == 2);
    std::cout << resources.GetBrickCount() << " bricks left"s << std::endl;
}