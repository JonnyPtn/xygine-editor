//
//  SplashState.hpp
//  cmake-template
//
//  Created by Jonny Paton on 20/12/2017.
//

#pragma once

#include <xyginext/core/State.hpp>

class OpenState : public xy::State
{
public:
    
    OpenState(xy::StateStack& stateStack, Context context, sf::IntRect editorRect);
    
    bool handleEvent(const sf::Event &evt) override;
    
    void handleMessage(const xy::Message &) override;
    
    bool update(float dt) override;
    
    void draw() override;
    
    xy::StateID stateID() const override;
    
private:
    sf::IntRect m_windowRect;
};
