#include "Actor.h"
#include "StudentWorld.h"
#include <iostream>

using namespace std;

// Students:  Add code to this file (if you wish), Actor.h, StudentWorld.h, and StudentWorld.cpp

//==============[ Actor ]=============================================
Actor::Actor(int id, int imageID, int startX, int startY,
             Direction dir, int depth)
        :GraphObject(imageID, startX, startY, dir, depth, 0.25)
{
    m_id = id;
    m_type = imageID;
    m_isInsect = false;
    setDirection(dir);
}

int Actor::type() const{
    return m_type;
}

int Actor::id() const{
    return m_id;
}

bool Actor::isInsect() const{
    return m_isInsect;
}

void Actor::setAsInsect(){
    m_isInsect = true;
}

void Actor::setRandomDir()
{
    int randDir = rand() % 4;
    switch(randDir)
    {
        case 0:
            setDirection(up);
            break;
        case 1:
            setDirection(down);
            break;
        case 2:
            setDirection(left);
            break;
        case 3:
            setDirection(right);
            break;
    }
}


//==============[ Actor > EnergyHolder ]====================================


EnergyHolder::EnergyHolder(int id, StudentWorld* sw,
             int imageID, int startX, int startY, Direction dir, int depth)
:Actor(id, imageID, startX, startY, dir, depth)
{
    m_world = sw;
}

bool EnergyHolder::isDead() const{
    return m_units <= 0;
}

int EnergyHolder::units() const
{
    return m_units;
}

void EnergyHolder::setUnits(int units)
{
    m_units = units;
}

void EnergyHolder::addUnits(int units)
{
    m_units += units;
}

void EnergyHolder::decreaseUnits(int units)
{
    m_units -= units;
}

bool EnergyHolder::updateStatus()
{
    decreaseUnits(1);
    if(isDead()){
        killMe();
        return true;
    }
    
    return false;
}

bool EnergyHolder::isBlocked(int x, int y) const{
    return m_world->isBlocked(x, y);
}

void EnergyHolder::addFood(int x, int y)
{
    m_world->addFood(x, y);
}

bool EnergyHolder::hasFood(int x, int y, Food*& f)
{
    return m_world->hasFood(x, y, f);
}

void EnergyHolder::moveMeTo(int x, int y)
{
    
    cout << "id #: " << id() << " moves to (" << x << ", " << y << ")" << endl;
    
    int thisX = getX();
    int thisY = getY();
    
    //update display
    moveTo(x, y);
    
    //update world
    m_world->moveActor(id(), thisX, thisY, x, y);
}

void EnergyHolder::killMe()
{
    cout << "killing id " << id() << ", which is type: ";
    if(type() == IID_FOOD)
        cout << "food." << endl;
    else if(type() == IID_BABY_GRASSHOPPER)
        cout << "baby grasshopper." << endl;
    else if(type() == IID_ADULT_GRASSHOPPER)
        cout << "adult grasshopper." << endl;
    
    //update display
    setVisible(false);
    
    //update world
    m_world->setFlag(getX(), getY());
    m_world->killActor(id(), getX(), getY());
}

void EnergyHolder::spawnAdultGrasshopper(int x, int y)
{
    m_world->spawnAdultGrasshopper(x, y);
}

bool EnergyHolder::hasEnemy(int x, int y, int colony, EnergyHolder*& a)
{
    return m_world->hasEnemy(x, y, colony, a);
}

void EnergyHolder::stunAll(int x, int y)
{
    m_world->stunAll(x, y);
}

//==============[ Actor > Pebble ]====================================
//sets depth = 1
Pebble::Pebble(int id, int startX, int startY)
        :Actor(id, IID_ROCK, startX, startY, right, 1) {}

//==============[ Actor > EnergyHolder > Food ]====================================

Food::Food(int id, StudentWorld* sw, int startX, int startY, bool dueToDeath)
    :EnergyHolder(id, sw, IID_FOOD, startX, startY, right, 2)
{
    if(dueToDeath)
        setUnits(100);
    else
        setUnits(6000);
}

void Food::addCarcass()
{
    addUnits(100);
}

int Food::eat()
{
    int food = units();
    
    if(food >= 200){
        decreaseUnits(200);
        return 200;
    }
    
    setUnits(0);
    killMe();
    return food;
}

//==============[ Actor > EnergyHolder > Pheromone ]====================================

Pheromone::Pheromone(int id, StudentWorld* sw, int idType, int startX, int startY)
    :EnergyHolder(id, sw, idType, startX, startY, right, 2)
{
    setUnits(256);
}

void Pheromone::doSomething()
{
    updateStatus();
}


//==============[ Actor > EnergyHolder > Anthill ]====================================


Anthill::Anthill(int id, StudentWorld* sw, Compiler* com, int colony, int startX, int startY)
    :EnergyHolder(id, sw, IID_ANT_HILL, startX, startY, right, 2)
{
    m_colony = colony;
    m_compiler = com;
}

void Anthill::doSomething()
{
    decreaseUnits(1);
    //TODO: more stuff
}

TriggerableActor::TriggerableActor(int id, StudentWorld* sw,
                                   int imageID, int startX, int startY, Direction dir, int depth)
 :EnergyHolder(id, sw, imageID, startX, startY, dir, depth)
{
    
}

//=====[ Actor > EnergyHolder > TriggerableActor > Pool ]============

