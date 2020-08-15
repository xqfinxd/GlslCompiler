#include "GlslCompiler.h"
#include <SPIRV/GlslangToSpv.h>
#include <glslang/Public/ShaderLang.h>

struct PrivateData {
	std::unique_ptr<glslang::TShader> fragment_shader_;
	std::unique_ptr<glslang::TShader> vertex_shader_;
	std::unique_ptr<glslang::TProgram> program_ = nullptr;
	const char* vertex_filename_ = nullptr;
	const char* fragment_filename_ = nullptr;
};

namespace {
	const int kGlslVersion = 400;
	const glslang::EShSource kSourceLanguage = glslang::EShSourceGlsl;
	const glslang::EShClient kClient = glslang::EShClientVulkan;
	const glslang::EShTargetClientVersion kClientVersion = glslang::EShTargetVulkan_1_1;
	const glslang::EShTargetLanguage kTargetLanguage = glslang::EShTargetSpv;
	const glslang::EShTargetLanguageVersion kTargetLanguageVersion = glslang::EShTargetSpv_1_2;
	const EShMessages kMessages = (EShMessages)(EShMsgSpvRules | EShMsgVulkanRules | EShMsgAST);
	void InitResources(TBuiltInResource& res) {
		res.maxLights = 32;
		res.maxClipPlanes = 6;
		res.maxTextureUnits = 32;
		res.maxTextureCoords = 32;
		res.maxVertexAttribs = 64;
		res.maxVertexUniformComponents = 4096;
		res.maxVaryingFloats = 64;
		res.maxVertexTextureImageUnits = 32;
		res.maxCombinedTextureImageUnits = 80;
		res.maxTextureImageUnits = 32;
		res.maxFragmentUniformComponents = 4096;
		res.maxDrawBuffers = 32;
		res.maxVertexUniformVectors = 128;
		res.maxVaryingVectors = 8;
		res.maxFragmentUniformVectors = 16;
		res.maxVertexOutputVectors = 16;
		res.maxFragmentInputVectors = 15;
		res.minProgramTexelOffset = -8;
		res.maxProgramTexelOffset = 7;
		res.maxClipDistances = 8;
		res.maxComputeWorkGroupCountX = 65535;
		res.maxComputeWorkGroupCountY = 65535;
		res.maxComputeWorkGroupCountZ = 65535;
		res.maxComputeWorkGroupSizeX = 1024;
		res.maxComputeWorkGroupSizeY = 1024;
		res.maxComputeWorkGroupSizeZ = 64;
		res.maxComputeUniformComponents = 1024;
		res.maxComputeTextureImageUnits = 16;
		res.maxComputeImageUniforms = 8;
		res.maxComputeAtomicCounters = 8;
		res.maxComputeAtomicCounterBuffers = 1;
		res.maxVaryingComponents = 60;
		res.maxVertexOutputComponents = 64;
		res.maxGeometryInputComponents = 64;
		res.maxGeometryOutputComponents = 128;
		res.maxFragmentInputComponents = 128;
		res.maxImageUnits = 8;
		res.maxCombinedImageUnitsAndFragmentOutputs = 8;
		res.maxCombinedShaderOutputResources = 8;
		res.maxImageSamples = 0;
		res.maxVertexImageUniforms = 0;
		res.maxTessControlImageUniforms = 0;
		res.maxTessEvaluationImageUniforms = 0;
		res.maxGeometryImageUniforms = 0;
		res.maxFragmentImageUniforms = 8;
		res.maxCombinedImageUniforms = 8;
		res.maxGeometryTextureImageUnits = 16;
		res.maxGeometryOutputVertices = 256;
		res.maxGeometryTotalOutputComponents = 1024;
		res.maxGeometryUniformComponents = 1024;
		res.maxGeometryVaryingComponents = 64;
		res.maxTessControlInputComponents = 128;
		res.maxTessControlOutputComponents = 128;
		res.maxTessControlTextureImageUnits = 16;
		res.maxTessControlUniformComponents = 1024;
		res.maxTessControlTotalOutputComponents = 4096;
		res.maxTessEvaluationInputComponents = 128;
		res.maxTessEvaluationOutputComponents = 128;
		res.maxTessEvaluationTextureImageUnits = 16;
		res.maxTessEvaluationUniformComponents = 1024;
		res.maxTessPatchComponents = 120;
		res.maxPatchVertices = 32;
		res.maxTessGenLevel = 64;
		res.maxViewports = 16;
		res.maxVertexAtomicCounters = 0;
		res.maxTessControlAtomicCounters = 0;
		res.maxTessEvaluationAtomicCounters = 0;
		res.maxGeometryAtomicCounters = 0;
		res.maxFragmentAtomicCounters = 8;
		res.maxCombinedAtomicCounters = 8;
		res.maxAtomicCounterBindings = 1;
		res.maxVertexAtomicCounterBuffers = 0;
		res.maxTessControlAtomicCounterBuffers = 0;
		res.maxTessEvaluationAtomicCounterBuffers = 0;
		res.maxGeometryAtomicCounterBuffers = 0;
		res.maxFragmentAtomicCounterBuffers = 1;
		res.maxCombinedAtomicCounterBuffers = 1;
		res.maxAtomicCounterBufferSize = 16384;
		res.maxTransformFeedbackBuffers = 4;
		res.maxTransformFeedbackInterleavedComponents = 64;
		res.maxCullDistances = 8;
		res.maxCombinedClipAndCullDistances = 8;
		res.maxSamples = 4;
		res.limits.nonInductiveForLoops = 1;
		res.limits.whileLoops = 1;
		res.limits.doWhileLoops = 1;
		res.limits.generalUniformIndexing = 1;
		res.limits.generalAttributeMatrixVectorIndexing = 1;
		res.limits.generalVaryingIndexing = 1;
		res.limits.generalSamplerIndexing = 1;
		res.limits.generalVariableIndexing = 1;
		res.limits.generalConstantMatrixVectorIndexing = 1;
	}

