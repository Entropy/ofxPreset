#include "Gui.h"

namespace ofxPreset
{
	//--------------------------------------------------------------
	Gui::Settings::Settings()
		: windowPos(kGuiMargin, kGuiMargin)
		, windowSize(ofVec2f::zero())
		, windowBlock(false)
		, treeLevel(0)
		, mouseOverGui(false)
	{}

	//--------------------------------------------------------------
	const char * Gui::GetUniqueName(ofAbstractParameter & parameter)
	{
		return GetUniqueName(parameter.getName());
	}

	//--------------------------------------------------------------
	const char * Gui::GetUniqueName(const std::string & candidate)
	{
		std::string result = candidate;
		while (std::find(windowOpen.usedNames.top().begin(), windowOpen.usedNames.top().end(), result) != windowOpen.usedNames.top().end())
		{
			result += " ";
		}
		windowOpen.usedNames.top().push_back(result);
		return windowOpen.usedNames.top().back().c_str();
	}

	//--------------------------------------------------------------
	void Gui::SetNextWindow(Settings & settings)
	{
		settings.windowSize.x = 0;
		settings.windowPos.y += settings.windowSize.y + kGuiMargin;
	}

	//--------------------------------------------------------------
	bool Gui::BeginWindow(ofParameter<bool> & parameter, Settings & settings, bool collapse)
	{
		if (settings.windowBlock)
		{
			ofLogWarning(__FUNCTION__) << "Already inside a window block!";
			return false;
		}
		
		// Reference this ofParameter until EndWindow().
		windowOpen.parameter = dynamic_pointer_cast<ofParameter<bool>>(parameter.newReference());
		windowOpen.value = parameter.get();

		auto result = Gui::BeginWindow(parameter.getName(), settings, collapse, &windowOpen.value);
		parameter = windowOpen.value;
		return result;
	}

	//--------------------------------------------------------------
	bool Gui::BeginWindow(const string & name, Settings & settings, bool collapse, bool * open)
	{
		if (settings.windowBlock)
		{
			ofLogWarning(__FUNCTION__) << "Already inside a window block!";
			return false;
		}

		settings.windowBlock = true;

		// Push a new list of names onto the stack.
		windowOpen.usedNames.push(std::vector<std::string>());

		ImGui::SetNextWindowPos(settings.windowPos, ImGuiSetCond_Appearing);
		ImGui::SetNextWindowSize(settings.windowSize, ImGuiSetCond_Appearing);
		ImGui::SetNextWindowCollapsed(collapse, ImGuiSetCond_Appearing);
		return ImGui::Begin(name.c_str(), open, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize | (collapse ? 0 : ImGuiWindowFlags_NoCollapse));
	}

	//--------------------------------------------------------------
	void Gui::EndWindow(Settings & settings)
	{
		if (!settings.windowBlock)
		{
			ofLogWarning(__FUNCTION__) << "Not inside a window block!";
			return;
		}

		settings.windowBlock = false;

		settings.windowPos = ImGui::GetWindowPos();
		settings.windowSize = ImGui::GetWindowSize();
		ImGui::End();

		// Unlink the referenced ofParameter.
		windowOpen.parameter.reset();

		// Clear the list of names from the stack.
		windowOpen.usedNames.pop();

		// Check if the mouse cursor is over this gui window.
		const auto windowBounds = ofRectangle(settings.windowPos, settings.windowSize.x, settings.windowSize.y);
		settings.mouseOverGui |= windowBounds.inside(ofGetMouseX(), ofGetMouseY());

		// Include this window's bounds in the total bounds.
		if (settings.totalBounds.isZero())
		{
			settings.totalBounds = windowBounds;
		}
		else
		{
			settings.totalBounds.growToInclude(windowBounds);
		}
	}

	//--------------------------------------------------------------
	bool Gui::BeginTree(ofAbstractParameter & parameter, Settings & settings)
	{
		return Gui::BeginTree(parameter.getName(), settings);
	}

