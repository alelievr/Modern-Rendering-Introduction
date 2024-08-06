#pragma once
#include <string>
#include <unordered_map>
#include <wrl.h>
#include <wrl/client.h>
#define __EMULATE_UUID
#include "dxcapi.h"

using namespace Microsoft::WRL;

#define FORCE_RELOAD_ON_STARTUP true

class ShaderCache
{
public:
	enum class ShaderType
	{
		Compute,
		Mesh,
		Amplification,
		Fragment,
	};

	const std::string k_ShaderCachePath = "artifacts/shader_cache/";

	ShaderCache();

	std::string GetShader(const std::string& shaderPath, const std::string& entryPoint, const ShaderType shaderType);

private:
	std::unordered_map<std::string, int> m_ShaderCache;
	ComPtr<IDxcCompiler3> m_Compiler;
	ComPtr<IDxcUtils> m_Utils;
	IDxcIncludeHandler* m_IncludeHandler;

	bool CompileShader(const std::string& shaderCode, const std::string& outputPath, const std::string& entryPoint, const ShaderType shaderType);
};