	ShaderStage GetStageFromMask(EShLanguageMask mask) {
		switch (mask) {
		case EShLanguageMask::EShLangVertexMask:
			return ShaderStage::eVertex;
			break;
		case EShLanguageMask::EShLangTessControlMask:
			return ShaderStage::eTessControl;
			break;
		case EShLanguageMask::EShLangTessEvaluationMask:
			return ShaderStage::eTessEvaluation;
			break;
		case EShLanguageMask::EShLangGeometryMask:
			return ShaderStage::eGeometry;
			break;
		case EShLanguageMask::EShLangFragmentMask:
			return ShaderStage::eFragment;
			break;
		case EShLanguageMask::EShLangComputeMask:
			return ShaderStage::eCompute;
			break;
		case EShLanguageMask::EShLangRayGenNVMask:
			return ShaderStage::eRayGenNV;
			break;
		case EShLanguageMask::EShLangIntersectNVMask:
			return ShaderStage::eIntersectNV;
			break;
		case EShLanguageMask::EShLangAnyHitNVMask:
			return ShaderStage::eAnyHitNV;
			break;
		case EShLanguageMask::EShLangClosestHitNVMask:
			return ShaderStage::eClosestHitNV;
			break;
		case EShLanguageMask::EShLangMissNVMask:
			return ShaderStage::eMissNV;
			break;
		case EShLanguageMask::EShLangCallableNVMask:
			return ShaderStage::eCallableNV;
			break;
		case EShLanguageMask::EShLangTaskNVMask:
			return ShaderStage::eTaskNV;
			break;
		case EShLanguageMask::EShLangMeshNVMask:
			return ShaderStage::eMeshNV;
			break;
		default:
			break;
		}
		return ShaderStage::eUndefine;
	}
};

ShaderCompiler::ShaderCompiler(const char* vertex_shader, const char* fragment_shader) {
	glslang::InitializeProcess();
	data_ = std::make_unique<PrivateData>();
	if (vertex_shader != nullptr) {
		use_vertex_shader = true;
		data_->vertex_shader_ = std::make_unique<glslang::TShader>(EShLangVertex);
	}
	if (fragment_shader != nullptr) {
		use_fragment_shader = true;
		data_->fragment_shader_ = std::make_unique<glslang::TShader>(EShLangFragment);
	}
	data_->vertex_filename_ = vertex_shader;
	data_->fragment_filename_ = fragment_shader;
	data_->program_ = std::make_unique<glslang::TProgram>();
}

