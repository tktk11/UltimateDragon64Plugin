#include "UDHooks.h"

#include "../skse64/PapyrusEvents.h"
#include "../skse64/GameData.h"
#include "../skse64/GameRTTI.h"

#include <cinttypes>

#include "UltimateDragons.h"
#include "ultimateplugins_common/EventFunctors.h"

UInt32 UDInitQuestFormID;
UInt32 UDSpellFormID;
UInt32 ActorTypeDragon;



bool HasNPCKeyword(TESNPC* thisNPC, UInt32 Key)
{
	if (thisNPC && Key)
	{
		BGSKeywordForm* keywordForm = DYNAMIC_CAST(thisNPC->race.race, TESRace, BGSKeywordForm);
		UInt32 count = keywordForm->numKeywords;
		BGSKeyword** keywords = keywordForm->keywords;
		if (keywords)
		{
			BGSKeyword* keyword = (BGSKeyword*)LookupFormByID(Key);
			for (int i = 0; i < count; i++)
			{
				BGSKeyword* pKey = keywords[i];
				if (pKey && pKey == keyword)
				{
					return true;
				}
			}
		}
	}
	return false;
}

static bool HasDragonKeyword(Actor* actor)
{
	if (actor)
	{
		D(const char* actorName = UPCommon::GetActorName(actor));


		if (ActorTypeDragon)
		{
			TESNPC* actorBase = DYNAMIC_CAST(actor->baseForm, TESForm, TESNPC);
			if (actorBase)
			{
				if (HasNPCKeyword(actorBase, ActorTypeDragon))
				{
					D(_DMESSAGE("[DEBUG] %s Has Keyword.", actorName));

					return true;
				}
			}
		}
	}
	return false;
}

static bool HasUDDragonAISpell(Actor* actor)
{
	Actor::SpellArray* spellArray = &actor->addedSpells;
	const size_t count = spellArray->Length();

	if (!UDSpellFormID)
		D(_DMESSAGE("[DEBUG] No spell form ID loaded"));

	bool bHasSpell = false;
	for (int i = 0; i < count; i++)
	{
		SpellItem* spell = spellArray->Get(i);
		if (spell->formID == UDSpellFormID)
		{
			bHasSpell = true;
			break;
		}
	}

	return bHasSpell;
}

class CombatEventHandler : public BSTEventSink<TESCombatEvent>
{
public:
	EventResult ReceiveEvent(TESCombatEvent* evn, EventDispatcher<TESCombatEvent>* dispatcher) override
	{
		const auto target = DYNAMIC_CAST(evn->target, TESObjectREFR, Actor);
		const auto source = DYNAMIC_CAST(evn->source, TESObjectREFR, Actor);

		D(
			const char* casterName = UPCommon::GetActorName(source);
			const char* targetName = UPCommon::GetActorName(target);

			switch (evn->state)
			{
				case 0:
				_DMESSAGE("[DEBUG] Not in combat:\n  caster: %s", casterName);
				break;
				case 1:
				_DMESSAGE("[DEBUG] In combat:\n  caster: %s\n  target: %s", casterName, targetName);
				break;
				case 2:
				_DMESSAGE("[DEBUG] Searching:\n  caster: %s\n  target: %s", casterName, targetName);
				break;
			}
		);


		if (!HasDragonKeyword(source))
			return kEvent_Continue;

		if (HasUDDragonAISpell(source))
		{
			D(_DMESSAGE("[DEBUG] %s has spell already", casterName));
			return kEvent_Continue;
		}

		D(_DMESSAGE("[DEBUG] Combat event with dragon as source, dispatch Event UD_OnCombat"));

		const auto form = LookupFormByID(UDInitQuestFormID);

		if (form)
		{
			const auto quest = DYNAMIC_CAST(form, TESForm, TESQuest);

			T(_DMESSAGE("[TRACE} quest ptr 0x%016" PRIXPTR, quest));
			if (quest)
			{
				const UInt64 vmhandle = UPCommon::GetVMHandleForQuest(quest);
				if (vmhandle)
				{
					static BSFixedString eventName("UD_OnCombat");
					UPCommon::EventFunctor2<Actor*, UInt32>(eventName, source, evn->state)(vmhandle);
				}
			}
		}

		return kEvent_Continue;
	}
};

CombatEventHandler g_combatEventHandler;

bool InitUDHooks()
{
	ActorTypeDragon = UPCommon::GetFormId("Skyrim.esm", DRAGON_KEYWORD_FORM_ID);
	UDInitQuestFormID = UPCommon::GetFormId(UD_ESP_NAME, UD_QUEST_FORM_ID);
	UDSpellFormID = UPCommon::GetFormId(UD_ESP_NAME, UD_SPELL_FORM_ID);

	_MESSAGE("[STARTUP] Registering combat event listener");
	auto eventDispatchers = GetEventDispatcherList();
	eventDispatchers->combatDispatcher.AddEventSink(&g_combatEventHandler);
	_MESSAGE("[STARTUP] Registered");

	return true;
}
