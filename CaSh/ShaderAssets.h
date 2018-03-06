#pragma once
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include "RBBasedata.h"
#include "RHIDef.h"

/**
* A map of shader parameter names to registers allocated to that parameter.
*/
class FShaderParameterMap
{
public:

	FShaderParameterMap()
	{}

	bool FindParameterAllocation(const char* ParameterName, uint16& OutBufferIndex, uint16& OutBaseIndex, uint16& OutSize) const
	{
		auto it = ParameterMap.find(ParameterName);
		const FParameterAllocation* Allocation = it == ParameterMap.end() ? nullptr : &(it->second);
		if (Allocation)
		{
			OutBufferIndex = Allocation->BufferIndex;
			OutBaseIndex = Allocation->BaseIndex;
			OutSize = Allocation->Size;

			if (Allocation->bBound)
			{
				// Can detect copy-paste errors in binding parameters.  Need to fix all the false positives before enabling.
				//UE_LOG(LogShaders, Warning, TEXT("Parameter %s was bound multiple times. Code error?"), ParameterName);
			}

			Allocation->bBound = true;
			return true;
		}
		else
		{
			return false;
		}
	}

	bool ContainsParameterAllocation(const char* ParameterName) const
	{
		auto it = ParameterMap.find(ParameterName);
		return it != ParameterMap.end();
	}

	void AddParameterAllocation(const char* ParameterName, uint16 BufferIndex, uint16 BaseIndex, uint16 Size)
	{
		FParameterAllocation Allocation;
		Allocation.BufferIndex = BufferIndex;
		Allocation.BaseIndex = BaseIndex;
		Allocation.Size = Size;
		ParameterMap[ParameterName] = Allocation;
	}

	void RemoveParameterAllocation(const char* ParameterName)
	{
		ParameterMap.erase(ParameterName);
	}

	/** Checks that all parameters are bound and asserts if any aren't in a debug build
	* @param InVertexFactoryType can be 0
	*/
	void VerifyBindingsAreComplete(const char* ShaderTypeName, EShaderFrequency Frequency, class FVertexFactoryType* InVertexFactoryType) const;

	/** Updates the hash state with the contents of this parameter map. */
	//void UpdateHash(FSHA1& HashState) const;

	/*
	friend FArchive& operator<<(FArchive& Ar, FShaderParameterMap& InParameterMap)
	{
	// Note: this serialize is used to pass between UE4 and the shader compile worker, recompile both when modifying
	return Ar << InParameterMap.ParameterMap;
	}
	*/

private:
	struct FParameterAllocation
	{
		uint16 BufferIndex;
		uint16 BaseIndex;
		uint16 Size;
		mutable bool bBound;

		FParameterAllocation() :
			bBound(false)
		{}
		/*
		friend FArchive& operator<<(FArchive& Ar, FParameterAllocation& Allocation)
		{
		return Ar << Allocation.BufferIndex << Allocation.BaseIndex << Allocation.Size << Allocation.bBound;
		}
		*/
	};

	std::map<std::string, FParameterAllocation> ParameterMap;
};

struct FShaderCompilerError
{
	FShaderCompilerError(const char* InStrippedErrorMessage = (""))
		: ErrorFile((""))
		, ErrorLineString((""))
		, StrippedErrorMessage(InStrippedErrorMessage)
	{}

	std::string ErrorFile;
	std::string ErrorLineString;
	std::string StrippedErrorMessage;

	std::string GetErrorString() const
	{
		return ErrorFile + ("(") + ErrorLineString + ("): ") + StrippedErrorMessage;
	}
};


struct FShaderCompilerInput
{
	EShaderFrequency Target;
	std::string ShaderFormat;
	std::string SourceFilePrefix;
	std::string SourceFilename;
	std::string EntryPointName;
	std::string DumpDebugInfoPath;
};

struct FShaderCompilerOutput
{
	EShaderFrequency Target;
	std::vector<FShaderCompilerError> Errors;
	std::vector<uint8> Code;
	uint32 NumInstructions;
	uint32 NumTextureSamplers;
	FShaderParameterMap ParameterMap;
	bool bSucceeded;
};

/**
* IShaderFormat, shader pre-compilation abstraction
*/
class IShaderFormat
{
public:

	/**
	* Compile the specified shader.
	*
	* @param Format The desired format
	* @param Input The input to the shader compiler.
	* @param Output The output from shader compiler.
	* @param WorkingDirectory The working directory.
	*/
	virtual void CompileShader(std::string Format, const FShaderCompilerInput& Input, FShaderCompilerOutput& Output, const std::string& WorkingDirectory) const = 0;

	/**
	* Gets the current version of the specified shader format.
	*
	* @param Format The format to get the version for.
	* @return Version number.
	*/
	//virtual uint16 GetVersion(std::string Format) const = 0;

	/**
	* Gets the list of supported formats.
	*
	* @param OutFormats Will hold the list of formats.
	*/
	//virtual void GetSupportedFormats(std::vector<std::string>& OutFormats) const = 0;

public:

	/** Virtual destructor. */
	virtual ~IShaderFormat() { }
};

/** Container for shader compiler definitions. */
class FShaderCompilerDefinitions
{
public:

	FShaderCompilerDefinitions()
	{
		// Presize to reduce re-hashing while building shader jobs
		//Definitions.Empty(50);
		Definitions.clear();
	}

	/**
	* Works for TCHAR
	* e.g. SetDefine(TEXT("NUM_SAMPLES"), TEXT("1"));
	*/
	void SetDefine(const char* Name, const char* Value)
	{
		Definitions.insert(std::pair<std::string, std::string>(Name, Value));
	}

	/**
	* Works for uint32 and bool
	* e.g. OutEnvironment.SetDefine(TEXT("REALLY"), bReally);
	* e.g. OutEnvironment.SetDefine(TEXT("NUM_SAMPLES"), NumSamples);
	*/
	void SetDefine(const char* Name, uint32 Value)
	{
		char s[64];
		sprintf_s(s, 64, "%u", Value);
		Definitions.insert(std::pair<std::string, std::string>(Name, s));
	}

	/**
	* Works for float
	*/
	void SetFloatDefine(const char* Name, float Value)
	{
		// can be optimized
		char s[64];
		sprintf_s(s, 64, "%f", Value);
		Definitions.insert(std::pair<std::string,std::string> (Name, s));
	}

	const std::map<std::string, std::string>& GetDefinitionMap() const
	{
		return Definitions;
	}

	/*
	friend FArchive& operator<<(FArchive& Ar, FShaderCompilerDefinitions& Defs)
	{
		return Ar << Defs.Definitions;
	}
	*/

	void Merge(const FShaderCompilerDefinitions& Other)
	{
		for (auto i = Other.Definitions.begin(); i != Other.Definitions.end(); ++i)
		{
			if (Definitions.find(i->first) == Definitions.end())
			{
				Definitions[i->first] = i->second;
			}
		}
	}

private:

	/** Map: definition -> value. */
	std::map<std::string, std::string> Definitions;
};

