//
//  SpriteEditState.cpp
//  xygine-editor
//
//  Created by Jonny Paton on 20/12/2017.
//

#include "ProjectEditState.hpp"
#include "States.hpp"
#include "Messages.hpp"
#include <xyginext/core/FileSystem.hpp>
#include <xyginext/gui/Gui.hpp>
#include <xyginext/core/Message.hpp>
#include <SFML/Graphics.hpp>
#include <xyginext/core/App.hpp>
#include "imgui.h"
#include "imgui-SFML.h"
#include "imgui_dock.hpp"
#include "imgui_tabs.h"
#include <xyginext/ecs/Scene.hpp>
#include <xyginext/ecs/components/Transform.hpp>
#include <xyginext/ecs/components/Drawable.hpp>
#include <xyginext/ecs/components/Camera.hpp>
#include <xyginext/ecs/systems/RenderSystem.hpp>
#include <xyginext/ecs/systems/ParticleSystem.hpp>
#include <xyginext/ecs/systems/ParticleSystem.hpp>
#include <xyginext/ecs/systems/SpriteSystem.hpp>
#include <xyginext/ecs/systems/SpriteAnimator.hpp>
#include <xyginext/ecs/systems/CameraSystem.hpp>

//err...
#include <unistd.h>


int ProjectEditState::m_instanceCount = 0;

const sf::Vector2u PreviewSize(800,600);

static const std::string SelectASpriteStr("Select A Sprite");
static const std::string SelectAnAnimStr("Select An Animation");

constexpr int InputBufMax = 1024;

ProjectEditState::ProjectEditState(xy::StateStack& stateStack, Context context) :
xy::State(stateStack, context),
m_initialised(false),
m_unsavedChanges(false),
m_SpritePreviewScene(context.appInstance.getMessageBus()),
m_ParticlePreviewScene(context.appInstance.getMessageBus()),
m_draggingPreview(false),
m_currentProject("")
{
    m_id = m_instanceCount++;
    
    auto& mb = getContext().appInstance.getMessageBus();
    m_SpritePreviewScene.addSystem<xy::CameraSystem>(mb);
    m_SpritePreviewScene.addSystem<xy::SpriteAnimator>(mb);
    m_SpritePreviewScene.addSystem<xy::RenderSystem>(mb);
    m_SpritePreviewScene.addSystem<xy::SpriteSystem>(mb);
    
    m_ParticlePreviewScene.addSystem<xy::CameraSystem>(mb);
    m_ParticlePreviewScene.addSystem<xy::RenderSystem>(mb);
    m_ParticlePreviewScene.addSystem<xy::ParticleSystem>(mb);
    
    // Add camera entity
    m_SpriteCamEntity = m_SpritePreviewScene.createEntity();
    m_SpriteCamEntity.addComponent<xy::Camera>().setView({static_cast<float>(PreviewSize.x), static_cast<float>(PreviewSize.y)});
    m_SpriteCamEntity.addComponent<xy::Transform>();
    m_SpritePreviewScene.setActiveCamera(m_SpriteCamEntity);
    
    m_ParticleCamEntity = m_ParticlePreviewScene.createEntity();
    m_ParticleCamEntity.addComponent<xy::Camera>().setView({static_cast<float>(PreviewSize.x), static_cast<float>(PreviewSize.y)});
    m_ParticleCamEntity.addComponent<xy::Transform>();
    m_ParticlePreviewScene.setActiveCamera(m_ParticleCamEntity);
    
    // and preview entity;
    m_spritePreviewEntity = m_ParticlePreviewScene.createEntity();
    m_spritePreviewEntity.addComponent<xy::Sprite>();
    m_spritePreviewEntity.addComponent<xy::Transform>();
    m_spritePreviewEntity.addComponent<xy::Drawable>();
    m_spritePreviewEntity.addComponent<xy::SpriteAnimation>();
    
    m_particlePreviewEntity = m_ParticlePreviewScene.createEntity();
    m_particlePreviewEntity.addComponent<xy::ParticleEmitter>();
    m_particlePreviewEntity.addComponent<xy::Transform>();
    m_particlePreviewEntity.addComponent<xy::Drawable>();
    
    m_previewBuffer.create(PreviewSize.x, PreviewSize.y);
}

bool ProjectEditState::handleEvent(const sf::Event &evt)
{
    switch (evt.type)
    {
        case sf::Event::MouseMoved:
        {
            // If we're dragging preview, translate camera appropriately
            if (m_draggingPreview)
            {
                auto dx = evt.mouseMove.x - m_lastMousePos.x;
                auto dy = evt.mouseMove.y - m_lastMousePos.y;
                m_SpriteCamEntity.getComponent<xy::Transform>().move(-dx,-dy);
                m_particlePreviewEntity.getComponent<xy::Transform>().move(-dx,-dy);
            }
            m_lastMousePos = {evt.mouseMove.x, evt.mouseMove.y};
            break;
        }
    }
}

