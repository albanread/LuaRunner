//
// LuaBindings.h
// LuaRunner2 - Lua bindings for SuperTerminal API
//
// This file declares the Lua binding registration function for use with
// LuaRunner2. These bindings provide the SuperTerminal API functions to
// Lua scripts (no BASIC dependencies).
//

#ifndef LUARUNNER2_BINDINGS_H
#define LUARUNNER2_BINDINGS_H

#include <vector>
#include <string>
#include <map>

struct lua_State;

namespace LuaRunner2 {

// Register all SuperTerminal API functions in the Lua state
void registerBindings(lua_State* L);

} // namespace LuaRunner2

#endif // LUARUNNER2_BINDINGS_H