Pool::Pool(int id, StudentWorld* sw, int startX, int startY)
    :TriggerableActor(id, sw, IID_WATER_POOL, startX, startY, right, 2) {}

void Pool::doSomething()
{
    stunAll(getX(), getY());
}



//=======[ Actor > EnergyHolder > TriggerableActor > Poison ]============

Poison::Poison(int id, StudentWorld* sw, int startX, int startY)
    :TriggerableActor(id, sw, IID_POISON, startX, startY, right, 2)
{

}

void Poison::doSomething()
{
    //TODO
    //poison all insects on square
}


//==============[ Actor > EnergyHolder > Insect ]====================================
//sets depth = 1, passes along all other params
Insect::Insect(int id, StudentWorld* sw,
               int imageID, int startX, int startY,
               Direction dir, int startingHealth, int stuns)
        :EnergyHolder(id, sw, imageID, startX, startY, dir, 1)
{
    setUnits(startingHealth);
    m_stunnedTicksRemaining = stuns;
    setAsInsect();
    m_stunned = false;
}

void Insect::stun()
{
    if(!m_stunned){ //prevent multiple stuns by same
        m_stunnedTicksRemaining = 2;
    }
    
    m_stunned = true;
}

void Insect::doSomething()
{
    if(updateStatus()) //if died, return immediately
        return;
    
    if(m_stunnedTicksRemaining > 0)
    {
        m_stunnedTicksRemaining--;
        return;
    }

    m_stunned = false;
    m_stunnedTicksRemaining = 2;
        
    //movement & death related
    doesAction();
    
}

bool Insect::attemptToEat()
{
    Food* food;
    if(hasFood(getX(), getY(), food))
    {
        addUnits(food->eat());
        return true;
    }
    
    return false;
}

//========[ Actor > EnergyHolder > Insect > Grasshopper ]======================
//sets stuns = 0
Grasshopper::Grasshopper(int id, StudentWorld *sw, int imageID, int startX, int startY, int health)
        :Insect(id, sw, imageID, startX, startY, right, health, 0)
{
    setRandomDistance();
    setRandomDir();
    m_bite = 0; //adult is 50
}

int Grasshopper::distance()
{
    return m_distance;
}

void Grasshopper::setRandomDistance()
{
    m_distance = rand() % 9 + 2; //between [2, 10]
}

void Grasshopper::move()
{
    if(makeChecks()){

        int x = getX();
        int y = getY();
        switch(getDirection())
        {
            case up:
                y--;
                break;
            case down:
                y++;
                break;
            case left:
                x--;
                break;
            case right:
                x++;
                break;
            default:
                break;
                
        }
        
        if(isBlocked(x, y))
        {
            m_distance = 0;
            stun();
            return;
        }
        
        moveMeTo(x, y);
        
        m_distance--;
    }
}

bool Grasshopper::makeChecks()
{
    if(distance() <= 0)
    {
        setRandomDir();
        setRandomDistance();
        return false;
    }
    
    return true;
}


//=========[ Actor > EnergyHolder > Insect > Grasshopper > BabyGrasshopper ]========
//sets imageID, health = 500
BabyGrasshopper::BabyGrasshopper(int id, StudentWorld *sw, int startX, int startY)
        :Grasshopper(id, sw, IID_BABY_GRASSHOPPER, startX, startY, 500)
{}

void BabyGrasshopper::doesAction()
{
    if(units() >= 1600)
    {
        //create adult
        cout << "creating adult " << endl;
        spawnAdultGrasshopper(getX(), getY());
        addFood(getX(), getY());
        killMe();
        return;
    }
    
    if(attemptToEat())
    {
        int random = rand() % 2;
        if(random == 0){
            stun();
            return;
        }
    }
    
    move();
    
}

//=======[ Actor > EnergyHolder > Insect > Grasshopper > AdultGrasshopper ]========
AdultGrasshopper::AdultGrasshopper(int id, StudentWorld* sw, int startX, int startY)
        :Grasshopper(id, sw, IID_ADULT_GRASSHOPPER, startX, startY, 1600)
{
    move();
}

#include <cmath>
void AdultGrasshopper::doesAction()
{
    int randTwo = rand() % 2;
    int randThree = rand() % 3;
    int randTen = rand() % 10;
    
    if(randThree == 0) //0.33 chance
    {
        //check enemy on square
        //if there, choose one random
        //bite => -50 HP
        
        EnergyHolder *e;
        if(hasEnemy(getX(), getY(), -1, e))
        {
            e->decreaseUnits(50);
        }
    }
    else if(randTen == 0) //0.1 chance
    {
        //select square in 10 square radius using cos(), sin()
        int randRadius = rand() % 10 + 1;
        int randRadians = (rand() % 360) * 3.1415926535 / 180;
        
        int x = getX() + randRadius * cos(randRadians);
        int y = getY() + randRadius * sin(randRadians);
        
        //move if within bounds and not blocked by rock
        if(!isBlocked(x, y) && x > 0 && y > 0 && x < VIEW_WIDTH && y < VIEW_HEIGHT)
            moveMeTo(x, y);
    }
    else if(!(attemptToEat() && randTwo == 0))
    {
        //if attempt to eat failed or attempt to eat successed & 50% chance
        move();
        return;
    }
    
    stun();
    
}

