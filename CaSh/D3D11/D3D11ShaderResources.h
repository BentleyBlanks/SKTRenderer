#pragma once
#include "RefCount.h"
#include "d3d11.h"
#include "d3dcompiler.h"
#include "ResourceManager.h"
#include "Logger.h"
#include "ShaderAssets.h"


typedef HRESULT (WINAPI *pD3DReflect)(_In_reads_bytes_(SrcDataSize) LPCVOID pSrcData,
_In_ SIZE_T SrcDataSize,
_In_ REFIID pInterface,
_Out_ void** ppReflector);

class D3D11ShaderCompiler
{
private:
  static void show_error_message(ID3D10Blob* error, const char* filename)
  {
    char* e = (char*)(error->GetBufferPointer());
    unsigned long bs = error->GetBufferSize();
    std::ofstream fout;

    fout.open("error.txt");


    fout << filename << " compile error : " << std::endl;
    for (unsigned long i = 0; i < bs; ++i)
    {
      fout << e[i];
    }

    fout << std::endl << "==================" << std::endl;

    error->Release();
    error = 0;

  }

public:
  static bool load_compiler(const char* compiler_path)
  {
    CompilerDLL = LoadLibrary(compiler_path);
    if (CompilerDLL)
    {
      compile_shader = (pD3DCompile)(void*)GetProcAddress(CompilerDLL, "D3DCompile");
      if (!compile_shader)
      {
        g_logger->debug_print(WIP_ERROR, "load function D3DCompile from %s failed!", compiler_path);
        return false;
      }
      reflect_shader = (pD3DReflect)(void*)GetProcAddress(CompilerDLL, "D3DReflect");
      if (!reflect_shader)
      {
        g_logger->debug_print(WIP_ERROR, "load function D3DReflect from %s failed!", compiler_path);
        return false;
      }
    }
    else
    {
      g_logger->debug_print(WIP_ERROR, "LoadLibrary %s failed!", compiler_path);
      return false;
    }
    return true;
  }

  static TRefCountPtr<ID3DBlob> compile(ID3D11Device* device, const char* shader_path, const char* entry, const char* shader_model)
  {
    if (!CompilerDLL)
    {
      g_logger->debug(WIP_WARNING, "Shader编译器没有初始化，请勿使用！");
      getchar();
    }
    FShaderCompilerOutput Output;
    if (!device) return nullptr;
    /** load shaders */
    {
      uint32 CompileFlags = D3D10_SHADER_ENABLE_BACKWARDS_COMPATIBILITY
        | D3D10_SHADER_PACK_MATRIX_ROW_MAJOR;
      if (0)//是否开启Debug
        CompileFlags |= D3D10_SHADER_DEBUG | D3D10_SHADER_SKIP_OPTIMIZATION;
      else
        if (1)//是否标准优化
          CompileFlags |= D3D10_SHADER_OPTIMIZATION_LEVEL1;
        else
          CompileFlags |= D3D10_SHADER_OPTIMIZATION_LEVEL3;
      if (1)//CFLAG_PreferFlowControl or CFLAG_AvoidFlowControl
        if (0)
          CompileFlags |= D3D10_SHADER_PREFER_FLOW_CONTROL;
        else
          CompileFlags |= D3D10_SHADER_AVOID_FLOW_CONTROL;

      auto res_handle = g_res_manager->load_resource(shader_path, WIPResourceType::TEXT);
      if (!res_handle)
      {
        g_logger->debug_print(WIP_ERROR, "load file %s failed!", shader_path);
        return nullptr;
      }
      TRefCountPtr<ID3DBlob> Shader;
      TRefCountPtr<ID3DBlob> Errors;

      HRESULT Result;

      if (compile_shader)
      {
        bool bException = false;

        Result = compile_shader(
          ((std::string*)res_handle->ptr)->data(),
          ((std::string*)res_handle->ptr)->length(),
          shader_path,
          /*pDefines=*/ NULL,
          /*pInclude=*/ NULL,
          entry,
          shader_model,
          CompileFlags,
          0,
          Shader.GetInitReference(),
          Errors.GetInitReference()
          );


#ifdef SHADER_REFLECT_
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
          }
        }
        //shader之间传递的插值量信息
        int32 NumInterpolants = 0;
        std::vector<std::string> InterpolantNames;
        std::vector<std::string> ShaderInputs;
        if (SUCCEEDED(Result))
        {
          Output.bSucceeded = true;

          ID3D11ShaderReflection* Reflector = NULL;
          Result = reflect_shader(Shader->GetBufferPointer(), Shader->GetBufferSize(), (IID_ID3D11ShaderReflection), (void**)&Reflector);
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

          if (shader_model[0] == 'v')
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
          else if (shader_model[0] == 'p')
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
                  char str[1024];
                  sprintf(str, ("%s%d"), SemanticName.c_str(), ParamDesc.SemanticIndex);
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

          }
        }
#endif //SHADER_REFLECT_

        if (FAILED(Result))
        {
          if (Errors)
          {
            show_error_message(Errors.GetReference(), shader_path);
          }
          assert(!"shader compile failed!");
        }
        else
        {
          return Shader;
        }

      }
      else
      {
        g_logger->debug_print(WIP_ERROR, "compile_shader failed!\n");
      }
      g_res_manager->free(res_handle, res_handle->size);
    }
	return nullptr;
  }

  static void unload()
  {
    if (CompilerDLL)
      FreeLibrary(CompilerDLL);
    CompilerDLL = 0;
    compile_shader = nullptr;
  }



  static   pD3DCompile compile_shader;
  static pD3DReflect reflect_shader;
  static HMODULE CompilerDLL;

};