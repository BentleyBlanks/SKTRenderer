#include "ShaderAssets.h"
#include "D3Dcompiler.h"
#include "ResourceManager.h"
#include "RefCount.h"
#include "d3d11.h"
#include "Logger.h"
#include "D3D11Resources.h"
#include "RHIDef.h"
#include "UniformBuffer.h"

// @return pointer to the D3DCompile function
pD3DCompile GetD3DCompileFunc(const std::string& NewCompilerPath)
{
	static std::string CurrentCompiler;
	static HMODULE CompilerDLL = 0;

	if (CurrentCompiler != NewCompilerPath)
	{
		CurrentCompiler = NewCompilerPath;

		if (CompilerDLL)
		{
			FreeLibrary(CompilerDLL);
			CompilerDLL = 0;
		}

		if (CurrentCompiler.length()>0)
		{
			CompilerDLL = LoadLibrary(CurrentCompiler.c_str());
		}

		if (!CompilerDLL && NewCompilerPath.length())
		{
			// Couldn't find HLSL compiler in specified path. We fail the first compile.
			return 0;
		}
	}

	if (CompilerDLL)
	{
		// from custom folder e.g. "C:/DXWin8/D3DCompiler_44.dll"
		return (pD3DCompile)(void*)GetProcAddress(CompilerDLL, "D3DCompile");
	}

	// D3D SDK we compiled with (usually D3DCompiler_43.dll from windows folder)
	return &D3DCompile;
}

HRESULT D3DCompileWrapper(
	pD3DCompile				D3DCompileFunc,
	bool&					bException,
	LPCVOID					pSrcData,
	SIZE_T					SrcDataSize,
	LPCSTR					pFileName,
	CONST D3D_SHADER_MACRO*	pDefines,
	ID3DInclude*			pInclude,
	LPCSTR					pEntrypoint,
	LPCSTR					pTarget,
	uint32					Flags1,
	uint32					Flags2,
	ID3DBlob**				ppCode,
	ID3DBlob**				ppErrorMsgs
	)
{
#ifdef PLATFORM_SEH_EXCEPTIONS_DISABLED
	__try
#endif
	{
		return D3DCompileFunc(
			pSrcData,
			SrcDataSize,
			pFileName,
			pDefines,
			pInclude,
			pEntrypoint,
			pTarget,
			Flags1,
			Flags2,
			ppCode,
			ppErrorMsgs
			);
	}
#ifdef PLATFORM_SEH_EXCEPTIONS_DISABLED
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		bException = true;
		return E_FAIL;
	}
#endif
}

static const char* GetShaderProfileName(EShaderFrequency Target)
{
	switch (Target)
	{
	case SF_Pixel:
		return ("ps_5_0");
	case SF_Vertex:
		return ("vs_5_0");
	case SF_Hull:
		return ("hs_5_0");
	case SF_Domain:
		return ("ds_5_0");
	case SF_Geometry:
		return ("gs_5_0");
	case SF_Compute:
		return ("cs_5_0");
	}
}

#if 0

