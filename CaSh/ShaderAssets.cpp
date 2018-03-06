#include "ShaderAssets.h"
#include "Logger.h"

void FShaderParameterMap::VerifyBindingsAreComplete(const char* ShaderTypeName, EShaderFrequency Frequency, class FVertexFactoryType* InVertexFactoryType) const
{
#if 0
	if (1)
	{
		const char* VertexFactoryName = InVertexFactoryType ? InVertexFactoryType->GetName() : TEXT("?");

		bool bBindingsComplete = true;
		std::string UnBoundParameters = TEXT("");
		for (auto i : ParameterMap)
		{
			const std::string& ParamName = i.first;
			const FParameterAllocation& ParamValue = i.second;
			if (!ParamValue.bBound)
			{
				// Only valid parameters should be in the shader map
				CHECK(ParamValue.Size > 0);
				bBindingsComplete = bBindingsComplete && ParamValue.bBound;
				UnBoundParameters += std::string(TEXT("		Parameter ")) + ParamName + TEXT(" not bound!\n");
			}
		}

		if (!bBindingsComplete)
		{
			std::string ErrorMessage = std::string(TEXT("Found unbound parameters being used in shadertype ")) + ShaderTypeName + TEXT(" (VertexFactory: ") + VertexFactoryName + TEXT(")\n") + UnBoundParameters;
			// An unbound parameter means the engine is not going to set its value (because it was never bound) 
			// but it will be used in rendering, which will most likely cause artifacts

			// We use a non-Slate message box to avoid problem where we haven't compiled the shaders for Slate.
			g_logger->debug_print(WIP_ERROR, ErrorMessage.c_str());
			//FPlatformMisc::MessageBoxExt(EAppMsgType::Ok, *ErrorMessage, TEXT("Error"));
		}
	}
#endif
}