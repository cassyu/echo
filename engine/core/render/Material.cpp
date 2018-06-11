#include "Material.h"
#include "Engine/modules/Light/Light.h"
#include "Engine/core/Scene/NodeTree.h"
#include "engine/core/render/render/ShaderProgramRes.h"
#include "engine/core/render/render/MaterialDesc.h"
#include "engine/core/render/render/Renderer.h"
#include <thirdparty/pugixml/pugixml.hpp>

namespace Echo
{
	Material::Uniform::~Uniform()
	{
		EchoSafeDeleteArray(m_value, Byte, getValueBytes());
	}

	// ��¡
	Material::Uniform* Material::Uniform::clone()
	{
		Uniform* result = EchoNew(Uniform);
		result->m_name = m_name;
		result->m_type = m_type;
		result->m_count = m_count;
		result->setValue( m_value);

		return result;
	}

	// get value bytes
	i32 Material::Uniform::getValueBytes()
	{
		// ���������ڴ��С
		int bytes = 0;
		switch (m_type)
		{
		case SPT_INT:		bytes = sizeof(int)*m_count;		break;
		case SPT_FLOAT:		bytes = sizeof(real32)*m_count;		break;
		case SPT_VEC2:		bytes = sizeof(Vector2)*m_count;	break;
		case SPT_VEC3:		bytes = sizeof(Vector3)*m_count;	break;
		case SPT_VEC4:		bytes = sizeof(Vector4)*m_count;	break;
		case SPT_MAT4:		bytes = sizeof(Matrix4)*m_count;	break;
		case SPT_TEXTURE:	bytes = sizeof(int)*m_count;		break;
		default:			bytes = 0;							break;
		}

		return bytes;
	}

	void Material::Uniform::setValue(const void* value)
	{
		EchoSafeDeleteArray( m_value, Byte, getValueBytes());
		if (value)
		{
			i32 bytes = getValueBytes();
			m_value = EchoNewArray(Byte, bytes);
			std::memcpy(m_value, value, bytes);
		}
	}

	// alloc Value
	void Material::Uniform::allocValue()
	{
		if (!m_value)
		{
			i32 bytes = getValueBytes();
			m_value = EchoNewArray(Byte, bytes);
			std::memset(m_value, 0, bytes);
		}
	}

	// ���캯��
	Material::Material(const ResourcePath& path)
		: Res(path)
		, m_shaderProgram(NULL)
		, m_officialMaterialContent(nullptr)
	{

	}

	// ��������
	Material::~Material()
	{
		m_unifroms.clear();

		unloadTexture();
		m_textures.clear();
	}

	// create a material instance
	Material* Material::create()
	{
		return EchoNew(Material);
	}

	// release
	void Material::release()
	{
		ECHO_DELETE_T(this, Material);
	}

	bool Material::applyLoadedData()
	{
		// ���ز���ģ��
		buildRenderQueue();

		// ��ȡ��ɫ��
		ShaderProgram* shaderProgram = m_shaderProgram->getShaderProgram();
		if (shaderProgram)
		{
			matchUniforms();
			return true;
		}

		return false;
	}

	// ���Ʋ���ʵ��
	void Material::clone(Material* orig)
	{
		// �������ƣ�����
		m_name = orig->m_name;
		m_renderStage = orig->m_renderStage;
		m_macros = orig->m_macros;
		m_materialTemplate = orig->m_materialTemplate;
		m_shaderProgram = orig->m_shaderProgram;

		for (auto it : orig->m_unifroms)
		{
			Uniform* uniform = it.second;
			m_unifroms[uniform->m_name] = uniform->clone();
		}

		m_textures = orig->m_textures;
	}

	// ��ȡ����ֵ
	void* Material::getUniformValue(const String& name)
	{
		const auto& it = m_unifroms.find(name);
		if (it != m_unifroms.end())
		{
			return  it->second->m_value;
		}

		const ShaderProgramRes::DefaultUniform* dUniform = m_shaderProgram->getDefaultUniformValue(name);
		return dUniform ? dUniform->value : NULL;
	}

	// ׼����ԴIO
	TextureRes* Material::prepareTextureImp(const String& texName)
	{
		TextureRes* pTexture;
		size_t cubePos = texName.find("_cube_");
		if (texName.find("_cube_") == String::npos)
		{
			pTexture = TextureRes::createTexture(texName.empty() ? "OFFICAL_MATERTAL_TEMPLATE.tga" : texName, Texture::TU_STATIC);
		}
		else
		{
			String cubeTexNamePre = texName.substr(0, cubePos + 6);
			String cubeTexNamePost = texName.substr(cubePos + 7);
			array<String, 6> texNames;
			for (size_t i = 0; i < 6; ++i)
			{
				texNames[i] = StringUtil::Format("%s%d%s", cubeTexNamePre.c_str(), i, cubeTexNamePost.c_str());
			}
			pTexture = TextureRes::createTextureCubeFromFiles(texNames[0], texNames[1], texNames[2], texNames[3], texNames[4], texNames[5]);
		}

		pTexture->prepareLoad();

		return pTexture;
	}

	// ��Դ�����߳�׼������
	void Material::prepareTexture()
	{
		for (const auto& element : m_textures)
		{
			m_textures[element.first].m_texture = prepareTextureImp(element.second.m_uri);
		}
	}

	void Material::loadTexture()
	{
		for (auto& it : m_textures)
		{
			TextureRes* texRes = it.second.m_texture;
			texRes->prepareLoad();
		}
	}

