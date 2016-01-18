#pragma once

#include <unordered_map>
#include <string>
#include <QDebug>

// This represents a command argument in SilkEdit keymap.yml file.
// e.g. { key: ctrl+b, command: move_cursor_left, args: {repeat: 1}}
// Be careful, args value (1 in this case) is stored as std::string because yaml-cpp store it as
// string internally and no way to know its actual type.
// key and value are UTF-8 encoded strings.
// According to the spec, if YAML file doesn't have BOM, it must be UTF-8 encoded.
// http://yaml.org/spec/current.html#id2513364
typedef std::unordered_map<std::string, std::string> CommandArgument;

Q_DECLARE_METATYPE(CommandArgument)
