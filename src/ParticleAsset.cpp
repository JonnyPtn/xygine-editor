//
//  ParticleAsset.cpp
//  xygine-editor
//
//  Created by Jonny Paton on 26/02/2018.
//

#include "ParticleAsset.hpp"
#include <xyginext/ecs/components/ParticleEmitter.hpp>
#include "imgui_dock.hpp"

#include <unistd.h>

ParticleAsset::ParticleAsset(const std::string &path, xy::Scene &previewScene, xy::TextureResource& textures)
{
    m_previewEntity = previewScene.createEntity();
    m_previewEntity.addComponent<xy::ParticleEmitter>().settings.loadFromFile(path, textures);
    m_previewEntity.getComponent<xy::ParticleEmitter>().start();
}

void ParticleAsset::drawProperties()
{
    if (ImGui::BeginDock("Particle properties"))
    {
        auto& em = m_previewEntity.getComponent<xy::ParticleEmitter>();
        
        ImGui::InputFloat("Emit rate", &em.settings.emitRate);
    }
    ImGui::EndDock();

}

AssetType ParticleAsset::type() const
{
    return AssetType::ParticleEmitter;
}