	void Material::unloadTexture()
	{
		m_textures.clear();
	}

	// ����������ȡ����
	TextureRes* Material::getTexture(const int& index)
	{
		auto it = m_textures.find(index);
		if (it != m_textures.end())
		{
			return it->second.m_texture;
		}

		return nullptr;
	}

	// ���ú궨��
	void Material::setMacros(const String& macros) 
	{ 
		m_macros = StringUtil::Split(macros, ";");
		std::sort(m_macros.begin(), m_macros.end());
	}

	// �жϱ����Ƿ����
	bool Material::isUniformExist(const String& name)
	{
		return m_unifroms.find(name)!=m_unifroms.end();
	}

	// �޸ı���
	void Material::setUniformValue(const String& name, const ShaderParamType& type, void* value)
	{
		const auto& it = m_unifroms.find(name);
		if (it != m_unifroms.end())
		{
			if (it->second->m_type == type)
				it->second->setValue(value);
			else
				EchoLogError("MaterialInstance::ModifyUnifromParam Type Error!");
		}
	}

	//void* MaterialInst::getUniformValue(const String& name, ShaderParamType type)
	//{
	//	if (type == SPT_TEXTURE)
	//	{
	//		void* index = getUniformValue(name);
	//		if (index)
	//		{
	//			return &m_textures[*(int*)index];
	//		}
	//		return NULL;
	//	}

	//	return getUniformValue(name);
	//}

	void Material::addTexture(int idx, const String& name)
	{
		TextureInfo info;
		info.m_idx = idx;
		info.m_name = name;
		m_textures[idx] = info;
	}

	TextureRes* Material::setTexture(const String& name, const String& uri)
	{
		return setTexture( name, prepareTextureImp(uri));
	}

	TextureRes* Material::setTexture(const String& name, TextureRes* textureRes)
	{
		if (!textureRes)
			return nullptr;

		for (auto& it : m_textures)
		{
			TextureInfo& info = it.second;
			if (info.m_name == name)
			{
				//TextureRes::releaseResource(info.m_texture);

				info.m_uri = textureRes->getName();
				info.m_texture = textureRes;

				return info.m_texture;
			}
		}

		return nullptr;
	}

	// ��ȡ����
	Material::Uniform* Material::getUniform(const String& name)
	{
		const auto& it = m_unifroms.find(name);
		if (it != m_unifroms.end())
			return it->second;

		return NULL;
	}

	// �Ƿ�ʹ���˺궨��
	bool Material::isMacroUsed(const String& macro)
	{
		for (const String& _macro : m_macros)
		{
			if (macro == _macro)
				return true;
		}

		return false;
	}

	// ���ú궨��
	void Material::setMacro(const String& macro, bool enabled)
	{
		if (enabled)
		{
			if (!isMacroUsed(macro))
			{
				m_macros.push_back(macro);
			}
		}
		else
		{
			if (isMacroUsed(macro))
			{
				m_macros.erase(std::remove(m_macros.begin(), m_macros.end(), macro), m_macros.end());
			}
			else
			{
				ui32 size = m_macros.size();
				for (ui32 i = 0; i < size; ++i)
				{
					if (StringUtil::StartWith(m_macros[i], macro))
					{
						m_macros.erase(std::remove(m_macros.begin(), m_macros.end(), m_macros[i]), m_macros.end());
						break;
					}
				}
			}
		}
		std::sort(m_macros.begin(), m_macros.end());
	}

	// ������Ⱦ����
	void Material::buildRenderQueue()
	{
		// make sure macros
		String finalMacros; finalMacros.reserve(512);
		for (const String& macro : m_macros)
		{
			finalMacros += "#define " + macro + "\n";
		}

		// create material
		m_shaderProgram = EchoNew(ShaderProgramRes);
		if (m_officialMaterialContent)
			m_shaderProgram->loadFromContent(m_officialMaterialContent, finalMacros);
		else if (!m_materialTemplate.empty())
			m_shaderProgram->loadFromFile( m_materialTemplate, finalMacros);
	}

	static bool MappingStringArrayIdx(const String* arry, int count, const String& value, int& idx)
	{
		for (int i = 0; i < count; i++)
		{
			if (value == arry[i])
			{
				idx = i;
				return true;
			}
		}

		return false;
	}

	// ����ƥ��
	void Material::matchUniforms()
	{
		ShaderProgram* shaderProgram = m_shaderProgram->getShaderProgram();
		if (shaderProgram)
		{
			// ����δ���ò���
			for (auto& it : *(shaderProgram->getUniforms()))
			{
				const ShaderProgram::Uniform& suniform = it.second;
				{
					Uniform* uniform = EchoNew(Uniform);
					uniform->m_name = suniform.m_name;
					uniform->m_type = suniform.m_type;
					uniform->m_count = suniform.m_count;

					// auto set texture value
					if (uniform->m_type == SPT_TEXTURE)
					{
						i32 textureNum = getTextureNum();
						uniform->setValue(&textureNum);
						addTexture(getTextureNum(), uniform->m_name);
					}
					else
					{
						uniform->allocValue();
					}

					// default value
					const ShaderProgramRes::DefaultUniform* defaultUniform = m_shaderProgram->getDefaultUniformValue(uniform->m_name);
					if (defaultUniform && uniform->m_count == defaultUniform->count && uniform->m_type == defaultUniform->type)
						uniform->setValue(defaultUniform->value);

					m_unifroms[uniform->m_name] = uniform;
				}
			}
		}
	}
}