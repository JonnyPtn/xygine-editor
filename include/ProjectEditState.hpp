//
//  SpriteEditState.hpp
//  xygine-editor
//
//  Created by Jonny Paton on 20/12/2017.
//

#pragma once

#include <xyginext/core/State.hpp>
#include <xyginext/graphics/SpriteSheet.hpp>
#include <xyginext/ecs/components/SpriteAnimation.hpp>
#include <xyginext/resources/Resource.hpp>
#include <xyginext/ecs/Entity.hpp>
#include <xyginext/ecs/Scene.hpp>

#include "Project.hpp"
#include "Asset.hpp"

#include <SFML/Graphics.hpp>

class ProjectEditState : public xy::State
{
public:
    ProjectEditState(xy::StateStack& stateStack, Context context);
    
    bool handleEvent(const sf::Event &evt) override;
    
    void handleMessage(const xy::Message &) override;
    
    bool update(float dt) override;
    
    void draw() override;
    
    xy::StateID stateID() const override;
    
private:
    
    // effectively a listing of everything in the project "assets" folder
    void    drawAssetBrowser();
    
    
    // Current project
    Project m_currentProject;
    
    // The the selected asset, and it's type
    std::unique_ptr<Asset>  m_selectedAsset;
    AssetType               m_selectedAssetType;
    
    // A preview scene, shown in a separate window
    xy::Scene               m_previewScene;
    sf::RenderWindow        m_previewWindow;
    xy::Entity              m_previewCamera;
    
    // Preview controls
    bool                                m_draggingPreview;
    sf::Vector2i                        m_lastMousePos;
    
    // The file currently selected by the project browser
    std::string m_selectedFile;
    
    bool        m_unsavedChanges;
};