	//--------------------------------------------------------------
	bool Gui::BeginTree(const string & name, Settings & settings)
	{
		bool result;
		ImGui::SetNextTreeNodeOpen(true, ImGuiSetCond_Appearing);
		if (settings.treeLevel == 0)
		{
			result = ImGui::TreeNodeEx(GetUniqueName(name), ImGuiTreeNodeFlags_CollapsingHeader);
		}
		else
		{
			result = ImGui::TreeNode(GetUniqueName(name));
		}
		if (result)
		{
			settings.treeLevel += 1;

			// Push a new list of names onto the stack.
			windowOpen.usedNames.push(std::vector<std::string>());
		}
		return result;
	}

	//--------------------------------------------------------------
	void Gui::EndTree(Settings & settings)
	{
		settings.treeLevel = std::max(0, settings.treeLevel - 1);

		// Clear the list of names from the stack.
		windowOpen.usedNames.pop();

		ImGui::TreePop();
	}

	//--------------------------------------------------------------
	void Gui::AddGroup(ofParameterGroup & group, Settings & settings)
	{
		bool prevWindowBlock = settings.windowBlock;
		if (settings.windowBlock)
		{
			if (!Gui::BeginTree(group, settings))
			{
				return;
			}
		}
		else
		{
			if (!Gui::BeginWindow(group.getName().c_str(), settings))
			{
				Gui::EndWindow(settings);
				return;
			}
		}

		for (auto parameter : group)
		{
			// Group.
			auto parameterGroup = dynamic_pointer_cast<ofParameterGroup>(parameter);
			if (parameterGroup)
			{
				// Recurse through contents.
				Gui::AddGroup(*parameterGroup, settings);
				continue;
			}

			// Parameter, try everything we know how to handle.
			auto parameterVec2f = dynamic_pointer_cast<ofParameter<glm::vec2>>(parameter);
			if (parameterVec2f)
			{
				Gui::AddParameter(*parameterVec2f);
				continue;
			}
			auto parameterVec3f = dynamic_pointer_cast<ofParameter<glm::vec3>>(parameter);
			if (parameterVec3f)
			{
				Gui::AddParameter(*parameterVec3f);
				continue;
			}
			auto parameterVec4f = dynamic_pointer_cast<ofParameter<glm::vec4>>(parameter);
			if (parameterVec4f)
			{
				Gui::AddParameter(*parameterVec4f);
				continue;
			}
			auto parameterOfVec2f = dynamic_pointer_cast<ofParameter<ofVec2f>>(parameter);
			if (parameterOfVec2f)
			{
				Gui::AddParameter(*parameterOfVec2f);
				continue;
			}
			auto parameterOfVec3f = dynamic_pointer_cast<ofParameter<ofVec3f>>(parameter);
			if (parameterOfVec3f)
			{
				Gui::AddParameter(*parameterOfVec3f);
				continue;
			}
			auto parameterOfVec4f = dynamic_pointer_cast<ofParameter<ofVec4f>>(parameter);
			if (parameterOfVec4f)
			{
				Gui::AddParameter(*parameterOfVec4f);
				continue;
			}
			auto parameterFloatColor = dynamic_pointer_cast<ofParameter<ofFloatColor>>(parameter);
			if (parameterFloatColor)
			{
				Gui::AddParameter(*parameterFloatColor);
				continue;
			}
			auto parameterFloat = dynamic_pointer_cast<ofParameter<float>>(parameter);
			if (parameterFloat)
			{
				Gui::AddParameter(*parameterFloat);
				continue;
			}
			auto parameterInt = dynamic_pointer_cast<ofParameter<int>>(parameter);
			if (parameterInt)
			{
				Gui::AddParameter(*parameterInt);
				continue;
			}
			auto parameterBool = dynamic_pointer_cast<ofParameter<bool>>(parameter);
			if (parameterBool)
			{
				Gui::AddParameter(*parameterBool);
				continue;
			}

			ofLogWarning(__FUNCTION__) << "Could not create GUI element for parameter " << parameter->getName();
		}

		if (settings.windowBlock && !prevWindowBlock)
		{
			// End window if we created it.
			Gui::EndWindow(settings);
		}
		else
		{
			// End tree.
			Gui::EndTree(settings);
		}
	}

