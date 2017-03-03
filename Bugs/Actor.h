#ifndef ACTOR_H_
#define ACTOR_H_

#include "GraphObject.h"
#include "GameConstants.h"
#include "Compiler.h"

class StudentWorld;
class Food;
// Students:  Add code to this file, Actor.cpp, StudentWorld.h, and StudentWorld.cpp

class Actor: public GraphObject{
public:
    
    //constructor
    Actor(int id, int imageID, int startX, int startY, Direction dir, int depth);
    
    //accessors
    int     type() const;       //according to IID
    int     id() const;         //from the unique ID produced at the start
    virtual bool isInsect() const {return false;}  //is biteable, stunnable, and poisonable
    bool    isDeleted() const;
    
    //mutators
    virtual void doSomething() {}
    void    deleteMe();
    
protected:
    void setRandomDir();
    
private:
    int m_id;
    int m_type;
    bool m_isDeleted;
};

class Pebble: public Actor{
public:
    
    //constructor
    Pebble(int id, int startX, int startY);
    
    //mutators
    virtual void doSomething() {}
    
private:
    
    
};

class EnergyHolder: public Actor{
    
public:
    
    //constructor
    EnergyHolder(int id, StudentWorld* sw,
                 int imageID, int startX, int startY, Direction dir, int depth);
    
    //accessors
    int     units() const;
    bool    isDead() const;
    StudentWorld* world() const;
    
    //mutators
    void setUnits(int units);
    void addUnits(int units);
    virtual void decreaseUnits(int units);

protected:
    
    //mutators
    void moveMeTo(int x, int y);
    bool updateStatus(); //decrease unit and check death
    void killMe();
    
private:
    StudentWorld* m_world;
    int m_units;
};

class Food: public EnergyHolder{
public:
    
    //constants
    const int STARTING_UNITS = 6000;
    const int CARCASS_UNITS = 100;
    
    //constructor
    Food(int id, StudentWorld* sw, int startX, int startY, bool dueToDeath);
    
    //mutators
    virtual void doSomething() {}
    void addCarcass();
    int eat(int amt);
    
private:
    int m_food;
};

class Pheromone: public EnergyHolder{
public:
    
    //constants
    const int STARTING_UNITS = 256;
    
    //constructor
    Pheromone(int id, StudentWorld* sw, int idType, int startX, int startY, int colony);
    
    //mutators
    virtual void doSomething();

private:
    int m_colony;
    
};


class Anthill: public EnergyHolder{
public:
    
    //constants
    const int STARTING_UNITS = 8999;
    const int ATTEMPT_TO_EAT = 10000;
    const int SPAWN_ANT_THRESHOLD = 2000;
    const int ANT_HEALTH_UNITS = 1500;
    
    //constructor
    Anthill(int id, StudentWorld* sw, Compiler* com, int colony, int startX, int startY);
    
    //mutators
    virtual void doSomething();
    void spawnAnt();
    
private:
    int m_colony;
    Compiler* m_compiler;
};


class TriggerableActor: public EnergyHolder{
public:
    
    //constructor
    TriggerableActor(int id, StudentWorld* sw,
                     int imageID, int startX, int startY, Direction dir, int depth);
};

class Pool: public TriggerableActor{
public:
    
    //constructor
    Pool(int id, StudentWorld* sw, int startX, int startY);
    
    //mutators
    virtual void doSomething();
    
};

class Poison: public TriggerableActor{
public:
    
    //constructor
    Poison(int id, StudentWorld* sw, int startX, int startY);
    
    //mutators
    virtual void doSomething();
};

class Insect: public EnergyHolder{
public:
    
    //constants
    const int POISON_STRENGTH = 150;
    const int CARCASS_UNITS = 100;
    
    //constructor
    Insect(int id, StudentWorld* sw,
           int imageID, int startX, int startY, Direction dir, int startingHealth, int stuns);
    
    
    //accessors
    virtual bool isInsect() const {return true;} //overrides base
    virtual bool isEnemy(int colony) const {return true;} //meant to be overriden in ant
                                                          //which will specify based on colony
    
    //mutators
    virtual void doSomething();
    virtual void doesAction() {} //meant to be overriden in derived
                                 //is called within doSomething()
                                 //and hence specializes the method
    void poison();
    void getHungrier();
    bool attemptToEat(int amt);
    void stun();

protected:
    virtual int getMaxStunnedTurns() const;
    void getNextPos(int &nextX, int &nextY);
    
private:
    int m_stunnedTicksRemaining;
    int m_maxStunnedTurns;
    bool m_stunned;
};

class Ant: public Insect{
    
public:
    
    //constants
    const int HUNGER_THRESHOLD = 25;
    const int ATTEMPT_TO_EAT = 100;
    const int ATTEMPT_TO_CARRY = 400;
    const int MAX_FOOD_CARRY = 1800;
    
    //constructor
    Ant(int id, StudentWorld *sw, Compiler *com, int imageID, int startX, int startY, int colony);

    //accessors
    virtual bool isEnemy(int colony) const;
    
    //mutators
    virtual void doesAction();
    virtual void decreaseUnits(int units);

protected:
    virtual int getMaxStunnedTurns() const;
    
private:
    int m_colony;
    int m_ic;
    int m_foodUnits;
    int m_lastRandomNumberGenerated;
    int m_maxStunnedTurns;
    
    int m_antHillX;
    int m_antHillY;
    
    bool m_wasBit;
    bool m_wasBlocked;
    
    Compiler *m_compiler;
    
    //helper methods for doesAction()
    void moveForward();
    void rotate(bool clockwise);
    bool conditionIsTrue(Compiler::Command cmd);
    bool interpret();
    
};

class Grasshopper: public Insect{
public:
    
    //constants
    const int ATTEMPT_TO_EAT = 200;
    
    //constructor
    Grasshopper(int id, StudentWorld *sw,
                int imageID, int startX, int startY, int health);
    
    //accessors
    int distance() const;
    
    //mutators
    virtual void doesAction() {}
    virtual void move();
    bool makeChecks(); //if makeChecks returns true, grasshopper will move
    void setRandomDistance();
    
private:
    int m_distance;
    
};


class BabyGrasshopper: public Grasshopper{
public:
    
    //constants
    const int MAX_HEALTH = 1600;
    
    //constructor
    BabyGrasshopper(int id, StudentWorld* sw, int startX, int startY);
    
    //mutators
    virtual void doesAction();
private:
    
};

class AdultGrasshopper: public Grasshopper{
public:
    
    //constants
    const int BITE_STRENGTH = 50;
    const int MAX_JUMP_RADIUS = 10;
    
    //constructor
    AdultGrasshopper(int id, StudentWorld* sw, int startX, int startY);
    
    //mutators
    virtual void doesAction();
private:
    
};

#endif // ACTOR_H_
