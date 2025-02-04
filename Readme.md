
# ofxImGuiAddonWidgets

Reuseable ofxImGui widgets for ofxAddons.

## Description

This repo is a try to centralise various ofxImGui widgets for openframeworks addons.

## Why ?

ImGui is an awesome OpenFrameworks GUI but it can be cumbersome to implement and fine-tune a whole GUI for your ofApps.  
This project aims to minimise the collective writing time of ImGui widgets for OpenFrameworks addons.  

## Setup

- Download this addon like any other ofxAddon.
- Download [ofxImGui/develop](https://github.com/jvcleave/ofxImGui/tree/develop).
- Configure the widgets that you need in each of your projects. (See the list below)  
  _The addon widgets are available on an opt-in basis to prevent having to download all supported ofxAddons._  
- Use the widgets. They are within the `ImGuiEx` namespace.

## Supported ofxAddons

You'll find the corresponding opt-in define next to the name.

- [x] [ofxNDI](https://github.com/leadedge/ofxNDI) : `ofxImGuiAddonWidgets_NDI`

## ofxAddon + ofxImGui integration philosophy

The ideal way to integrate ofxImGui support for an ofxAddon is directly within the addon itself by following [the ofxImGui addon developer instructions](https://github.com/jvcleave/ofxImGui/blob/develop/Developers.md#ofxImGui-integration-within-ofxAddons).  
ofxImGui users can then simply enable the GUI layer by compiling with `ofxAddons_ENABLE_IMGUI` defined.

But not everybody uses ImGui and some addon maintainers may not wish this additional maintenance load.  
When integrating the widget code within the addon itself is not possible, there's 2 options :

- Create your own ofxImGuiSomeAddonWidgets repo.
- Submit a PR to have it included in here.

## Contributions

I am not aiming to provide widgets for all addons, neither willing to maintain them all in this repo.  
But I'm happy to share those that I made for my personal use.

It's simply a centralised repo to bundle ofxImGui widgets for various ofxAddons.  
If you submit a PR for supporting a new addon, please put your code into a separate file with a new opt-in define. (I'll ask you to maintain that part, or it will stay unmaintained).  
Of course, always feel free to submit PRs for improvements and fixes.