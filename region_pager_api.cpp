/* Copyright: (c) Kayne Ruse 2013-2016
 * 
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 * 
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 * 
 * 1. The origin of this software must not be misrepresented; you must not
 * claim that you wrote the original software. If you use this software
 * in a product, an acknowledgment in the product documentation would be
 * appreciated but is not required.
 * 
 * 2. Altered source versions must be plainly marked as such, and must not be
 * misrepresented as being the original software.
 * 
 * 3. This notice may not be removed or altered from any source
 * distribution.
*/
#include "region_pager_api.hpp"

#include "region_pager_lua.hpp"
#include "region.hpp"

#include <stdexcept>

//DOCS: These glue functions simply wrap RegionPagerLua's methods
//DOCS: These functions take a pager as an optional first parameter

//NOTE: zero indexing is used here, but not in the region API

static RegionPagerLua* getPager(lua_State* L) {
	RegionPagerLua* pager = nullptr;

	//get the pager, either the global pager or the parameter pager
	lua_getglobal(L, REGION_PAGER_NAME);
	if (lua_isnil(L, -1)) {
		pager = reinterpret_cast<RegionPagerLua*>(lua_touserdata(L, 1));
	}
	else {
		pager = reinterpret_cast<RegionPagerLua*>(lua_touserdata(L, -1));
	}
	lua_pop(L, 1);

	return pager;
}

static int setTile(lua_State* L) {
	//get the pager
	RegionPagerLua* pager = getPager(L);

	//set tile
	int ret = pager->SetTile(lua_tointeger(L, -4), lua_tointeger(L, -3), lua_tointeger(L, -2), lua_tointeger(L, -1));
	lua_pushinteger(L, ret);
	return 1;
}

static int getTile(lua_State* L) {
	//get the pager
	RegionPagerLua* pager = getPager(L);

	//get tile
	int ret = pager->GetTile(lua_tointeger(L, -3), lua_tointeger(L, -2), lua_tointeger(L, -1));
	lua_pushinteger(L, ret);
	return 1;
}

static int setSolid(lua_State* L) {
	//get the pager
	RegionPagerLua* pager = getPager(L);

	//set solid
	bool ret = pager->SetSolid(lua_tointeger(L, -3), lua_tointeger(L, -2), lua_toboolean(L, -1));
	lua_pushboolean(L, ret);
	return 1;
}

static int getSolid(lua_State* L) {
	//get the pager
	RegionPagerLua* pager = getPager(L);

	//get solid
	bool ret = pager->GetSolid(lua_tointeger(L, -2), lua_tointeger(L, -1));
	lua_pushboolean(L, ret);
	return 1;
}

static int getRegion(lua_State* L) {
	//get the pager
	RegionPagerLua* pager = getPager(L);

	//get region
	Region* region = pager->GetRegion(lua_tointeger(L, -2), lua_tointeger(L, -1));
	lua_pushlightuserdata(L, region);
	return 1;
}

static int loadRegion(lua_State* L) {
	//get the pager
	RegionPagerLua* pager = getPager(L);

	//load region
	Region* region = pager->LoadRegion(lua_tointeger(L, -2), lua_tointeger(L, -1));
	lua_pushlightuserdata(L, region);
	return 1;
}

static int saveRegion(lua_State* L) {
	//get the pager
	RegionPagerLua* pager = getPager(L);

	//save region
	Region* region = pager->SaveRegion(lua_tointeger(L, -2), lua_tointeger(L, -1));
	lua_pushlightuserdata(L, region);
	return 1;
}

static int createRegion(lua_State* L) {
	//get the pager
	RegionPagerLua* pager = getPager(L);

	//create region
	Region* region = pager->CreateRegion(lua_tointeger(L, -2), lua_tointeger(L, -1));
	lua_pushlightuserdata(L, region);
	return 1;
}

static int unloadRegion(lua_State* L) {
	//get the pager
	RegionPagerLua* pager = getPager(L);

	//two argument types: coords & the region itself
	switch(lua_type(L, -1)) {
		case LUA_TNUMBER:
			//unload using coords
			pager->UnloadIf([&](Region const& region) -> bool {
				int x = lua_tointeger(L, -2);
				int y = lua_tointeger(L, -1);
				return region.GetX() == x && region.GetY() == y;
			});
		break;
		case LUA_TLIGHTUSERDATA:
			//unloading using reference
			pager->UnloadIf([&](Region const& region) -> bool {
				return (&region) == lua_touserdata(L, -1);
			});
		break;
	}
	return 0;
}

static int setOnLoad(lua_State* L) {
	//get the pager
	RegionPagerLua* pager = getPager(L);

	//set on load
	luaL_unref(L, LUA_REGISTRYINDEX, pager->GetLoadReference());
	pager->SetLoadReference(luaL_ref(L, LUA_REGISTRYINDEX));
	return 0;
}

static int setOnSave(lua_State* L) {
	//get the pager
	RegionPagerLua* pager = getPager(L);

	//set on save
	luaL_unref(L, LUA_REGISTRYINDEX, pager->GetSaveReference());
	pager->SetSaveReference(luaL_ref(L, LUA_REGISTRYINDEX));
	return 0;
}

static int setOnCreate(lua_State* L) {
	//get the pager
	RegionPagerLua* pager = getPager(L);

	//set on create
	luaL_unref(L, LUA_REGISTRYINDEX, pager->GetCreateReference());
	pager->SetCreateReference(luaL_ref(L, LUA_REGISTRYINDEX));
	return 0;
}

static int setOnUnload(lua_State* L) {
	//get the pager
	RegionPagerLua* pager = getPager(L);

	//set on unload
	luaL_unref(L, LUA_REGISTRYINDEX, pager->GetUnloadReference());
	pager->SetUnloadReference(luaL_ref(L, LUA_REGISTRYINDEX));
	return 0;
}

static int forEach(lua_State* L) {
	//get the pager
	RegionPagerLua* pager = getPager(L);

	//run the given closure on each region
	pager->ForEach([L](Region& r) -> void {
		lua_pushvalue(L, -1); //copy the closure passed
		lua_pushlightuserdata(L, &r); //push this region

		//call the function, catching any errors
		if (lua_pcall(L, 1, 0, 0)) {
			throw(std::runtime_error( lua_tostring(L, -1) ));
		}
	});

	return 0;
}

//debugging
static int containerSize(lua_State* L) {
	//get the pager
	RegionPagerLua* pager = getPager(L);

	//push container size
	lua_pushinteger(L, pager->GetContainer()->size());
	return 1;
}

static const luaL_Reg regionPagerLib[] = {
	//curry
	{"SetTile", setTile},
	{"GetTile", getTile},
	{"SetSolid", setSolid},
	{"GetSolid", getSolid},

	//access and control
	{"GetRegion", getRegion},
	{"LoadRegion", loadRegion},
	{"SaveRegion", saveRegion},
	{"CreateRegion", createRegion},
	{"UnloadRegion", unloadRegion},

	//triggers
	{"SetOnLoad",setOnLoad},
	{"SetOnSave",setOnSave},
	{"SetOnCreate",setOnCreate},
	{"SetOnUnload",setOnUnload},

	{"ForEach", forEach},

	//debugging
	{"ContainerSize", containerSize},

	//sentinel
	{nullptr, nullptr}
};

LUAMOD_API int openRegionPagerAPI(lua_State* L) {
	luaL_newlib(L, regionPagerLib);
	return 1;
}