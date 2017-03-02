#include "StudentWorld.h"
#include "Field.h"
#include "Actor.h"
#include "Compiler.h"
#include <string>

using namespace std;


GameWorld* createStudentWorld(string assetDir)
{
	return new StudentWorld(assetDir);
}

// Students:  Add code to this file (if you wish), StudentWorld.h, Actor.h and Actor.cpp

StudentWorld::StudentWorld(string assetDir)
    : GameWorld(assetDir)
{
    m_flagIndex = indexPair(-1,-1);
}

void StudentWorld::setFlag(int x, int y)
{
    m_flagIndex = indexPair(x, y);
}

void StudentWorld::resetFlag()
{
    m_flagIndex = indexPair(-1, -1);
}

StudentWorld::~StudentWorld()
{
    cleanUp();
}

void StudentWorld::cleanUp()
{
    for(int i = 0; i < VIEW_WIDTH; i++)
    {
        for(int j = 0; j < VIEW_HEIGHT; j++)
        {
            indexPair index (i, j);
            while(m_map.count(index) > 0)
            {
                mmap::iterator it = m_map.find(index);
                delete it->second;
                m_map.erase (it);
            }
        }
    }
}

int StudentWorld::init()
{
    m_tickCount = 0;
    m_currentUniqueId = 0;
    
    Field f;
    string fileName = getFieldFilename();
    string error;
    
    if(f.loadField(fileName, error) != Field::LoadResult::load_success){
        setError(fileName + " " + error);
        return false;
    }
    
    vector<string> files = getFilenamesOfAntPrograms();
    
    Compiler* compilers[4];
    string cError;
    
    for(int i = 0; i < 4; i++)
    {
        compilers[i] = new Compiler;
        if (!compilers[i]->compile(files[i], cError))
        {
            setError(files[i] + " " + cError);
            return GWSTATUS_LEVEL_ERROR;
        }

        m_colonyAntCount.push_back(0);
        m_colonyNames.push_back(compilers[i]->getColonyName());
    }
    
    for(int i = 0; i < VIEW_WIDTH; i++)
    {
        for(int j = 0; j < VIEW_HEIGHT; j++)
        {
            Field::FieldItem
            fItem = f.getContentsOf(i, j);
            indexPair index (i, j);
            
            int anthill = -1;
            switch(fItem)
            {
                case Field::FieldItem::rock:
                    m_map.insert(mmapPair(index, new Pebble(m_currentUniqueId++, i, j)));
                    break;
                case Field::FieldItem::food:
                    m_map.insert(mmapPair(index, new Food(m_currentUniqueId++, this, i, j, false)));
                    break;
                case Field::FieldItem::water:
                    m_map.insert(mmapPair(index, new Pool(m_currentUniqueId++, this, i, j)));
                    break;
                case Field::FieldItem::poison:
                    m_map.insert(mmapPair(index, new Poison(m_currentUniqueId++, this, i, j)));
                    break;
                case Field::FieldItem::anthill0:
                    anthill = 0;
                    break;
                case Field::FieldItem::anthill1:
                    anthill = 1;
                    break;
                case Field::FieldItem::anthill2:
                    anthill = 2;
                    break;
                case Field::FieldItem::anthill3:
                    anthill = 3;
                    break;
                case Field::FieldItem::grasshopper:
                    m_map.insert(mmapPair(index, new BabyGrasshopper(m_currentUniqueId++, this, i, j)));
                    break;
                default:
                    break;
            }
            
            if(anthill != -1)
                m_map.insert(mmapPair(index, new Anthill(m_currentUniqueId++, this, compilers[anthill], anthill, i, j)));
        }
    }
    
    return GWSTATUS_CONTINUE_GAME;
}

