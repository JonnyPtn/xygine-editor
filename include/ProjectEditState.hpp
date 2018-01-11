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
    
    // Project tabs, bool for selected item
    std::map<std::unique_ptr<Project>,bool> m_projectTabs;
    
    // Pointer for access to the current project (i.e. the currently selected project tab)
    std::weak_ptr<Project>              m_currentProject;
    
    // The names of currently selected resource. I'm sure there's a smarter way of doing this...
    std::string                         m_currentSpritesheet;
    std::string                         m_currentParticleEmitter;
    std::string                         m_currentTexture;
    
    // When editing a spriteshee, this stores the currently selected sprite and animation
    std::string                         m_selectedSprite;
    std::string                         m_selectedAnim;
    
    void                                imDrawSpritesheet();
    void                                imDrawParticleEmitter();
    void                                imDrawTexture();
    
    xy::Scene                           m_SpritePreviewScene;
    xy::Scene                           m_ParticlePreviewScene;
    
    xy::Entity                          m_spritePreviewEntity;
    
    xy::SpriteSheet                     m_sheet;
    xy::Entity                          m_camEntity;
    
    // Preview controls
    bool                                m_draggingPreview;
    sf::Vector2i                        m_lastMousePos;
    
    xy::TextureResource                 m_textures;
    sf::RenderTexture                   m_previewBuffer;
    
    // The file currently selected by the project browser
    std::string m_selectedFile;
    
    int        m_id;
    static int m_instanceCount;
    bool       m_initialised;
    sf::IntRect m_tabRect;
    bool        m_unsavedChanges;
};
