//
//  Project.hpp
//  xygine-editor
//
//  Created by Jonny Paton on 09/01/2018.
//

#pragma once

#include <string>

#include <xyginext/core/ConfigFile.hpp>
#include <xyginext/graphics/SpriteSheet.hpp>
#include <xyginext/ecs/components/ParticleEmitter.hpp>
#include <xyginext/resources/Resource.hpp>

#include <SFML/Graphics.hpp>

#include <map>

// A xygine project
class Project
{
public:
    Project(std::string path);
    
    bool loadFromFile(const std::string& filePath);
    bool create(std::string name, std::string folder);
    
    std::string getName();
    std::string getFilePath();
    
    void imDrawProperties();
    
    std::map<std::string, xy::SpriteSheet>& getSpriteSheets();
    std::map<std::string, xy::ParticleEmitter>& getParticleEmitters();
    std::map<std::string, sf::Texture>& getTextures();
    
    // Unsaved changes
    bool needsSaving();
    
private:
    std::string m_path;
    std::string m_name;
    
    xy::ConfigFile  m_file;
    
    // File names and whether they need saving
    std::map<std::string, xy::SpriteSheet> m_spriteFiles;
    std::map<std::string, xy::ParticleEmitter> m_particleFiles;
    std::map<std::string, sf::Texture> m_textureFiles;
    
    // keep track of unsaved resources
    std::vector<std::string> m_unsavedResources;
 
    xy::TextureResource m_textures;
};
