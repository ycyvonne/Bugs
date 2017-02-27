#ifndef ACTOR_H_
#define ACTOR_H_

#include "GraphObject.h"
#include "GameConstants.h"

class StudentWorld;
class Compiler;
class Food;

// Students:  Add code to this file, Actor.cpp, StudentWorld.h, and StudentWorld.cpp

class Actor: public GraphObject{
public:
    Actor(int id, int imageID, int startX, int startY, Direction dir, int depth);
    virtual void doSomething() {}
    
    //accessors
    int     type() const;       //according to IID
    int     id() const;         //from the unique ID produced at the start
    bool    isInsect() const;  //is biteable, stunnable, and poisonable
    
protected:
    //mutators
    void setRandomDir();
    void setAsInsect();
    
    
private:
    bool m_isInsect;
    int m_id;
    int m_type;
    
};

class Pebble: public Actor{
public:
    Pebble(int id, int startX, int startY);
    virtual void doSomething() {}
    
private:
    
    
};

class EnergyHolder: public Actor{
    
public:
    EnergyHolder(int id, StudentWorld* sw,
                 int imageID, int startX, int startY, Direction dir, int depth);
    
    //accessors
    int     units() const;
    bool    isDead() const;
    bool    isBlocked(int x, int y) const; //looks into m_world to determine if a rock is blocking
    bool    hasFood(int x, int y, Food*& a);


protected:
    
    //mutators
    void moveMeTo(int x, int y);
    bool updateStatus(); //decrease unit and check death
    void killMe();
    void spawnAdultGrasshopper(int x, int y, int health);
    
    void setUnits(int units);
    void addUnits(int units);
    void decreaseUnits(int units);
    
    
private:
    StudentWorld* m_world;
    int m_units;
};

class Food: public EnergyHolder{
public:
    Food(int id, StudentWorld* sw, int startX, int startY, bool dueToDeath);
    virtual void doSomething() {}
    
    //mutator
    void addCarcass();
    int eat();
    
private:
    int m_food;
};

class Pheromone: public EnergyHolder{
public:
    Pheromone(int id, StudentWorld* sw, int idType, int startX, int startY);
    virtual void doSomething();

private:
    
    
};


class Anthill: public EnergyHolder{
public:
    Anthill(int id, StudentWorld* sw, Compiler* com, int colony, int startX, int startY);
    virtual void doSomething();
private:
    int m_colony;
    Compiler* m_compiler;
};


class TriggerableActor: public EnergyHolder{
public:
    TriggerableActor(int id, StudentWorld* sw,
                     int imageID, int startX, int startY, Direction dir, int depth);
private:
    
};

class Pool: public TriggerableActor{
public:
    Pool(int id, StudentWorld* sw, int startX, int startY);
    virtual void doSomething();
    
private:
};

class Poison: public TriggerableActor{
public:
    Poison(int id, StudentWorld* sw, int startX, int startY);
    virtual void doSomething();
    
private:
    
};

class Insect: public EnergyHolder{
public:
    Insect(int id, StudentWorld* sw,
           int imageID, int startX, int startY, Direction dir, int startingHealth, int stuns);
    
    //overrides base
    virtual void doSomething();
    //mean to be overriden in subclasses
    virtual void doesAction() {}
    
    //mutators
    void getHungrier();
    bool attemptToEat();
    void stun();
    
    
private:
    int m_stunnedTicksRemaining;
};

class Grasshopper: public Insect{
public:
    Grasshopper(int id, StudentWorld *sw,
                int imageID, int startX, int startY, int health);
    
    //mutators
    virtual void doesAction() {}
    virtual bool makeChecks() {return false;} //if makeChecks returns true, grasshopper will move
    virtual void move();
    
    void setRandomDistance();
    
    //accessors
    int distance();
    
private:
    int m_distance;
    int m_bite;
    
};


class BabyGrasshopper: public Grasshopper{
public:
    BabyGrasshopper(int id, StudentWorld* sw, int startX, int startY);
    
    //mutators
    virtual void doesAction();
    virtual bool makeChecks();
private:
    
};

class AdultGrasshopper: public Grasshopper{
public:
    AdultGrasshopper(int id, StudentWorld* sw, int startX, int startY, int health);
    
    //mutators
    virtual void doesAction();
    virtual bool makeChecks();
private:
    
};

#endif // ACTOR_H_
