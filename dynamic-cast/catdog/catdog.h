#pragma once

#include <cstdio>

struct Animal {
    virtual void speak() { puts("hi"); }
    virtual ~Animal() {}
    Animal *as_animal() { return this; }
    int legs;
};

struct Cat : virtual Animal {
    void speak() override { puts("meow"); }
    Cat *as_cat() { return this; }
    int tails;
};

struct Dog : virtual Animal {
    int dogdata;
    Dog *as_dog() { return this; }
};

struct CatDog : Cat, Dog {
    int catdogdata;
    CatDog *as_catdog() { return this; }
};

struct Coral : protected Animal {
    int coraldata;
    Animal *as_animal() { return this; }
    Coral *as_coral() { return this; }
};

struct Fish : Animal {
    int fishdata;
    Fish *as_fish() { return this; }
};

struct Nemo : Fish, Coral {
    int nemodata;
    Nemo *as_nemo() { return this; }
};
