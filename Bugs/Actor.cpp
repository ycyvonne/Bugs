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

StudentWorld* EnergyHolder::world() const
{
    return m_world;
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

int Food::eat(int amt)
{
    int food = units();
    
    if(food >= amt){
        decreaseUnits(amt);
        return amt;
    }
    
    setUnits(0);
    killMe();
    return food;
}

//==============[ Actor > EnergyHolder > Pheromone ]====================================

Pheromone::Pheromone(int id, StudentWorld* sw, int idType, int startX, int startY, int colony)
    :EnergyHolder(id, sw, idType, startX, startY, right, 2)
{
    setUnits(256);
    m_colony = colony;
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
    setUnits(8999);
}

void Anthill::doSomething()
{
    updateStatus();
    
    Food* food;
    if(world()->hasFood(getX(), getY(), food))
    {
        addUnits(food->eat(10000));
    }
    
    //checks if enough to produce ant
    if(units() >= 2000)
    {
        //add ant
        decreaseUnits(1500);
        world()->spawnAnt(getX(), getY(), m_colony, m_compiler); //increments count of produced
    }
    
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
    world()->stunAll(getX(), getY());
}



//=======[ Actor > EnergyHolder > TriggerableActor > Poison ]============

Poison::Poison(int id, StudentWorld* sw, int startX, int startY)
    :TriggerableActor(id, sw, IID_POISON, startX, startY, right, 2)
{}

void Poison::doSomething()
{
    world()->poisonAll(getX(), getY());
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

void Insect::poison()
{
    decreaseUnits(150);
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

bool Insect::attemptToEat(int amt)
{
    Food* food;
    if(world()->hasFood(getX(), getY(), food))
    {
        addUnits(food->eat(amt));
        return true;
    }
    
    return false;
}



void Insect::getNextPos(int &nextX, int &nextY)
{
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
    nextX = x;
    nextY = y;
}

//========[ Actor > EnergyHolder > Insect > Ant ]======================

Ant::Ant(int id, StudentWorld *sw, Compiler *com, int imageID, int startX, int startY, int colony)
    :Insect(id, sw, imageID, startX, startY, right, 1500, 0)
{
    setRandomDir();
    m_colony = colony;
    m_compiler = com;
    m_ic = 0;
    m_foodUnits = 0;
    
    m_antHillX = startX;
    m_antHillY = startY;
    
    m_wasBit = false;
    m_wasBlocked = false;
}

bool Ant::isEnemy(int colony)
{
    return m_colony != colony;
}

void Ant::decreaseUnits(int units)
{
    EnergyHolder::decreaseUnits(units);
    m_wasBit = true;
}

void Ant::doesAction()
{
    if(!interpret())
        killMe();
}

void Ant::moveForward()
{
    //reset last blocked/bit on move
    m_wasBlocked = false;
    m_wasBit = false;
    
    int x, y;
    getNextPos(x,y);
    
    if(isBlocked(x, y))
    {
        m_wasBlocked = true;
        stun();
        return;
    }
    
    moveMeTo(x, y);
}

bool Ant::conditionIsTrue(Compiler::Command cmd)
{
    switch(stoi(cmd.operand1))
    {
        case Compiler::Condition::invalid_if:
            return false;
            
        case Compiler::Condition::i_smell_danger_in_front_of_me:
            int x, y;
            getNextPos(x, y);
            
            EnergyHolder *e;
            return world()->hasEnemy(x, y, m_colony, e);
            
        case Compiler::Condition::i_smell_pheromone_in_front_of_me:
            int x2, y2;
            getNextPos(x2, y2);
            world()->hasPheromone(x2, y2, m_colony);
            break;
            
        case Compiler::Condition::i_was_bit:
            return m_wasBit;
            
        case Compiler::Condition::i_am_carrying_food:
            return m_foodUnits > 0;
            
        case Compiler::Condition::i_am_hungry:
            return units() <= 25;
            
        case Compiler::Condition::i_am_standing_on_my_anthill:
            return getX() == m_antHillX && getY() == m_antHillY;
            
        case Compiler::Condition::i_am_standing_on_food:
            Food *f;
            return world()->hasFood(getX(), getY(), f);
            
        case Compiler::Condition::i_am_standing_with_an_enemy:
            EnergyHolder *e2;
            return world()->hasEnemy(getX(), getY(), m_colony, e2);
            
        case Compiler::Condition::i_was_blocked_from_moving:
            return m_wasBlocked;
            
        case Compiler::Condition::last_random_number_was_zero:
            return m_lastRandomNumberGenerated == 0;
            
    }
    return false;
}

void Ant::rotate(bool clockwise)
{
    Direction dir[4] = {up, right, down, left};
    int index = -1;
    for(int i = 0; i < 4; i++){
        if(dir[i] == getDirection()){
            index = i;
            break;
        }
    }
    
    if(index == -1) return;
    
    if(clockwise) index--;
    else index++;
    
    if(index > 3) index -= 4;
    else if(index < 0) index += 4;
    
    setDirection(dir[index]);
}

bool Ant::interpret()
{
    Compiler::Command cmd;
    bool shouldReturn = false;
    int numCommands = 0;
    
    while(!shouldReturn && numCommands < 10)
    {
        if (!m_compiler->getCommand(m_ic, cmd))
            return false;
        
        switch (cmd.opcode)
        {
            case Compiler::Opcode::moveForward:
                moveForward();
                ++m_ic;
                shouldReturn = true;
                break;
            case Compiler::Opcode::eatFood:
                if(m_foodUnits > 100)
                {
                    addUnits(100);
                    m_foodUnits -= 100;
                }
                else
                {
                    addUnits(m_foodUnits);
                    m_foodUnits = 0;
                }
                ++m_ic;
                break;
            case Compiler::Opcode::dropFood:
                world()->addFood(getX(), getY(), m_foodUnits);
                m_foodUnits = 0;
                ++m_ic;
                break;
            case Compiler::Opcode::bite:
                EnergyHolder *e;
                if(world()->hasEnemy(getX(), getY(), m_colony, e))
                {
                    e->decreaseUnits(15);
                }
                ++m_ic;
                break;
            case Compiler::Opcode::pickupFood:
                Food *f;
                if(world()->hasFood(getX(), getY(), f))
                {
                    int amtToEat;
                    if(m_foodUnits >= 1400) //max is 1800
                        amtToEat = 1800 - m_foodUnits;
                    else
                        amtToEat = 400;
                    
                    m_foodUnits += f->eat(amtToEat);
                }
                ++m_ic;
                break;
            case Compiler::Opcode::emitPheromone:
                world()->spawnPheromone(getX(), getY(), m_colony);
                ++m_ic;
                break;
            case Compiler::Opcode::faceRandomDirection:
                setRandomDir();
                ++m_ic;
                break;
            case Compiler::Opcode::rotateClockwise:
                rotate(true);
                ++m_ic;
                break;
            case Compiler::Opcode::rotateCounterClockwise:
                rotate(false);
                ++m_ic;
                break;
            case Compiler::Opcode::generateRandomNumber:
                m_lastRandomNumberGenerated = rand() % stoi(cmd.operand1);
                ++m_ic;
                break;
            case Compiler::Opcode::goto_command:
                m_ic = stoi(cmd.operand1);
                break;
            case Compiler::Opcode::if_command:
                if (conditionIsTrue(cmd))//conditionIsTrue(cmd)
                    m_ic = stoi(cmd.operand2);
                else
                    ++m_ic;
                break;
            default:
                ++m_ic;
                
        }
        numCommands++;
    }
    
    return true;
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

        int x, y;
        getNextPos(x, y);
        
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
        world()->spawnAdultGrasshopper(getX(), getY());
        world()->addFood(getX(), getY(), 100);
        killMe();
        return;
    }
    
    if(attemptToEat(200))
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
        if(world()->hasEnemy(getX(), getY(), -1, e))
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
    else if(!(attemptToEat(200) && randTwo == 0))
    {
        //if attempt to eat failed or attempt to eat successed & 50% chance
        move();
        return;
    }
    
    stun();
    
}