ShaderCompiler::~ShaderCompiler() {
	glslang::FinalizeProcess();
	data_.release();
}

bool ShaderCompiler::LoadFile(const char * file, std::vector<char>& out) {
	FILE* input = fopen(file, "r");
	if (input == nullptr) {
		PRINT("open file [%s] failure.\n", file);
		return false;
	}
	fseek(input, 0, SEEK_END);
	size_t size = ftell(input);
	if (size <= 0) {
		PRINT("file [%s] is empty.\n", file);
		return false;
	}
	out.resize(size + 1);
	fseek(input, 0, SEEK_SET);
	fread(out.data(), size, sizeof(char), input);
	out[size] = '\0';
	fclose(input);
	return true;
}

bool ShaderCompiler::compile(const char * filename, ShaderStage stage) {
	if (filename == nullptr) {
		return false;
	}
	TBuiltInResource resource;
	InitResources(resource);
	std::vector<char> content = std::vector<char>();
	if (!LoadFile(filename, content)) {
		PRINT("load [%s] failure, this shader has ignored.\n", filename);
		return false;
	}
	const char * const shader_strs[] = {
		content.data()
	};
	auto& shader = (stage == eVertex) ? data_->vertex_shader_ : data_->fragment_shader_;
	shader->setStrings(shader_strs, 1);
	shader->setEntryPoint("main");
	shader->setEnvInput(kSourceLanguage, static_cast<EShLanguage>(stage), kClient, kGlslVersion);
	shader->setEnvClient(kClient, kClientVersion);
	shader->setEnvTarget(kTargetLanguage, kTargetLanguageVersion);
	if (!shader->parse(&resource, kGlslVersion, false, kMessages)) {
		PRINT("compile [%s] failure, this shader has ignored.\n", filename);
		PRINT(shader->getInfoLog());
		return false;
	}
	return true;
}

bool ShaderCompiler::Compile() {
	bool ret = true;
	if (use_vertex_shader) {
		if (!compile(data_->vertex_filename_, eVertex)) {
			ret = false;
		}
	}
	if (use_fragment_shader) {
		if (!compile(data_->fragment_filename_, eFragment)) {
			ret = false;
		}
	}
	return ret;
}

bool ShaderCompiler::Link() {
	if (use_vertex_shader && data_->vertex_shader_) {
		data_->program_->addShader(data_->vertex_shader_.get());
	}
	if (use_fragment_shader && data_->fragment_shader_) {
		data_->program_->addShader(data_->fragment_shader_.get());
	}
	if (use_vertex_shader || use_fragment_shader) {
		if (!data_->program_->link(kMessages)) {
			PRINT("program link failure, these shaders has ignored.\n");
			return false;
		}
		return true;
	}
	return false;
}

bool ShaderCompiler::GenSpv(std::vector<uint32_t>& vertex, std::vector<uint32_t>& fragment) {
	if (use_vertex_shader) {
		glslang::GlslangToSpv(*data_->program_->getIntermediate(static_cast<EShLanguage>(eVertex)), vertex);
		if (vertex.size() <= 0) {
			PRINT("vertex shader [%s] generate failure.\n", data_->vertex_filename_);
		} else {
			PRINT("vertex shader [%s] generate file success, data size : %d.\n", data_->vertex_filename_, vertex.size());
		}
	}
	if (use_fragment_shader) {
		glslang::GlslangToSpv(*data_->program_->getIntermediate(static_cast<EShLanguage>(eFragment)), fragment);
		if (fragment.size() <= 0) {
			PRINT("fragment shader [%s] generate failure.\n", data_->fragment_filename_);
		} else {
			PRINT("fragment shader [%s] generate file success, data size : %d.\n", data_->fragment_filename_, fragment.size());
		}
	}
	return vertex.size() > 0 && fragment.size() > 0;
}

