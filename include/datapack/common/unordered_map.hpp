#pragma once

#include <unordered_map>
#include "datapack/packers.hpp"


namespace datapack {

template <writeable K, writeable V>
void pack(const std::unordered_map<K, V>& value, Writer& writer) {
    writer.map_begin();
    for (const auto& pair: value) {
        writer.map_key();
        writer.value(pair.first);
        writer.map_value();
        writer.value(pair.second);
    }
    writer.map_end();
}

template <readable K, readable V>
void pack(std::unordered_map<K, V>& value, Reader& reader) {
    std::pair<K, V> pair;
    value.clear();
    reader.map_begin();
    auto iter = value.begin();
    while (reader.map_key()) {
        reader.value(pair.first);
        reader.map_value();
        reader.value(pair.second);
    }
    reader.map_end();
}

template <readable K, readable V>
void pack(std::unordered_map<K, V>& value, Editor& editor) {
    editor.map_begin();
    for (auto& pair: value) {
        editor.map_key();
        // TODO: Handle this more nicely, the key is fixed so
        // should disable editing
        K temp = pair.first;
        editor.value(temp);
        editor.map_value();
        editor.value(pair.second);
    }
    editor.map_end();
    if (!editor.map_action_begin()) {
        return;
    }

    K temp;
    editor.value(temp);
    switch (editor.map_action_end()) {
        case ContainerAction::None:
            break;
        case ContainerAction::Push:
            value.emplace(temp, V());
            break;
        case ContainerAction::Pop:
            value.erase(temp);
            break;
    }
}

} // namespace datapack
