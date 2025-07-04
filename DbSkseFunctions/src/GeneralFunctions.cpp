#include <Windows.h>
//#undef GetObject
#include <cstdint>
#include <iostream>
#include <array>
#include <algorithm>
#include <cmath>
#include "GeneralFunctions.h"
#include "editorID.hpp"
#include "SharedVariables.h"

namespace gfuncs {
    std::chrono::system_clock::time_point lastTimeGameWasLoaded;

    const std::string hexDigits = "0123456789abcdef";
    const std::string reverseHexDigits = "fedcba9876543210";

    bool srandSet = false;

    enum logLevel { trace, debug, info, warn, error, critical };
    enum debugLevel { notification, messageBox };

    void LogAndMessage(std::string message, int logLevel, int debugLevel) {
        switch (logLevel) {
        case trace:
            logger::trace("{}", message);
            break;

        case debug:
            logger::debug("{}", message);
            break;

        case info:
            logger::info("{}", message);
            break;

        case warn:
            logger::warn("{}", message);
            break;

        case error:
            logger::error("{}", message);
            break;

        case critical:
            logger::critical("{}", message);
            break;
        }

        /*switch (debugLevel) {
        case notification:
            RE::DebugNotification(message.data());
            break;
        case messageBox:
            RE::DebugMessageBox(message.data());
            break;
        }*/
    }

    void ConvertToLowerCase(std::string& s) {
        transform(s.begin(), s.end(), s.begin(), ::tolower);
    }

    std::string uint32_to_string(uint32_t value) {
        std::array<char, 4> r;
        r[0] = static_cast<char>((value >> 24) & 0xFF);
        r[1] = static_cast<char>((value >> 16) & 0xFF);
        r[2] = static_cast<char>((value >> 8) & 0xFF);
        r[3] = static_cast<char>(value & 0xFF);
        return std::string{r[0], r[1], r[2], r[3]};
    }
    
    int GetRandomInt(int min, int max) {
        return (min + (rand() % (max - min + 1)));
    }

    bool IsHexString(std::string s) {
        if (s == "") {
            return false;
        }

        auto beginItr = s.begin();

        if (s.find("0x") == 0) {
            beginItr += 2;
        }

        return(std::all_of(beginItr, s.end(), ::isxdigit));
    }

    bool IsDecString(std::string s) {
        if (s == "") {
            return false;
        }

        return(std::all_of(s.begin(), s.end(), ::isdigit));
    }

    std::string IntToHex(int i) {
        std::stringstream stream;
        stream << "0x"
            << std::setfill('0') << std::setw(sizeof(int) * 2)
            << std::hex << i;
        return stream.str();
    }

    std::string IntToHexPapyrus(int i) {
        if (i == 0) {
            return "0";
        }

        std::string s = ""; 

        if (i >= 0) {
            while (i > 0) {
                s = (hexDigits.at((i % 16)) + s);
                i /= 16;
            }
        }
        else {
            i = (std::abs(i) - 1);
            while (i > 0) {
                s = (reverseHexDigits.at((i % 16)) + s);
                i /= 16;
            }
        }
        return s;
    }

    int HexToInt(std::string hex) {
        return stoi(hex, 0, 16);
    }

    uint64_t StringToUint64_t(std::string s) {
        uint64_t value;
        std::istringstream iss(s);
        iss >> value; 
        return value;
    }

    //does the string have at least minNumber of c characters in it?
    bool StringHasNCharacters(std::string s, char c, int minNumber) {
        int size = s.size();
        int count = 0;
        for (int i = 0; i < size && count < minNumber; i++) {
            if (s[i] == c) {
                count++;
            }
        }
        return (count >= minNumber);
    }
    //get the int after start char in string s. Example: GetIntAfterCharInString("arrows (21)") returns 21
    int GetIntAfterCharInString(std::string s, char startChar, int iDefault, int startIndex) {
        int iStart = -1;
        int iEnd = -1;
        int L = s.length();
        int i = startIndex;
        while (i < L) {
            if (s.at(i) == startChar) {
                if (isdigit(s.at(i + 1))) {
                    iStart = i + 1;
                    i++;
                    break;
                }
            }
            i++;
        }

        if (iStart != -1) {
            while (i < L) {
                if (!isdigit(s.at(i))) {
                    iEnd = i;
                    break;
                }
                i++;
            }
            if (iEnd == -1) {
                iEnd = L;
            }
            return stoi(s.substr(iStart, (iEnd - iStart)));
        }
        return iDefault;
    }

    bool IsFormValid(RE::TESForm* form, bool checkDeleted) {
        if (!form) {
            return false;
        }

        if (IsBadReadPtr(form, sizeof(form))) {
            return false;
        }

        if (form->GetFormID() == 0) {
            return false;
        }

        if (checkDeleted) {
            if (form->IsDeleted()) {
                return false;
            }
        }

        return true;
    }

    //forms and formParamNames should be the same size
    bool AllFormsValid(std::vector<RE::TESForm*> forms, std::vector<std::string> formParamNames, std::string callingFunctionName, bool checkDeleted) {
        for (size_t i = 0; i < forms.size(); i++) {
            auto* form = forms[i];
            if (!IsFormValid(form)) {
                logger::warn("[{}] [{}] doesn't exist", callingFunctionName, formParamNames[i]);
                return false;
            }
        }
        return true;
    }

