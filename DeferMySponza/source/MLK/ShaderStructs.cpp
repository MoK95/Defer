#include "ShaderStructs.hpp"

#include "Utils.hpp"

#include <sponza/Material.hpp>
#include <sponza/PointLight.hpp>
#include <sponza/SpotLight.hpp>
#include <sponza/DirectionalLight.hpp>

#include <glm/gtc/matrix_transform.hpp>

namespace MLK
{
    ShaderMaterial::ShaderMaterial(const sponza::Material& material) :
        DiffuseColour((const glm::vec3&)material.getDiffuseColour())
        , SpecularColour((const glm::vec3&)material.getSpecularColour())
        , Shininess(material.getShininess())
    {
		IsShiny = material.isShiny() ? 1 : 0;
    }

	ShaderLight::ShaderLight(const sponza::PointLight& pointLight) :
		Position((const glm::vec3&)pointLight.getPosition())
		, Intensity((const glm::vec3&)pointLight.getIntensity())
		, Range(pointLight.getRange())
	{
		ModelTransform = glm::translate(glm::mat4(), Position);
		ModelTransform = glm::scale(ModelTransform, glm::vec3(Range));
	}

	ShaderLight::ShaderLight(const sponza::SpotLight& spotLight) :
		Position((const glm::vec3&)spotLight.getPosition())
		, Intensity((const glm::vec3&)spotLight.getIntensity())
		, Direction((const glm::vec3&)spotLight.getDirection())
		, Range(spotLight.getRange())
		, Angle(glm::radians(spotLight.getConeAngleDegrees() / 2.f))
        , CastsShadows(spotLight.getCastShadow() ? 1 : 0)
	{
		glm::mat4 translate = glm::translate(glm::mat4(), Direction * Range);
		glm::mat4 rotate = glm::inverse(glm::lookAt(Position, Position + Direction, glm::vec3(0,1,0)));
		auto opp = glm::tan(Angle) * Range;

		glm::mat4 scale = glm::scale(glm::mat4(), glm::vec3(opp, opp, Range));

		ModelTransform = translate * rotate * scale;
	}

	DirectionalLight::DirectionalLight(const sponza::DirectionalLight& directionalLight) :
		Direction((const glm::vec3&)directionalLight.getDirection())
		, Intensity((const glm::vec3&)directionalLight.getIntensity())
	{
	}
}