//
//  SpriteEditState.cpp
//  xygine-editor
//
//  Created by Jonny Paton on 20/12/2017.
//

#include "SpriteEditState.hpp"
#include "States.hpp"
#include "Messages.hpp"
#include <xyginext/core/FileSystem.hpp>
#include <xyginext/gui/Gui.hpp>
#include <xyginext/core/Message.hpp>
#include <SFML/Graphics.hpp>
#include <xyginext/core/App.hpp>
#include "imgui.h"
#include "imgui-SFML.h"
#include "imgui_tabs.h"
#include "imgui_extras.h"
#include <xyginext/ecs/Scene.hpp>
#include <xyginext/ecs/components/Transform.hpp>
#include <xyginext/ecs/components/Drawable.hpp>
#include <xyginext/ecs/components/Camera.hpp>
#include <xyginext/ecs/systems/RenderSystem.hpp>
#include <xyginext/ecs/systems/SpriteSystem.hpp>
#include <xyginext/ecs/systems/SpriteAnimator.hpp>
#include <xyginext/ecs/systems/CameraSystem.hpp>

int SpriteEditState::m_instanceCount = 0;

constexpr float PreviewWidth = 0.6f;

static const std::string SelectASpriteStr("Select A Sprite");
static const std::string SelectAnAnimStr("Select An Animation");

constexpr int InputBufMax = 1024;

SpriteEditState::SpriteEditState(xy::StateStack& stateStack, Context context) :
xy::State(stateStack, context),
m_initialised(false),
m_selectedSpriteName(SelectASpriteStr),
m_selectedAnimName(SelectAnAnimStr),
m_unsavedChanges(false),
m_previewScene(context.appInstance.getMessageBus()),
m_previewEntity(0),
m_draggingPreview(false)
{
    m_id = m_instanceCount++;
    
    auto& mb = getContext().appInstance.getMessageBus();
    m_previewScene.addSystem<xy::CameraSystem>(mb);
    m_previewScene.addSystem<xy::SpriteAnimator>(mb);
    m_previewScene.addSystem<xy::RenderSystem>(mb);
    m_previewScene.addSystem<xy::SpriteSystem>(mb);
    
    // Add camera entity
    m_camEntity = m_previewScene.createEntity();
    m_camEntity.addComponent<xy::Camera>().setViewport({1.f - PreviewWidth, 0.f, PreviewWidth, 1.f});
    auto winSize = context.appInstance.getRenderWindow()->getSize();
    m_camEntity.getComponent<xy::Camera>().setView({winSize.x * PreviewWidth, static_cast<float>(winSize.y)});
    m_camEntity.addComponent<xy::Transform>();
    m_previewScene.setActiveCamera(m_camEntity);
    
    // and preview entity;
    m_previewEntity = m_previewScene.createEntity();
    m_previewEntity.addComponent<xy::Sprite>();
    m_previewEntity.addComponent<xy::Transform>();
    m_previewEntity.addComponent<xy::Drawable>();
    m_previewEntity.addComponent<xy::SpriteAnimation>();
}

bool SpriteEditState::handleEvent(const sf::Event &evt)
{
    switch (evt.type)
    {
    }
}

void SpriteEditState::handleMessage(const xy::Message & msg)
{
    switch(msg.id)
    {
        case Messages::NEW_WORKING_FILE:
            if (!m_initialised)
            {
                m_path = msg.getData<std::string>();
                m_name = xy::FileSystem::getFileName(m_path);
                m_sheet.loadFromFile(m_path, m_texture);
                
                m_initialised = true;
            }
            break;
    }
    m_previewScene.forwardMessage(msg);
}

bool SpriteEditState::update(float dt)
{
    m_previewScene.update(dt);
}

