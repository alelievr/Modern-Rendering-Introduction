#include "ShaderCache.h"
#include <fstream>
#include <litefx/logging.hpp>
#include <filesystem>
#include <vector>

#define DEBUG_SHADER


ShaderCache::ShaderCache()
{
	HRESULT res = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&m_Compiler));
	HRESULT res2 = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&m_Utils));

	if (res != S_OK || res2 != S_OK)
		LITEFX_ERROR("ShaderCache", "Could not create DXC Compiler");

	res = m_Utils->CreateDefaultIncludeHandler(&m_IncludeHandler);

	if (res != S_OK)
		LITEFX_ERROR("ShaderCache", "Could not create DXC Include Handler");
}

bool ShaderCache::CompileShader(const std::string& shaderCode, const std::string& outputPath, const std::string& entryPoint, const ShaderType shaderType)
{
    std::vector<LPCWSTR> arguments;

	arguments.push_back(nullptr);
	// -E for the entry point (eg. 'main')
	arguments.push_back(L"-E");
	arguments.push_back((LPCWSTR)entryPoint.c_str());

	arguments.push_back(L"-Fo");
	arguments.push_back((LPCWSTR)outputPath.c_str());

	// -T for the target profile (eg. 'ps_6_6')
	arguments.push_back(L"-T");
	switch (shaderType)
	{
		case ShaderType::Compute:
			arguments.push_back(L"cs_6_6");
			break;
		case ShaderType::Mesh:
			arguments.push_back(L"ms_6_6");
			break;
		case ShaderType::Amplification:
			arguments.push_back(L"as_6_6");
			break;
		case ShaderType::Fragment:
			arguments.push_back(L"ps_6_6");
			break;
	}

#ifdef DEBUG_SHADER
	arguments.push_back(DXC_ARG_DEBUG); //-Zi
#else
	arguments.push_back(L"-Qstrip_debug");
	arguments.push_back(L"-Qstrip_reflect");

	 arguments.push_back(DXC_ARG_OPTIMIZATION_LEVEL3);
#endif

	//arguments.push_back(DXC_ARG_WARNINGS_ARE_ERRORS); //-WX

	//for (const std::wstring& define : defines)
	//{
	//	arguments.push_back(L"-D");
	//	arguments.push_back(define.c_str());
	//}

	DxcBuffer sourceBuffer;
	//sourceBuffer.Ptr = shaderCode.c_str();
	//sourceBuffer.Size = shaderCode.size();
	sourceBuffer.Ptr = "";
	sourceBuffer.Size = 0;
	sourceBuffer.Encoding = DXC_CP_UTF8;

	ComPtr<IDxcResult> pCompileResult{};
	HRESULT resulut = m_Compiler->Compile(&sourceBuffer, arguments.data(), (uint32_t)arguments.size(), nullptr, IID_PPV_ARGS(&pCompileResult));

	// Error Handling. Note that this will also include warnings unless disabled.
	ComPtr<IDxcBlobUtf8> pErrors;
	pCompileResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&pErrors), nullptr);
	if (pErrors && pErrors->GetStringLength() > 0)
	{
		LITEFX_ERROR("ShaderCache", "%s", (char*)pErrors->GetBufferPointer());
		return false;
	}

	return true;
}


std::string ShaderCache::GetShader(const std::string& shaderPath, const std::string& entryPoint, const ShaderType shaderType)
{
	// Check that shader file exists and read it
	std::ifstream shader(shaderPath, std::ifstream::in);
	if (!shader.is_open())
	{
		LITEFX_ERROR("ShaderCache", "Could not open shader file: %s", shaderPath);
		return {};
	}

	std::string shaderCode((std::istreambuf_iterator<char>(shader)), std::istreambuf_iterator<char>());
	shader.close();

	if (!std::filesystem::exists(k_ShaderCachePath))
		std::filesystem::create_directories(k_ShaderCachePath);
	
	std::string shaderName = std::filesystem::path(shaderPath).filename().string();
	std::string cachedPath = k_ShaderCachePath + shaderName + ".dxi";
	auto shaderTime = std::filesystem::last_write_time(shaderPath);

	bool compile = false;

	if ((!m_ShaderCache.contains(shaderPath) && FORCE_RELOAD_ON_STARTUP))
		compile = true;
	if (std::filesystem::exists(cachedPath) && shaderTime > std::filesystem::last_write_time(cachedPath))
		compile = true;

	// Get timestamp of last modified the cached file
	if (compile)
	{
		if (CompileShader(shaderCode, cachedPath, entryPoint, shaderType))
		{
			m_ShaderCache[shaderPath] = 0;
			// write shader to cachedPath
		}
		else
		{
			m_ShaderCache[shaderPath]++;
			LITEFX_ERROR("ShaderCache", "Could not compile shader: %s", shaderPath);
			return {};
		}
	}

	return cachedPath;
}
