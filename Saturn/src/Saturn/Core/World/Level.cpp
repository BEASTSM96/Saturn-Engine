#include "sppch.h"
#include "Level.h"

#include "Saturn/Application.h"
#include "Saturn/Core/Serialisation/Serialiser.h"

namespace Saturn {

    Level::Level()
    {
//        m_id++;

        m_id = rand() * 10000;

        Application::Get().m_gameLayer = new GameLayer();

        Application::Get().PushLayer(Application::Get().m_gameLayer);

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

    void * Level::GetGameLayer()
    {
        return Application::Get().m_gameLayer;
    }

    float Level::GetID()
    {
        return m_id;
    }
}