void BuildResourceTableMapping(
  const std::map<std::string, RBResourceTableEntry>& ResourceTableMap,
  const std::map<std::string, uint32>& ResourceTableLayoutHashes,
  uint32 UsedUniformBufferSlots,
  FShaderParameterMap& ParameterMap,
  RBShaderCompilerResourceTable& OutSRT)
{
  CHECK(OutSRT.ResourceTableBits == 0);
  CHECK(OutSRT.ResourceTableLayoutHashes.size() == 0);

  // Build resource table mapping
  int32 MaxBoundResourceTable = -1;
  std::vector<uint32> ResourceTableSRVs;
  std::vector<uint32> ResourceTableSamplerStates;
  std::vector<uint32> ResourceTableUAVs;

  for (auto MapIt : ResourceTableMap)
  {
    const std::string& Name = MapIt.first;
    const RBResourceTableEntry& Entry = MapIt.second;

    uint16 BufferIndex, BaseIndex, Size;
    if (ParameterMap.FindParameterAllocation(Name.c_str(), BufferIndex, BaseIndex, Size))
    {
      ParameterMap.RemoveParameterAllocation(Name.c_str());

      uint16 UniformBufferIndex = -1, UBBaseIndex, UBSize;
      if (ParameterMap.FindParameterAllocation(Entry.UniformBufferName.c_str(), UniformBufferIndex, UBBaseIndex, UBSize) == false)
      {
        UniformBufferIndex = UsedUniformBufferSlots.FindAndSetFirstZeroBit();
        ParameterMap.AddParameterAllocation(Entry.UniformBufferName.c_str(), UniformBufferIndex, 0, 0);
      }

      OutSRT.ResourceTableBits |= (1 << UniformBufferIndex);
      MaxBoundResourceTable = RBMath::get_max<int32>(MaxBoundResourceTable, (int32)UniformBufferIndex);

      while (OutSRT.ResourceTableLayoutHashes.size() <= MaxBoundResourceTable)
      {
        OutSRT.ResourceTableLayoutHashes.push_back(0);
      }
      OutSRT.ResourceTableLayoutHashes[UniformBufferIndex] = ResourceTableLayoutHashes.FindChecked(Entry.UniformBufferName);

      auto ResourceMap = FRHIResourceTableEntry::Create(UniformBufferIndex, Entry.ResourceIndex, BaseIndex);
      switch (Entry.Type)
      {
      case UBMT_TEXTURE:
        OutSRT.TextureMap.push_back(ResourceMap);
        break;
      case UBMT_SAMPLER:
        OutSRT.SamplerMap.push_back(ResourceMap);
        break;
      case UBMT_SRV:
        OutSRT.ShaderResourceViewMap.push_back(ResourceMap);
        break;
      case UBMT_UAV:
        OutSRT.UnorderedAccessViewMap.push_back(ResourceMap);
        break;
      default:
        CHECK(0);
      }
    }
  }

  OutSRT.MaxBoundResourceTable = MaxBoundResourceTable;
}

#endif