std::unique_ptr<ShaderDetail> ShaderCompiler::GenDetail() {
	std::unique_ptr<ShaderDetail> detail = std::make_unique<ShaderDetail>();
	//uniform, uniformblock, attribute
	//uniform - sampler
	for (int32_t i = 0; i < data_->program_->getNumUniformVariables(); i++) {
		auto ttype = data_->program_->getUniformTType(i);
		if (ttype->getBasicType() == glslang::EbtSampler) {
			ShaderDetail::Uniform sampler;
			if (ttype->getSampler().isCombined()) {
				sampler.type = ShaderDetail::Uniform::eCombined;
			}
			sampler.set = ttype->getQualifier().layoutSet;
			sampler.binding = ttype->getQualifier().layoutBinding;
			sampler.stage = GetStageFromMask(data_->program_->getUniform(i).stages);
			sampler.size = ttype->getSampler().dim;
			sampler.name = data_->program_->getUniform(i).name;
			detail->uniforms.push_back(sampler);
		}
	}
	//uniform - block
	for (int32_t i = 0; i < data_->program_->getNumUniformBlocks(); i++) {
		auto ttype = data_->program_->getUniformBlockTType(i);
		ShaderDetail::Uniform block;
		block.type = ShaderDetail::Uniform::eUniformBlock;
		block.set = ttype->getQualifier().layoutSet;
		block.binding = ttype->getQualifier().layoutBinding;
		block.stage = GetStageFromMask(data_->program_->getUniformBlock(i).stages);
		block.size = data_->program_->getUniformBlock(i).size;
		block.name = data_->program_->getUniformBlock(i).name;
		detail->uniforms.push_back(block);
	}
	//pipe in
	for (int32_t i = 0; i < data_->program_->getNumPipeInputs(); i++) {
		auto& reflect = data_->program_->getPipeInput(i);
		ShaderDetail::Attribute pipe_in;
		pipe_in.io = ShaderDetail::Attribute::eIn;
		pipe_in.name = reflect.name;
		pipe_in.size = reflect.size;
		pipe_in.stage = GetStageFromMask(reflect.stages);
		pipe_in.location = reflect.getType()->getQualifier().layoutLocation;
		detail->pipeline_io.push_back(pipe_in);
	}
	//pipe out
	for (int32_t i = 0; i < data_->program_->getNumPipeOutputs(); i++) {
		auto& reflect = data_->program_->getPipeOutput(i);
		ShaderDetail::Attribute pipe_out;
		pipe_out.io = ShaderDetail::Attribute::eOut;
		pipe_out.name = reflect.name;
		pipe_out.size = reflect.size;
		pipe_out.stage = GetStageFromMask(reflect.stages);
		pipe_out.location = reflect.getType()->getQualifier().layoutLocation;
		detail->pipeline_io.push_back(pipe_out);
	}
	return detail;
}



void ShaderCompiler::Reflect() {
	data_->program_->buildReflection();
#ifdef _DEBUG
	data_->program_->dumpReflection();
#endif
}

std::vector<uint32_t> GlslToSpv(const char* filename, ShaderStage stage) {
	std::unique_ptr<ShaderCompiler> compiler;
	switch (stage) {
	case eVertex:
		compiler = std::make_unique<ShaderCompiler>(filename, nullptr);
		break;
	case eFragment:
		compiler = std::make_unique<ShaderCompiler>(nullptr, filename);
		break;
	default:
		return std::vector<uint32_t>();
		break;
	}
	if (!compiler->Compile()) {
		return std::vector<uint32_t>();
	}
	if (!compiler->Link()) {
		return std::vector<uint32_t>();
	}
	std::vector<uint32_t> vertex(0);
	std::vector<uint32_t> fragment(0);
	compiler->GenSpv(vertex, fragment);
	switch (stage) {
	case eVertex:
		return vertex;
		break;
	case eFragment:
		return fragment;
		break;
	default:
		return std::vector<uint32_t>();
		break;
	}
}

std::unique_ptr<ShaderDetail> GenGlslDetail(const char* vertex, const char* fragment) {
	std::unique_ptr<ShaderCompiler> compiler = std::make_unique<ShaderCompiler>(
		vertex, fragment);
	if (!compiler->Compile()) {
		return nullptr;
	}
	if (!compiler->Link()) {
		return nullptr;
	}
	compiler->Reflect();
	return std::move(compiler->GenDetail());
}
