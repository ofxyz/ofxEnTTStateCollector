meta:
	ADDON_NAME = ofxEnTTStateCollector
	ADDON_DESCRIPTION = ECS field-level state presets, morphing, and simple timeline for openFrameworks
	ADDON_AUTHOR = @GitBruno
	ADDON_TAGS = "ecs" "entt" "preset" "state" "animation" "imgui"
	ADDON_URL = https://github.com/ofxyz/ofxEnTTStateCollector

common:
	# ofxEnTTInspector provides ReflectedProperty, PropertyValue, and PinDataType.
	# ofxTanim is an optional dependency — only needed if you use TimelinePanel.
	ADDON_DEPENDENCIES = ofxEnTTKit ofxImGui ofxEnTTInspector

	ADDON_INCLUDES += $(OF_ROOT)/addons
