#pragma once
#include <toml++/toml.hpp>
#include <fstream>
#include <utils.h>
#include <filesystem>
#include <log.h>
#include <stdexcept>

/**
 * @brief 配置解析器
 * @details 用于获取配置文件的配置项
 */
class Config
{
public:
    toml::table config;

public:
    Config(const std::filesystem::path &path);

    template <class T>
    T get(const std::string &key_dir, std::string_view error_message_view = "") const
    {
        std::string error_message;
        if (error_message_view.empty())
            error_message = "Error: Unable to find configuration item: '" + key_dir + "'.";
        else
            error_message = std::string(error_message_view);
        auto keys = split(key_dir, ".");
        auto table_keys = std::vector<std::string>(keys.begin(), keys.end() - 1);
        auto table = &this->config;
        for (auto key : table_keys)
        {
            if (!table->contains(key) || !table->at(key).is_table())
                throw std::runtime_error(std::string(error_message));
            table = table->at(key).as_table();
        }
        if (!table->contains(keys.back()))
            throw std::runtime_error(std::string(error_message));
        toml::v3::optional<T> value = table->at(keys.back()).value<T>();
        if (!value.has_value())
            throw std::runtime_error(std::string(error_message));
        return value.value();
    }

    template <class T>
    T need(const std::string &key_dir, T default_, bool show_warning = true, std::string_view warning_message_view = "") const
    {
        const char *info = "'. It will use the default value: '";
        std::string warning_message;
        if (warning_message_view.empty())
            warning_message = "Warning: Unable to find configuration item: '" + key_dir;
        else
            warning_message = std::string(warning_message_view);
        auto keys = split(key_dir, ".");
        auto table_keys = std::vector<std::string>(keys.begin(), keys.end() - 1);
        auto table = &this->config;
        for (auto key : table_keys)
        {
            if (!table->contains(key) || !table->at(key).is_table())
            {
                if (show_warning)
                    LOG_WARN(warning_message, info, default_, "'.");
                return default_;
            }
            table = table->at(key).as_table();
        }
        if (!table->contains(keys.back()))
        {
            if (show_warning)
                LOG_WARN(warning_message, info, default_, "'.");
            return default_;
        }
        toml::v3::optional<T> value = table->at(keys.back()).value<T>();
        if (!value.has_value())
        {
            if (show_warning)
                LOG_WARN(warning_message, info, default_, "'.");
            return default_;
        }
        return value.value();
    }
};