#ifndef __IRR_EXT_OPTIX_MANAGER_H_INCLUDED__
#define __IRR_EXT_OPTIX_MANAGER_H_INCLUDED__

#include "irrlicht.h"

#include "../ext/OptiX/SbtRecord.h"
#include "../ext/OptiX/IContext.h"
#include "../ext/OptiX/IDenoiser.h"

#include "optix.h"
#include "optix_stubs.h"

namespace irr
{
namespace ext
{
namespace OptiX
{


class Manager final : public core::IReferenceCounted
{
	public:
		//
		_IRR_STATIC_INLINE_CONSTEXPR uint32_t MaxSLI = 4u;
		static core::smart_refctd_ptr<Manager> create(video::IVideoDriver* _driver, io::IFileSystem* _filesystem);

		static void defaultCallback(unsigned int level, const char* tag, const char* message, void* cbdata);

		// cuda managment
		CUcontext getDeviceContext(uint32_t localCUDAContextID)
		{
			return context[localCUDAContextID];
		}
		CUstream getDeviceStream(uint32_t localCUDAContextID)
		{
			return stream[localCUDAContextID];
		}
		uint32_t findLUID(CUcontext CUDAContext)
		{
			uint32_t luid = 0u;
			for (; luid<MaxSLI; luid++)
			if (context[luid]==CUDAContext)
				break;
			return luid;
		}
		uint32_t findLUID(CUstream CUDAStream)
		{
			uint32_t luid = 0u;
			for (; luid<MaxSLI; luid++)
			if (stream[luid]==CUDAStream)
				break;
			return luid;
		}

		// contexts
		core::smart_refctd_ptr<IContext> createContext(uint32_t localCUDAContextID, const OptixDeviceContextOptions& options={}, OptixLogCallback callback=&defaultCallback, uint32_t callbackLevel=3u)
		{
			if (localCUDAContextID>=MaxSLI)
				return nullptr;

			OptixDeviceContext optixContext;
			if (optixDeviceContextCreate(context[localCUDAContextID],&options,&optixContext)!=OPTIX_SUCCESS)
				return nullptr;

			optixDeviceContextSetLogCallback(optixContext,callback,reinterpret_cast<void*>(context[localCUDAContextID]),callbackLevel);
			return core::smart_refctd_ptr<IContext>(new IContext(this,context[localCUDAContextID],optixContext),core::dont_grab);
		}

		// headers
		inline core::SRange<const io::IReadFile* const> getOptiXHeaders() const
		{
			auto begin = reinterpret_cast<const io::IReadFile* const*>(&optixHeaders[0].get());
			return { begin,begin + optixHeaders.size() };
		}
		inline const auto& getOptiXHeaderContents() const { return optixHeaderContents; }
		inline const auto& getOptiXHeaderNames() const { return optixHeaderNames; }


		// modules
		template<typename OptionsT = const std::initializer_list<const char*>&>
		inline std::string compileOptiXProgram(const char* source, const char* filename, OptionsT options={_IRR_OPTIX_DEFAULT_NVRTC_OPTIONS}, std::string* log=nullptr)
		{
			std::string ptx;
			const auto headerCount = optixHeaders.size();
			bool ok = cuda::CCUDAHandler::defaultHandleResult(
				cuda::CCUDAHandler::compileDirectlyToPTX<OptionsT>(
					ptx,source,filename,
					optixHeaderContents.data(),optixHeaderContents.data()+headerCount,
					optixHeaderNames.data(),optixHeaderNames.data()+headerCount,
					std::forward<OptionsT>(options),log
				)
			);
			return ptx;
		}

		template<typename OptionsT = const std::initializer_list<const char*>&>
		inline std::string compileOptiXProgram(io::IReadFile* file, OptionsT options={_IRR_OPTIX_DEFAULT_NVRTC_OPTIONS}, std::string* log=nullptr)
		{
			if (!file)
				return "";

			auto sz = file->getSize();
			core::vector<char> tmp(sz+1ull);
			file->read(tmp.data(),sz);
			tmp.back() = 0;
			return compileOptixProgram(tmp.data(), file->getFileName().c_str(), std::forward<OptionsT>(options),log);
		}

	protected:
		Manager(video::IVideoDriver* _driver, io::IFileSystem* _filesystem, uint32_t _contextCount, CUcontext* _context, bool* _ownContext=nullptr);
		~Manager();

		video::IVideoDriver* driver;
		CUcontext context[MaxSLI];
		bool ownContext[MaxSLI];
		CUstream stream[MaxSLI];

		core::vector<core::smart_refctd_ptr<const io::IReadFile> > optixHeaders;
		core::vector<const char*> optixHeaderContents;
		core::vector<const char*> optixHeaderNames;
};

}
}
}

#endif