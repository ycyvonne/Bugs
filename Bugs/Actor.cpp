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
    setDirection(dir);
    m_isDeleted = false;
}

int Actor::type() const{
    return m_type;
}

int Actor::id() const{
    return m_id;
}

bool Actor::isDeleted() const{
    return m_isDeleted;
}

void Actor::deleteMe(){
    m_isDeleted = true;
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

StudentWorld* EnergyHolder::world() const
{
    return m_world;
}

void EnergyHolder::moveMeTo(int x, int y)
{
    int thisX = getX();
    int thisY = getY();
    
    //update display
    moveTo(x, y);
    
    //update world
    m_world->moveActor(id(), thisX, thisY, x, y);
}
void EnergyHolder::killMe()
{    
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
        setUnits(Food::CARCASS_UNITS);
    else
        setUnits(Food::STARTING_UNITS);
}

void Food::addCarcass()
{
    addUnits(Food::CARCASS_UNITS);
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
    setUnits(Pheromone::STARTING_UNITS);
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
    setUnits(Anthill::STARTING_UNITS);
}

void Anthill::doSomething()
{
    updateStatus();
    
    Food* food;
    if(world()->hasFood(getX(), getY(), food))
    {
        addUnits(food->eat(Anthill::ATTEMPT_TO_EAT));
    }
    
    //checks if enough to produce ant
    if(units() >= SPAWN_ANT_THRESHOLD)
    {
        //add ant
        decreaseUnits(Anthill::ANT_HEALTH_UNITS);
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
    m_stunned = false;
    m_maxStunnedTurns = 2;
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
    decreaseUnits(Insect::POISON_STRENGTH);
}

int Insect::getMaxStunnedTurns() const
{
    return m_maxStunnedTurns;
}

void Insect::doSomething()
{
    if(updateStatus()) //if died, drop food and return immediately
    {
        world()->addFood(getX(), getY(), Insect::CARCASS_UNITS);
        return;
    }
    
    
    if(m_stunnedTicksRemaining > 0)
    {
        m_stunnedTicksRemaining--;
        return;
    }

    m_stunned = false;
    m_stunnedTicksRemaining = getMaxStunnedTurns();
        
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
            y++;
            break;
        case down:
            y--;
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
    m_lastRandomNumberGenerated = 0;
    
    m_maxStunnedTurns = 0;
}

int Ant::getMaxStunnedTurns() const
{
    return m_maxStunnedTurns;
}

bool Ant::isEnemy(int colony) const
{
    return m_colony != colony;
}

void Ant::decreaseUnits(int units)
{
    EnergyHolder::decreaseUnits(units);
    
    if(units != 1)
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
    
    if(world()->isBlocked(x, y))
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
            return world()->hasPheromone(x2, y2, m_colony);
            
        case Compiler::Condition::i_was_bit:
            return m_wasBit;
            
        case Compiler::Condition::i_am_carrying_food:
            return m_foodUnits > 0;
            
        case Compiler::Condition::i_am_hungry:
            return units() <= Ant::HUNGER_THRESHOLD;
            
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
    
    if(clockwise) index++;
    else index--;
    
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
        
        shouldReturn = true;
        
        switch (cmd.opcode)
        {
            case Compiler::Opcode::moveForward:
                moveForward();
                ++m_ic;
                break;
            case Compiler::Opcode::eatFood:
                if(m_foodUnits > Ant::ATTEMPT_TO_EAT)
                {
                    addUnits(Ant::ATTEMPT_TO_EAT);
                    m_foodUnits -= Ant::ATTEMPT_TO_EAT;
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
                    if(m_foodUnits >= Ant::MAX_FOOD_CARRY - Ant::ATTEMPT_TO_CARRY) //max is 1800
                        amtToEat = Ant::MAX_FOOD_CARRY - m_foodUnits;
                    else
                        amtToEat = Ant::ATTEMPT_TO_CARRY;
                    
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
                shouldReturn = false;
                break;
            case Compiler::Opcode::goto_command:
                m_ic = stoi(cmd.operand1);
                shouldReturn = false;
                break;
            case Compiler::Opcode::if_command:
                if (conditionIsTrue(cmd))//conditionIsTrue(cmd)
                    m_ic = stoi(cmd.operand2);
                else
                    ++m_ic;
                shouldReturn = false;
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
}

int Grasshopper::distance() const
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
        
        if(world()->isBlocked(x, y))
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
    if(units() >= BabyGrasshopper::MAX_HEALTH)
    {
        //create adult
        world()->spawnAdultGrasshopper(getX(), getY());
        world()->addFood(getX(), getY(), Insect::CARCASS_UNITS);
        killMe();
        return;
    }
    
    if(attemptToEat(Grasshopper::ATTEMPT_TO_EAT))
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
            e->decreaseUnits(AdultGrasshopper::BITE_STRENGTH);
        }
    }
    else if(randTen == 0) //0.1 chance
    {
        //select square in 10 square radius using cos(), sin()
        int randRadius = rand() % AdultGrasshopper::MAX_JUMP_RADIUS + 1;
        int randRadians = (rand() % 360) * 3.1415926535 / 180;
        
        int x = getX() + randRadius * cos(randRadians);
        int y = getY() + randRadius * sin(randRadians);
        
        //move if within bounds and not blocked by rock
        if(!world()->isBlocked(x, y) && x > 0 && y > 0 && x < VIEW_WIDTH && y < VIEW_HEIGHT)
            moveMeTo(x, y);
    }
    else if(!(attemptToEat(Grasshopper::ATTEMPT_TO_EAT) && randTwo == 0))
    {
        //if attempt to eat failed or attempt to eat successed & 50% chance
        move();
        return;
    }
    
    stun();
    
}

