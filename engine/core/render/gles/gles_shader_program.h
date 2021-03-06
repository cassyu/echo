#pragma once

#include <engine/core/util/Array.hpp>
#include "base/shader_program.h"
#include "base/renderable.h"
#include "base/mesh/mesh.h"
#include "gles_shader.h"
#include "gles_renderable.h"

namespace Echo
{
	class GLESShaderProgram : public ShaderProgram
	{
	public:
		typedef array<GLint, VS_MAX> AttribLocationArray;
		typedef array<GLESShader*, GLESShader::ST_SHADERCOUNT> ShaderArray;

	public:
		GLESShaderProgram();
		~GLESShaderProgram();

		// attch|detach shader
		bool attachShader(GLESShader* pShader);
		GLESShader*	detachShader(GLESShader::ShaderType type);

		// link shader
		bool linkShaders();

		// bind Uniforms
		void bindUniforms();

		// bind|unbind
		virtual void bind() override;
		virtual void unbind() override;

		// bind Renderable
		void bindRenderable(Renderable* renderable);

		// get attribute location
		i32 getAtrribLocation(VertexSemantic vertexSemantic);

		// Create
		virtual bool createShaderProgram(const String& vsContent, const String& psContent) override;
		void clearShaderProgram();

	public:
		// get shader
		GLESShader* getShader(GLESShader::ShaderType type) const { return m_shaders[(ui32)type]; }
	
	private:
        bool                m_isLinked = false;
		ShaderArray			m_shaders;
		GLESRenderable*	m_preRenderable;				// Geomerty
		AttribLocationArray	m_attribLocationMapping;		// Attribute location
		GLuint				m_glesProgram = 0;
	};
}