	//--------------------------------------------------------------
	bool Gui::AddParameter(ofParameter<glm::tvec2<int>> & parameter)
	{
		auto tmpRef = parameter.get();
		if (ImGui::SliderInt2(GetUniqueName(parameter), glm::value_ptr(tmpRef), parameter.getMin().x, parameter.getMax().x))
		{
			parameter.set(tmpRef);
			return true;
		}
		return false;
	}

	//--------------------------------------------------------------
	bool Gui::AddParameter(ofParameter<glm::tvec3<int>> & parameter)
	{
		auto tmpRef = parameter.get();
		if (ImGui::SliderInt3(GetUniqueName(parameter), glm::value_ptr(tmpRef), parameter.getMin().x, parameter.getMax().x))
		{
			parameter.set(tmpRef);
			return true;
		}
		return false;
	}

	//--------------------------------------------------------------
	bool Gui::AddParameter(ofParameter<glm::tvec4<int>> & parameter)
	{
		auto tmpRef = parameter.get();
		if (ImGui::SliderInt4(GetUniqueName(parameter), glm::value_ptr(tmpRef), parameter.getMin().x, parameter.getMax().x))
		{
			parameter.set(tmpRef);
			return true;
		}
		return false;
	}

	//--------------------------------------------------------------
	bool Gui::AddParameter(ofParameter<glm::vec2> & parameter)
	{
		auto tmpRef = parameter.get();
		if (ImGui::SliderFloat2(GetUniqueName(parameter), glm::value_ptr(tmpRef), parameter.getMin().x, parameter.getMax().x))
		{
			parameter.set(tmpRef);
			return true;
		}
		return false;
	}

	//--------------------------------------------------------------
	bool Gui::AddParameter(ofParameter<glm::vec3> & parameter)
	{
		auto tmpRef = parameter.get();
		if (ImGui::SliderFloat3(GetUniqueName(parameter), glm::value_ptr(tmpRef), parameter.getMin().x, parameter.getMax().x))
		{
			parameter.set(tmpRef);
			return true;
		}
		return false;
	}

	//--------------------------------------------------------------
	bool Gui::AddParameter(ofParameter<glm::vec4> & parameter)
	{
		auto tmpRef = parameter.get();
		if (ImGui::SliderFloat4(GetUniqueName(parameter), glm::value_ptr(tmpRef), parameter.getMin().x, parameter.getMax().x))
		{
			parameter.set(tmpRef);
			return true;
		}
		return false;
	}

	//--------------------------------------------------------------
	bool Gui::AddParameter(ofParameter<ofVec2f> & parameter)
	{
		auto tmpRef = parameter.get();
		if (ImGui::SliderFloat2(GetUniqueName(parameter), tmpRef.getPtr(), parameter.getMin().x, parameter.getMax().x))
		{
			parameter.set(tmpRef);
			return true;
		}
		return false;
	}

	//--------------------------------------------------------------
	bool Gui::AddParameter(ofParameter<ofVec3f> & parameter)
	{
		auto tmpRef = parameter.get();
		if (ImGui::SliderFloat3(GetUniqueName(parameter), tmpRef.getPtr(), parameter.getMin().x, parameter.getMax().x))
		{
			parameter.set(tmpRef);
			return true;
		}
		return false;
	}

	//--------------------------------------------------------------
	bool Gui::AddParameter(ofParameter<ofVec4f> & parameter)
	{
		auto tmpRef = parameter.get();
		if (ImGui::SliderFloat4(GetUniqueName(parameter), tmpRef.getPtr(), parameter.getMin().x, parameter.getMax().x))
		{
			parameter.set(tmpRef);
			return true;
		}
		return false;
	}

	//--------------------------------------------------------------
	bool Gui::AddParameter(ofParameter<ofFloatColor> & parameter, bool alpha)
	{
		auto tmpRef = parameter.get();
		if (alpha)
		{
			if (ImGui::ColorEdit4(GetUniqueName(parameter), &tmpRef.r))
			{
				parameter.set(tmpRef);
				return true;
			}
		}
		else if (ImGui::ColorEdit3(GetUniqueName(parameter), &tmpRef.r))
		{
			parameter.set(tmpRef);
			return true;
		}
		return false;
	}

