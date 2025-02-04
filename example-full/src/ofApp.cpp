/*

	OpenFrameworks ofxNDI + ofxImGui example

	Requires ofxImGui : https://github.com/jvcleave/ofxImGui/tree/develop

	Copyright (C) 2024 Daan de Lange.

	=========================================================================

	04.10.24 - Initial creation


*/
#include "ofApp.h"

// Individual includes :
#ifdef ofxImGuiAddonWidgets_NDI
#include "ofxNDIImGuiEx.h"
#endif

//--------------------------------------------------------------
void ofApp::setup(){

	ofBackground(0);
	ofSetColor(255);
	ofSetWindowTitle("ofxImGuiAddonWidgets/example-full");
	ofSetVerticalSync(false);
	ofSetFrameRate(60);

	// GUI Setup
	gui.setup(nullptr, true, ImGuiConfigFlags_ViewportsEnable);
	bShowGui=true;
	ofDisableArbTex(); // Needed for displaying imgui images !

	// Create an RGBA fbo for collection of data
	appFboWidth  = 1920;
	appFboHeight = 1080;
	appFbo.allocate(appFboWidth, appFboHeight, GL_RGBA);

	// Cube rotation
	rotX = 0;
	rotY = 0;

#ifdef ofxImGuiAddonWidgets_NDI
	// Sender setup
	senderName = "ofxImGuiAddonWidgets/example-full";

	// Sender
	ndiSender.SetReadback(false);
	ndiSender.SetFrameRate(30);
	ndiSender.SetAsync();
	ndiSender.CreateSender(senderName.c_str(), appFboWidth, appFboHeight);

	// Receiver setup
	//ndiReceiver.CreateReceiver();
	//ndiReceiver.CreateFinder();
	ndiReceiver.SetSenderName(senderName);

	std::cout << ndiReceiver.GetNDIversion() << " (https://www.ndi.tv/)" << std::endl;

	// ofTexture
	receiverTexture.allocate(ofGetWidth(), ofGetHeight(), GL_RGBA, false); // Fixme : ofxNDI will override the ARB argument when the receiver size changes.
#endif // ofxNDI
}


//--------------------------------------------------------------
void ofApp::update() {

}

//--------------------------------------------------------------
void ofApp::draw() {

#ifdef ofxImGuiAddonWidgets_NDI
	// Receive ofTexture
	ndiReceiver.ReceiveImage(receiverTexture);
#endif

	// Render to FBO
	drawAppGraphics();

#ifdef ofxImGuiAddonWidgets_NDI
	// Send FBO
	ndiSender.SendImage(appFbo);
#endif // ofxNDI

	// Draw the GUI
	if(bShowGui){
		gui.begin();

		// Menu bar
		if(ImGui::BeginMainMenuBar()){

			// Openframeworks runtime settings
			if(ImGui::BeginMenu("OpenFrameworks")){
				ImGui::TextDisabled("Here you can change some OF settings.");
				ImGui::TextDisabled("They affect ofxNDI's performance.");
				static int frameRate = ofGetTargetFrameRate();
				ImGui::Text("FPS    : %03.0f / %03i", ofGetFrameRate(), frameRate);
				ImGui::InputInt("Target FPS", &frameRate, 1, 10, ImGuiInputTextFlags_None );
				if(ImGui::IsItemDeactivatedAfterEdit()){
					ofSetFrameRate(frameRate);
					frameRate = ofGetTargetFrameRate();
				}
				ImGui::Text("V-Sync : ");
				ImGui::SameLine();
				if(ImGui::Button("Disable")){
					ofSetVerticalSync(false);
				}
				ImGui::SameLine();
				if(ImGui::Button("Enable")){
					ofSetVerticalSync(true);
				}
				ImGui::Text("Uptime     : %.1f seconds", ofGetElapsedTimef() );
				ImGui::Text("Resolution : %i x %i (ratio %.2f)", ofGetWindowWidth(), ofGetWindowHeight(), (((float)ofGetWindowWidth())/ofGetWindowHeight()) );

				ImGui::SeparatorText("GUI");
				ImGui::Checkbox("Show GUI", &bShowGui);
				ImGui::SameLine();
				ImGui::TextDisabled(" Cmd/Ctrl + G");

				ImGui::SeparatorText("ofApp");
				ImGui::DragFloat("rot.X speed", &rotXSpeed, 0.005, 0.f, 1.f);
				ImGui::DragFloat("rot.Y speed", &rotYSpeed, 0.005, 0.f, 1.f);
				ImGui::DragFloat("hue speed", &hueSpeed, 0.005, 0.f, 1.f);

				ImGui::EndMenu();
			}

			// ofxNDI widgets
#ifdef ofxImGuiAddonWidgets_NDI
			if(ImGui::BeginMenu("ofxNDI")){
				// Show some status messages in the menu
				if(ImGui::BeginMenu("Sender")){
					ImGui::Checkbox("Show sender controls", &bShowSenderCtrls);
					ImGui::SeparatorText("Sender");
					ImGuiEx::ofxNdiSenderStatusText(ndiSender);
					ImGui::EndMenu();
				}
				if(ImGui::BeginMenu("Receiver")){
					ImGui::Checkbox("Show receiver controls", &bShowSenderCtrls);
					ImGui::SeparatorText("Receiver");
					ImGuiEx::ofxNdiReceiverStatusText(ndiReceiver);

					// A preview of the receiver image
					if(ImGui::BeginMenu("Receiver Image")){
						ImGuiEx::ofxNdiReceiverImage(receiverTexture, &ndiReceiver);
						ImGui::EndMenu();
					}
					ImGui::EndMenu();
				}
				if(ImGui::BeginMenu("Help")){
					ImGui::TextWrapped("This is an example layout. You can re-arrange the items to your wishes using the ImGui API.");
					ImGui::SeparatorText("About");
					ImGui::TextWrapped("In this example, ofxNDI is used to create a recursive video generator, feeding back the image of the receiver into the sender.");
					ImGui::EndMenu();
				}
				ImGui::EndMenu();
			}
#endif // ofxNDI
		}
		ImGui::EndMainMenuBar();

		// ofxNDI windows
#ifdef ofxImGuiAddonWidgets_NDI
		// Receiver window with settings
		if(ImGui::Begin("ofxNDI Receiver", &bShowReceiverCtrls)){
			ImGui::SeparatorText("Receiver Settings");
			ImGuiEx::ofxNdiReceiverSetup(ndiReceiver, true);

			ImGui::SeparatorText("Server Selection");
			ImGuiEx::ofxNdiReceiverServerSelector(ndiReceiver, true);

			ImGui::SeparatorText("Frame Information");
			ImGuiEx::ofxNdiReceiverFrameInfo(ndiReceiver, true);
		}
		ImGui::End();

		// Sender window with settings
		if(ImGui::Begin("ofxNDI Sender", &bShowSenderCtrls)){
			ImGui::SeparatorText("Sender Setup");
			ImGuiEx::ofxNdiSenderSetup(ndiSender, senderName.c_str(), appFboWidth, appFboHeight);

			ImGui::Spacing();
			ImGui::SeparatorText("Sender Settings");
			ImGuiEx::ofxNdiSenderSettings(ndiSender);
		}
		ImGui::End();
#endif // ofxNDI

		gui.end();
	} // Endif bShowGui
}