void SpriteEditState::draw() {
    
    bool open(true);
    
    auto window = getContext().appInstance.getRenderWindow();
    
    bool selected = ImGui::TabItem(m_name.c_str(), &open, m_unsavedChanges ? ImGuiTabItemFlags_UnsavedDocument : 0);
    
    
    if (!open)
        requestStackPop();
    
    if (!selected)
        return;
    
    ImGui::SetNextWindowPos({ImGui::GetWindowPos().x, ImGui::GetWindowPos().y + ImGui::GetWindowHeight()});
    ImGui::SetNextWindowSize({window->getSize().x * (1.f - PreviewWidth),static_cast<float>(window->getSize().y - (ImGui::GetWindowPos().y + ImGui::GetWindowHeight()))});

    
    ImGui::Begin("Properties" , nullptr,ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

    
    // Show the texture being used first
    if (ImGui::TreeNode("Texture"))
    {
        auto texPath = m_sheet.getTexturePath();
        std::array<char, InputBufMax> texPathInput = {{0}};
        texPath.copy(texPathInput.begin(),texPath.size());
        ImGui::BeginChild("Texture Preview",{ImGui::GetContentRegionAvailWidth(),ImGui::GetContentRegionAvailWidth()});
        auto& tex = m_texture.get(xy::FileSystem::getFilePath(m_path) + m_sheet.getTexturePath());
        
        // Draw a highlight on the current texture rect
        if (m_previewEntity > 0)
        {
            auto rect = m_previewEntity.getComponent<xy::Sprite>().getTextureRect();
            ImGui::DrawRect(rect, sf::Color::Red);
        }
        ImGui::Image(tex);
        ImGui::EndChild();
        if (ImGui::InputText("Texture", texPathInput.data(), texPathInput.size()))
        {
            m_sheet.setTexturePath(std::string(texPathInput.data()));
            m_unsavedChanges = true;
        }
        ImGui::SameLine();
        if (ImGui::Button("Browse"))
        {
            auto path = xy::FileSystem::openFileDialogue();
            if (!path.empty())
            {
                // Maybe xygine should handle this?
                m_sheet.setTexturePath(xy::FileSystem::getRelativePath(path,m_path.substr(0,m_path.find_last_of('/'))));
            
                updatePreview();
                m_previewEntity.getComponent<xy::Sprite>().setTexture(m_texture.get(path));
            }
            m_unsavedChanges = true;
        }
        ImGui::TreePop();
    }
    
    // Sprite settings
    if (ImGui::TreeNode("Sprites"))
    {
        static std::string createString("Create new...");
        if (!m_sheet.getTexturePath().empty())
        {
            if (ImGui::BeginCombo("Sprites", m_selectedSpriteName.c_str()))
            {
                for (auto& spr : m_sheet.getSprites())
                {
                    bool sprSelected(false);
                    ImGui::Selectable(spr.first.c_str(), &sprSelected);
                    if (sprSelected)
                    {
                        m_previewEntity.getComponent<xy::Sprite>() = spr.second;
                        m_selectedSpriteName = spr.first;
                    }
                }
                
                // Final selection for creating new
                bool createSelected(false);
                ImGui::Selectable(createString.c_str(), &createSelected);
                if (createSelected)
                {
                    m_selectedSpriteName = createString;
                }
                
                ImGui::EndCombo();
            }
        }
        
        
        // such string comparison...
        if(m_selectedSpriteName == createString)
        {
            char buf[1024] = {0};
            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::ColorConvertFloat4ToU32( ImColor(255, 0, 0)));
            if (ImGui::InputText("New Sprite Name", buf, 1024, ImGuiInputTextFlags_EnterReturnsTrue))
            {
                // This just adds a default sprite
                m_sheet.setSprite(buf, xy::Sprite());
                m_selectedSpriteName = buf;
                updatePreview();
            }
            ImGui::PopStyleColor();
        }
        
        if (m_selectedSpriteName != SelectASpriteStr && m_selectedSpriteName != createString)
        {
            auto& sprite = m_previewEntity.getComponent<xy::Sprite>();
            
            // Texture rect
            auto rect = sf::IntRect(sprite.getTextureRect());
            if (ImGui::InputInt4("Texture Rect",(int*)&rect, ImGuiInputTextFlags_EnterReturnsTrue))
            {
                // If we're modifying an anim, change the frames tex rect
                if (m_selectedAnimName != SelectAnAnimStr)
                {
                    auto& anim = sprite.getAnimations()[m_sheet.getAnimationIndex(m_selectedSpriteName, m_selectedSpriteName)];
                    anim.frames[m_previewEntity.getComponent<xy::SpriteAnimation>().getFrameID()] = sf::FloatRect(rect);
                    sprite.setTextureRect(sf::FloatRect(rect));
                    m_sheet.setSprite(m_selectedSpriteName, sprite);
                }
                else
                {
                    sprite.setTextureRect(sf::FloatRect(rect));
                    m_sheet.setSprite(m_selectedSpriteName, sprite);
                    updatePreview();
                }
                m_unsavedChanges = true;
            }
            
            // Colour
            ImVec4 col= sprite.getColour();
            if (ImGui::ColorEdit3("Colour", (float*)&col))
            {
                sprite.setColour(col);
                m_sheet.setSprite(m_selectedSpriteName, sprite);
                
                updatePreview();
            }
            // Delete Sprite
            if (xy::Nim::button("Delete Sprite"))
            {
                m_sheet.removeSprite(m_selectedSpriteName);
                m_selectedSpriteName = SelectASpriteStr;
                m_selectedAnimName = SelectAnAnimStr;
            }
            
            // Animations
            if (ImGui::TreeNode("Animations"))
            {
                if (ImGui::BeginCombo("Animations", m_selectedAnimName.c_str()))
                {
                    auto& anims = sprite.getAnimations();
                    for (auto& anim : anims)
                    {
                        bool animSelected(false);
                        ImGui::Selectable(anim.id.data(), &animSelected);
                        if (animSelected)
                        {
                            m_selectedAnimName = std::string(anim.id.data());
                            
                            //hacky
                            if (m_previewEntity.hasComponent<xy::SpriteAnimation>())
                            {
                                m_previewEntity.getComponent<xy::SpriteAnimation>().play( m_sheet.getAnimationIndex(m_selectedAnimName, m_selectedSpriteName));
                            }
                            else
                            {
                                // Not sure why I have to do this
                                m_previewScene.getSystem<xy::SpriteAnimator>().addEntity(m_previewEntity);
                                m_previewEntity.addComponent<xy::SpriteAnimation>().play( m_sheet.getAnimationIndex(m_selectedAnimName, m_selectedSpriteName));;
                            }
                            updatePreview();
                        }
                    }
                    ImGui::EndCombo();
                }
                
                // Select an animation
                if (m_selectedAnimName != SelectAnAnimStr)
                {
                    auto index = m_sheet.getAnimationIndex(m_selectedAnimName, m_selectedSpriteName);
                    auto& anim = sprite.getAnimations()[index];
                    int fc = anim.frameCount;
                    if (ImGui::InputInt("Frames", &fc))
                    {
                        anim.frameCount = fc;
                        m_sheet.setSprite(m_selectedSpriteName, sprite);
                        updatePreview();
                    }
                    if (ImGui::InputFloat("Framerate", &anim.framerate))
                    {
                        m_sheet.setSprite(m_selectedSpriteName, sprite);
                        updatePreview();
                    }
                    if (ImGui::Checkbox("Looped", &anim.looped))
                    {
                        m_sheet.setSprite(m_selectedSpriteName, sprite);
                        updatePreview();
                    }
                    
                    // Timeline
                    auto& c = m_previewEntity.getComponent<xy::SpriteAnimation>();
                    int frame = c.getFrameID()+1;
                    if (ImGui::SliderInt("Frame", &frame, 1, anim.frameCount))
                    {
                        // bc 0 index
                        --frame;
                        c.setFrameID(frame);
                        m_previewEntity.getComponent<xy::Sprite>().setTextureRect(anim.frames[frame]);
                    }
                    
                    if (ImGui::Button("||"))
                    {
                        c.pause();

                    }
                    
                    ImGui::SameLine();
                    if (ImGui::Button(">"))
                    {
                        
                        c.play(m_sheet.getAnimationIndex(m_selectedAnimName, m_selectedSpriteName));
                    }
                    
                    // Delete Sprite
                    if (xy::Nim::button("Delete Animation"))
                    {
                        // Err....
                        m_selectedAnimName = SelectAnAnimStr;
                    }
                }
                ImGui::TreePop();
            }
        }
        ImGui::TreePop();
    }
    
    // Save button
    if (m_unsavedChanges)
    {
        if (xy::Nim::button("Save"))
        {
            m_sheet.saveToFile(m_path);
            m_unsavedChanges = false;
        }
    }
    
    ImGui::End();
    window->draw(m_previewScene);
    
    // Draw preview scene controls
    ImGui::Begin("Preview Settings");
    ImVec4 col = xy::App::getClearColour();
    if (ImGui::ColorEdit3("Clear colour", (float*)&col))
        xy::App::setClearColour(col);
    ImGui::End();
    
}

void SpriteEditState::updatePreview()
{
    // Adjust the camera so the sprite fills the view
    auto view = m_camEntity.getComponent<xy::Camera>().getView();
    auto size = m_previewEntity.getComponent<xy::Sprite>().getSize();
    
    float zoom(1.f);
    if (size.x > 0 && size.y > 0)
        zoom = std::min(float(view.x)/float(size.x), float(view.y)/float(size.y));
    
    m_camEntity.getComponent<xy::Camera>().zoom(zoom);
    m_camEntity.getComponent<xy::Transform>().setPosition(size.x/2,size.y/2);
}

xy::StateID SpriteEditState::stateID() const {
    return States::SPRITE_EDIT;
}