	//--------------------------------------------------------------
	template<typename ParameterType>
	bool Gui::AddParameter(ofParameter<ParameterType> & parameter)
	{
		auto tmpRef = parameter.get();
		const auto & info = typeid(ParameterType);
		if (info == typeid(float))
		{
			if (ImGui::SliderFloat(GetUniqueName(parameter), (float *)&tmpRef, parameter.getMin(), parameter.getMax()))
			{
				parameter.set(tmpRef);
				return true;
			}
			return false;
		}
		if (info == typeid(int))
		{
			if (ImGui::SliderInt(GetUniqueName(parameter), (int *)&tmpRef, parameter.getMin(), parameter.getMax()))
			{
				parameter.set(tmpRef);
				return true;
			}
			return false;
		}
		if (info == typeid(bool))
		{
			if (ImGui::Checkbox(GetUniqueName(parameter), (bool *)&tmpRef))
			{
				parameter.set(tmpRef);
				return true;
			}
			return false;
		}

		ofLogWarning(__FUNCTION__) << "Could not create GUI element for type " << info.name();
		return false;
	}

	//--------------------------------------------------------------
	bool Gui::AddRadio(ofParameter<int> & parameter, vector<string> labels, int columns)
	{
		ImGui::Text(parameter.getName().c_str());
		auto result = false;
		auto tmpRef = parameter.get();
		ImGui::PushID(parameter.getName().c_str());
		{
			ImGui::Columns(columns);
			for (int i = 0; i < labels.size(); ++i)
			{
				result |= ImGui::RadioButton(GetUniqueName(labels[i]), &tmpRef, i); 
				ImGui::NextColumn();
			}
			ImGui::Columns(1);
		}
		ImGui::PopID();
		if (result)
		{
			parameter.set(tmpRef);
		}
		return result;
	}

	//--------------------------------------------------------------
	bool Gui::AddStepper(ofParameter<int> & parameter, int step, int stepFast)
	{
		auto tmpRef = parameter.get();
		if (ImGui::InputInt(GetUniqueName(parameter), &tmpRef, step, stepFast))
		{
			parameter.set(tmpRef);
			return true;
		}
		return false;
	}

	//--------------------------------------------------------------
	bool Gui::AddRange(const string & name, ofParameter<float> & parameterMin, ofParameter<float> & parameterMax, float speed)
	{
		auto tmpRefMin = parameterMin.get();
		auto tmpRefMax = parameterMax.get();
		if (ImGui::DragFloatRange2(GetUniqueName(name), &tmpRefMin, &tmpRefMax, speed, parameterMin.getMin(), parameterMax.getMax()))
		{
			parameterMin.set(tmpRefMin);
			parameterMax.set(tmpRefMax);
			return true;
		}
		return false;
	}

	//--------------------------------------------------------------
	bool Gui::AddValues(const string & name, vector<glm::tvec2<int>> & values, int minValue, int maxValue)
	{
		auto result = false;
		for (int i = 0; i < values.size(); ++i)
		{
			const auto iname = name + " " + ofToString(i);
			result |= ImGui::SliderInt2(GetUniqueName(iname), glm::value_ptr(values[i]), minValue, maxValue);
		}
		return result;
	}

	//--------------------------------------------------------------
	bool Gui::AddValues(const string & name, vector<glm::tvec3<int>> & values, int minValue, int maxValue)
	{
		auto result = false;
		for (int i = 0; i < values.size(); ++i)
		{
			const auto iname = name + " " + ofToString(i);
			result |= ImGui::SliderInt3(GetUniqueName(iname), glm::value_ptr(values[i]), minValue, maxValue);
		}
		return result;
	}

	//--------------------------------------------------------------
	bool Gui::AddValues(const string & name, vector<glm::tvec4<int>> & values, int minValue, int maxValue)
	{
		auto result = false;
		for (int i = 0; i < values.size(); ++i)
		{
			const auto iname = name + " " + ofToString(i);
			result |= ImGui::SliderInt4(GetUniqueName(iname), glm::value_ptr(values[i]), minValue, maxValue);
		}
		return result;
	}

