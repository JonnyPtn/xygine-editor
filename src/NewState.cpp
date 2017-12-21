//
//  SplashState.cpp
//  xygine-editor
//
//  Created by Jonny Paton on 20/12/2017.
//

#include "NewState.hpp"

#include <xyginext/gui/Gui.hpp>
#include <xyginext/core/Log.hpp>
#include <xyginext/core/FileSystem.hpp>
#include <xyginext/core/Message.hpp>

#include <xyginext/ecs/components/Sprite.hpp>
#include <xyginext/graphics/SpriteSheet.hpp>

#include "imgui.h"

#include "States.hpp"
#include "Messages.hpp"

NewState::NewState(xy::StateStack &stateStack, xy::State::Context context) :
xy::State(stateStack, context)
{
    
}

bool NewState::handleEvent(const sf::Event &evt) {
    
}

void NewState::handleMessage(const xy::Message &) {

}

bool NewState::update(float dt) {
    
}

void NewState::draw()
{
    xy::Nim::begin("New SpriteSheet");
    
    static std::string path = "", name = "";
    
    const int bufSize = 1024;
    static char nameBuf[bufSize], pathBuf[bufSize];
    ImGui::InputText("SpriteSheet name", nameBuf, bufSize);
    ImGui::InputText("Path", pathBuf, bufSize);
    ImGui::SameLine();
    if (xy::Nim::button("Browse"))
    {
        path = xy::FileSystem::nativeOpenFolder();
        path.copy(pathBuf,bufSize);
    }
    
    ImGui::Spacing();
    if (xy::Nim::button("Create"))
    {
        name = nameBuf;
        createNewSpriteSheet(path, name);
        requestStackClear();
        requestStackPush(States::SPRITE_EDIT);
    }
    
    xy::Nim::end(); // New Sprite
}

xy::StateID NewState::stateID() const
{
    return States::NEW;
}

bool NewState::createNewSpriteSheet(const std::string &folder, const std::string &name)
{
    const auto extension = ".spt";
    if (xy::FileSystem::fileExists(folder + "/" + name + extension))
    {
        xy::Logger::log("Can't create sprite, file exists already");
        return false;
    }
    else if (!xy::FileSystem::directoryExists(folder))
    {
        xy::Logger::log("Directory doesn't exist, creating...");
        xy::FileSystem::createDirectory(folder);
    }
    xy::SpriteSheet sheet;
    sheet.saveToFile(folder + "/" + name + extension);
    return true;
}

