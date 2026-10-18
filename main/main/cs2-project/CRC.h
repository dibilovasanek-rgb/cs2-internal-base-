#pragma once

#include <iostream>
#include "sdk/math/vector.h"
#include "sdk/game_class/usercmd.h"
#include "sdk/game_class/utl.h"
#include "sdk/game_class/utlbuffer.h"
#include "sdk/game_class/interfaces/i_mem_alloc.h"		

namespace CRC
{
//	struct CInButtonStateNoVTable
//	{
//	public:
//		std::uint64_t nValue;
//		std::uint64_t nValueChanged;
//		std::uint64_t nValueScroll;
//	};
//
//	struct SavedData_t
//	{
//		CInButtonStateNoVTable nButtons;
//		qangle_t angView;
//	};
//
//	inline SavedData_t savedData;
//
//	inline void Save(c_user_cmd* pCmd)
//	{
//		if (!pCmd)
//			return;
//
//		if (pCmd->pb.base().has_viewangles() /*!= nullptr*/) // переделать
//			savedData.angView = { pCmd->pb.mutable_base()->viewangles().x(),pCmd->pb.mutable_base()->viewangles().y(), pCmd->pb.mutable_base()->viewangles().z() };
//
//		pCmd->pb.mutable_base()->mutable_buttons_pb()->set_buttonstate1(pCmd->m_button_state.nValue);
//		pCmd->pb.mutable_base()->mutable_buttons_pb()->set_buttonstate2(pCmd->m_button_state.nValueChanged);
//		pCmd->pb.mutable_base()->mutable_buttons_pb()->set_buttonstate3(pCmd->m_button_state.nValueScroll);
//
//		savedData.nButtons.nValue = pCmd->pb.mutable_base()->mutable_buttons_pb()->buttonstate1();
//		savedData.nButtons.nValueChanged = pCmd->pb.mutable_base()->mutable_buttons_pb()->buttonstate2();
//		savedData.nButtons.nValueScroll = pCmd->pb.mutable_base()->mutable_buttons_pb()->buttonstate3();
//	}
//
//	inline void Apply(c_user_cmd* pCmd)
//	{
//		if (!pCmd)
//			return;
//
//		CBaseUserCmdPB* pBaseCmd = pCmd->pb.mutable_base();
//		if (pBaseCmd == nullptr)
//			return;
//
//		pCmd->m_button_state.nValue = savedData.nButtons.nValue;
//		pCmd->m_button_state.nValueChanged = savedData.nButtons.nValueChanged;
//		pCmd->m_button_state.nValueScroll = savedData.nButtons.nValueScroll;
//
//		if (pCmd->pb.base().has_viewangles()) {
//			pBaseCmd->mutable_viewangles()->set_x(savedData.angView.x);
//			pBaseCmd->mutable_viewangles()->set_y(savedData.angView.y);
//			pBaseCmd->mutable_viewangles()->set_z(savedData.angView.z);
//		}
//	}
//
//	inline bool CalculateCRC(CBaseUserCmdPB* pBaseCmd)
//	{
//		if (!pBaseCmd)
//			return false;
//
//		int nCalcualtedCRCSize = pBaseCmd->CalculateCmdCRCSize();
//		CUtlBuffer protobufBuffer(0, 0, 0);
//		protobufBuffer.EnsureCapacity(nCalcualtedCRCSize + 1);
//
//		using fnSerializePartialToArray = bool(__fastcall*)(CBaseUserCmdPB*, CUtlBuffer, int);
//		static const fnSerializePartialToArray oSerializePartialToArray = reinterpret_cast<fnSerializePartialToArray>(g_memory->find_pattern(CLIENT_DLL, "48 89 5C 24 18 55 56 57 48 81 EC 90"));
//
//#ifdef CS_PARANOID
//		CS_ASSERT(oSerializePartialToArray != nullptr);
//#endif
//
//		if (oSerializePartialToArray(pBaseCmd, protobufBuffer, nCalcualtedCRCSize))
//		{
//			std::uintptr_t* pMessage = reinterpret_cast<uintptr_t*>(g_interface->mem_alloc->alloc(0x18));
//			pBaseCmd->nCachedBits |= 1;
//			auto nHasBits = static_cast<uint32_t>(pBaseCmd->nHasBits & 0xFFFFFFFFFFFFFFFC);
//			if ((pBaseCmd->nHasBits & 1) != 0)
//				nHasBits = static_cast<uint32_t>(nHasBits);
//
//			using fnWriteMessage = void(__fastcall*)(std::uintptr_t*, CUtlBuffer, int);
//			static const fnWriteMessage oWriteMessage = reinterpret_cast<fnWriteMessage>(g_memory->find_pattern(CLIENT_DLL,"48 89 5C 24 10 48 89 6C 24 18 48 89 7C 24 20 41 56 48 83 EC 20 48 BF"));
//
//#ifdef CS_PARANOID
//			CS_ASSERT(oWriteMessage != nullptr);
//#endif
//
//			using fnSetMessageData = std::string* (__fastcall*)(void*, std::uintptr_t*, void*);
//			static const fnSetMessageData oSetMessageData = reinterpret_cast<fnSetMessageData>(g_memory->find_pattern(CLIENT_DLL, "48 89 5C 24 20 55 56 57 48 83 EC 30 49"));
//
//#ifdef CS_PARANOID
//			CS_ASSERT(oSetMessageData != nullptr);
//#endif
//
//			oWriteMessage(pMessage, protobufBuffer, nCalcualtedCRCSize);
//			pBaseCmd->strMoveCrc = oSetMessageData(&pBaseCmd->strMoveCrc, pMessage, &nHasBits);
//			g_interface->mem_alloc->free(pMessage);
//
//			return true;
//		}
//
//		return false;
//	}
}
