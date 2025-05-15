#pragma once

#include <string>
#include <vector>

enum class ProgrammingLanguage { CPP, JAVA, PYTHON, PHP };

enum class Gender { MALE, FEMALE };

class Person {
public:
    Person(const std::string& name, int age, Gender gender);

    const std::string& GetName() const;
    int GetAge() const;
    Gender GetGender() const;


private:
    std::string name_;
    int age_;
    Gender gender_;
};

class Programmer : public Person {
public:
    Programmer(const std::string& name, int age, Gender gender);
    
    void AddProgrammingLanguage(ProgrammingLanguage language);
    bool CanProgram(ProgrammingLanguage language) const;

private:
    std::vector<ProgrammingLanguage> language_;
};

enum class WorkerSpeciality { BLACKSMITH, CARPENTER, WOOD_CHOPPER, ENGINEER, PLUMBER };

class Worker : public Person {
public:
    Worker(const std::string& name, int age, Gender gender);
    
    void AddSpeciality(WorkerSpeciality speciality);
    
    bool HasSpeciality(WorkerSpeciality speciality) const;
private:
    std::vector<WorkerSpeciality> speciality_;
};