void StudentWorld::setDisplayText()
{
    string s = "Ticks: ";
    s += to_string(END_GAME_TICKS - m_tickCount);
    s += " - ";
    
    for(int i = 0; i < m_colonyNames.size(); i++)
    {
        s += m_colonyNames[i];
        s += ": ";
        s += to_string(m_colonyAntCount[i]);
        s += " ants ";
    }
    
    setGameStatText(s);
}

int StudentWorld::move()
{
    
    for(int i = 0; i < VIEW_HEIGHT; i++)
    {
        for(int j = 0; j < VIEW_WIDTH; j++)
        {
            indexPair index (i, j);
            mmap::iterator it = m_map.equal_range(index).first;
            
            while(it != m_map.equal_range(index).second)
            {
                Actor *cur = it->second;
                int oldX = cur->getX();
                int oldY = cur->getY();
                
                cur->doSomething();
                
                int newX = cur->getX();
                int newY = cur->getY();
                
                if(oldX != newX || oldY != newY || i == m_flagIndex.first || j == m_flagIndex.second)
                {
                    it = m_map.equal_range(index).first;
                    resetFlag();
                }
                else
                {
                    it++;
                }
                
            }
        }
    }
    
    setDisplayText();
    
    m_tickCount++;
    
    //check endgame
    if(m_tickCount >= END_GAME_TICKS)
    {
        if(winningAntExists())
        {
            setWinner(getWinnerName(m_winner));
            return GWSTATUS_PLAYER_WON;
        }
        return GWSTATUS_NO_WINNER;
    }
    
    return GWSTATUS_CONTINUE_GAME;
}

bool StudentWorld::isBlocked(int x, int y)
{
    indexPair index (x, y);
    mmap::iterator it;
    for(it = m_map.equal_range(index).first; it != m_map.equal_range(index).second; ++it){
        if(it->second->type() == IID_ROCK)
            return true;
    }
    
    return false;
}

void StudentWorld::killActor(int id, int x, int y)
{
    removeActor(true, id, x, y);
}


//if hardDelete, will delete Actor* at the location
//else it is a soft delete, to move Actor* elsewhere
void StudentWorld::removeActor(bool hardDelete, int id, int x, int y)
{
    indexPair index (x, y);
    mmap::iterator it;
    for (it = m_map.equal_range(index).first; it != m_map.equal_range(index).second; ++it){
        if(it->second->id() == id)
        {
            if(hardDelete)
                delete it->second;
            
            m_map.erase(it);
            return;
        }
    }
}

void StudentWorld::insertActor(Actor* a, int x, int y)
{
    indexPair index (x, y);
    m_map.insert(mmapPair(index, a));
}

bool StudentWorld::moveActor(int id, int xStart, int yStart, int xEnd, int yEnd)
{
    indexPair index (xStart, yStart);
    mmap::iterator it;
    
    for (it = m_map.equal_range(index).first; it != m_map.equal_range(index).second; ++it){
        if(it->second->id() == id)
        {
            Actor* temp = it->second;
            insertActor(temp, xEnd, yEnd);
            removeActor(false, id, xStart, yStart);
            return true;
        }
    }
    
    return false;
}

void StudentWorld::spawnAdultGrasshopper(int x, int y)
{
    Actor* a = new AdultGrasshopper(m_currentUniqueId++, this, x, y);
    insertActor(a, x, y);
}

void StudentWorld::spawnAnt(int x, int y, int colony, Compiler* com)
{
    int imageId;
    
    switch(colony)
    {
        case 0:
            imageId = IID_ANT_TYPE0;
            break;
        case 1:
            imageId = IID_ANT_TYPE1;
            break;
        case 2:
            imageId = IID_ANT_TYPE2;
            break;
        case 3:
            imageId = IID_ANT_TYPE3;
            break;
        default:
            imageId = IID_ANT_TYPE0;
    }
    
    Actor* a = new Ant(m_currentUniqueId++, this, com, imageId, x, y, colony);
    insertActor(a, x, y);
    
    m_colonyAntCount[colony]++;
}

