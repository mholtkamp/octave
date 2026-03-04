#pragma once

#if EDITOR

#include "TemplateData.h"
#include <string>
#include <vector>

/**
 * @brief Singleton manager for project templates.
 *
 * Handles discovery, installation, and removal of project templates.
 * Templates are stored in {AppData}/OctaveEditor/Templates/
 */
class TemplateManager
{
public:
    static void Create();
    static void Destroy();
    static TemplateManager* Get();

    /** @brief Get the templates directory: {AppData}/OctaveEditor/Templates/ */
    std::string GetTemplatesDirectory();

    /** @brief Get the settings file path: {AppData}/OctaveEditor/templates.json */
    std::string GetSettingsPath();

    /** @brief Load templates from disk */
    void LoadTemplates();

    /** @brief Save template list to disk */
    void SaveTemplates();

    /** @brief Add a template from a zip file */
    bool AddTemplateFromZip(const std::string& zipPath, std::string& outError);

    /** @brief Add a template from a GitHub URL */
    bool AddTemplateFromGitHub(const std::string& gitHubUrl, std::string& outError);

    /** @brief Remove an installed template */
    bool RemoveTemplate(const std::string& templateId);

    /** @brief Get list of installed templates */
    const std::vector<Template>& GetTemplates() const { return mTemplates; }

    /** @brief Find a template by ID */
    const Template* FindTemplate(const std::string& templateId) const;

    /** @brief Create a new project from a template */
    bool CreateProjectFromTemplate(const std::string& templateId,
                                   const std::string& projectPath,
                                   const std::string& projectName,
                                   std::string& outError);

private:
    static TemplateManager* sInstance;
    TemplateManager();
    ~TemplateManager();

    /** @brief Ensure the templates directory exists */
    void EnsureTemplatesDirectory();

    /** @brief Scan templates directory and populate template list */
    void ScanTemplatesDirectory();

    /** @brief Load template metadata from package.json */
    bool LoadTemplateMetadata(const std::string& templateDir, Template& outTemplate);

    /** @brief Download a file from URL using system commands */
    bool DownloadFile(const std::string& url, const std::string& destPath, std::string& outError);

    /** @brief Extract a zip file to destination directory */
    bool ExtractZip(const std::string& zipPath, const std::string& destDir, std::string& outError);

    /** @brief Convert GitHub URL to download URL */
    std::string ConvertGitHubUrlToDownloadUrl(const std::string& gitHubUrl);

    std::vector<Template> mTemplates;
};

#endif
