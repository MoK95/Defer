#include "Profiler.hpp"

#include <iostream>

namespace MLK
{
	Profiler::Profiler()
    {
    }

	Profiler::~Profiler()
    {

    }

	void Profiler::beginQuery(ProfileKey key)
	{
		if (m_queries.find(key) == m_queries.end())
		{
			m_queries[key].resize(g_maxQueries);
			for (GLuint i = 0; i < g_maxQueries; ++i)
			{
				glGenQueries(1, &m_queries.at(key)[i]);
			}
		}
	}

	void Profiler::endQuery(ProfileKey key)
	{
		GLuint result;
		glGetQueryObjectuiv(m_queries.at(key)[0], GL_QUERY_RESULT_AVAILABLE, &result);
		if (result == GL_TRUE)
		{
			glGetQueryObjectuiv(m_queries.at(key)[0], GL_QUERY_RESULT, &result);
			std::cout << result << std::endl;
		}
	}
}