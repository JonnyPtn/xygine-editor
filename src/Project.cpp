//
//  Project.cpp
//  xygine-editor
//
//  Created by Jonny Paton on 09/01/2018.
//

#include "Project.hpp"
#include <imgui.h>
#include <assert.h>

Project::Project(std::string path)
{
    loadFromFile(path);
}


bool Project::loadFromFile(const std::string &filePath)
{
    m_path = xy::FileSystem::getFilePath(filePath);
    m_name = xy::FileSystem::getFileName(filePath);
    
    // First load the project config
    if (!m_file.loadFromFile(filePath))
    {
        // xy logs the error anyway, no point doubling up
        return false;
    }
    
    // Create the file
    m_file.save(filePath);
    auto assetsFolder = m_path + "/assets";
    
    std::function<void(const std::string&)> recurseFiles = [&](const std::string& path)
    {
        for (auto& dir : xy::FileSystem::listDirectories(path))
        {
            recurseFiles(path + "/" + dir);
        }
        
        if (xy::FileSystem::listFiles(path).empty())
        {
            // Check the extension to get the type
            auto ext = xy::FileSystem::getFileExtension(path);
            auto file = xy::FileSystem::getFileName(path);
            if (ext == ".spt")
            {
                // xygine sprite file
                m_spriteFiles[file] = xy::SpriteSheet();
                m_spriteFiles[file].loadFromFile(path, m_textures);
            }
            else if (ext == ".xyp")
            {
                // xygine particle file
                m_particleFiles[file] = xy::ParticleEmitter();
                m_particleFiles[file].settings.loadFromFile(path, m_textures);
            }
            else if (ext == ".png" || ext == ".jpg")
            {
                // Texture
                m_textureFiles[file] = sf::Texture();
                m_textureFiles[file] = m_textures.get(path);
                
            }
        }
    };
    
    recurseFiles(assetsFolder);
}

bool Project::needsSaving()
{
    return !m_unsavedResources.empty();
}

std::string Project::getName()
{
    return m_name;
}

std::string Project::getFilePath()
{
    return m_path;
}

std::map<std::string, xy::SpriteSheet>& Project::getSpriteSheets()
{
    return m_spriteFiles;
}