    std::string GetFormEditorId(RE::StaticFunctionTag*, RE::TESForm* akForm, std::string nullFormString) {
        if (!IsFormValid(akForm)) {
            return nullFormString;
        }
        else {
            std::string editorId = akForm->GetFormEditorID();
            if (editorId == "") {
                editorId = clib_util::editorID::get_editorID(akForm);
            }
            if (editorId == "") {
                RE::TESObjectREFR* ref = akForm->As<RE::TESObjectREFR>();
                if (IsFormValid(ref)) {
                    RE::TESForm* baseForm = ref->GetBaseObject(); 
                    if (IsFormValid(baseForm)) {
                        editorId = baseForm->GetFormEditorID();
                        if (editorId == "") {
                            editorId = clib_util::editorID::get_editorID(baseForm);
                        }
                    }
                }
            }

            return editorId;
        }
    }

    void SetFormName(RE::TESForm* baseForm, RE::BSFixedString nuName) {
        RE::TESFullName* pFullName = baseForm->As<RE::TESFullName>();
        // is a const string, so have to just reassign it.
        if (pFullName) {
            pFullName->fullName = nuName;
        }
    }

    RE::BSFixedString GetFormName(RE::TESForm* akForm, RE::BSFixedString nullString, RE::BSFixedString noNameString, bool returnIdIfNull) {
        if (!IsFormValid(akForm)) {
            return nullString;
        }

        RE::TESObjectREFR* ref = akForm->As<RE::TESObjectREFR>();
        RE::BSFixedString name;
        if (IsFormValid(ref)) {
            name = ref->GetDisplayFullName();
            if (name == "") {
                auto* baseForm = ref->GetBaseObject();
                if (IsFormValid(baseForm)) {
                    name = baseForm->GetName();
                }
            }
        }
        else {
            name = akForm->GetName();
        }

        if (name == "") {
            if (returnIdIfNull) {
                name = GetFormEditorId(nullptr, akForm, "");
            }
            else {
                name = noNameString;
            }
        }
        return name;
    }

    std::string GetFormNameAndId(RE::TESForm* akForm, std::string nullString, std::string noNameString) {
        if (!IsFormValid(akForm)) {
            return nullString;
        }

        std::string name = std::string(GetFormName(akForm, nullString, noNameString, false));
        std::string id = std::format("{:x}", akForm->GetFormID());
        std::string s = "(name[" + name + "] id[" + id + "])";
        return s;
    }

    std::string GetFormDataString(RE::TESForm* akForm, std::string nullString, std::string noNameString) {
        if (!IsFormValid(akForm)) {
            return nullString;
        }

        std::string name = static_cast<std::string>(gfuncs::GetFormName(akForm, nullString, noNameString, false));

        std::string editorID = GetFormEditorId(nullptr, akForm, "");

        std::string dataString = std::format("(name[{}] editorID[{}] formID[{:x}]) type[{}])", name, editorID, akForm->GetFormID(), akForm->GetFormType());

        return dataString;
    }

    RE::TESFile* GetFileForForm(RE::TESForm* akForm) {
        if (!gfuncs::IsFormValid(akForm)) {
            return nullptr;
        }

        if (!sv::dataHandler) {
            return nullptr;
        }

        auto id = akForm->GetFormID();

        for (auto* file : sv::dataHandler->files) {
            if (file) {
                if (file->IsFormInMod(id)) {
                    return file;
                }
            }
        }

        return nullptr;
    }

    RE::TESFile* GetFileForRawFormId(RE::FormID rawFormID, RE::TESFile* file) {
        if (!file || file->compileIndex == 0xFF) {
            return nullptr;
        }

        auto rawIndex = (rawFormID & 0xFF000000) >> 24;
        if (REL::Module::IsVR() && !sv::dataHandler->VRcompiledFileCollection) {
            if (rawIndex >= file->masterCount) {
                return file;
            }
            return file->masterPtrs[rawIndex];
        }
        else {
            bool isLight = rawIndex == 0xFE;
            if (isLight) {
                rawIndex = (rawFormID & 0x00FFF000) >> 12;
            }

            std::uint32_t index = 0;
            for (std::uint32_t i = 0; i < file->masterCount; ++i) {
                auto* master = file->masterPtrs[i];
                if ((master->compileIndex == 0xFE) != isLight)
                { // is isLight = true execute it if master is smth different than light
                    // is isLight = false execute it if master is smth different than full
                    continue;
                }
                if (index++ == rawIndex) {
                    return master;
                }
            }
            return file;
        }
    }

    std::string GetModName(RE::TESForm* form) {
        if (!IsFormValid(form)) {
            return "";
        }

        const auto array = form->sourceFiles.array;
        if (!array || array->empty()) {
            return "";
        }

        const auto file = array->front();
        std::string_view filename = file ? file->GetFilename() : "";

        return (filename.data());
    }

    void logFormMap(auto& map) {
        logger::trace("logging form map");
        for (auto const& x : map)
        {
            RE::TESForm* akForm = x.first;
            if (akForm) {
                logger::trace("Form[{}] ID[{:x}] value[{}]", gfuncs::GetFormName(akForm), akForm->GetFormID(), x.second);
            }
        }
    }

    //return difference of time points in seconds as float
    float timePointDiffToFloat(std::chrono::system_clock::time_point end, std::chrono::system_clock::time_point start) {
        auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        return float(milliseconds.count()) * 0.001;
    }

    RE::VMHandle GetHandle(RE::TESForm* akForm) {
        if (!IsFormValid(akForm)) {
            logger::warn("akForm doesn't exist or isn't valid");
            return NULL;
        }

        if (!sv::skyrimVm) {
            logger::error("sv::skyrimVm* not found");
            return NULL;
        }

        RE::VMTypeID id = static_cast<RE::VMTypeID>(akForm->GetFormType());
        RE::VMHandle handle = sv::skyrimVm->handlePolicy.GetHandleForObject(id, akForm);

        return handle;
    }

