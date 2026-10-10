#pragma once

#include "Dumper-7_5.3.2-29314046+++UE5+Release-5.3-ChainedTogether/CppSDK/SDK/CustomGI_classes.hpp"
#include "Dumper-7_5.3.2-29314046+++UE5+Release-5.3-ChainedTogether/CppSDK/SDK/BP_Character_classes.hpp"
#include "Dumper-7_5.3.2-29314046+++UE5+Release-5.3-ChainedTogether/CppSDK/SDK/Engine_classes.hpp"
#include "Dumper-7_5.3.2-29314046+++UE5+Release-5.3-ChainedTogether/CppSDK/SDK/GS_Game_classes.hpp"
#include "Dumper-7_5.3.2-29314046+++UE5+Release-5.3-ChainedTogether/CppSDK/SDK/UMG_classes.hpp"
#include "Dumper-7_5.3.2-29314046+++UE5+Release-5.3-ChainedTogether/CppSDK/SDK/W_ComboBox_classes.hpp"
#include "Dumper-7_5.3.2-29314046+++UE5+Release-5.3-ChainedTogether/CppSDK/SDK/W_CreateSession_classes.hpp"

#include <cctype>

namespace max16_players
{
	inline constexpr int32_t kMaximumPlayers = 16;
	inline int32_t selected_players = 4;
	inline bool host_widget_seen = false;
	inline int32_t last_logged_selection = 0;

	inline bool parse_player_count(const SDK::FString& value, int32_t& result)
	{
		const auto text = value.ToString();
		const auto first_digit = text.find_first_not_of(' ');
		if (first_digit == std::string::npos)
		{
			return false;
		}

		auto digits_end = first_digit;
		while (digits_end < text.size() && std::isdigit(static_cast<unsigned char>(text[digits_end])))
		{
			++digits_end;
		}
		if (digits_end == first_digit)
		{
			return false;
		}

		auto suffix = text.substr(digits_end);
		const auto suffix_start = suffix.find_first_not_of(' ');
		if (suffix_start != std::string::npos)
		{
			suffix = suffix.substr(suffix_start);
			if (suffix != "Player" && suffix != "Players")
			{
				return false;
			}
		}

		result = std::stoi(text.substr(first_digit, digits_end - first_digit));
		return result >= 1 && result <= kMaximumPlayers;
	}

	inline bool is_player_count_combo(const SDK::UW_ComboBox_C* combo)
	{
		if (!combo || combo->Options.Num() == 0)
		{
			return false;
		}

		const auto first_label = combo->Options[0].ToString();
		int32_t count = 0;
		return first_label.find("Player") != std::string::npos && parse_player_count(combo->Options[0], count);
	}

	inline void add_player_options(SDK::UW_ComboBox_C* combo)
	{
		if (!is_player_count_combo(combo))
		{
			return;
		}

		const auto label_has_players = combo->Options[0].ToString().find("Player") != std::string::npos;
		const auto initial_option_count = combo->Options.Num();
		for (int32_t count = 5; count <= kMaximumPlayers; ++count)
		{
			const auto option_text = std::to_string(count) + (label_has_players ? " Players" : "");
			bool already_present = false;
			for (int32_t index = 0; index < combo->Options.Num(); ++index)
			{
				if (combo->Options[index].ToString() == option_text)
				{
					already_present = true;
					break;
				}
			}
			if (already_present)
			{
				continue;
			}

			const auto option = std::wstring(option_text.begin(), option_text.end());
			combo->AddOneOption(SDK::FString(option.c_str()));
		}

		if (initial_option_count < kMaximumPlayers && combo->Options.Num() >= kMaximumPlayers)
		{
			LOG(INFO) << "[Max16] Player dropdown now has " << combo->Options.Num() << " options";
		}
	}

	inline void sync_host_widget(SDK::UW_CreateSession_C* widget)
	{
		if (!widget)
		{
			return;
		}
		if (!host_widget_seen)
		{
			host_widget_seen = true;
			LOG(INFO) << "[Max16] Host session widget found";
		}

		// The current host widget stores W_ComboBox_NumPlayerSelect after the three older combo fields.
		auto* combo = *reinterpret_cast<SDK::UW_ComboBox_C**>(reinterpret_cast<char*>(widget) + 0x350);
		auto& widget_max_players = *reinterpret_cast<int32_t*>(reinterpret_cast<char*>(widget) + 0x3B0);
		if (combo)
		{
			add_player_options(combo);
			int32_t count = 0;
			const auto& current_selection = combo->ComboBoxString_75
			    ? combo->ComboBoxString_75->SelectedOption
			    : combo->SelectedOption;
			if (is_player_count_combo(combo) && parse_player_count(current_selection, count))
			{
				if (last_logged_selection != count)
				{
					last_logged_selection = count;
					LOG(INFO) << "[Max16] Selected player count: " << count;
				}
				selected_players = count;
				widget_max_players = count;
			}
		}

		if (widget_max_players >= 1 && widget_max_players <= kMaximumPlayers)
		{
			selected_players = widget_max_players;
		}
	}