void ProjectEditState::handleMessage(const xy::Message & msg)
{
    switch(msg.id)
    {
        case Messages::OPEN_PROJECT:
        {
            auto file = msg.getData<std::string>();
            
            // change working directory to project folder
            chdir(xy::FileSystem::getFilePath(file).c_str());
            m_currentProject.loadFromFile(file);
            break;
        }
            
        case Messages::ASSET_SELECTED:
        {
            // Check the type, based on extension. I know this sucks.
            auto file = msg.getData<std::string>();
            if (file.find(".spt"))
                m_selectedAssetType = AssetType::Sprite;
            else if (file.find(".tmx"))
                m_selectedAssetType = AssetType::TileMap;
            else if (file.find(".xyp"))
                m_selectedAssetType = AssetType::ParticleEmitter;
            else if (file.find(".wav"))
                m_selectedAssetType = AssetType::Sound;
            else if (file.find(".png"))
                m_selectedAssetType = AssetType::Texture;
            else if (file.find(".jpg"))
                m_selectedAssetType = AssetType::Texture;
            break;
        }
    }
    m_SpritePreviewScene.forwardMessage(msg);
    m_ParticlePreviewScene.forwardMessage(msg);
}

bool ProjectEditState::update(float dt)
{
    m_SpritePreviewScene.update(dt);
    m_ParticlePreviewScene.update(dt);
}

void ProjectEditState::draw()
{
    auto menuBarHeight = ImGui::GetWindowHeight(); // For use below
    
    // Wrap all other states in a dockspace, so dockable windows can be used
    auto windowSize = getContext().appInstance.getRenderWindow()->getSize();
    
    // Because this is called from within BeginMainMenuBar we can query imgui window height to get
    // the height of the menu bar
    ImGui::SetNextWindowSize({static_cast<float>(windowSize.x) ,windowSize.y - menuBarHeight});
    ImGui::SetNextWindowPos({0,menuBarHeight});
    ImGui::Begin("DockSpace", nullptr, ImGuiWindowFlags_NoTitleBar);
    ImGui::BeginDockspace();
    
    drawAssetBrowser();
    
    ImGui::EndDockspace();
    ImGui::End();
}