    RE::VMHandle GetHandle(RE::BGSBaseAlias* akAlias) {
        if (!akAlias) {
            logger::warn("akAlias doesn't exist");
            return NULL;
        }

        if (!sv::skyrimVm) {
            logger::error("sv::skyrimVm* not found");
            return NULL;
        }

        RE::VMTypeID id = akAlias->GetVMTypeID();
        RE::VMHandle handle = sv::skyrimVm->handlePolicy.GetHandleForObject(id, akAlias);

        return handle;
    }

    RE::VMHandle GetHandle(RE::ActiveEffect* akEffect) {
        if (!akEffect) {
            logger::warn("akEffect doesn't exist");
            return NULL;
        }

        if (!sv::skyrimVm) {
            logger::error("sv::skyrimVm* not found");
            return NULL;
        }

        RE::VMTypeID id = akEffect->VMTYPEID;
        //RE::VMTypeID id = RE::ActiveEffect::VMTYPEID;
        RE::VMHandle handle = sv::skyrimVm->handlePolicy.GetHandleForObject(id, akEffect);

        return handle;
    }

    RE::ActiveEffect* GetActiveEffectFromHandle(RE::VMHandle handle) {
        RE::ActiveEffect* activeEffect = nullptr;
        if (!sv::skyrimVm) {
            logger::error("sv::skyrimVm* not found");
            return activeEffect;
        }

        RE::VMTypeID id = RE::ActiveEffect::VMTYPEID;
        auto* obj = sv::skyrimVm->handlePolicy.GetObjectForHandle(id, handle);
        if (obj) {
            activeEffect = static_cast<RE::ActiveEffect*>(obj);
        }
        return activeEffect;
    }

    RE::TESObjectREFR* GetRefFromHandle(RE::RefHandle& handle) {
        if (handle) {
            auto niPtr = RE::TESObjectREFR::LookupByHandle(handle);
            if (niPtr) {
                return niPtr.get();
            }
        }
        return nullptr;
    }

    RE::TESObjectREFR* GetRefFromObjectRefHandle(RE::ObjectRefHandle refHandle) {
        RE::TESObjectREFR* itemReference = nullptr;
        //std::string msg = "";

        if (refHandle) {
            //logger::debug("refHandle found");

            //msg = "Getting ref from refHandle.Get()";
            auto refPtr = refHandle.get();
            if (refPtr) {
                itemReference = refPtr.get();
            }

            if (!IsFormValid(itemReference)) {
                //msg = "Getting ref by RE::TESForm::LookupByID";
                RE::TESForm* refForm = RE::TESForm::LookupByID(refHandle.native_handle());
                if (IsFormValid(refForm)) {
                    itemReference = refForm->AsReference();
                }
            }

            if (!IsFormValid(itemReference)) {
                //msg = "Getting ref from RE::TESObjectREFR::LookupByHandle";
                auto niPtr = RE::TESObjectREFR::LookupByHandle(refHandle.native_handle());
                if (niPtr) {
                    itemReference = niPtr.get();
                }
            }

            /*if (!IsFormValid(itemReference)) {
                msg = "Getting ref by comparing handle to all object references";
                const auto& [allForms, lock] = RE::TESForm::GetAllForms();
                for (auto& [id, form] : *allForms) {
                    if (IsFormValid(form)) {
                        auto* ref = form->AsReference();
                        if (IsFormValid(ref)) {
                            if (ref->GetHandle() == refHandle) {
                                itemReference = ref;
                                break;
                            }
                        }
                    }
                }
            }*/
        }
        if (!IsFormValid(itemReference)) {
            itemReference = nullptr;
        }
        //logger::trace("{}", msg);
        return itemReference;
    }

    bool IsScriptAttachedToHandle(RE::VMHandle& handle, RE::BSFixedString& sScriptName) {
        if (handle == NULL) {
            logger::error("handle is null");
            return false;
        }

        if (!sv::vm) {
            logger::error("vm not found");
            return false;
        }

        auto it = sv::vm->attachedScripts.find(handle);
        if (it != sv::vm->attachedScripts.end()) {
            //logger::error("it->second.size() = {}", it->second.size());
            for (int i = 0; i < it->second.size(); i++) {
                auto& attachedScript = it->second[i];
                if (attachedScript) {
                    auto* script = attachedScript.get();
                    if (script) {
                        auto info = script->GetTypeInfo();
                        if (info) {
                            //logger::error("info->name = [{}]", info->name);
                            if (info->name == sScriptName) {
                                return true;
                            }
                        }
                    }
                }
            }
        }
        else {
            logger::error("vm* couldnt find handle");
        }
        return false;
    }

    bool IsScriptAttachedToRef(RE::TESObjectREFR* ref, RE::BSFixedString sScriptName) {
        if (!gfuncs::IsFormValid(ref)) {
            return false;
        }

        RE::VMHandle handle = GetHandle(ref);
        return IsScriptAttachedToHandle(handle, sScriptName);
    }

    bool IsScriptAttachedToForm(RE::TESForm* akForm, RE::BSFixedString sScriptName) {
        if (!IsFormValid(akForm)) {
            return false;
        }

        RE::VMHandle handle = gfuncs::GetHandle(akForm);
        return IsScriptAttachedToHandle(handle, sScriptName);
    }

    RE::BSScript::Object* GetAttachedScriptObject(RE::VMHandle& handle, RE::BSFixedString& sScriptName) {
        if (handle == NULL) {
            return nullptr;
        }

        if (!sv::vm) {
            return nullptr;
        }

        auto it = sv::vm->attachedScripts.find(handle);
        if (it != sv::vm->attachedScripts.end()) {
            for (int i = 0; i < it->second.size(); i++) {
                auto& attachedScript = it->second[i];
                if (attachedScript) {
                    auto* script = attachedScript.get();
                    if (script) {
                        auto info = script->GetTypeInfo();
                        if (info) {
                            if (info->name == sScriptName) {
                                return script;
                            }
                        }
                    }
                }
            }
        }
        return nullptr;
    }