	inline void sync_session_limit()
	{
		auto* world = SDK::UWorld::GetWorld();
		if (!world)
		{
			return;
		}

		if (world->OwningGameInstance && world->OwningGameInstance->IsA(SDK::UCustomGI_C::StaticClass()))
		{
			static_cast<SDK::UCustomGI_C*>(world->OwningGameInstance)->MaxPlayers = selected_players;
		}

		if (world->AuthorityGameMode && world->AuthorityGameMode->GameSession)
		{
			world->AuthorityGameMode->GameSession->MaxPlayers = selected_players;
		}
	}

	inline int32_t property_offset(const SDK::UClass* class_type, const char* property_name)
	{
		for (auto* current = static_cast<const SDK::UStruct*>(class_type); current; current = current->Super)
		{
			for (auto* field = current->ChildProperties; field; field = field->Next)
			{
				if (field->Name.GetRawString() == property_name)
				{
					return static_cast<const SDK::FProperty*>(field)->Offset;
				}
			}
		}
		return -1;
	}

	inline bool contains_pawn(const SDK::TArray<SDK::APawn*>& pawns, const SDK::APawn* pawn)
	{
		for (int32_t index = 0; index < pawns.Num(); ++index)
		{
			if (pawns[index] == pawn)
			{
				return true;
			}
		}
		return false;
	}

	inline void reconcile_chain_links(SDK::AGS_Game_C* game_state)
	{
		static bool reconciling = false;
		if (reconciling || !game_state || selected_players <= 4 || !game_state->HasAuthority())
		{
			return;
		}

		auto* character_class = SDK::ABP_Character_C::StaticClass();
		if (!character_class ||
			!character_class->GetFunction("BP_Character_C", "AddAttachedCharacter") ||
			!character_class->GetFunction("BP_Character_C", "Add Chain") ||
			!character_class->GetFunction("BP_Character_C", "ChainAlreadyExist?"))
		{
			return;
		}

		const auto pawn_array_offset = property_offset(game_state->Class, "PawnsArray");
		const auto attached_array_offset = property_offset(character_class, "Rep_Attached_Characters");
		if (pawn_array_offset < 0 || attached_array_offset < 0)
		{
			return;
		}

		const auto& pawns = *reinterpret_cast<const SDK::TArray<SDK::APawn*>*>(
			reinterpret_cast<const char*>(game_state) + pawn_array_offset);
		if (pawns.Num() <= 4 || pawns.Num() > kMaximumPlayers || !pawns.IsValid())
		{
			return;
		}

		reconciling = true;
		int32_t repaired_links = 0;
		auto repair_pair = [&](SDK::APawn* first, SDK::APawn* second)
		{
			if (!first || !second || first == second ||
				!first->IsA(character_class) || !second->IsA(character_class))
			{
				return;
			}

			auto* first_character = static_cast<SDK::ABP_Character_C*>(first);
			auto* second_character = static_cast<SDK::ABP_Character_C*>(second);
			const auto& first_attached = *reinterpret_cast<const SDK::TArray<SDK::APawn*>*>(
				reinterpret_cast<const char*>(first_character) + attached_array_offset);
			const auto& second_attached = *reinterpret_cast<const SDK::TArray<SDK::APawn*>*>(
				reinterpret_cast<const char*>(second_character) + attached_array_offset);
			if (first_attached.Num() < 0 || first_attached.Num() > kMaximumPlayers ||
				second_attached.Num() < 0 || second_attached.Num() > kMaximumPlayers)
			{
				return;
			}

			if (!contains_pawn(first_attached, second))
			{
				first_character->AddAttachedCharacter(second);
			}
			if (!contains_pawn(second_attached, first))
			{
				second_character->AddAttachedCharacter(first);
			}
			if (!first_character->ChainAlreadyExist_(first, second))
			{
				first_character->Add_Chain(second);
				++repaired_links;
			}
		};

		for (int32_t index = 0; index + 1 < pawns.Num(); ++index)
		{
			repair_pair(pawns[index], pawns[index + 1]);
		}

		bool opened_chain = true;
		if (auto* world = SDK::UWorld::GetWorld(); world && world->OwningGameInstance &&
			world->OwningGameInstance->IsA(SDK::UCustomGI_C::StaticClass()))
		{
			const auto opened_chain_offset = property_offset(world->OwningGameInstance->Class, "OpenedChain");
			if (opened_chain_offset >= 0)
			{
				opened_chain = *reinterpret_cast<const bool*>(
					reinterpret_cast<const char*>(world->OwningGameInstance) + opened_chain_offset);
			}
		}
		if (!opened_chain)
		{
			repair_pair(pawns[pawns.Num() - 1], pawns[0]);
		}
		reconciling = false;
		static int32_t last_checked_count = 0;
		if (last_checked_count != pawns.Num())
		{
			last_checked_count = pawns.Num();
			LOG(INFO) << "[Max16] Checked " << pawns.Num() << " chain members (opened=" << opened_chain << ")";
		}
		if (repaired_links)
		{
			LOG(INFO) << "[Max16] Repaired " << repaired_links << " chain links for " << pawns.Num() << " players";
		}
	}

} // namespace max16_players




