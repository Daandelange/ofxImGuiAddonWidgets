/*
	ofxNDI + ImGui Helpers

	Providing easy to use widgets for controlling ofxNDI components with ofxImGui.

	Enable with project-level compilation define `ofxImGuiAddonWidgets_NDI`.
	For more information: https://github.com/jvcleave/ofxImGui/blob/develop/Developers.md#ofxaddons-with-custom-ofximgui-widgets

	Note: You need the develop branch available here. https://github.com/jvcleave/ofxImGui/tree/develop

	IMPORTANT SUPPORT NOTE !
	This GUI layer is provided by contributors and is not actively maintained nor supported by the addon author.
	Please don't ask for support for it in the ofxNDI repo (but you can in the ofxImGui repo).
	Community pull requests are welcome for updating the codebase.

	Copyright (C) 2024 Daan de Lange.

	=========================================================================

    29.09.24 - Initial Implementation with ofxNDIsender gui helpers
    04.01.25 - Move to ofxImGuiAddonWidgets

*/

#pragma once

#ifndef __ofxNDIImGuiEx__
#define __ofxNDIImGuiEx__

#include "ofxImGuiAddonWidgetsConfig.h"

// Only compile this when ofxImGui is enabled
//#ifdef ofxAddons_ENABLE_IMGUI
#ifdef ofxImGuiAddonWidgets_NDI

#include "ofMain.h" // todo: removeme ?!
#include "imgui.h"
#include "ofxNDIsender.h" // Ensure that you have leadedge/ofxNDI if this complains !
#include "ofxNDIreceiver.h"

#ifndef IMGUI_HELPMARKER
#define IMGUI_HELPMARKER(STR) \
    ImGui::SameLine();\
    ImGui::TextDisabled("[?]");\
    if (ImGui::IsItemHovered()) {\
        if (ImGui::BeginTooltip()) {\
            ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);\
            ImGui::TextUnformatted(STR);\
            ImGui::PopTextWrapPos();\
            ImGui::EndTooltip();\
        }\
    }
#endif // IMGUI_HELPMARKER

namespace ImGuiEx {

    //--------------------------------------------------------------
    // NDIsender widgets
    //--------------------------------------------------------------
    // Sender setup
    // Server name, width and height : only if ofxNdiSenderImGuiSettingsFlags_EnableDisable is set, for when the server is enabled.
    // Either all 3 optional args are needed or none is considered (they are bound together, changing one requires resetting the whole server)
    bool ofxNdiSenderSetup(ofxNDIsender& sender, const char* serverNameToCreate=nullptr, unsigned int serverWidth=0u, unsigned int serverHeight=0u);

    // Sender runtime settings
    bool ofxNdiSenderSettings(ofxNDIsender& sender);
    inline bool ofxNdiSenderFrameRate(ofxNDIsender& ndiSender);
    inline bool ofxNdiSenderAsync(ofxNDIsender& ndiSender);
    inline bool ofxNdiSenderReadback(ofxNDIsender& ndiSender);
    inline bool ofxNdiSenderProgressive(ofxNDIsender& ndiSender);
    inline bool ofxNdiSenderClockVideo(ofxNDIsender& ndiSender);
    inline bool ofxNdiSenderFormat(ofxNDIsender& ndiSender);

    // Sender status text
    void ofxNdiSenderStatusText(ofxNDIsender& sender);

    //--------------------------------------------------------------
    // NDIreceiver widgets
    //--------------------------------------------------------------
    // Receiver setup
    bool ofxNdiReceiverSetup(ofxNDIreceiver& receiver, bool showAdvancedOptions=false);

    // Receiver runtime settings
    bool ofxNdiReceiverSettings(ofxNDIreceiver& ndiReceiver);
    bool ofxNdiReceiverServerSelector(ofxNDIreceiver& ndiReceiver, bool showAdvancedOptions=false);
    void ofxNdiReceiverFrameInfo(ofxNDIreceiver& ndiReceiver, bool showAdvancedOptions=false);

    // Receiver status text
    void ofxNdiReceiverStatusText(ofxNDIreceiver& ndiReceiver);
    void ofxNdiReceiverImage(ofTexture& texture, ofxNDIreceiver* ndiReceiver=nullptr);
}

#endif // ofxAddons_ENABLE_IMGUI

#endif // ofxNDIImGuiEx