void StudentWorld::spawnPheromone(int x, int y, int colony)
{
    int imageId;
    switch(colony)
    {
        case 0:
            imageId = IID_PHEROMONE_TYPE0;
            break;
        case 1:
            imageId = IID_PHEROMONE_TYPE1;
            break;
        case 2:
            imageId = IID_PHEROMONE_TYPE2;
            break;
        case 3:
            imageId = IID_PHEROMONE_TYPE3;
            break;
        default:
            imageId = IID_PHEROMONE_TYPE0;
    }
    
    Actor* a = new Pheromone(m_currentUniqueId++, this, imageId, x, y, colony);
    insertActor(a, x, y);
    
}

bool StudentWorld::hasPheromone(int x, int y, int myColony)
{
    int types[4] = {IID_PHEROMONE_TYPE0, IID_PHEROMONE_TYPE1, IID_PHEROMONE_TYPE2, IID_PHEROMONE_TYPE3};
    
    indexPair index (x, y);
    mmap::iterator it;
    for(it = m_map.equal_range(index).first; it != m_map.equal_range(index).second; ++it){
        if(it->second->type() == types[myColony])
        {
            return true;
        }
    }
    
    return false;
}

void StudentWorld::stunAll(int x, int y)
{    
    indexPair index (x, y);
    mmap::iterator it;
    for(it = m_map.equal_range(index).first; it != m_map.equal_range(index).second; ++it){
        if(it->second->isInsect()){
            static_cast<Insect*>(it->second)->stun();
        }
    }
}

void StudentWorld::poisonAll(int x, int y)
{
    indexPair index (x, y);
    mmap::iterator it;
    for(it = m_map.equal_range(index).first; it != m_map.equal_range(index).second; ++it){
        if(it->second->isInsect()){
            static_cast<Insect*>(it->second)->poison();
        }
    }
}

bool StudentWorld::winningAntExists()
{
    int maxAntsProduced = 0;
    for(int i = 0; i < m_colonyAntCount.size(); i++)
    {
        if(m_colonyAntCount[i] > maxAntsProduced)
        {
            maxAntsProduced = m_colonyAntCount[i];
            m_winner = i;
        }
    }
    
    int count = 0;
    
    //check duplicates
    for(int i = 0; i < m_colonyAntCount.size(); i++)
    {
        if(m_colonyAntCount[i] == maxAntsProduced)
            count++;
    }
    
    return count == 1 && maxAntsProduced != 0;
}

string StudentWorld::getWinnerName(int colony)
{
    return m_colonyNames[colony];
}

void StudentWorld::addFood(int x, int y, int amt)
{
    Food *f;
    if(hasFood(x, y, f))
    {
        f->addCarcass();
    }
    else
    {
        insertActor(new Food(m_currentUniqueId++, this, x, y, true), x, y);
    }
}

bool StudentWorld::hasFood(int x, int y, Food*& a)
{
    indexPair index (x, y);
    mmap::iterator it;
    for(it = m_map.equal_range(index).first; it != m_map.equal_range(index).second; ++it){
        if(it->second->type() == IID_FOOD){
            a = static_cast<Food*>(it->second);
            return true;
        }
    }
    
    return false;
}

bool StudentWorld::hasEnemy(int x, int y, int colony, EnergyHolder*& a)
{
    indexPair index (x, y);
    mmap::iterator it;
    for(it = m_map.equal_range(index).first; it != m_map.equal_range(index).second; ++it){
        Actor* cur = it->second;
        if(colony == -1 && cur->isInsect()) //grasshoppers & ants
        {
            a = static_cast<EnergyHolder*>(cur);
            return true;
        }
        else if(colony >= 0 && colony <= 3 &&
                cur->isInsect() && static_cast<Insect*>(cur)->isEnemy(colony))
        {
            a = static_cast<EnergyHolder*>(cur);
            return true;
        }
    }
    return false;
}
