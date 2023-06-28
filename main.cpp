#include "../skse64/PluginAPI.h"
#include "../skse64_common/skse_version.h"

#include "../skse64/GameData.h"

#include <ShlObj.h>

#include "UltimateDragons.h"
#include "UDHooks.h"

IDebugLog gLog;

PluginHandle g_pluginHandle = kPluginHandle_Invalid;

// Interfaces
SKSEMessagingInterface* g_messaging = nullptr;

void SKSEMessageHandler(SKSEMessagingInterface::Message* msg)
{
	switch (msg->type)
	{
	case SKSEMessagingInterface::kMessage_DataLoaded:
		if (DataHandler::GetSingleton()->LookupModByName(UD_ESP_NAME) && DataHandler::GetSingleton()->LookupModByName(UD_ESP_NAME)->IsActive())
		{
			_MESSAGE("[STARTUP] UD ESP %s found, loading UD hooks", UD_ESP_NAME);
			InitUDHooks();
		}

		break;
	}
}

extern "C" {
bool SKSEPlugin_Query(const SKSEInterface* skse, PluginInfo* info)
{
	gLog.OpenRelative(CSIDL_MYDOCUMENTS, R"(\My Games\Skyrim Special Edition\SKSE\UltimateDragons.log)");
	gLog.SetPrintLevel(IDebugLog::kLevel_Error);
	gLog.SetLogLevel(IDebugLog::kLevel_DebugMessage);

	_MESSAGE("[STARTUP] Ultimate Dragons - 64 bit");

	// populate info structure
	info->infoVersion = PluginInfo::kInfoVersion;
	info->name = "UltimateDragons64";
	info->version = 1;

	g_pluginHandle = skse->GetPluginHandle();

	if (skse->isEditor)
	{
		_FATALERROR("[ERROR] loaded in editor, marking as incompatible");
		return false;
	}
	if (skse->runtimeVersion != RUNTIME_VERSION_1_5_73)
	{
		_FATALERROR("[ERROR] unsupported runtime version %08X", skse->runtimeVersion);
		return false;
	}

	g_messaging = static_cast<SKSEMessagingInterface *>(skse->QueryInterface((kInterface_Messaging)));
	if (!g_messaging)
	{
		_FATALERROR("[ERROR] couldn't get messaging interface");
		return false;
	}

	return true;
}

bool SKSEPlugin_Load(const SKSEInterface* skse)
{
	g_messaging->RegisterListener(g_pluginHandle, "SKSE", SKSEMessageHandler);
	return true;
}
}