	//--------------------------------------------------------------
	bool Gui::AddValues(const string & name, vector<glm::vec2> & values, float minValue, float maxValue)
	{
		auto result = false;
		for (int i = 0; i < values.size(); ++i)
		{
			const auto iname = name + " " + ofToString(i);
			result |= ImGui::SliderFloat2(GetUniqueName(iname), glm::value_ptr(values[i]), minValue, maxValue);
		}
		return result;
	}

	//--------------------------------------------------------------
	bool Gui::AddValues(const string & name, vector<glm::vec3> & values, float minValue, float maxValue)
	{
		auto result = false;
		for (int i = 0; i < values.size(); ++i)
		{
			const auto iname = name + " " + ofToString(i);
			result |= ImGui::SliderFloat3(GetUniqueName(iname), glm::value_ptr(values[i]), minValue, maxValue);
		}
		return result;
	}

	//--------------------------------------------------------------
	bool Gui::AddValues(const string & name, vector<glm::vec4> & values, float minValue, float maxValue)
	{
		auto result = false;
		for (int i = 0; i < values.size(); ++i)
		{
			const auto iname = name + " " + ofToString(i);
			result |= ImGui::SliderFloat4(GetUniqueName(iname), glm::value_ptr(values[i]), minValue, maxValue);
		}
		return result;
	}

	//--------------------------------------------------------------
	bool Gui::AddValues(const string & name, vector<ofVec2f> & values, float minValue, float maxValue)
	{
		auto result = false;
		for (int i = 0; i < values.size(); ++i)
		{
			const auto iname = name + " " + ofToString(i);
			result |= ImGui::SliderFloat2(GetUniqueName(iname), values[i].getPtr(), minValue, maxValue);
		}
		return result;
	}

	//--------------------------------------------------------------
	bool Gui::AddValues(const string & name, vector<ofVec3f> & values, float minValue, float maxValue)
	{
		auto result = false;
		for (int i = 0; i < values.size(); ++i)
		{
			const auto iname = name + " " + ofToString(i);
			result |= ImGui::SliderFloat3(GetUniqueName(iname), values[i].getPtr(), minValue, maxValue);
		}
		return result;
	}

	//--------------------------------------------------------------
	bool Gui::AddValues(const string & name, vector<ofVec4f> & values, float minValue, float maxValue)
	{
		auto result = false;
		for (int i = 0; i < values.size(); ++i)
		{
			const auto iname = name + " " + ofToString(i);
			result |= ImGui::SliderFloat4(GetUniqueName(iname), values[i].getPtr(), minValue, maxValue);
		}
		return result;
	}

	//--------------------------------------------------------------
	template<typename DataType>
	bool Gui::AddValues(const string & name, vector<DataType> & values, DataType minValue, DataType maxValue)
	{
		auto result = false;
		const auto & info = typeid(DataType);
		for (int i = 0; i < values.size(); ++i)
		{
			const auto iname = name + " " + ofToString(i);
			if (info == typeid(float))
			{
				result |= ImGui::SliderFloat(GetUniqueName(iname), *values[i], minValue, maxValue);
			}
			else if (info == typeid(int))
			{
				result |= ImGui::SliderInt(GetUniqueName(iname), *values[i], minValue, maxValue);
			}
			else if (info == typeid(bool))
			{
				result |= ImGui::Checkbox(GetUniqueName(iname), *values[i]);
			}
			else
			{
				ofLogWarning("Gui::AddValues") << "Could not create GUI element for type " << info.name();
				return false;
			}
		}
		return result;
	}

	//--------------------------------------------------------------
	void Gui::AddImage(ofBaseHasTexture & hasTexture, const ofVec2f & size)
	{
		Gui::AddImage(hasTexture.getTexture(), size);
	}

	//--------------------------------------------------------------
	void Gui::AddImage(ofTexture & texture, const ofVec2f & size)
	{
		ImTextureID textureID = (ImTextureID)(uintptr_t)texture.texData.textureID;
		ImGui::Image(textureID, size);
	}
}
