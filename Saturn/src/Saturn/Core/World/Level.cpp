#include "sppch.h"
#include "Level.h"

#include "Saturn/Application.h"
#include "Saturn/Core/Serialisation/Serialiser.h"
#include "Saturn/GameBase/GameLayer.h"

namespace Saturn {

    Level::Level()
    {
        SAT_PROFILE_FUNCTION();
        m_id = Random::Float() * 10000;
    }

    Level::~Level()
    {
        //Save the into a file
        
        Json::Value sa;

        {
            std::ofstream s = std::ofstream("assets/Levels/test.smap");

            sa["Level"]["ID"] = m_id;
            sa["Level"]["Name"] = m_name;
            s << sa;
        }
    }

    void* Level::GetLevel()
    {
        return this;
    }

    std::string Level::GetLevelName()
    {
        return m_name;
    }

    uint64_t Level::GetAllGameObjects()
    {
        return (uint64_t)gameObjects.size();
    }

    GameLayer* Level::CreateGameLayer()
    {
        m_Gamelayer = new GameLayer();

        Application::Get().m_gameLayer = m_Gamelayer;

        Application::Get().PushLayer(m_Gamelayer);

        return m_Gamelayer;
    }

    void * Level::GetGameLayer()
    {
        return m_Gamelayer;
    }

    float Level::GetID()
    {
        return m_id;
    }
}