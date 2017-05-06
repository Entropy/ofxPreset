#include "Serializer.h"

namespace ofxPreset
{
	//--------------------------------------------------------------
	template<typename DataType>
	nlohmann::json & Serializer::Serialize(nlohmann::json & json, const vector<DataType> & values, const string & name)
	{
		auto & jsonGroup = name.empty() ? json : json[name];
		
		for (const auto & val : values)
		{
			ostringstream oss;
			oss << val;
			jsonGroup.push_back(oss.str());
		}

		return jsonGroup;
	}
	
	//--------------------------------------------------------------
	template<typename DataType>
	const nlohmann::json & Serializer::Deserialize(const nlohmann::json & json, vector<DataType> & values, const string & name)
	{
		if (!name.empty() && !json.count(name))
		{
			ofLogWarning(__FUNCTION__) << "Name " << name << " not found in JSON!";
			return json;
		}

		const auto & jsonGroup = name.empty() ? json : json[name];
		values.clear();

		for (const auto & jsonValue : jsonGroup)
		{	
			istringstream iss;
			iss.str(jsonValue);
			DataType val;
			iss >> val;
			values.push_back(val);
		}

		return jsonGroup;
	}
	
	//--------------------------------------------------------------
	inline nlohmann::json & Serializer::Serialize(nlohmann::json & json, const ofAbstractParameter & parameter)
	{
		if (parameter.isSerializable())
		{
			const auto name = parameter.getName();
			json[name] = parameter.toString();
		}

		return json;
	}
	
	//--------------------------------------------------------------
	inline const nlohmann::json & Serializer::Deserialize(const nlohmann::json & json, ofAbstractParameter & parameter)
	{
		if (parameter.isSerializable())
		{
			const auto name = parameter.getName();
			if (json.count(name))
			{
				string valueString = json[name];
				if (!valueString.empty())
				{
					parameter.fromString(valueString);
				}
			}
		}

		return json;
	}

	//--------------------------------------------------------------
    nlohmann::json & Serializer::Serialize(nlohmann::json & json, const ofParameterGroup & group)
    {
		if (group.isSerializable())
		{
			const auto name = group.getName();
			auto & jsonGroup = name.empty() ? json : json[name];
			for (const auto & parameter : group)
			{
				// Group.
				auto parameterGroup = dynamic_pointer_cast<ofParameterGroup>(parameter);
				if (parameterGroup)
				{
					// Recurse through contents.
					Serializer::Serialize(jsonGroup, *parameterGroup);
					continue;
				}

				// Parameter.
				auto parameterAbstract = dynamic_pointer_cast<ofAbstractParameter>(parameter);
				if (parameterAbstract)
				{
					Serializer::Serialize(jsonGroup, *parameterAbstract);
					continue;
				}
			}

			return jsonGroup;
		}
		
		ofLogWarning(__FUNCTION__) << "Group " << group.getName() << " is not serializable";
		return json;
    }

    //--------------------------------------------------------------
    const nlohmann::json & Serializer::Deserialize(const nlohmann::json & json, ofParameterGroup & group)
    {
		if (group.isSerializable())
		{
			const auto name = group.getName();
			if (!name.empty() && !json.count(name))
			{
				ofLogWarning(__FUNCTION__) << "Name " << name << " not found in JSON!";
				return json;
			}

			const auto & jsonGroup = name.empty() ? json : json[name];
			for (const auto & parameter : group)
			{
				if (!parameter)
				{
					continue;
				}

				// Group.
				auto parameterGroup = dynamic_pointer_cast<ofParameterGroup>(parameter);
				if (parameterGroup)
				{
					// Recurse through contents.
					Serializer::Deserialize(jsonGroup, *parameterGroup);
					continue;
				}

				// Parameter.
				auto parameterAbstract = dynamic_pointer_cast<ofAbstractParameter>(parameter);
				if (parameterAbstract)
				{
					Serializer::Deserialize(jsonGroup, *parameterAbstract);
				}
			}

			return jsonGroup;
		}

		ofLogWarning(__FUNCTION__) << "Group " << group.getName() << " is not serializable";
		return json;
    }

	//--------------------------------------------------------------
	inline nlohmann::json & Serializer::Serialize(nlohmann::json & json, const ofEasyCam & easyCam, const string & name)
	{
		auto & jsonGroup = Serializer::Serialize(json, (ofCamera &)easyCam, name);

		jsonGroup["target"] = ofToString(easyCam.getTarget().getPosition());
		jsonGroup["distance"] = easyCam.getDistance();
		jsonGroup["drag"] = easyCam.getDrag();
		jsonGroup["mouseInputEnabled"] = easyCam.getMouseInputEnabled();
		jsonGroup["mouseMiddleButtonEnabled"] = easyCam.getMouseMiddleButtonEnabled();
		jsonGroup["translationKey"] = easyCam.getTranslationKey();
		jsonGroup["relativeYAxis"] = easyCam.getRelativeYAxis();
		jsonGroup["upAxis"] = ofToString(easyCam.getUpAxis());
		jsonGroup["inertiaEnabled"] = easyCam.getInertiaEnabled();
		return jsonGroup;
	}

