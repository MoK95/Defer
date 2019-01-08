#pragma once

#include <unordered_map>
#include <tgl/tgl.h>

namespace MLK
{
	const GLuint g_maxQueries = 10;

	enum ProfileKey
	{
		GBufferTime,
		AmbientTime
	};

	/// <summary>
    /// Implementation to help manage witching of OpenGl variables. The different passes require multiple
    /// GL variables to be set, this groups them and ensures they aren't switched unnecessarily.
    /// </summary>
	class Profiler
	{
	public:
		Profiler();
		~Profiler();

		void beginQuery(ProfileKey key);
		void endQuery(ProfileKey key);

	private:
		std::unordered_map<ProfileKey, std::vector<GLuint>> m_queries;
	};
}