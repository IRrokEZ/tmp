#pragma once

#include <string>

class Person;

class PersonObserver {
public:
    virtual void OnSatisfactionChanged(Person& person, int old_value, int new_value) {
        (void)person;
        (void)old_value;
        (void)new_value;
    }

protected:
    virtual ~PersonObserver() = default;
};

class Person {
public:
    Person(const std::string& name, int age)
        : name_(name), age_(age) {}

    virtual ~Person() = default;

    int GetSatisfaction() const {
        return satisfaction_;
    }

    const std::string& GetName() const {
        return name_;
    }

    void SetObserver(PersonObserver* observer) {
        observer_ = observer;
    }

    int GetAge() const {
        return age_;
    }

    virtual void Dance() {
        ++ dance_count_;
        SetSatisfaction(satisfaction_ + 1);
    }

    int GetDanceCount() const {
        return dance_count_;
    }

    virtual void LiveADay() {
    }

protected:
    void SetSatisfaction(int satisfaction) {
        if (satisfaction != satisfaction_) {
            int old_value = satisfaction_;
            satisfaction_ = satisfaction;
            if (observer_) {
                observer_->OnSatisfactionChanged(*this, old_value, satisfaction_);
            }
        }
    }
    int satisfaction_ = 100;
    int dance_count_ = 0;

private:
    std::string name_;
    PersonObserver* observer_ = nullptr;
    int age_;
};

class Worker : public Person {
public:
    Worker(const std::string& name, int age)
        : Person(name, age) {}

    void Dance() override {
        ++ dance_count_; 
        if ((GetAge() > 30) && (GetAge() < 40)) {
            SetSatisfaction(GetSatisfaction() + 2);
        } else {
            SetSatisfaction(GetSatisfaction() + 1);
        }
    }

    void LiveADay() override {
        Work();
    }

    void Work() {
        ++ work_done_;
        SetSatisfaction(GetSatisfaction() - 5);
    }

    int GetWorkDone() const {
        return work_done_;
    }

private:
    int work_done_ = 0;
};

class Student : public Person {
public:
    Student(const std::string& name, int age)
        : Person(name, age) {}

    void LiveADay() override {
        Study();
    }

    void Study() {
        ++ knowledge_level_;
        SetSatisfaction(GetSatisfaction() - 3);
    }

    int GetKnowledgeLevel() const {
        return knowledge_level_;
    }

private:
    int knowledge_level_ = 0;
};