	//--------------------------------------------------------------
	inline const nlohmann::json & Serializer::Deserialize(const nlohmann::json & json, ofEasyCam & easyCam, const string & name)
	{
		if (!name.empty() && !json.count(name))
		{
			ofLogWarning(__FUNCTION__) << "Name " << name << " not found in JSON!";
			return json;
		}

		easyCam.setAutoDistance(false);

		const auto & jsonGroup = name.empty() ? json : json[name];

		try
		{
			const auto target = ofFromString<glm::vec3>(jsonGroup["target"]);
			easyCam.setTarget(target);
			easyCam.setDistance(jsonGroup["distance"]);
			easyCam.setDrag(jsonGroup["drag"]);
			jsonGroup["mouseInputEnabled"] ? easyCam.enableMouseInput() : easyCam.disableMouseInput();
			jsonGroup["mouseMiddleButtonEnabled"] ? easyCam.enableMouseMiddleButton() : easyCam.disableMouseMiddleButton();
			int translationKey = jsonGroup["translationKey"];
			easyCam.setTranslationKey(translationKey);
		}
		catch (std::exception & exc)
		{
			ofLogError(__FUNCTION__) << exc.what();
		}

		Serializer::Deserialize(jsonGroup, (ofCamera &)easyCam);

		return jsonGroup;
	}

	//--------------------------------------------------------------
	inline nlohmann::json & Serializer::Serialize(nlohmann::json & json, const ofCamera & camera, const string & name)
	{
		auto & jsonGroup = Serializer::Serialize(json, (ofNode &)camera, name);

		jsonGroup["fov"] = camera.getFov();
		jsonGroup["nearClip"] = camera.getNearClip();
		jsonGroup["farClip"] = camera.getFarClip();
		jsonGroup["lensOffset"] = ofToString(camera.getLensOffset());
		jsonGroup["aspectRatio"] = camera.getAspectRatio();
		jsonGroup["forceAspectRatio"] = camera.getForceAspectRatio();
		jsonGroup["ortho"] = camera.getOrtho();

		return jsonGroup;
	}

	//--------------------------------------------------------------
	inline const nlohmann::json & Serializer::Deserialize(const nlohmann::json & json, ofCamera & camera, const string & name)
	{
		if (!name.empty() && !json.count(name))
		{
			ofLogWarning(__FUNCTION__) << "Name " << name << " not found in JSON!";
			return json;
		}
		
		const auto & jsonGroup = name.empty() ? json : json[name];

		try
		{
			camera.setFov(jsonGroup["fov"]);
			camera.setNearClip(jsonGroup["nearClip"]);
			camera.setFarClip(jsonGroup["farClip"]);
			const auto lensOffset = ofFromString<glm::vec2>(jsonGroup["lensOffset"]);
			camera.setLensOffset(lensOffset);
			camera.setForceAspectRatio(jsonGroup["forceAspectRatio"]);
			if (camera.getForceAspectRatio())
			{
				camera.setAspectRatio(jsonGroup["aspectRatio"]);
			}
			jsonGroup["ortho"] ? camera.enableOrtho() : camera.disableOrtho();
		}
		catch (std::exception & exc)
		{
			ofLogError(__FUNCTION__) << exc.what();
		}

		Serializer::Deserialize(jsonGroup, (ofNode &)camera);

		return jsonGroup;
	}

	//--------------------------------------------------------------
	inline nlohmann::json & Serializer::Serialize(nlohmann::json & json, const ofNode & node, const string & name)
	{
		auto & jsonGroup = name.empty() ? json : json[name];

		jsonGroup["transform"] = ofToString(node.getLocalTransformMatrix());

		return jsonGroup;
	}

	//--------------------------------------------------------------
	inline const nlohmann::json & Serializer::Deserialize(const nlohmann::json & json, ofNode & node, const string & name)
	{
		if (!name.empty() && !json.count(name))
		{
			ofLogWarning(__FUNCTION__) << "Name " << name << " not found in JSON!";
			return json;
		}

		const auto & jsonGroup = name.empty() ? json : json[name];

		try
		{
			const auto transform = ofFromString<glm::mat4>(jsonGroup["transform"]);
			node.setTransformMatrix(transform);
		}
		catch (std::exception & exc)
		{
			ofLogError(__FUNCTION__) << exc.what();
		}

		return jsonGroup;
	}
}
