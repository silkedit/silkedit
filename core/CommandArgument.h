#pragma once

#include <unordered_map>
#include <string>
#include <QDebug>

// This represents a command argument in SilkEdit keymap.yml file.
// e.g. { key: ctrl+b, command: move_cursor_left, args: {repeat: 1}}
// key is UTF-8 encoded strings.
// According to the spec, if YAML file doesn't have BOM, it must be UTF-8 encoded.
// http://yaml.org/spec/current.html#id2513364
typedef std::unordered_map<std::string, QVariant> CommandArgument;

Q_DECLARE_METATYPE(CommandArgument)