//--------------------------------------------------------------
void ofApp::drawAppGraphics() {

	// Draw graphics into an fbo
	appFbo.begin();
	ofClear(0, 0, 0, 255);
	ofPushStyle();
	float elapsed = glm::mod(ofGetElapsedTimef()*hueSpeed, 1.f);

	// Draw ofxNDI receiver behind
#ifdef ofxImGuiAddonWidgets_NDI
	ofFill();
	ofSetColor(ofColor::fromHsb(elapsed*255, 20, 255, 255));
	static const int offset = 5;
	receiverTexture.draw(-offset, -offset, appFbo.getWidth()+offset*2, appFbo.getHeight()+offset*2);
	ofEnableDepthTest();
	ofPushMatrix();
#endif // ofxNDI

	ofTranslate((float)appFboWidth / 2.0, (float)appFboHeight / 2.0, 0);
	ofRotateYDeg(rotX);
	ofRotateXDeg(rotY);

	// Draw box with texture
	ofSetColor(255,255,255,255);
#ifdef ofxImGuiAddonWidgets_NDI
	receiverTexture.bind();
#endif
	ofDrawBox(0.7 * (float)appFboHeight);
#ifdef ofxImGuiAddonWidgets_NDI
	receiverTexture.unbind();
#endif

	// Box Lines
	ofNoFill();
	ofSetLineWidth(20);
	ofDrawBox(0.8 * (float)appFboHeight);
	ofPopMatrix();

	// Some line behind the cube
	ofSetLineWidth(10);
	ofDrawLine(appFboWidth*elapsed,0, appFboWidth*elapsed, appFboHeight);
	ofDrawLine(appFboWidth-appFboWidth*elapsed,0, appFboWidth-appFboWidth*elapsed, appFboHeight);

	ofDisableDepthTest();
	ofPopStyle();
	appFbo.end();

	// Rotate the cube
	rotX += rotXSpeed;
	rotY += rotYSpeed;

	// Draw the fbo result fitted to the display window
	appFbo.draw(0, 0, ofGetWidth(), ofGetHeight());
}

//--------------------------------------------------------------
void ofApp::exit() {

#ifdef ofxImGuiAddonWidgets_NDI
	// Release NDI objects
	ndiReceiver.ReleaseReceiver();
	ndiSender.ReleaseSender();
#endif // ofxNDI

}

//--------------------------------------------------------------
void ofApp::keyPressed( ofKeyEventArgs & key ){
	// Toggle GUI
	if((key.key=='G'||key.key=='g') && key.hasModifier(OF_KEY_COMMAND | OF_KEY_CONTROL)){
		bShowGui = !bShowGui;
	}
}