void CompileD3D11Shader(const FShaderCompilerInput& Input, FShaderCompilerOutput& Output, FShaderCompilerDefinitions& AdditionalDefines, const std::string& WorkingDirectory)
{
	std::string PreprocessedShaderSource;
	std::string CompilerPath;
	const char* ShaderProfile = GetShaderProfileName(Input.Target);

	if (!ShaderProfile)
	{
		Output.Errors.push_back(FShaderCompilerError(("Unrecognized shader frequency")));
		return;
	}

	// Set additional defines.
	AdditionalDefines.SetDefine(TEXT("COMPILER_HLSL"), 1);

	//read file content
	auto res_handle = g_res_manager->load_resource(Input.SourceFilename.c_str(), WIPResourceType::TEXT);

	// @TODO - currently d3d11 uses d3d10 shader compiler flags... update when this changes in DXSDK
	// @TODO - implement different material path to allow us to remove backwards compat flag on sm5 shaders
	uint32 CompileFlags = D3D10_SHADER_ENABLE_BACKWARDS_COMPATIBILITY
		// Unpack uniform matrices as row-major to match the CPU layout.
		| D3D10_SHADER_PACK_MATRIX_ROW_MAJOR;

	if (1)//是否开启Debug
	{
		CompileFlags |= D3D10_SHADER_DEBUG | D3D10_SHADER_SKIP_OPTIMIZATION;
	}
	else
	{
		if (1)//是否标准优化
		{
			CompileFlags |= D3D10_SHADER_OPTIMIZATION_LEVEL1;
		}
		else
		{
			CompileFlags |= D3D10_SHADER_OPTIMIZATION_LEVEL3;
		}
	}

	if (0)//CFLAG_PreferFlowControl or CFLAG_AvoidFlowControl
	{
		if (1)
		{
			CompileFlags |= D3D10_SHADER_PREFER_FLOW_CONTROL;
		}
		else
		{
			CompileFlags |= D3D10_SHADER_AVOID_FLOW_CONTROL;
		}

	}

	std::vector<std::string> FilteredErrors;
	{
		TRefCountPtr<ID3DBlob> Shader;
		TRefCountPtr<ID3DBlob> Errors;

		HRESULT Result;
		pD3DCompile D3DCompileFunc = GetD3DCompileFunc(CompilerPath);

		if (D3DCompileFunc)
		{
			bool bException = false;

			Result = D3DCompileWrapper(
				D3DCompileFunc,
				bException,
				res_handle->ptr,
				res_handle->size,
				Input.SourceFilename.c_str(),
				/*pDefines=*/ NULL,
				/*pInclude=*/ NULL,
				Input.EntryPointName.c_str(),
				ShaderProfile,
				CompileFlags,
				0,
				Shader.GetInitReference(),
				Errors.GetInitReference()
				);

			if (bException)
			{
				FilteredErrors.push_back(("D3DCompile exception"));
			}
		}
		else
		{
			char s[256];
			sprintf_s(s, 256, ("Couldn't find shader compiler: %s"), CompilerPath);
			FilteredErrors.push_back(s);
			Result = E_FAIL;
		}

		// Filter any errors.
		void* ErrorBuffer = Errors ? Errors->GetBufferPointer() : NULL;
		if (ErrorBuffer)
		{
			//D3D11FilterShaderCompileWarnings(ANSI_TO_TCHAR(ErrorBuffer), FilteredErrors);
		}

		//D3D11硬件不支持Double数据，如果使用了报错。
		// Fail the compilation if double operations are being used, since those are not supported on all D3D11 cards
		if (SUCCEEDED(Result))
		{
			TRefCountPtr<ID3DBlob> Dissasembly;

			if (SUCCEEDED(D3DDisassemble(Shader->GetBufferPointer(), Shader->GetBufferSize(), 0, "", Dissasembly.GetInitReference())))
			{
				char* DissasemblyString = new char[Dissasembly->GetBufferSize() + 1];
				::memcpy(DissasemblyString, Dissasembly->GetBufferPointer(), Dissasembly->GetBufferSize());
				DissasemblyString[Dissasembly->GetBufferSize()] = 0;
				std::string DissasemblyStringW(DissasemblyString);
				delete[] DissasemblyString;

				// dcl_globalFlags will contain enableDoublePrecisionFloatOps when the shader uses doubles, even though the docs on dcl_globalFlags don't say anything about this
				if (std::string::npos != DissasemblyStringW.find(TEXT("enableDoublePrecisionFloatOps")))
				{
					FilteredErrors.push_back(TEXT("Shader uses double precision floats, which are not supported on all D3D11 hardware!"));
					Result = E_FAIL;
				}
			}
		}

		// Write out the preprocessed file and a batch file to compile it if requested (DumpDebugInfoPath is valid)
		//这里仅仅只是输出错误过滤信息
		if (Input.DumpDebugInfoPath != TEXT(""))
		{
			/*
			for (auto i : FilteredErrors)
			g_logger->debug(WIP_INFO, i.c_str());
			*/

		}

		//shader之间传递的插值量信息
		int32 NumInterpolants = 0;
		std::vector<std::string> InterpolantNames;
		std::vector<std::string> ShaderInputs;
		if (SUCCEEDED(Result))
		{
			Output.bSucceeded = true;

			ID3D11ShaderReflection* Reflector = NULL;
      Result = D3DReflect(Shader->GetBufferPointer(), Shader->GetBufferSize(), (IID_ID3D11ShaderReflection), (void**)&Reflector);
			if (FAILED(Result))
			{
				g_logger->debug(WIP_ERROR, TEXT("D3DReflect failed: Result=%08x"), Result);
			}

			// Read the constant table description.
			D3D11_SHADER_DESC ShaderDesc;
			Reflector->GetDesc(&ShaderDesc);

			bool bGlobalUniformBufferUsed = false;
			uint32 NumSamplers = 0;
			uint32 NumSRVs = 0;
			uint32 NumCBs = 0;
			uint32 NumUAVs = 0;
			std::vector<std::string> UniformBufferNames;
			std::vector<std::string> ShaderOutputs;
			uint32 UsedUniformBufferSlots = 0;

			if (Input.Target == SF_Vertex)
			{
				for (uint32 Index = 0; Index < ShaderDesc.OutputParameters; ++Index)
				{
					D3D11_SIGNATURE_PARAMETER_DESC ParamDesc;
					Reflector->GetOutputParameterDesc(Index, &ParamDesc);
					if (ParamDesc.SystemValueType == D3D_NAME_UNDEFINED && ParamDesc.Mask != 0)
					{
						++NumInterpolants;
						char *buf = new char[1024];
						sprintf_s(buf, 1024, "%s%d", ParamDesc.SemanticName, ParamDesc.SemanticIndex);
						InterpolantNames.push_back(buf);
						ShaderOutputs.push_back(buf);
						//new(InterpolantNames)FString(FString::Printf(TEXT("%s%d"), ANSI_TO_TCHAR(ParamDesc.SemanticName), ParamDesc.SemanticIndex));
					}
				}
			}
			else if (Input.Target == SF_Pixel)
			{
				/*
				if (GD3DAllowRemoveUnused != 0 && Input.bCompilingForShaderPipeline)
				{
				// Handy place for a breakpoint for debugging...
				++GBreakpoint;
				}
				*/

				bool bFoundUnused = false;
				for (uint32 Index = 0; Index < ShaderDesc.InputParameters; ++Index)
				{
					D3D11_SIGNATURE_PARAMETER_DESC ParamDesc;
					Reflector->GetInputParameterDesc(Index, &ParamDesc);
					if (ParamDesc.SystemValueType == D3D_NAME_UNDEFINED)
					{
						if (ParamDesc.ReadWriteMask != 0)
						{
							std::string SemanticName = (ParamDesc.SemanticName);

							auto it = std::find(ShaderInputs.begin(), ShaderInputs.end(), SemanticName);
							if (it == ShaderInputs.end())
								ShaderInputs.push_back(SemanticName);

							// Add the number (for the case of TEXCOORD)
							char *str = new char[1024];
							sprintf_s(str, 1024, ("%s%d"), SemanticName, ParamDesc.SemanticIndex);
							std::string SemanticIndexName = str;

							it = std::find(ShaderInputs.begin(), ShaderInputs.end(), SemanticIndexName);
							if (it == ShaderInputs.end())
								ShaderInputs.push_back(SemanticIndexName);

							// Add _centroid
							std::string a = SemanticName + TEXT("_centroid");
							it = std::find(ShaderInputs.begin(), ShaderInputs.end(), a);
							if (it == ShaderInputs.end())
								ShaderInputs.push_back(a);
							//ShaderInputs.AddUnique();
							a = SemanticIndexName + TEXT("_centroid");
							it = std::find(ShaderInputs.begin(), ShaderInputs.end(), a);
							if (it == ShaderInputs.end())
								ShaderInputs.push_back(a);
							//ShaderInputs.AddUnique(SemanticIndexName + TEXT("_centroid"));
						}
						else
						{
							bFoundUnused = true;
						}
					}
					else
					{
						//if (ParamDesc.ReadWriteMask != 0)
						{
							std::string str = ParamDesc.SemanticName;
							auto it = std::find(ShaderInputs.begin(), ShaderInputs.end(), str);
							if (it == ShaderInputs.end())
								ShaderInputs.push_back(str);

							// Keep system values
							//ShaderInputs.AddUnique(FString(ANSI_TO_TCHAR(ParamDesc.SemanticName)));
						}
					}
				}

				// Add parameters for shader resources (constant buffers, textures, samplers, etc. */
				for (uint32 ResourceIndex = 0; ResourceIndex < ShaderDesc.BoundResources; ResourceIndex++)
				{
					D3D11_SHADER_INPUT_BIND_DESC BindDesc;
					Reflector->GetResourceBindingDesc(ResourceIndex, &BindDesc);

					if (BindDesc.Type == D3D10_SIT_CBUFFER || BindDesc.Type == D3D10_SIT_TBUFFER)
					{
						const uint32 CBIndex = BindDesc.BindPoint;
						ID3D11ShaderReflectionConstantBuffer* ConstantBuffer = Reflector->GetConstantBufferByName(BindDesc.Name);
						D3D11_SHADER_BUFFER_DESC CBDesc;
						ConstantBuffer->GetDesc(&CBDesc);
						bool bGlobalCB = (::strcmp(CBDesc.Name, "$Globals") == 0);

						if (bGlobalCB)
						{
							// Track all of the variables in this constant buffer.
							for (uint32 ConstantIndex = 0; ConstantIndex < CBDesc.Variables; ConstantIndex++)
							{
								ID3D11ShaderReflectionVariable* Variable = ConstantBuffer->GetVariableByIndex(ConstantIndex);
								D3D11_SHADER_VARIABLE_DESC VariableDesc;
								Variable->GetDesc(&VariableDesc);
								if (VariableDesc.uFlags & D3D10_SVF_USED)
								{
									bGlobalUniformBufferUsed = true;

									Output.ParameterMap.AddParameterAllocation(
										VariableDesc.Name,
										CBIndex,
										VariableDesc.StartOffset,
										VariableDesc.Size
										);

									UsedUniformBufferSlots |= CBIndex;
								}
							}
						}
						else
						{
							// Track just the constant buffer itself.
							Output.ParameterMap.AddParameterAllocation(
								CBDesc.Name,
								CBIndex,
								0,
								0
								);
							UsedUniformBufferSlots |= CBIndex;

							if (UniformBufferNames.size() <= (int32)CBIndex)
							{
								uint32 n = CBIndex - UniformBufferNames.size() + 1;
								for (int i = 0; i < n; ++i)
									UniformBufferNames.push_back("");
							}
							UniformBufferNames[CBIndex] = CBDesc.Name;
						}

						NumCBs = RBMath::get_max(NumCBs, BindDesc.BindPoint + BindDesc.BindCount);
					}
					else if (BindDesc.Type == D3D10_SIT_TEXTURE || BindDesc.Type == D3D10_SIT_SAMPLER)
					{
						TCHAR OfficialName[1024];
						uint32 BindCount = BindDesc.BindCount;
						::strcpy(OfficialName, BindDesc.Name);

						if (1/*SP_PCD3D_SM5*/)
						{
							BindCount = 1;
							//处理数组，略过
							// Assign the name and optionally strip any "[#]" suffixes
							char *BracketLocation = ::strchr(OfficialName, TEXT('['));
							if (BracketLocation)
							{
								*BracketLocation = 0;

								const int32 NumCharactersBeforeArray = BracketLocation - OfficialName;

								// In SM5, for some reason, array suffixes are included in Name, i.e. "LightMapTextures[0]", rather than "LightMapTextures"
								// Additionally elements in an array are listed as SEPERATE bound resources.
								// However, they are always contiguous in resource index, so iterate over the samplers and textures of the initial association
								// and count them, identifying the bindpoint and bindcounts

								while (ResourceIndex + 1 < ShaderDesc.BoundResources)
								{
									D3D11_SHADER_INPUT_BIND_DESC BindDesc2;
									Reflector->GetResourceBindingDesc(ResourceIndex + 1, &BindDesc2);

									if (BindDesc2.Type == BindDesc.Type && ::strncmp(BindDesc2.Name, BindDesc.Name, NumCharactersBeforeArray) == 0)
									{
										BindCount++;
										// Skip over this resource since it is part of an array
										ResourceIndex++;
									}
									else
									{
										break;
									}
								}
							}
						}

						if (BindDesc.Type == D3D10_SIT_SAMPLER)
						{
							NumSamplers = RBMath::get_max(NumSamplers, BindDesc.BindPoint + BindDesc.BindCount);
						}
						else if (BindDesc.Type == D3D10_SIT_TEXTURE)
						{
							NumSRVs = RBMath::get_max(NumSRVs, BindDesc.BindPoint + BindDesc.BindCount);
						}

						// Add a parameter for the texture only, the sampler index will be invalid
						Output.ParameterMap.AddParameterAllocation(
							OfficialName,
							0,
							BindDesc.BindPoint,
							BindCount
							);
					}
					else if (BindDesc.Type == D3D11_SIT_UAV_RWTYPED || BindDesc.Type == D3D11_SIT_UAV_RWSTRUCTURED ||
						BindDesc.Type == D3D11_SIT_UAV_RWBYTEADDRESS || BindDesc.Type == D3D11_SIT_UAV_RWSTRUCTURED_WITH_COUNTER ||
						BindDesc.Type == D3D11_SIT_UAV_APPEND_STRUCTURED)
					{
						TCHAR OfficialName[1024];
						::strcpy(OfficialName, BindDesc.Name);

						Output.ParameterMap.AddParameterAllocation(
							OfficialName,
							0,
							BindDesc.BindPoint,
							1
							);

						NumUAVs = RBMath::get_max(NumUAVs, BindDesc.BindPoint + BindDesc.BindCount);
					}
					else if (BindDesc.Type == D3D11_SIT_STRUCTURED || BindDesc.Type == D3D11_SIT_BYTEADDRESS)
					{
						TCHAR OfficialName[1024];
						::strcpy(OfficialName, BindDesc.Name);

						Output.ParameterMap.AddParameterAllocation(
							OfficialName,
							0,
							BindDesc.BindPoint,
							1
							);

						NumSRVs = RBMath::get_max(NumSRVs, BindDesc.BindPoint + BindDesc.BindCount);
					}
				}

				TRefCountPtr<ID3DBlob> CompressedData;

				//是否在编译code中保存调试信息
				bool keepDebugInfo = true;
				if (keepDebugInfo)
				{
					CompressedData = Shader;
				}
				else
				{
					// Strip shader reflection and debug info
					D3D_SHADER_DATA ShaderData;
					ShaderData.pBytecode = Shader->GetBufferPointer();
					ShaderData.BytecodeLength = Shader->GetBufferSize();
					Result = D3DStripShader(Shader->GetBufferPointer(),
						Shader->GetBufferSize(),
						D3DCOMPILER_STRIP_REFLECTION_DATA | D3DCOMPILER_STRIP_DEBUG_INFO | D3DCOMPILER_STRIP_TEST_BLOBS,
						CompressedData.GetInitReference());

					if (FAILED(Result))
					{
						g_logger->debug(WIP_ERROR, TEXT("D3DStripShader failed: Result=%08x"), Result);
					}
				}

#if 0

				// Build the SRT for this shader.
				RBD3D11ShaderResourceTable SRT;
				{
					// Build the generic SRT for this shader.
					RBShaderCompilerResourceTable GenericSRT;
					BuildResourceTableMapping(Input.Environment.ResourceTableMap, Input.Environment.ResourceTableLayoutHashes, UsedUniformBufferSlots, Output.ParameterMap, GenericSRT);

					// At runtime textures are just SRVs, so combine them for the purposes of building token streams.
					GenericSRT.ShaderResourceViewMap.Append(GenericSRT.TextureMap);

					// Copy over the bits indicating which resource tables are active.
					SRT.ResourceTableBits = GenericSRT.ResourceTableBits;

					SRT.ResourceTableLayoutHashes = GenericSRT.ResourceTableLayoutHashes;

					// Now build our token streams.
					BuildResourceTableTokenStream(GenericSRT.ShaderResourceViewMap, GenericSRT.MaxBoundResourceTable, SRT.ShaderResourceViewMap);
					BuildResourceTableTokenStream(GenericSRT.SamplerMap, GenericSRT.MaxBoundResourceTable, SRT.SamplerMap);
					BuildResourceTableTokenStream(GenericSRT.UnorderedAccessViewMap, GenericSRT.MaxBoundResourceTable, SRT.UnorderedAccessViewMap);

				}

				FMemoryWriter Ar(Output.Code, true);
				Ar << SRT;
				Ar.Serialize(CompressedData->GetBufferPointer(), CompressedData->GetBufferSize());
#endif
				// Pack bGlobalUniformBufferUsed and resource counts in the last few bytes
				Output.Code.push_back(bGlobalUniformBufferUsed);
				Output.Code.push_back(NumSamplers);
				Output.Code.push_back(NumSRVs);
				Output.Code.push_back(NumCBs);
				Output.Code.push_back(NumUAVs);

				// Set the number of instructions.
				Output.NumInstructions = ShaderDesc.InstructionCount;

				Output.NumTextureSamplers = NumSamplers;

				// Reflector is a com interface, so it needs to be released.
				Reflector->Release();

				// Pass the target through to the output.
				Output.Target = Input.Target;
			}

			if (FAILED(Result) && !FilteredErrors.size())
			{
				FilteredErrors.push_back(TEXT("Compile Failed without errors!"));
			}

			for (int32 ErrorIndex = 0; ErrorIndex < FilteredErrors.size(); ErrorIndex++)
			{
				const std::string& CurrentError = FilteredErrors[ErrorIndex];
				FShaderCompilerError NewError;
				// Extract the filename and line number from the shader compiler error message for PC whose format is:
				// "d:\UnrealEngine3\Binaries\BasePassPixelShader(30,7): error X3000: invalid target or usage string"
				int32 FirstParenIndex = CurrentError.find(TEXT("("));
				int32 LastParenIndex = CurrentError.find(TEXT("):"));
				if (FirstParenIndex != std::string::npos
					&& LastParenIndex != std::string::npos
					&& LastParenIndex > FirstParenIndex)
				{
					/*
					std::string ErrorFileAndPath = CurrentError.Left(FirstParenIndex);
					if (FPaths::GetExtension(ErrorFileAndPath).ToUpper() == TEXT("USF"))
					{
					NewError.ErrorFile = FPaths::GetCleanFilename(ErrorFileAndPath);
					}
					else
					{
					NewError.ErrorFile = FPaths::GetCleanFilename(ErrorFileAndPath) + TEXT(".usf");
					}

					NewError.ErrorLineString = CurrentError.Mid(FirstParenIndex + 1, LastParenIndex - FirstParenIndex - FCString::Strlen(TEXT("(")));
					NewError.StrippedErrorMessage = CurrentError.Right(CurrentError.Len() - LastParenIndex - FCString::Strlen(TEXT("):")));
					*/
					NewError.StrippedErrorMessage = CurrentError;
				}
				else
				{
					NewError.StrippedErrorMessage = CurrentError;
				}
				Output.Errors.push_back(NewError);
			}
		}
	}
}

void CompileShader_Windows_SM5(const FShaderCompilerInput& Input, FShaderCompilerOutput& Output, const std::string& WorkingDirectory)
{
	FShaderCompilerDefinitions AdditionalDefines;
	AdditionalDefines.SetDefine(TEXT("SM5_PROFILE"), 1);
	CompileD3D11Shader(Input, Output, AdditionalDefines, WorkingDirectory);
}


class FShaderFormatD3D : public IShaderFormat
{
	virtual void CompileShader(std::string Format, const FShaderCompilerInput& Input, FShaderCompilerOutput& Output, const std::string& WorkingDirectory) const
	{
		CompileShader_Windows_SM5(Input, Output, WorkingDirectory);
	}
};