void ProjectEditState::imDrawSpritesheet()
{
    auto root = xy::FileSystem::getFilePath(m_currentProject.getFilePath());
    auto file = root + "/" + m_sheet->getTexturePath();
    
    auto& tex = m_textures.get(xy::FileSystem::getRelativePath(file,xy::FileSystem::getResourcePath().substr(0,xy::FileSystem::getResourcePath().find_last_of("/"))));
    
    // Show the texture being used first
    if (ImGui::TreeNode("Texture"))
    {
        auto texPath = m_sheet->getTexturePath();
        std::array<char, InputBufMax> texPathInput = {{0}};
        texPath.copy(texPathInput.begin(),texPath.size());
        ImGui::BeginChild("Texture Preview",{ImGui::GetContentRegionAvailWidth(),ImGui::GetContentRegionAvailWidth()});
        
        
        // Draw a highlight on the current texture rect
        if (m_spritePreviewEntity > 0)
        {
            auto rect = m_spritePreviewEntity.getComponent<xy::Sprite>().getTextureRect();
            ImGui::DrawRect(rect, sf::Color::Red);
        }
        
        // Show the texture preview
        ImGui::Image(tex);
        ImGui::EndChild();
        if (ImGui::InputText("Texture", texPathInput.data(), texPathInput.size()))
        {
            m_sheet->setTexturePath(std::string(texPathInput.data()));
            m_unsavedChanges = true;
        }
        ImGui::SameLine();
        if (ImGui::Button("Browse"))
        {
            auto path = xy::FileSystem::openFileDialogue();
            if (!path.empty())
            {
                m_sheet->setTexturePath(xy::FileSystem::getRelativePath(path,m_currentProject.getFilePath().substr(0,m_currentProject.getFilePath().find_last_of("/"))));
                
                m_spritePreviewEntity.getComponent<xy::Sprite>().setTexture(m_textures.get(path));
            }
            m_unsavedChanges = true;
            
        }
        ImGui::TreePop();
    }
    
    // Sprite settings
    if (ImGui::TreeNode("Sprites"))
    {
        static std::string createString("Create new...");
        if (!m_sheet->getTexturePath().empty())
        {
           // if (ImGui::BeginCombo("Sprites", m_selectedSprite.c_str()))
            {
                for (auto& spr : m_sheet->getSprites())
                {
                    bool sprSelected(false);
                    ImGui::Selectable(spr.first.c_str(), &sprSelected);
                    if (sprSelected)
                    {
                        m_spritePreviewEntity.getComponent<xy::Sprite>() = spr.second;
                    //    m_selectedSprite = spr.first;
                    }
                }
                
                // Final selection for creating new
                bool createSelected(false);
                ImGui::Selectable(createString.c_str(), &createSelected);
                if (createSelected)
                {
//                    m_selectedSprite = createString;
//                    m_spritePreviewEntity.getComponent<xy::Sprite>() = m_sheet->getSprite(m_selectedSprite);
                }
                
                ImGui::EndCombo();
            }
        }
        
        
        // such string comparison...
       // if(m_selectedSprite == createString)
        {
            char buf[1024] = {0};
            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::ColorConvertFloat4ToU32( ImColor(255, 0, 0)));
            if (ImGui::InputText("New Sprite Name", buf, 1024, ImGuiInputTextFlags_EnterReturnsTrue))
            {
                // This just adds a default sprite
                m_sheet->setSprite(buf, xy::Sprite());
             //   m_selectedSprite = buf;
            }
            ImGui::PopStyleColor();
        }
        
       // if (m_selectedSprite != SelectASpriteStr && m_selectedSprite != createString)
        {
            auto& sprite = m_spritePreviewEntity.getComponent<xy::Sprite>();
            
            // Texture rect
            auto rect = sf::IntRect(sprite.getTextureRect());
            if (ImGui::InputInt4("Texture Rect",(int*)&rect))
            {
                // If we're modifying an anim, change the frames tex rect
//                if (m_selectedAnim != SelectAnAnimStr)
                {
                 //   auto& anim = sprite.getAnimations()[m_sheet->getAnimationIndex(m_selectedAnim, m_selectedSprite)];
//                    anim.frames[m_spritePreviewEntity.getComponent<xy::SpriteAnimation>().getFrameID()] = sf::FloatRect(rect);
                    sprite.setTextureRect(sf::FloatRect(rect));
                 //   m_sheet->setSprite(m_selectedSprite, sprite);
                }
//                else
                {
                    sprite.setTextureRect(sf::FloatRect(rect));
                 //   m_sheet->setSprite(m_selectedSprite, sprite);
                }
                m_unsavedChanges = true;
            }
            
            // Colour
            ImVec4 col= sprite.getColour();
            if (ImGui::ColorEdit3("Colour", (float*)&col))
            {
                sprite.setColour(col);
               // m_sheet->setSprite(m_selectedSprite, sprite);
                
            }
            // Delete Sprite
            if (xy::Nim::button("Delete Sprite"))
            {
               // m_sheet->removeSprite(m_selectedSprite);
             //   m_selectedSprite = SelectASpriteStr;
//                m_selectedAnim = SelectAnAnimStr;
            }
            
            // Animations
            if (ImGui::TreeNode("Animations"))
            {
               // if (ImGui::BeginCombo("Animations", m_selectedAnim.c_str()))
                {
                    auto& anims = sprite.getAnimations();
                    for (auto& anim : anims)
                    {
                        bool animSelected(false);
                        ImGui::Selectable(anim.id.data(), &animSelected);
                        if (animSelected)
                        {
                          //  m_selectedAnim = std::string(anim.id.data());
                            
                            //hacky
                            if (m_spritePreviewEntity.hasComponent<xy::SpriteAnimation>())
                            {
//                                m_spritePreviewEntity.getComponent<xy::SpriteAnimation>().play( m_sheet->getAnimationIndex(m_selectedAnim, m_selectedSprite));
                            }
                            else
                            {
                                // Not sure why I have to do this
                                m_SpritePreviewScene.getSystem<xy::SpriteAnimator>().addEntity(m_spritePreviewEntity);
//                                m_spritePreviewEntity.addComponent<xy::SpriteAnimation>().play( m_sheet->getAnimationIndex(m_selectedAnim, m_selectedSprite));;
                            }
                        }
                    }
                    ImGui::EndCombo();
                }
                
                // Select an animation
//                if (m_selectedAnim != SelectAnAnimStr)
                {
//                    auto index = m_sheet->getAnimationIndex(m_selectedAnim, m_selectedSprite);
//                    auto& anim = sprite.getAnimations()[index];
//                    int fc = anim.frameCount;
//                    if (ImGui::InputInt("Frames", &fc))
                    {
//                        anim.frameCount = fc;
//                        m_sheet->setSprite(m_selectedSprite, sprite);
                    }
//                    if (ImGui::InputFloat("Framerate", &anim.framerate))
                    {
//                        m_sheet->setSprite(m_selectedSprite, sprite);
                    }
//                    if (ImGui::Checkbox("Looped", &anim.looped))
                    {
//                        m_sheet->setSprite(m_selectedSprite, sprite);
                    }
                    
                    // Timeline
                    auto& c = m_spritePreviewEntity.getComponent<xy::SpriteAnimation>();
                    int frame = c.getFrameID()+1;
//                    if (ImGui::SliderInt("Frame", &frame, 1, anim.frameCount))
                    {
                        // bc 0 index
                        --frame;
                        c.setFrameID(frame);
//                        m_spritePreviewEntity.getComponent<xy::Sprite>().setTextureRect(anim.frames[frame]);
                    }
                    
                    if (ImGui::Button("||"))
                    {
                        c.pause();
                        
                    }
                    
                    ImGui::SameLine();
                    if (ImGui::Button(">"))
                    {
                        
//                        c.play(m_sheet->getAnimationIndex(m_selectedAnim, m_selectedSprite));
                    }
                    
                    // Delete Sprite
                    if (xy::Nim::button("Delete Animation"))
                    {
                        // Err....
//                        m_selectedAnim = SelectAnAnimStr;
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
            m_sheet->saveToFile(m_currentProject.getFilePath() + m_selectedFile);
            m_unsavedChanges = false;
        }
    }
    
    // Preview Column
    m_previewBuffer.clear({128,128,128});
    m_previewBuffer.draw(m_SpritePreviewScene);
    m_previewBuffer.display();
    ImGui::NextColumn();
    ImGui::Image(m_previewBuffer);
    
    // If hovering over preview, and mouse is clicked, must be dragging
    if (ImGui::IsItemHovered())
    {
        if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
        {
            m_draggingPreview = true;
        }
        else
        {
            m_draggingPreview = false;
        }
    }
    else
    {
        m_draggingPreview = false;
    }
}

void ProjectEditState::imDrawParticleEmitter()
{
    auto& p = *m_emitter; // wtf am I doing
    if (ImGui::Button("Start"))
    {
        m_particlePreviewEntity.getComponent<xy::ParticleEmitter>().start();
    }
    if (ImGui::Button("Stop"))
    {
        m_particlePreviewEntity.getComponent<xy::ParticleEmitter>().stop();
    }
    
    ImGui::InputFloat("Emit rate", &p.settings.emitRate);
    int ec = p.settings.emitCount;
    if (ImGui::InputInt("Emit count", &ec))
        {
            p.settings.emitCount = ec;
        }
    ImGui::InputFloat("Lifetime", &p.settings.lifetime);
    ImGui::InputFloat("Lifetime Variance", &p.settings.lifetimeVariance);
    ImGui::Checkbox("Random rotation", &p.settings.randomInitialRotation);
    ImGui::InputInt("Release Count", &p.settings.releaseCount);
    ImGui::InputFloat("Rotation speed", &p.settings.rotationSpeed );
    ImGui::InputFloat("Scale modifier", &p.settings.scaleModifier);
    ImGui::InputFloat("size", &p.settings.size);
    ImGui::InputFloat("Spawn Radius", &p.settings.spawnRadius);
    ImGui::InputFloat("Spread", &p.settings.spread);
    
    m_currentProject.getParticleEmitters().find(m_selectedFile)->second.settings = p.settings;
    m_particlePreviewEntity.getComponent<xy::ParticleEmitter>().settings = p.settings;

    // Preview Column
    m_previewBuffer.clear({128,128,128});
    m_previewBuffer.draw(m_ParticlePreviewScene);
    m_previewBuffer.display();
    ImGui::NextColumn();
    ImGui::Image(m_previewBuffer);
}

void ProjectEditState::imDrawTexture()
{
    // Just show the texture I suppose...
    auto& tex = m_currentProject.getTextures()[m_selectedFile];
    auto size = tex.getSize();
    if (ImGui::InputInt2("Texture Size",(int*) &size))
    {
        //hmm...
    }
    
    ImGui::NextColumn();
    ImGui::Image(tex);
    
}

void ProjectEditState::drawAssetBrowser()
{
    if (ImGui::BeginDock("Asset Browser"))
    {
        std::function<void(std::string)> imFileTreeRecurse = [&](std::string path)
        {
            // List directories, recurse if selected
            for (auto& dir : xy::FileSystem::listDirectories(path))
            {
                if (ImGui::TreeNode(dir.c_str()))
                {
                    imFileTreeRecurse(path + "/" + dir);
                    ImGui::TreePop();
                }
            }
            
            // List files, broadcast message if selected
            for (auto& file : xy::FileSystem::listFiles(path))
            {
                auto selected = m_selectedFile == file;
                if (ImGui::Selectable(file.c_str(), &selected))
                {
                    auto msg = getContext().appInstance.getMessageBus().post<std::string>(Messages::ASSET_SELECTED);
                    *msg = m_selectedFile = file;
                }
            }
        };
        
        imFileTreeRecurse(m_currentProject.getFilePath() + "/assets");
    }
    ImGui::EndDock();
}

xy::StateID ProjectEditState::stateID() const {
    return States::PROJECT_EDIT;
}