    RE::TESObjectREFR* GetDialogueTarget(RE::Actor* actor) {
        RE::TESObjectREFR* ref = nullptr;
        if (gfuncs::IsFormValid(actor)) {
            ref = GetRefFromObjectRefHandle(actor->GetActorRuntimeData().dialogueItemTarget);
            if (!gfuncs::IsFormValid(ref)) {
                ref = nullptr;
            }
        }
        return ref;
    }

    RE::Actor* GetPlayerDialogueTarget() {
        if (!sv::player) {
            logger::error("sv::player not found");
            return nullptr;
        }

        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            if (IsFormValid(form)) {
                RE::Actor* actor = skyrim_cast<RE::Actor*>(form);
                if (IsFormValid(actor)) {
                    RE::TESObjectREFR* ref = GetRefFromObjectRefHandle(actor->GetActorRuntimeData().dialogueItemTarget);
                    if (IsFormValid(ref)) {
                        RE::Actor* dialogueActorRef = skyrim_cast<RE::Actor*>(ref);
                        if (IsFormValid(dialogueActorRef)) {
                            if (dialogueActorRef == sv::player) {
                                return actor;
                            }
                        }
                    }
                }
            }
        }
        return nullptr;
    }

    void RefreshItemMenu() {
        if (!sv::ui) {
            logger::error("sv::ui* not found");
            return;
        }

        if (sv::ui->IsItemMenuOpen()) {
            if (!sv::player) {
                logger::error("sv::player not found");
                return;
            }

            RE::SendUIMessage::SendInventoryUpdateMessage(sv::player, nullptr);
        }
    }

    bool IsRefActivatedMenu(RE::BSFixedString menu) {
        if (menu == RE::DialogueMenu::MENU_NAME) { return true; }
        if (menu == RE::LockpickingMenu::MENU_NAME) { return true; }
        if (menu == RE::ContainerMenu::MENU_NAME) { return true; }
        if (menu == RE::BarterMenu::MENU_NAME) { return true; }
        if (menu == RE::CraftingMenu::MENU_NAME) { return true; }
        if (menu == RE::BookMenu::MENU_NAME) { return true; }
        if (menu == RE::GiftMenu::MENU_NAME) { return true; }
        return false;
    }

    bool IsRefActivatedMenuOpen() {
        if (sv::ui) {
            if (sv::ui->IsMenuOpen(RE::DialogueMenu::MENU_NAME)) { return true; }
            if (sv::ui->IsMenuOpen(RE::LockpickingMenu::MENU_NAME)) { return true; }
            if (sv::ui->IsMenuOpen(RE::ContainerMenu::MENU_NAME)) { return true; }
            if (sv::ui->IsMenuOpen(RE::BarterMenu::MENU_NAME)) { return true; }
            if (sv::ui->IsMenuOpen(RE::CraftingMenu::MENU_NAME)) { return true; }
            if (sv::ui->IsMenuOpen(RE::BookMenu::MENU_NAME)) { return true; }
            if (sv::ui->IsMenuOpen(RE::GiftMenu::MENU_NAME)) { return true; }
        }
        return false;
    }

    bool SetAliasQuestObjectFlag(RE::BGSBaseAlias* akAlias, bool set) {
        if (!akAlias) {
            return false;
        }
        if (akAlias->IsQuestObject() != set) {
            if (set) {
                akAlias->flags.set(RE::BGSBaseAlias::FLAGS::kQuestObject);
            }
            else {
                akAlias->flags.reset(RE::BGSBaseAlias::FLAGS::kQuestObject);
            }
        }
        return (akAlias->IsQuestObject() == set);
    }

    bool IsAliasQuestObjectFlagSet(RE::BGSBaseAlias* akAlias) {
        if (!akAlias) {
            return false;
        }
        return akAlias->IsQuestObject();
    }

    bool IsQuestObject(RE::TESObjectREFR* ref) {
        if (!IsFormValid(ref)) {
            return false;
        }

        auto aliasArray = ref->extraList.GetByType<RE::ExtraAliasInstanceArray>();
        if (aliasArray) {
            //logger::trace("alias array for ref[{}] found", ref->GetDisplayFullName());
            for (auto* akAlias : aliasArray->aliases) {
                if (akAlias) {
                    if (akAlias->alias) {
                        if (akAlias->alias->IsQuestObject()) {
                            return true;
                        }
                    }
                }
            }
        }
        //else {
            //logger::trace("alias array for ref[{}] not found", ref->GetDisplayFullName());
        //}
        return false;
    }

    //Thanks to Meridiano, author of Papyrus Ini Manipulator for this.
    bool ContainerContainsRef(RE::TESObjectREFR* containerRef, RE::TESObjectREFR* ref) {
        if (!IsFormValid(containerRef)) {
            return false;
        }

        if (!IsFormValid(ref)) {
            return false;
        }

        auto* baseCont = containerRef->GetBaseObject();
        if (!IsFormValid(baseCont)) {
            return false;
        }

        auto* container = skyrim_cast<RE::TESObjectCONT*>(baseCont);
        if (!IsFormValid(container)) {
            auto* npc = skyrim_cast<RE::TESNPC*>(baseCont);
            if (!IsFormValid(npc)) {
                return false;
            }
        }

        using func_t = std::int32_t(*)(RE::TESObjectREFR*, RE::TESForm*, std::int64_t, std::int32_t);
        REL::Relocation<func_t> func{ REL::VariantID(56062, 56497, 0x9E4710) };
        std::int32_t result = func(containerRef, ref, NULL, NULL);
        return (result == 1);
    }

    //Thanks to Meridiano, author of Papyrus Ini Manipulator for this.
    std::int32_t GetBaseFormCount(RE::TESObjectREFR* containerRef, RE::TESBoundObject* akForm) {
        std::int32_t result = 0;

        if (!IsFormValid(containerRef)) {
            return result;
        }

        if (!IsFormValid(akForm)) {
            return result;
        }

        auto* baseCont = containerRef->GetBaseObject();
        if (!IsFormValid(baseCont)) {
            return result;
        }

        auto* container = skyrim_cast<RE::TESObjectCONT*>(baseCont);
        if (!IsFormValid(container)) {
            auto* npc = skyrim_cast<RE::TESNPC*>(baseCont);
            if (!IsFormValid(npc)) {
                return result;
            }
        }

        using func_t = std::int32_t(*)(RE::TESObjectREFR*, RE::TESForm*, std::int64_t, std::int32_t);
        REL::Relocation<func_t> func{ REL::VariantID(56062, 56497, 0x9E4710) };
        result = func(containerRef, akForm, NULL, NULL);
        return result;
    }

    //Thanks to Meridiano, author of Papyrus Ini Manipulator for this.
    std::int32_t GetItemCount(RE::TESObjectREFR* containerRef, RE::TESForm* item) {
        std::int32_t result = 0;

        if (!IsFormValid(containerRef)) {
            return result;
        }

        if (!IsFormValid(item)) {
            return result;
        } 

        auto* baseCont = containerRef->GetBaseObject();
        if (!IsFormValid(baseCont)) {
            return result;
        } 

        auto* container = skyrim_cast<RE::TESObjectCONT*>(baseCont);
        if (!IsFormValid(container)) {
            auto* npc = skyrim_cast<RE::TESNPC*>(baseCont);
            if (!IsFormValid(npc)) {
                return result;
            }
        }

        auto* boundObj = skyrim_cast<RE::TESBoundObject*>(item);
        if (!IsFormValid(boundObj)) {
            auto* ref = skyrim_cast<RE::TESObjectREFR*>(item);
            if (!IsFormValid(ref)) {
                return result;
            }
        }

        using func_t = std::int32_t(*)(RE::TESObjectREFR*, RE::TESForm*, std::int64_t, std::int32_t);
        REL::Relocation<func_t> func{ REL::VariantID(56062, 56497, 0x9E4710) };
        result = func(containerRef, item, NULL, NULL);
        return result;
    }

    bool formIsBowOrCrossbow(RE::TESForm* akForm) {
        if (!gfuncs::IsFormValid(akForm)) {
            return false;
        }
        RE::TESObjectWEAP* weapon = akForm->As<RE::TESObjectWEAP>();
        if (gfuncs::IsFormValid(weapon)) {
            if (weapon->IsBow() || weapon->IsCrossbow()) {
                return true;
            }
        }
        return false;
    }

    void String_ReplaceAll(std::string& s, std::string searchString, std::string replaceString) {
        if (s == "" || searchString == "") {
            return;
        }

        int sSize = searchString.size();
        std::size_t index = s.find(searchString);

        while (index != std::string::npos)
        {
            s.replace(index, sSize, replaceString);
            index = s.find(searchString);
        }
    }

    void String_ReplaceAll(std::string& s, std::vector<std::string> searchStrings, std::vector<std::string> replaceStrings) {
        int m = searchStrings.size();
        if (replaceStrings.size() < m) {
            m = replaceStrings.size();
        }
        for (int i = 0; i < m; i++) {
            String_ReplaceAll(s, searchStrings[i], replaceStrings[i]);
        }
    }

    bool StringContainsStringInVector(std::vector<std::string>& v, std::string value) {
        gfuncs::ConvertToLowerCase(value);
        for (auto& s : v) {
            if (value.find(s) != std::string::npos) {
                return true;
            }
        }
        return false;
    }

    int GetIndexInVector(std::vector<uint32_t>& v, uint32_t& element) {
        if (element == NULL) {
            return -1;
        }

        if (v.size() == 0) {
            return -1;
        }

        auto it = std::find(v.begin(), v.end(), element);

        if (it != v.end()) {
            return int(std::distance(v.begin(), it));
        }

        return -1;
    }

    int GetIndexInVector(std::vector<RE::TESForm*>& v, RE::TESForm* element) {
        if (!IsFormValid(element)) {
            return -1;
        }

        if (v.size() == 0) {
            return -1;
        }

        auto it = std::find(v.begin(), v.end(), element);

        if (it != v.end()) {
            return int(std::distance(v.begin(), it));
        }

        return -1;
    }

    int GetIndexInVector(std::vector<RE::BGSProjectile*>& v, RE::BGSProjectile* element) {
        if (!IsFormValid(element)) {
            return -1;
        }

        if (v.size() == 0) {
            return -1;
        }

        auto it = std::find(v.begin(), v.end(), element);

        if (it != v.end()) {
            return int(std::distance(v.begin(), it));
        }

        return -1;
    }

    int GetIndexInVector(std::vector<RE::BSSoundHandle*>& v, RE::BSSoundHandle* element) {
        if (!element) {
            return -1;
        }

        if (v.size() == 0) {
            return -1;
        }

        for (int i = 0; i < v.size(); i++) {
            if (v[i] == element) {
                return i;
            }
        }

        return -1;
    }

    int GetIndexInVector(std::vector<RE::BSSoundHandle> v, RE::BSSoundHandle element) {
        if (v.size() == 0) {
            return -1;
        }

        for (int i = 0; i < v.size(); i++) {
            RE::BSSoundHandle handle = v[i];
            if (&handle == &element) {
                return i;
            }
        }

        return -1;
    }

    int GetIndexInVector(std::vector<RE::VMHandle> v, RE::VMHandle& element) {
        if (element == NULL) {
            return -1;
        }

        if (v.size() == 0) {
            return -1;
        }

        auto it = std::find(v.begin(), v.end(), element);

        if (it != v.end()) {
            return int(std::distance(v.begin(), it));
        }

        return -1;
    }

    int GetIndexInVector(std::vector<RE::BSFixedString> v, RE::BSFixedString& element) {
        if (element == NULL) {
            return -1;
        }

        if (v.size() == 0) {
            return -1;
        }

        auto it = std::find(v.begin(), v.end(), element);

        if (it != v.end()) {
            return int(std::distance(v.begin(), it));
        }

        return -1;
    }

    int GetIndexInVector(std::vector<std::string> v, std::string& element) {
        if (element == "") {
            return -1;
        }

        if (v.size() == 0) {
            return -1;
        }

        auto it = std::find(v.begin(), v.end(), element);

        if (it != v.end()) {
            return int(std::distance(v.begin(), it));
        }

        return -1;
    }

    int GetIndexInVector(std::vector<RE::TESObjectREFR*> v, RE::TESObjectREFR* element) {
        if (!IsFormValid(element)) {
            return -1;
        }

        if (v.size() == 0) {
            return -1;
        }

        auto it = std::find(v.begin(), v.end(), element);

        if (it != v.end()) {
            return int(std::distance(v.begin(), it));
        }

        return -1;
    }

    bool IsObjectInBSTArray(RE::BSTArray<RE::ObjectRefHandle>* v, RE::ObjectRefHandle& element) {
        if (v->size() == 0) {
            return false;
        }

        auto it = std::find(v->begin(), v->end(), element);
        return (it != v->end());
    }

    RE::NiAVObject* GetNiAVObjectForRef(RE::TESObjectREFR* ref) {
        if (!IsFormValid(ref)) {
            return nullptr;
        }

        RE::NiAVObject* obj = ref->Get3D();
        if (!obj) {
            obj = ref->Get3D1(false);

            if (!obj) {
                obj = ref->Get3D2();

                if (!obj) {
                    obj = ref->Get3D1(true);
                }
            }
        }

        return obj;
    }

    RE::BGSBaseAlias* GetQuestAliasById(RE::TESQuest* quest, int id) {
        if (gfuncs::IsFormValid(quest)) {
            if (quest->aliases.size() > 0) {
                for (int i = 0; i < quest->aliases.size(); i++) {
                    if (quest->aliases[i]) {
                        if (quest->aliases[i]->aliasID == id) {
                            return quest->aliases[i];
                        }
                    }
                }
            }
        }

        return nullptr;
    }

    int GetWeatherType(RE::TESWeather* weather) {
        if (gfuncs::IsFormValid(weather)) {
            const auto flags = weather->data.flags;
            if (flags.any(RE::TESWeather::WeatherDataFlag::kNone)) {
                return -1;
            }
            if (flags.any(RE::TESWeather::WeatherDataFlag::kPleasant)) {
                return 0;
            }
            if (flags.any(RE::TESWeather::WeatherDataFlag::kCloudy)) {
                return 1;
            }
            if (flags.any(RE::TESWeather::WeatherDataFlag::kRainy)) {
                return 2;
            }
            if (flags.any(RE::TESWeather::WeatherDataFlag::kSnow)) {
                return 3;
            }
        }
        return -2;
    }

    RE::BSFixedString GetBSUIMessageDataTypeString(RE::BSUIMessageData* msgData) {
        if (msgData) {
            if (sv::userEvents) {
                if (msgData->fixedStr == sv::userEvents->forward) {
                    return "forward";
                }
                else if (msgData->fixedStr == sv::userEvents->back) {
                    return "back";
                }
                else if (msgData->fixedStr == sv::userEvents->strafeLeft) {
                    return "strafeLeft";
                }
                else if (msgData->fixedStr == sv::userEvents->strafeRight) {
                    return "strafeRight";
                }
                else if (msgData->fixedStr == sv::userEvents->move) {
                    return "move";
                }
                else if (msgData->fixedStr == sv::userEvents->look) {
                    return "look";
                }
                else if (msgData->fixedStr == sv::userEvents->activate) {
                    return "activate";
                }
                else if (msgData->fixedStr == sv::userEvents->leftAttack) {
                    return "leftAttack";
                }
                else if (msgData->fixedStr == sv::userEvents->rightAttack) {
                    return "rightAttack";
                }
                else if (msgData->fixedStr == sv::userEvents->dualAttack) {
                    return "dualAttack";
                }
                else if (msgData->fixedStr == sv::userEvents->forceRelease) {
                    return "forceRelease";
                }
                else if (msgData->fixedStr == sv::userEvents->pause) {
                    return "pause";
                }
                else if (msgData->fixedStr == sv::userEvents->readyWeapon) {
                    return "readyWeapon";
                }
                else if (msgData->fixedStr == sv::userEvents->togglePOV) {
                    return "togglePOV";
                }
                else if (msgData->fixedStr == sv::userEvents->jump) {
                    return "jump";
                }
                else if (msgData->fixedStr == sv::userEvents->journal) {
                    return "journal";
                }
                else if (msgData->fixedStr == sv::userEvents->sprint) {
                    return "sprint";
                }
                else if (msgData->fixedStr == sv::userEvents->sneak) {
                    return "sneak";
                }
                else if (msgData->fixedStr == sv::userEvents->shout) {
                    return "shout";
                }
                else if (msgData->fixedStr == sv::userEvents->kinectShout) {
                    return "kinectShout";
                }
                else if (msgData->fixedStr == sv::userEvents->grab) {
                    return "grab";
                }
                else if (msgData->fixedStr == sv::userEvents->run) {
                    return "run";
                }
                else if (msgData->fixedStr == sv::userEvents->toggleRun) {
                    return "toggleRun";
                }
                else if (msgData->fixedStr == sv::userEvents->autoMove) {
                    return "autoMove";
                }
                else if (msgData->fixedStr == sv::userEvents->quicksave) {
                    return "quicksave";
                }
                else if (msgData->fixedStr == sv::userEvents->quickload) {
                    return "quickload";
                }
                else if (msgData->fixedStr == sv::userEvents->newSave) {
                    return "newSave";
                }
                else if (msgData->fixedStr == sv::userEvents->inventory) {
                    return "inventory";
                }
                else if (msgData->fixedStr == sv::userEvents->stats) {
                    return "stats";
                }
                else if (msgData->fixedStr == sv::userEvents->map) {
                    return "map";
                }
                else if (msgData->fixedStr == sv::userEvents->screenshot) {
                    return "screenshot";
                }
                else if (msgData->fixedStr == sv::userEvents->multiScreenshot) {
                    return "multiScreenshot";
                }
                else if (msgData->fixedStr == sv::userEvents->console) {
                    return "console";
                }
                else if (msgData->fixedStr == sv::userEvents->cameraPath) {
                    return "cameraPath";
                }
                else if (msgData->fixedStr == sv::userEvents->tweenMenu) {
                    return "tweenMenu";
                }
                else if (msgData->fixedStr == sv::userEvents->takeAll) {
                    return "takeAll";
                }
                else if (msgData->fixedStr == sv::userEvents->accept) {
                    return "accept";
                }
                else if (msgData->fixedStr == sv::userEvents->cancel) {
                    return "cancel";
                }
                else if (msgData->fixedStr == sv::userEvents->up) {
                    return "up";
                }
                else if (msgData->fixedStr == sv::userEvents->down) {
                    return "down";
                }
                else if (msgData->fixedStr == sv::userEvents->left) {
                    return "left";
                }
                else if (msgData->fixedStr == sv::userEvents->right) {
                    return "right";
                }
                else if (msgData->fixedStr == sv::userEvents->pageUp) {
                    return "pageUp";
                }
                else if (msgData->fixedStr == sv::userEvents->pageDown) {
                    return "pageDown";
                }
                else if (msgData->fixedStr == sv::userEvents->pick) {
                    return "pick";
                }
                else if (msgData->fixedStr == sv::userEvents->pickNext) {
                    return "pickNext";
                }
                else if (msgData->fixedStr == sv::userEvents->pickPrevious) {
                    return "pickPrevious";
                }
                else if (msgData->fixedStr == sv::userEvents->cursor) {
                    return "cursor";
                }
                else if (msgData->fixedStr == sv::userEvents->kinect) {
                    return "kinect";
                }
                else if (msgData->fixedStr == sv::userEvents->sprintStart) {
                    return "sprintStart";
                }
                else if (msgData->fixedStr == sv::userEvents->sprintStop) {
                    return "sprintStop";
                }
                else if (msgData->fixedStr == sv::userEvents->sneakStart) {
                    return "sneakStart";
                }
                else if (msgData->fixedStr == sv::userEvents->sneakStop) {
                    return "sneakStop";
                }
                else if (msgData->fixedStr == sv::userEvents->blockStart) {
                    return "blockStart";
                }
                else if (msgData->fixedStr == sv::userEvents->blockStop) {
                    return "blockStop";
                }
                else if (msgData->fixedStr == sv::userEvents->blockBash) {
                    return "blockBash";
                }
                else if (msgData->fixedStr == sv::userEvents->attackStart) {
                    return "attackStart";
                }
                else if (msgData->fixedStr == sv::userEvents->attackPowerStart) {
                    return "attackPowerStart";
                }
                else if (msgData->fixedStr == sv::userEvents->reverseDirection) {
                    return "reverseDirection";
                }
                else if (msgData->fixedStr == sv::userEvents->unequip) {
                    return "unequip";
                }
                else if (msgData->fixedStr == sv::userEvents->zoomIn) {
                    return "zoomIn";
                }
                else if (msgData->fixedStr == sv::userEvents->zoomOut) {
                    return "zoomOut";
                }
                else if (msgData->fixedStr == sv::userEvents->rotateItem) {
                    return "rotateItem";
                }
                else if (msgData->fixedStr == sv::userEvents->leftStick) {
                    return "leftStick";
                }
                else if (msgData->fixedStr == sv::userEvents->prevPage) {
                    return "prevPage";
                }
                else if (msgData->fixedStr == sv::userEvents->nextPage) {
                    return "nextPage";
                }
                else if (msgData->fixedStr == sv::userEvents->prevSubPage) {
                    return "prevSubPage";
                }
                else if (msgData->fixedStr == sv::userEvents->nextSubPage) {
                    return "nextSubPage";
                }
                else if (msgData->fixedStr == sv::userEvents->leftEquip) {
                    return "leftEquip";
                }
                else if (msgData->fixedStr == sv::userEvents->rightEquip) {
                    return "rightEquip";
                }
                else if (msgData->fixedStr == sv::userEvents->toggleFavorite) {
                    return "toggleFavorite";
                }
                else if (msgData->fixedStr == sv::userEvents->favorites) {
                    return "favorites";
                }
                else if (msgData->fixedStr == sv::userEvents->hotkey1) {
                    return "hotkey1";
                }
                else if (msgData->fixedStr == sv::userEvents->hotkey2) {
                    return "hotkey2";
                }
                else if (msgData->fixedStr == sv::userEvents->hotkey3) {
                    return "hotkey3";
                }
                else if (msgData->fixedStr == sv::userEvents->hotkey4) {
                    return "hotkey4";
                }
                else if (msgData->fixedStr == sv::userEvents->hotkey5) {
                    return "hotkey5";
                }
                else if (msgData->fixedStr == sv::userEvents->hotkey6) {
                    return "hotkey6";
                }
                else if (msgData->fixedStr == sv::userEvents->hotkey7) {
                    return "hotkey7";
                }
                else if (msgData->fixedStr == sv::userEvents->hotkey8) {
                    return "hotkey8";
                }
                else if (msgData->fixedStr == sv::userEvents->quickInventory) {
                    return "quickInventory";
                }
                else if (msgData->fixedStr == sv::userEvents->quickMagic) {
                    return "quickMagic";
                }
                else if (msgData->fixedStr == sv::userEvents->quickStats) {
                    return "quickStats";
                }
                else if (msgData->fixedStr == sv::userEvents->quickMap) {
                    return "quickMap";
                }
                else if (msgData->fixedStr == sv::userEvents->toggleCursor) {
                    return "toggleCursor";
                }
                else if (msgData->fixedStr == sv::userEvents->wait) {
                    return "wait";
                }
                else if (msgData->fixedStr == sv::userEvents->click) {
                    return "click";
                }
                else if (msgData->fixedStr == sv::userEvents->mapLookMode) {
                    return "mapLookMode";
                }
                else if (msgData->fixedStr == sv::userEvents->equip) {
                    return "equip";
                }
                else if (msgData->fixedStr == sv::userEvents->dropItem) {
                    return "dropItem";
                }
                else if (msgData->fixedStr == sv::userEvents->rotate) {
                    return "rotate";
                }
                else if (msgData->fixedStr == sv::userEvents->nextFocus) {
                    return "nextFocus";
                }
                else if (msgData->fixedStr == sv::userEvents->prevFocus) {
                    return "prevFocus";
                }
                else if (msgData->fixedStr == sv::userEvents->setActiveQuest) {
                    return "setActiveQuest";
                }
                else if (msgData->fixedStr == sv::userEvents->placePlayerMarker) {
                    return "placePlayerMarker";
                }
                else if (msgData->fixedStr == sv::userEvents->xButton) {
                    return "xButton";
                }
                else if (msgData->fixedStr == sv::userEvents->yButton) {
                    return "yButton";
                }
                else if (msgData->fixedStr == sv::userEvents->chargeItem) {
                    return "chargeItem";
                }
                else if (msgData->fixedStr == sv::userEvents->unk318) {
                    return "unk318";
                }
                else if (msgData->fixedStr == sv::userEvents->playerPosition) {
                    return "playerPosition";
                }
                else if (msgData->fixedStr == sv::userEvents->localMap) {
                    return "localMap";
                }
                else if (msgData->fixedStr == sv::userEvents->localMapMoveMode) {
                    return "localMapMoveMode";
                }
                else if (msgData->fixedStr == sv::userEvents->itemZoom) {
                    return "itemZoom";
                }
            }
        }
        return "Unrecognized";
    }

    void RemoveDuplicates(std::vector<std::string>& vec)
    {
        std::sort(vec.begin(), vec.end());
        vec.erase(std::unique(vec.begin(), vec.end()), vec.end());
    } 

    void RemoveDuplicates(std::vector<RE::FormID>& vec)
    {
        std::sort(vec.begin(), vec.end());
        vec.erase(std::unique(vec.begin(), vec.end()), vec.end());
    }

    void RemoveDuplicates(std::vector<RE::VMHandle>& vec)
    {
        std::sort(vec.begin(), vec.end());
        vec.erase(std::unique(vec.begin(), vec.end()), vec.end());
    }

    void RemoveDuplicates(std::vector<RE::TESForm*>& vec)
    {
        std::sort(vec.begin(), vec.end());
        vec.erase(std::unique(vec.begin(), vec.end()), vec.end());
    }

    void CombineEventHandles(std::vector<RE::VMHandle>& handles, RE::TESForm* akForm, std::map<RE::TESForm*, std::vector<RE::VMHandle>>& formHandles) {
        if (formHandles.size() == 0) {
            return;
        }

        if (!IsFormValid(akForm)) {
            return;
        }

        auto it = formHandles.find(akForm);

        if (it != formHandles.end()) {
            logger::trace("events for form: [{}] ID[{:x}] found", GetFormName(akForm), akForm->GetFormID());
            handles.reserve(handles.size() + it->second.size());
            handles.insert(handles.end(), it->second.begin(), it->second.end());
        }
        else {
            logger::trace("events for form: [{}] ID[{:x}] not found", GetFormName(akForm), akForm->GetFormID());
        }
    }

    void SendEvents(std::vector<RE::VMHandle> handles, RE::BSFixedString& sEvent, RE::BSScript::IFunctionArguments* args) {
        int max = handles.size();

        if (max == 0) {
            return;
        }

        if (!sv::skyrimVm) {
            logger::error("sv::skyrimVm* not found");
            return;
        }

        for (int i = 0; i < max; i++) {
            sv::skyrimVm->SendAndRelayEvent(handles[i], &sEvent, args, nullptr);
        }

        delete args; //args is created using makeFunctionArguments. Delete as it's no longer needed.
    }

    RE::TESForm* FindNullForm() {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();

        for (auto& [id, form] : *allForms) {
            if (form) {
                if (form->GetFormType() == RE::FormType::None) {
                    logger::trace("null TESForm* found");
                    return form;
                }
            }
        }
        return nullptr;
    }

    void Install() {
        if (!srandSet) {
            srandSet = true;
            std::srand((unsigned)std::time(NULL));
        }
    }
}
