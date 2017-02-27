#ifndef STUDENTWORLD_H_
#define STUDENTWORLD_H_

#include "GameWorld.h"
#include "GameConstants.h"
#include "Actor.h"
#include <string>
#include <map>

// Students:  Add code to this file, StudentWorld.cpp, Actor.h, and Actor.cpp

//reduce typing with typedefs
typedef std::multimap<std::pair<int, int>, Actor* > mmap;
typedef std::pair<int, int> indexPair;
typedef std::pair<indexPair, Actor*> mmapPair;

const int END_GAME_TICKS = 2000; //2000

class StudentWorld : public GameWorld
{
public:
	StudentWorld(std::string assetDir);
    
    ~StudentWorld();

    virtual int init();
    virtual int move();
    virtual void cleanUp();
    
    void killActor(int id, int x, int y);
    void setFlag(int x, int y);
    void resetFlag();
    void spawnAdultGrasshopper(int x, int y, int health);
    
    bool isBlocked(int x, int y);
    bool hasFood(int x, int y, Food*& a);
    bool moveActor(int id, int xStart, int yStart, int xEnd, int yEnd);
    bool winningAntExists();

private:
    mmap m_map;
    int  m_tickCount;
    int  m_currentUniqueId;
    indexPair   m_flagIndex;
    
    //helper functions
    void removeActor(bool hardDelete, int id, int x, int y);
    void insertActor(Actor* a, int x, int y);
    void setDisplayText();
    
};

#endif // STUDENTWORLD_H_
