#pragma once

#ifdef _DEBUG
#define PRINT printf
#else
#define PRINT(...) void()
#endif 

#ifdef GLSLCOMPILER_EXPORT
#define DLL_DECLARE __declspec(dllexport)
#else 
#define DLL_DECLARE __declspec(dllimport)
#endif

#include <memory>
#include <string>
#include <vector>

enum DLL_DECLARE ShaderStage {
	eVertex = 0,
	eTessControl,
	eTessEvaluation,
	eGeometry,
	eFragment = 4,
	eCompute,
	eRayGenNV,		//unuse
	eIntersectNV,		//unuse
	eAnyHitNV,		//unuse
	eClosestHitNV,	//unuse
	eMissNV,			//unuse
	eCallableNV,		//unuse
	eTaskNV,			//unuse
	eMeshNV,			//unuse
	eCount,			//unuse
	eUndefine
};

struct DLL_DECLARE ShaderDetail {
	struct Uniform {
		uint32_t set;
		uint32_t binding;
		uint32_t size;
		enum {
			eUniformBlock,
			eCombined
		} type;
		ShaderStage stage;
		std::string name;
	};
	std::vector<Uniform> uniforms;
	struct Attribute {
		uint32_t location;
		uint32_t size;
		enum {
			eIn, eOut
		} io;
		ShaderStage stage;
		std::string name;
	};
	std::vector<Attribute> pipeline_io;
};

struct PrivateData;

class DLL_DECLARE ShaderCompiler {
public:
	ShaderCompiler(const char* vertex_shader, const char* fragment_shader);
	~ShaderCompiler();
	bool Compile();
	bool Link();
	void Reflect();
	bool GenSpv(std::vector<uint32_t>& vertex, std::vector<uint32_t>& fragment);
	std::unique_ptr<ShaderDetail> GenDetail();

private:
	bool compile(const char* shader, ShaderStage stage);
	bool LoadFile(const char* file, std::vector<char>& out);
private:
	std::unique_ptr<PrivateData> data_;
	bool use_vertex_shader = false;
	bool use_fragment_shader = false;
};

std::vector<uint32_t> DLL_DECLARE GlslToSpv(const char* filename, ShaderStage stage);
std::unique_ptr<ShaderDetail> DLL_DECLARE GenGlslDetail(const char* vertex, const char* fragment);