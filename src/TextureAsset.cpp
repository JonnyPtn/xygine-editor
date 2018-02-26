//
//  TextureAsset.cpp
//  xygine-editor
//
//  Created by Jonny Paton on 26/02/2018.
//

#include "TextureAsset.hpp"
#include "imgui_dock.hpp"
#include "imgui-SFML.h"

TextureAsset::TextureAsset(const std::string& path, xy::TextureResource& textures) :
Asset(),
m_texture(textures.get(path))
{
    // Make path relative to our package
    auto resPath = xy::FileSystem::getResourcePath();
    resPath = resPath.substr(0,resPath.find_last_of("/"));
    auto relPath = xy::FileSystem::getRelativePath(path,resPath);
    
    m_texture = textures.get(relPath);
}

void TextureAsset::drawProperties()
{
    if (ImGui::BeginDock("Texture properties", nullptr, ImGuiWindowFlags_HorizontalScrollbar))
    {
        ImGui::Image(m_texture);
    }
    ImGui::EndDock();
}
AssetType TextureAsset::type() const { 
    return AssetType::Texture;
}
