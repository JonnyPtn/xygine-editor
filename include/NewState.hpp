//
//  SplashState.hpp
//  cmake-template
//
//  Created by Jonny Paton on 20/12/2017.
//

#pragma once

#include <xyginext/core/State.hpp>

class NewState : public xy::State
{
public:
    
    NewState(xy::StateStack& stateStack, Context context);
    
    bool handleEvent(const sf::Event &evt) override;
    
    void handleMessage(const xy::Message &) override;
    
    bool update(float dt) override;
    
    void draw() override;
    
    xy::StateID stateID() const override;
    
private:
    bool createNewProject(const std::string& folder, const std::string& name);
    bool createNewSprite(const std::string& folder, const std::string& name);
    
    int m_fileTypeSelection;
    
};
