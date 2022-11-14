//
// Created by ddodd on 8/26/2021.
//

#ifndef FATPARENT_ENVIRONMENTREGISTRY_H
#define FATPARENT_ENVIRONMENTREGISTRY_H

#include <map>
#include <any>
#include <string>

#include "gtest/gtest.h"

using namespace std;

class EnvironmentRegistry {
    map<string , ::testing::Environment*> m_registry;
public:
    void register_environment(string name, ::testing::Environment* data) {
        m_registry[name] = data;
    };

    ::testing::Environment* GetEnvironment(string name) {
        return m_registry[name];
    }
};

extern EnvironmentRegistry g_environment_registry;

#endif //FATPARENT_ENVIRONMENTREGISTRY_H


