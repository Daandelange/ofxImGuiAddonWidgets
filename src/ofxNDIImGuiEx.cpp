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
	04.10.24 - Add Receiver helpers
    04.01.25 - Move to ofxImGuiAddonWidgets

*/

// #ifdef ofxAddons_ENABLE_IMGUI

#include "ofxImGuiAddonWidgetsConfig.h"

#ifdef ofxImGuiAddonWidgets_NDI

#include "ofxNDIImGuiEx.h"
#include <string>
#include <map>
#include "ofAppRunner.h"
#include "ImHelpers.h" // for AddImage
#include "imgui_internal.h" // for ImMin

namespace ImGuiEx {

//--------------------------------------------------------------
// Private Helpers
// Defined in CPP so nobody relies on it, for internal use only.
// Mainly used to cache the server name, width, height which are unset when turned off.
//--------------------------------------------------------------
struct ofxNDIsenderCreationSettings {
    char name[256];
    unsigned int width = 0;
    unsigned int height = 0;
    ofxNDIsenderCreationSettings() = delete;
    ofxNDIsenderCreationSettings(const char* serverNameToCreate, unsigned int serverWidth, unsigned int serverHeight) :
        name(""),
        width(serverWidth),
        height(serverHeight)
    {
        if(serverNameToCreate!=nullptr)
            std::strncpy(&name[0], &serverNameToCreate[0], IM_ARRAYSIZE(name)-1);
    }
    ofxNDIsenderCreationSettings(ofxNDIsender& sender) :
        name(""),
        width(0),
        height(0)
    {
        matchSender(sender);
    }
    void matchSender(ofxNDIsender& sender){
        if(sender.SenderCreated()){
            std::strncpy(&name[0], sender.GetSenderName().c_str(), IM_ARRAYSIZE(name)-1);
            width = sender.GetWidth();
            height = sender.GetHeight();
        }
    }
};

std::map<ofxNDIsender*, ofxNDIsenderCreationSettings> ndiSenderIdentities;
std::pair<ofxNDIsender*, ofxNDIsenderCreationSettings> ndiSenderEditing = {nullptr, {"", 0u, 0u}};

inline ofxNDIsenderCreationSettings& getIdentity(ofxNDIsender& sender, const char* serverNameToCreate, unsigned int serverWidth, unsigned int serverHeight){
    auto it = ndiSenderIdentities.find(&sender);
    if(it==ndiSenderIdentities.end()){
        // Init first entry with args
        if(sender.SenderCreated()){
            // Use sender for actual settings
            it = ndiSenderIdentities.emplace(&sender, sender).first;
        }
        else {
            // Init from desired settings
            if(serverNameToCreate!=nullptr && strlen(serverNameToCreate)!=0 && serverWidth!=0 && serverHeight!=0){
                it = ndiSenderIdentities.emplace(std::make_pair<ofxNDIsender*, ofxNDIsenderCreationSettings>(&sender, {serverNameToCreate, serverWidth, serverHeight})).first;
            }
            else {
                it = ndiSenderIdentities.emplace(std::make_pair<ofxNDIsender*, ofxNDIsenderCreationSettings>(&sender, {"ofxNDI Sender", (unsigned int)ofGetWidth(), (unsigned int)ofGetHeight()})).first;
            }
        }
    }
    // If not editing and hints are different, sync them
    const bool bIsEditing = &sender == ndiSenderEditing.first;
    if(!bIsEditing){
        if(serverNameToCreate!=nullptr && std::strcmp(serverNameToCreate, it->second.name) != 0 ){
            std::strncpy(&it->second.name[0], serverNameToCreate, IM_ARRAYSIZE(it->second.name)-1);
        }
    }
    return it->second;
}

// Maths from ofxNDIsend::SetFormat
struct ofxNDIVideoFormatGui {
    NDIlib_FourCC_video_type_e code;
    const char name[5] = "";
    ofxNDIVideoFormatGui()=delete;
    ofxNDIVideoFormatGui(NDIlib_FourCC_video_type_e format) :
        code(format),
        name { static_cast<char>(format & 0xFF), static_cast<char>((format >> 8) & 0xFF), static_cast<char>((format >> 16) & 0xFF), static_cast<char>((format >> 24) & 0xFF), '\0' }
    {}
};
// Hardcoded list of supported ofxNDI formats
const static ofxNDIVideoFormatGui supportedFormats[] = {
  {NDIlib_FourCC_video_type_BGRA},
  {NDIlib_FourCC_video_type_BGRX},
  {NDIlib_FourCC_video_type_RGBA},
  {NDIlib_FourCC_video_type_RGBX},
  {NDIlib_FourCC_video_type_UYVY}
};


const char* getFrameType(const NDIlib_frame_type_e& type){
    const char* frameType = "Unknown";

    switch(type){
        case NDIlib_frame_type_none:
            frameType = "None";
            break;
        case NDIlib_frame_type_video:
            frameType = "Video";
            break;
        case NDIlib_frame_type_audio:
            frameType = "Audio";
            break;
        case NDIlib_frame_type_metadata:
            frameType = "MetaData";
            break;
        case NDIlib_frame_type_error:
            frameType = "Error";
            break;
        case NDIlib_frame_type_status_change:
            frameType = "StatusChange";
            break;
        default:
            break;
    }
    return frameType;
}
// from ofGetTimeStampString, but with extra time arg instead of now.
std::string getTimestampString(uint64_t timestamp, const string& timestampFormat){
	std::stringstream str;
	double timeStampSeconds = ((double)timestamp)/10000000;
	auto now = std::chrono::system_clock::from_time_t(timeStampSeconds);
	auto t = std::chrono::system_clock::to_time_t(now);
	std::chrono::duration<double> s = now - std::chrono::system_clock::from_time_t(t);
	int ms = s.count() * 1000;

	auto tm = *std::localtime(&t);
	constexpr int bufsize = 256;
	char buf[bufsize];

	// Beware! an invalid timestamp string crashes windows apps.
	// so we have to filter out %i (which is not supported by vs)
	// earlier.
	auto tmpTimestampFormat = timestampFormat;
	ofStringReplace(tmpTimestampFormat, "%i", ofToString(ms, 3, '0'));

	if (strftime(buf,bufsize, tmpTimestampFormat.c_str(),&tm) != 0){
		str << buf;
	}
	str << " " << std::setfill('0') << std::setw(3) << ((int)(glm::mod(timeStampSeconds, 1.0)*1000)) << "ms";
	auto ret = str.str();
	return ret;
}

//--------------------------------------------------------------
// NDISender widgets
//--------------------------------------------------------------

// Draws sender setup settings
bool ofxNdiSenderSetup(ofxNDIsender& ndiSender, const char* serverNameToCreate, unsigned int serverWidth, unsigned int serverHeight){
    bool didChange = false;
    const bool bIsEditing = &ndiSender == ndiSenderEditing.first;
    bool bNdiEnabled = ndiSender.SenderCreated();
    ofxNDIsenderCreationSettings& id = getIdentity(ndiSender, serverNameToCreate, serverWidth, serverHeight);

    // Enabler button (Can always be disabled, enabled only with name)
    const bool bCanBeDisabled = bNdiEnabled || (std::strlen(id.name)!=0 && id.width!=0 && id.height!=0);
    if(!bCanBeDisabled) ImGui::BeginDisabled();
    if(ImGui::Checkbox("Enable NDI output", &bNdiEnabled)){
        if(bNdiEnabled && !ndiSender.SenderCreated()){
            if(!ndiSender.CreateSender(id.name, id.width, id.height)){
                ofLogWarning("ImGuiEx::ofxNdiSenderSetup()") << "Couldn't create NDI server !";
            }
        }
        else if(!bNdiEnabled && ndiSender.SenderCreated()){
            // Store last used settings
            id.matchSender(ndiSender);
            // Release
            ndiSender.ReleaseSender();
        }
        didChange = true;
    }
    if(!bCanBeDisabled) ImGui::EndDisabled();

    // Name
    if(!bIsEditing) ImGui::BeginDisabled();
    ImGui::InputText("Server name", bIsEditing?&ndiSenderEditing.second.name[0]:&id.name[0], IM_ARRAYSIZE(id.name), ImGuiInputTextFlags_None);
    if(ImGui::IsItemDeactivatedAfterEdit()){
        didChange = true;
    }

    // Width
    unsigned int whStep = 1u, whStepFast=100;
    ImGui::InputScalar("Width", ImGuiDataType_U32, bIsEditing?&ndiSenderEditing.second.width:&id.width, &whStep, &whStepFast, "%u", ImGuiInputTextFlags_None);
    if(ImGui::IsItemDeactivatedAfterEdit()){
        didChange = true;
    }

    // Height
    ImGui::InputScalar("Height", ImGuiDataType_U32, bIsEditing?&ndiSenderEditing.second.height:&id.height, &whStep, &whStepFast, "%u", ImGuiInputTextFlags_None);
    if(ImGui::IsItemDeactivatedAfterEdit()){
        didChange = true;
    }
    if(!bIsEditing) ImGui::EndDisabled();

    // Apply + Edit btn
    if(!bIsEditing){
        if(ImGui::Button("Change Server Setup")){
            ndiSenderEditing.first = &ndiSender;
            ndiSenderEditing.second.matchSender(ndiSender);
        }
    }
    else {
        if(ImGui::Button("Cancel")){
            ndiSenderEditing.first = nullptr;
        }
        ImGui::SameLine();
        if(ImGui::Button("Apply and (re)Start") || didChange){
            didChange = true;
        }
    }

    // Apply
    if(bIsEditing && didChange){
        std::strncpy(id.name, ndiSenderEditing.second.name, IM_ARRAYSIZE(id.name)-1);
        id.width = ndiSenderEditing.second.width;
        id.height = ndiSenderEditing.second.height;
        if(ndiSender.SenderCreated()) ndiSender.ReleaseSender();
        if(!ndiSender.CreateSender(id.name, id.width, id.height)){
            ofLogWarning("ImGuiEx::ofxNdiSenderSetup()") << "Couldn't create NDI server !";
        }
        ndiSenderEditing.first = nullptr;
    }

    return didChange;
}

// Draws sender runtime settings
bool ofxNdiSenderSettings(ofxNDIsender& ndiSender){
    bool didChange = false;
    didChange |= ofxNdiSenderFrameRate(ndiSender);
    didChange |= ofxNdiSenderFormat(ndiSender);
    didChange |= ofxNdiSenderAsync(ndiSender);
    didChange |= ofxNdiSenderReadback(ndiSender);
    didChange |= ofxNdiSenderProgressive(ndiSender);
    didChange |= ofxNdiSenderClockVideo(ndiSender);
    return didChange;
}

inline bool ofxNdiSenderFrameRate(ofxNDIsender& ndiSender){
    double ndiFpsCap = ndiSender.GetFrameRate();
    ImGui::InputDouble("FPS Cap", &ndiFpsCap, 1.f, 1.f, "%.0f", ImGuiInputTextFlags_None);
    if(ImGui::IsItemDeactivatedAfterEdit()){
        ndiSender.SetFrameRate(ndiFpsCap);
        return true;
    }
    return false;
}

inline bool ofxNdiSenderAsync(ofxNDIsender& ndiSender){
    bool didChange = false;
    bool bNdiAsync = ndiSender.GetAsync();
    if(ImGui::Checkbox("Asynchronous", &bNdiAsync)){
        ndiSender.SetAsync(bNdiAsync);
        didChange = true;
    }
    IMGUI_HELPMARKER("Disables clocked video. If enabled, best without vsync. If disabled, the render rate is clocked to the sending framerate.");
    return didChange;
}

inline bool ofxNdiSenderReadback(ofxNDIsender& ndiSender){
    bool didChange = false;
    bool bNdiRb = ndiSender.GetReadback();
    if(ImGui::Checkbox("ReadBack", &bNdiRb)){
        ndiSender.SetReadback(bNdiRb);
        didChange = true;
    }
    IMGUI_HELPMARKER("Alternative way of transfering pixel data from the GPU, performance depends on your CPU and GPU.");
    return didChange;
}

inline bool ofxNdiSenderProgressive(ofxNDIsender& ndiSender){
    bool bNdiProgressive = ndiSender.GetProgressive();
    if(ImGui::Checkbox("Progressive", &bNdiProgressive)){
        ndiSender.SetProgressive(bNdiProgressive);
        return true;
    }
    return false;
}

inline bool ofxNdiSenderClockVideo(ofxNDIsender& ndiSender){
    bool bNdiClockedVideo = ndiSender.GetClockVideo();
    if(ImGui::Checkbox("Clocked Video", &bNdiClockedVideo)){
        ndiSender.SetClockVideo(bNdiClockedVideo);
        return true;
    }
    return false;
}

inline bool ofxNdiSenderFormat(ofxNDIsender& ndiSender){
    bool didChange = false;
    auto currentCC = ofxNDIVideoFormatGui(ndiSender.GetFormat());
    if(ImGui::BeginCombo("Color Format", currentCC.name)){
        bool knownSelected = false;
        for(auto fcc : supportedFormats){
            bool selected = (fcc.code == currentCC.code);
            if(ImGui::Selectable(fcc.name, selected)){
                ndiSender.SetFormat(fcc.code);
                didChange = true;
            }
            knownSelected |= selected;
        }
        if(!knownSelected){
            ImGui::Selectable("Other / Unknown", true);
        }

        ImGui::EndCombo();
    }
    IMGUI_HELPMARKER("Not recommended to change!");
    return didChange;
}

// Draws the whole status as text
// todo: make arg const when ofxNDIsender getters are marked const.
void ofxNdiSenderStatusText(ofxNDIsender& ndiSender){
    ImGui::Text("Initialised : %s", ndiSender.SenderCreated()?"Yes":"No");
    ImGui::Text("NDI name    : %s", ndiSender.GetNDIname().c_str());
    ImGui::Text("Server name : %s", ndiSender.GetSenderName().c_str());
    static float ndiRatio; ndiSender.GetAspectRatio(ndiRatio);
    ImGui::Text("Resolution  : %u x %u (%.3f)", ndiSender.GetWidth(), ndiSender.GetHeight(), ndiRatio);

    ImGui::Text("Target FPS  : %.3f", ndiSender.GetFrameRate());
    ImGui::Text("Real FPS    : %.3f", ndiSender.GetFps());

    auto currentCC = ofxNDIVideoFormatGui(ndiSender.GetFormat());
    ImGui::Text("PixelFormat : %s\n", currentCC.name);

    const char* ndiFormatStr = "Other / Unknown";
    switch(ndiSender.GetFormat()){
        case NDIlib_FourCC_video_type_UYVY :
            ndiFormatStr = "YUV";
            break;
        case NDIlib_FourCC_video_type_RGBA :
            ndiFormatStr = "RGBA";
            break;
        default:
            break;
    }
    ImGui::Text("ofxNDI mode : %s", ndiFormatStr);
    IMGUI_HELPMARKER("The YUV codec decodes data on the GPU.\nPerformance varies depending on your GPU.");

    ImGui::Text("Asynchronous: %s", ndiSender.GetAsync()?"Yes":"No");
    ImGui::Text("Readback    : %s", ndiSender.GetReadback()?"Yes":"No");
    ImGui::Text("Progressive : %s", ndiSender.GetProgressive()?"Yes":"No");
    ImGui::Text("ClockVideo  : %s", ndiSender.GetClockVideo()?"Yes":"No");

    ImGui::Text("NDI version : %s", ndiSender.GetNDIversion().c_str());
}

//--------------------------------------------------------------
// NDIreceiver widgets
//--------------------------------------------------------------


bool ofxNdiReceiverSetup(ofxNDIreceiver& ndiReceiver, bool showAdvancedOptions){
    bool didChange = false;
    bool breceiverEnabled = ndiReceiver.ReceiverCreated();
    bool bAsyncUpload = ndiReceiver.GetUpload();

    // Enable / Disable
    if(ImGui::Checkbox("Receiver enabled", &breceiverEnabled)){
        if(!breceiverEnabled)
            ndiReceiver.ReleaseReceiver();
        else
            ndiReceiver.CreateReceiver();
        didChange |= true;
    }

    // Finder controls
    if(showAdvancedOptions){
        // No getter : buttons to enable/disable
        ImGui::Text("Finder: ");
        ImGui::SameLine();
        if(ImGui::Button("Create")){
            ndiReceiver.CreateFinder();
            didChange |= true;
        }
        ImGui::SameLine();
        if(ImGui::Button("Release")){
            ndiReceiver.ReleaseFinder();
            didChange |= true;
        }
    }

    // Bandwidth has no getter, so use buttons here
    ImGui::Text("Bandwidth: ");
    ImGui::SameLine();
    if(ImGui::Button("Low")){
        ndiReceiver.SetLowBandwidth(true);
        ndiReceiver.CreateReceiver(); // Fixme: Needs a restart !
        didChange |= true;
    }
    ImGui::SameLine();
    if(ImGui::Button("High")){
        ndiReceiver.SetLowBandwidth(false);
        ndiReceiver.CreateReceiver();
        didChange |= true;
    }

    // Audio has no getter, so use buttons here
    ImGui::Text("Audio: ");
    ImGui::SameLine();
    if(ImGui::Button("Enable")){
        ndiReceiver.SetAudio(true);
        didChange |= true;
    }
    ImGui::SameLine();
    if(ImGui::Button("Disable")){
        ndiReceiver.SetAudio(false);
        didChange |= true;
    }

    // Upload method
    if(ImGui::Checkbox("Async upload", &bAsyncUpload)){
        ndiReceiver.SetUpload(bAsyncUpload);
        didChange |= true;
    }

    if(showAdvancedOptions) ImGui::TextDisabled("(Changes above may need a restart)");

    return didChange;
}

bool ofxNdiReceiverServerSelector(ofxNDIreceiver& ndiReceiver, bool showAdvancedOptions){
    bool bConnected = ndiReceiver.ReceiverConnected();
    int nsenders = ndiReceiver.GetSenderCount();
    bool ret = false;

    // Connection status
    ImGui::BeginDisabled();
    ImGui::Checkbox("Receiver Connected", &bConnected);
    ImGui::EndDisabled();

    // Server listening to
    if(showAdvancedOptions){
        if(!bConnected){
            ImGui::TextDisabled("Connecting...");
        }
        else {
            ImGui::Text("%s", ndiReceiver.GetSenderName().c_str());
            if(ImGui::IsItemHovered(ImGuiHoveredFlags_Stationary)){
                if(ImGui::BeginTooltip()){
                    ImGui::Text("[%i/%i] %s", ndiReceiver.GetSenderIndex(), nsenders, ndiReceiver.GetSenderName().c_str());
                }
                ImGui::EndTooltip();
            }
        }
    }
    ImGui::Text("Server FPS : %3.0f", ndiReceiver.GetSenderFps());

    // Servers list
    std::string curSenderName = ndiReceiver.GetSenderName();
    if(ImGui::BeginCombo("Server Selection", curSenderName.c_str())){
        auto servers = ndiReceiver.GetSenderList();
        for(auto serverName : servers){
            if(ImGui::Selectable(serverName.c_str(), curSenderName == serverName)){
                ndiReceiver.SetSenderName(serverName);
                ret = true;
            }
        }

        if(servers.size()==0){
            ImGui::TextDisabled("[ No Servers Available ]");
        }
        ImGui::EndCombo();
    }
    if(showAdvancedOptions && ImGui::SmallButton("Refresh Servers")){
        ndiReceiver.RefreshSenders(100);
    }

    return ret;
}

void ofxNdiReceiverFrameInfo(ofxNDIreceiver& ndiReceiver, bool showAdvancedOptions){
    unsigned int width = ndiReceiver.GetSenderWidth();
    unsigned int height = ndiReceiver.GetSenderHeight();
    int fps = ndiReceiver.GetFps();

    if(!showAdvancedOptions){
        ImGui::Text("Frame info : %u x %u @ %3ifps", width, height, fps);
    }
    else {
        ImGui::SetNextItemOpen(true, ImGuiCond_FirstUseEver);
        if(ImGui::TreeNode("Frame information", "Frame Info (%u x %u @ %3ifps)", width, height, fps)){
            uint64_t timecode = ndiReceiver.GetVideoTimecode();
            uint64_t timestamp = ndiReceiver.GetVideoTimestamp();
            ofxNDIVideoFormatGui format = ofxNDIVideoFormatGui(ndiReceiver.NDIreceiver.GetVideoType());

            ImGui::BulletText("Resolution    : %u x %u", width, height);
            ImGui::BulletText("Real FPS      : %i", fps);
            ImGui::BulletText("Pixel format  : %s\n", format.name);
            ImGui::BulletText("Frame type    : %s", getFrameType(ndiReceiver.GetFrameType()));
            ImGui::BulletText("Frame meta    : %s", ndiReceiver.GetMetadataString().c_str());
            ImGui::BulletText("Frame is meta : %s", ndiReceiver.IsMetadata()?"Yes":"No");
            ImGui::BulletText("Time code     : %llu", timecode);
            //ImGui::BulletText("Timestamp     : %llu", timestamp); // To display the raw data
            ImGui::BulletText("Timestamp     : %s", getTimestampString(timestamp, "%F %T").c_str());

            ImGui::BulletText("Audio Frame       : %s", ndiReceiver.IsAudioFrame()?"Yes":"No");
            ImGui::BulletText("Audio channels    : %i", ndiReceiver.GetAudioChannels());
            ImGui::BulletText("Audio samples     : %i", ndiReceiver.GetAudioSamples());
            ImGui::BulletText("Audio sample rate : %i", ndiReceiver.GetAudioSampleRate());

            ImGui::TreePop();
        }
    }
}

void ofxNdiReceiverStatusText(ofxNDIreceiver& ndiReceiver){
    const bool connected = ndiReceiver.ReceiverConnected();
    int connectedTo = ndiReceiver.GetSenderIndex();
    uint64_t timecode = ndiReceiver.GetVideoTimecode();
    uint64_t timestamp = ndiReceiver.GetVideoTimestamp();
    int64_t ftime = ndiReceiver.GetVideoTimestamp();
    unsigned int width = ndiReceiver.GetSenderWidth();
    unsigned int height = ndiReceiver.GetSenderHeight();
    float ratio = height>0?(((float)width)/height):0.f;

    // Status
    ImGui::Text("Receiver.init: %s", ndiReceiver.ReceiverCreated()?"Yes":"No");
    //ImGui::Text("Finder.init  : %s", ndiReceiver.FinderCreated()?"Yes":"No"); // unavailable
    ImGui::Text("Async upload : %s", ndiReceiver.GetUpload()?"Yes":"No");
    // todo: bandwidth when getter is available :(

    // Connection info
    ImGui::Text("Server connected : %s", connected?"Yes":"No");
    if(ImGui::TreeNode("Connection information")){
        ImGui::Text("Server name : %s", ndiReceiver.GetSenderName().c_str());
        ImGui::Text("Server FPS  : %.3f", ndiReceiver.GetSenderFps());
        ImGui::TreePop();
    }

    // Frame information
    ImGuiEx::ofxNdiReceiverFrameInfo(ndiReceiver, true);

    // List senders
    int nsenders = ndiReceiver.GetSenderCount();
    if(ImGui::TreeNode("Available Servers", "Available Servers : %i", nsenders)){
        auto servers = ndiReceiver.GetSenderList();
        for(auto serverName : servers){
            ImGui::BulletText("%s", serverName.c_str());
        }

        if(servers.size()==0){
            ImGui::TextDisabled("[ No Servers Available ]");
        }
        ImGui::TreePop();
    }


    ImGui::Text("NDI version : %s", ndiReceiver.GetNDIversion().c_str());
}

void ofxNdiReceiverImage(ofTexture& texture, ofxNDIreceiver* sender){
    // Display info
    if(sender!=nullptr)
        ImGui::Text("Received image (%u x %u @ %ufps)", sender->GetSenderWidth() , sender->GetSenderHeight(), sender->GetFps());
    else
        ImGui::Text("Received image (%.0f x %.0f)", texture.getWidth(), texture.getHeight());

    // Calculate proportional size
    ImVec2 availableSpace = ImMax(ImGui::GetContentRegionAvail(), ImVec2(200,200));
    float ratio = texture.getHeight() / texture.getWidth();

    // Display the image
    ofxImGui::AddImage(texture, glm::vec2(availableSpace.x, availableSpace.x*ratio));
}

} // namespace ImGuiEx

#endif // ofxAddons_ENABLE_IMGUI
