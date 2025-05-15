#include "people.h"

#include <algorithm>
#include <stdexcept>

Person::Person(const std::string& name, int age, Gender gender)
    : name_(name),
    age_(age),
    gender_(gender) {}

const std::string& Person::GetName() const {
    return name_;
}

int Person::GetAge() const {
    return age_;
}

Gender Person::GetGender() const {
    return gender_;
}


Programmer::Programmer(const std::string& name, int age, Gender gender)
    : Person(name, age, gender) {}

void Programmer::AddProgrammingLanguage(ProgrammingLanguage language) {
    language_.push_back(language);
}

bool Programmer::CanProgram(ProgrammingLanguage language) const {
    return std::find(language_.begin(), language_.end(), language) != language_.end();
}

Worker::Worker(const std::string& name, int age, Gender gender)
    : Person(name, age, gender) {}

void Worker::AddSpeciality(WorkerSpeciality speciality) {
    speciality_.push_back(speciality);
}

bool Worker::HasSpeciality(WorkerSpeciality speciality) const {
    return std::find(speciality_.begin(), speciality_.end(), speciality) != speciality_.end();
}