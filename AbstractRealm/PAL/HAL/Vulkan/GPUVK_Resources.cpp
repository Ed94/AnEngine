// Parent Header
#include "GPUVK_Resources.hpp"



#include "GPUVK_PayloadDeck.hpp"
#include "GPUVK_Memory.hpp"




namespace HAL::GPU::Vulkan
{
#pragma region Image

	EResult Image::Create(const LogicalDevice& _device, const CreateInfo& _info)
	{
		info = _info;

		return Parent::Create(_device, info);
	}

	EResult Image::Create(const LogicalDevice& _device, const CreateInfo& _info, const Memory::AllocationCallbacks& _allocator)
	{
		info      = _info;
		allocator = &_allocator;

		return Parent::Create(_device, _info);
	}

	Extent3D Image::GetExtent() const
	{
		return info.Extent;
	}

	EFormat Image::GetFormat() const
	{
		return info.Format;
	}

	ui32 Image::GetMipmapLevels() const
	{
		return info.MipmapLevels;
	}

	bool Image::HasStencilComponent() const
	{
		return info.Format == EFormat::D32_SFloat_S8_UInt ||  info.Format  == EFormat::D24_UNormalized_S8_UInt;
	}

	void Image::TransitionLayout(EImageLayout _old, EImageLayout _new)
	{
		auto commandBuffer = Deck::RecordOnTransient();

		Image::Memory_Barrier barrier {};

		barrier.OldLayout = _old;
		barrier.NewLayout = _new;

		barrier.SrcQueueFamilyIndex = QueueFamily_Ignored;
		barrier.DstQueueFamilyIndex = QueueFamily_Ignored;

		barrier.Image = handle;

		barrier.SubresourceRange.AspectMask.Set(EImageAspect::Color);

		if (_new == EImageLayout::DepthStencil_AttachmentOptimal)
		{
			barrier.SubresourceRange.AspectMask.Set(EImageAspect::Depth);

			if (HasStencilComponent())
			{
				barrier.SubresourceRange.AspectMask.Add(EImageAspect::Stencil);
			}
		}
		else
		{
			barrier.SubresourceRange.AspectMask.Set(EImageAspect::Color);
		}

		barrier.SubresourceRange.BaseMipLevel   = 0                ;
		barrier.SubresourceRange.LevelCount     = info.MipmapLevels;
		barrier.SubresourceRange.BaseArrayLayer = 0                ;
		barrier.SubresourceRange.LayerCount     = 1                ;

		Pipeline::StageFlags sourceStage     ;
		Pipeline::StageFlags destinationStage;

		if (_old == EImageLayout::Undefined && _new == EImageLayout::TransferDestination_Optimal)
		{
			barrier.SrcAccessMask = 0;

			barrier.DstAccessMask.Set(EAccessFlag::TransferWrite);

			sourceStage     .Set(EPipelineStageFlag::TopOfPipe);
			destinationStage.Set(EPipelineStageFlag::Transfer );
		}
		else if (_old == EImageLayout::TransferDestination_Optimal && _new == EImageLayout::Shader_ReadonlyOptimal)
		{
			barrier.SrcAccessMask.Set(EAccessFlag::TransferWrite);
			barrier.DstAccessMask.Set(EAccessFlag::ShaderRead   );

			sourceStage     .Set(EPipelineStageFlag::Transfer       );
			destinationStage.Set(EPipelineStageFlag::FragementShader);
		}
		else if (_old == EImageLayout::Undefined && _new == EImageLayout::DepthStencil_AttachmentOptimal)
		{
			barrier.SrcAccessMask = 0;

			barrier.DstAccessMask.Set(EAccessFlag::DepthStencilAttachmentRead, EAccessFlag::DepthStencilAttachmentWrite);

			sourceStage     .Set(EPipelineStageFlag::TopOfPipe         );
			destinationStage.Set(EPipelineStageFlag::EarlyFragmentTests);
		}
		else
		{
			throw std::invalid_argument("unsupported layout transition!");
		}

		commandBuffer.SubmitPipelineBarrier
		(
			sourceStage, destinationStage,   // TODO
			0, 
			1, &barrier
		);

		Deck::EndRecordOnTransient(commandBuffer);
	}


	Image::operator Parent& ()
	{
		return *static_cast<Parent*>(this);
	}

#pragma endregion Image

#pragma region ImageView

	EResult ImageView::Create(const LogicalDevice& _device, const CreateInfo& _info)
	{
		info = _info;

		return Parent::Create(_device, _info);
	}

	EResult ImageView::Create(const LogicalDevice& _device, const CreateInfo& _info, const Memory::AllocationCallbacks& _allocator)
	{
		info = _info;

		return Parent::Create(_device, _info, _allocator);
	}

#pragma endregion ImageView

#pragma region BufferPackage

	BufferPackage::BufferPackage() : buffer(), memory(), memoryOffset(0), view()
	{}

	BufferPackage::BufferPackage(const LogicalDevice& _device) : buffer(_device), memory(nullptr), memoryOffset(0), view(_device)
	{}

	BufferPackage::BufferPackage(const LogicalDevice& _device, const Memory& _memory) : 
		buffer(_device), memory(&_memory), memoryOffset(0), view(_device)
	{}

	BufferPackage::~BufferPackage()
	{
		// Individual objects handle their destruction.
	}

	void BufferPackage::Create(const Buffer::CreateInfo& _bufferInfo, Memory::PropertyFlags _memoryFlags)
	{
		EResult returnCode = buffer.Create(_bufferInfo);

		if (returnCode != EResult::Success)
			throw RuntimeError("Failed to initialize buffer package.");

		Memory::AllocateInfo allocInfo;

		auto& memReq = buffer.GetMemoryRequirements();

		allocInfo.AllocationSize = memReq.Size;

		allocInfo.MemoryTypeIndex = 
			buffer.GetDevice().GetPhysicalDevice().FindMemoryType(memReq.MemoryTypeBits, _memoryFlags);

		memory = &RequestMemory(allocInfo);

		buffer.BindMemory(*memory, memoryOffset);
	}

	void BufferPackage::Destroy()
	{
		buffer.Destroy();
		view  .Destroy();

		memory = nullptr;

		memoryOffset = 0;
	}

	const Buffer& BufferPackage::GetBuffer()
	{
		return buffer;
	}

	const Memory& BufferPackage::GetMemory()
	{
		return *memory;
	}

	const DeviceSize& BufferPackage::GetMemoryOffset()
	{
		return memoryOffset;
	}

	const BufferView& BufferPackage::GetView()
	{
		return view;
	}

#pragma endregion BufferPackage

#pragma region ImagePackage

	Extent3D ImagePackage::GetExtent() const
	{
		return image.GetExtent();
	}

	EFormat ImagePackage::GetFormat() const
	{
		return image.GetFormat();
	}

	ui32 ImagePackage::GetMipmapLevels() const
	{
		return image.GetMipmapLevels();
	}

#pragma endregion ImagePackage

#pragma region VertexBuffer

	EResult VertexBuffer::Create(ptr<const void> data, DeviceSize _dataSize, DeviceSize /*_stride*/)
	{
		EResult result = EResult::Incomplete;

		Buffer stagingBuffer;

		Memory stagingBufferMemory;

		Buffer::CreateInfo stagingBufferInfo;

		stagingBufferInfo.SharingMode = ESharingMode::Exclusive;
		stagingBufferInfo.Size        = _dataSize;

		stagingBufferInfo.Usage.Set(EBufferUsage::TransferSource);

		result = stagingBuffer.Create(GPU_Comms::GetEngagedDevice(), stagingBufferInfo);

		if (result != EResult::Success) return result;

		Memory::AllocateInfo allocInfo;

		auto& memReq = stagingBuffer.GetMemoryRequirements();

		allocInfo.AllocationSize = memReq.Size;

		allocInfo.MemoryTypeIndex =
			stagingBuffer.GetDevice().GetPhysicalDevice().FindMemoryType
			(memReq.MemoryTypeBits, Memory::PropertyFlags(EMemoryPropertyFlag::HostVisible, EMemoryPropertyFlag::HostCoherent));

		result = stagingBufferMemory.Allocate(GPU_Comms::GetEngagedDevice(), allocInfo);

		if (result != EResult::Success) return result;

		result = stagingBuffer.BindMemory(stagingBufferMemory, Memory::ZeroOffset);

		if (result != EResult::Success) return result;

		Memory::MapFlags mapflags;

		stagingBufferMemory.WriteToGPU(0, _dataSize, mapflags, data);

		Buffer::CreateInfo vertexBufferInfo {};

		vertexBufferInfo.Size        = _dataSize;
		vertexBufferInfo.SharingMode = ESharingMode::Exclusive;

		vertexBufferInfo.Usage.Set(EBufferUsage::TransferDestination, EBufferUsage::VertexBuffer);

		result = buffer.Create(GPU_Comms::GetEngagedDevice(), vertexBufferInfo);

		if (result != EResult::Success) return result;

		auto& memReqVert = buffer.GetMemoryRequirements();

		allocInfo.AllocationSize = memReqVert.Size;

		allocInfo.MemoryTypeIndex =
			buffer.GetDevice().GetPhysicalDevice().FindMemoryType
			(memReqVert.MemoryTypeBits, Memory::PropertyFlags(EMemoryPropertyFlag::DeviceLocal));

		result = memory.Allocate(GPU_Comms::GetEngagedDevice(), allocInfo);

		if (result != EResult::Success) return result;

		memoryOffset = Memory::ZeroOffset;   // Hardcoded

		result = buffer.BindMemory(memory, memoryOffset);

		if (result != EResult::Success) return result;

		Buffer::CopyInfo copyInfo {}; copyInfo.DestinationOffset = 0; copyInfo.SourceOffset = 0; copyInfo.Size = _dataSize;

		auto commandBuffer = Deck::RecordOnTransient();

		commandBuffer.CopyBuffer(stagingBuffer, buffer, 1, &copyInfo);

		Deck::EndRecordOnTransient(commandBuffer);

		stagingBuffer      .Destroy();
		stagingBufferMemory.Free();

		return result;
	}

	void VertexBuffer::Destroy()
	{
		buffer.Destroy();

		memory.Free();
	}
	
	const Buffer& VertexBuffer::GetBuffer() const { return buffer; }

#pragma endregion VertexBuffer

#pragma region IndexBuffer

	EResult IndexBuffer::Create(ptr<const void> _data, DeviceSize _dataSize, DeviceSize /*_stride*/)
	{
		EResult result = EResult::Incomplete;

		Buffer             stagingBuffer;
		Buffer::CreateInfo stagingBufferInfo;
		Memory             stagingBufferMemory;

		stagingBufferInfo.SharingMode = ESharingMode::Exclusive;
		stagingBufferInfo.Size = _dataSize;

		stagingBufferInfo.Usage.Set(EBufferUsage::TransferSource);

		result = stagingBuffer.Create(GPU_Comms::GetEngagedDevice(), stagingBufferInfo);

		if (result != EResult::Success) return result;

		Memory::AllocateInfo allocInfo;

		auto& memReq = stagingBuffer.GetMemoryRequirements();

		allocInfo.AllocationSize = memReq.Size;

		allocInfo.MemoryTypeIndex =
			stagingBuffer.GetDevice().GetPhysicalDevice().FindMemoryType
			(memReq.MemoryTypeBits, Memory::PropertyFlags(EMemoryPropertyFlag::HostVisible, EMemoryPropertyFlag::HostCoherent));

		result = stagingBufferMemory.Allocate(GPU_Comms::GetEngagedDevice(), allocInfo);

		if (result != EResult::Success) return result;

		result = stagingBuffer.BindMemory(stagingBufferMemory, Memory::ZeroOffset);

		if (result != EResult::Success) return result;

		// Hard coded vertex data...

		RoVoidPtr address = _data;

		stagingBufferMemory.WriteToGPU(0, _dataSize, Memory::MapFlags(), address);

		Buffer::CreateInfo indexBufferInfo{};

		indexBufferInfo.SharingMode = ESharingMode::Exclusive;
		indexBufferInfo.Size = _dataSize;

		indexBufferInfo.Usage.Set(EBufferUsage::TransferDestination, EBufferUsage::IndexBuffer);

		result = buffer.Create(GPU_Comms::GetEngagedDevice(), indexBufferInfo);

		if (result != EResult::Success) return result;

		auto& memReqIndex = buffer.GetMemoryRequirements();

		allocInfo.AllocationSize = memReqIndex.Size;

		allocInfo.MemoryTypeIndex =
			buffer.GetDevice().GetPhysicalDevice().FindMemoryType
			(memReqIndex.MemoryTypeBits, Memory::PropertyFlags(EMemoryPropertyFlag::DeviceLocal));

		result = memory.Allocate(GPU_Comms::GetEngagedDevice(), allocInfo);

		if (result != EResult::Success) return result;

		memoryOffset = Memory::ZeroOffset;

		result = buffer.BindMemory(memory, memoryOffset);

		if (result != EResult::Success) return result;

		Buffer::CopyInfo copyInfo{}; copyInfo.DestinationOffset = 0; copyInfo.SourceOffset = 0; copyInfo.Size = _dataSize;

		auto commandBuffer = Deck::RecordOnTransient();

		commandBuffer.CopyBuffer(stagingBuffer, buffer, 1, &copyInfo);

		Deck::EndRecordOnTransient(commandBuffer);

		stagingBuffer.Destroy();
		stagingBufferMemory.Free();

		return result;
	}

	void IndexBuffer::Destroy()
	{
		buffer.Destroy();

		memory.Free();
	}

	const Buffer& IndexBuffer::GetBuffer() const
	{
		return buffer;
	}

#pragma endregion IndexBuffer

#pragma region TextureImage

	// Public

	EResult TextureImage::Create(ptr<const uI8> _imageData, ui32 _width, ui32 _height)
	{
		DeviceSize imageSize = _width * _height * 4;

		ui32 mipmapLevels = SCast<ui32>(std::floor(std::log2(std::max<ui32>(_width, _height)))) + 1;

		Buffer stagingBuffer;

		Memory stagingBufferMemory;

		Buffer::CreateInfo stagingBufferInfo{};

		stagingBufferInfo.Size = imageSize;
		stagingBufferInfo.SharingMode = ESharingMode::Exclusive;

		stagingBufferInfo.Usage.Set(EImageUsage::TransferSource);

		stagingBuffer.Create(GPU_Comms::GetEngagedDevice(), stagingBufferInfo);

		Memory::AllocateInfo allocInfo;

		auto& memReq = stagingBuffer.GetMemoryRequirements();

		allocInfo.AllocationSize = memReq.Size;

		allocInfo.MemoryTypeIndex =
			stagingBuffer.GetDevice().GetPhysicalDevice().FindMemoryType
			(memReq.MemoryTypeBits, Memory::PropertyFlags(EMemoryPropertyFlag::HostVisible, EMemoryPropertyFlag::HostCoherent));

		stagingBufferMemory.Allocate(GPU_Comms::GetEngagedDevice(), allocInfo);

		stagingBuffer.BindMemory(stagingBufferMemory, Memory::ZeroOffset);

		RoVoidPtr address = _imageData;

		stagingBufferMemory.WriteToGPU(0, imageSize, 0, address);

		Image::CreateInfo info;

		info.ImageType     = EImageType::_2D       ;
		info.Extent.Width  = SCast<ui32>(_width );
		info.Extent.Height = SCast<ui32>(_height);
		info.Extent.Depth  = 1                     ;
		info.MipmapLevels  = mipmapLevels            ;
		info.ArrayLayers   = 1                     ;

		info.Format       = EFormat::R8_G8_B8_A8_UNormalized;
		info.Tiling       = EImageTiling::Optimal  ;
		info.InitalLayout = EImageLayout::Undefined;
		info.Usage        = Image::UsageFlags(EImageUsage::TransferDestination, EImageUsage::Sampled, EImageUsage::TransferSource);
		info.SharingMode  = ESharingMode::Exclusive ;
		info.Samples      = ESampleCount::_1;
		info.Flags        = 0                      ;

		image.Create(GPU_Comms::GetEngagedDevice(), info);

		Memory::AllocateInfo allocationInfo{};

		auto& gpu = GPU_Comms::GetEngagedDevice().GetPhysicalDevice();

		allocationInfo.AllocationSize  = image.GetMemoryRequirements().Size;
		allocationInfo.MemoryTypeIndex = gpu.FindMemoryType(image.GetMemoryRequirements().MemoryTypeBits, Memory::PropertyFlags(EMemoryPropertyFlag::DeviceLocal));

		if (memory.Allocate(image.GetMemoryRequirements(), Memory::PropertyFlags(EMemoryPropertyFlag::DeviceLocal)) != EResult::Success)
			throw RuntimeError("Failed to allocate image memory!");

		image.BindMemory(memory, 0);

		image.TransitionLayout(EImageLayout::Undefined, EImageLayout::TransferDestination_Optimal);

		auto& buffer = Deck::RecordOnTransient();

		CommandBuffer::BufferImageRegion region{};

		region.BufferOffset = 0;
		region.BufferRowLength = 0;

		region.ImageSubresource.AspectMask.Set(EImageAspect::Color);

		region.ImageSubresource.MipLevel = 0;
		region.ImageSubresource.BaseArrayLayer = 0;
		region.ImageSubresource.LayerCount = 1;

		region.ImageOffset.X = 0;
		region.ImageOffset.Y = 0;
		region.ImageOffset.Z = 0;

		region.ImageExtent.Width  = _width;
		region.ImageExtent.Height = _height;
		region.ImageExtent.Depth  = 1;

		buffer.CopyBufferToImage(stagingBuffer, image, EImageLayout::TransferDestination_Optimal, 1, &region);

		Deck::EndRecordOnTransient(buffer);

		stagingBuffer.Destroy();
		stagingBufferMemory.Free();

		GenerateMipmaps();

		CreateImageView();

		CreateSampler();
	}

	void TextureImage::Destroy()
	{
		image.Destroy();

		memory.Free();

		imageView.Destroy();

		sampler.Destroy();
	}

	// Protected

	void TextureImage::CreateImageView()
	{
		ImageView::CreateInfo viewInfo;

		viewInfo.Image    = image;
		viewInfo.ViewType = ImageView::EViewType::_2D;
		viewInfo.Format   = GetFormat();

		viewInfo.Components.R = EComponentSwizzle::Identitity;
		viewInfo.Components.G = EComponentSwizzle::Identitity;
		viewInfo.Components.B = EComponentSwizzle::Identitity;
		viewInfo.Components.A = EComponentSwizzle::Identitity;

		viewInfo.SubresourceRange.AspectMask     = Image::AspectFlags(EImageAspect::Color);
		viewInfo.SubresourceRange.BaseMipLevel   = 0;
		viewInfo.SubresourceRange.LevelCount     = 1;  //_miplevels  ;
		viewInfo.SubresourceRange.BaseArrayLayer = 0;
		viewInfo.SubresourceRange.LayerCount     = 1;

		if (imageView.Create(GPU_Comms::GetEngagedDevice(), viewInfo) != EResult::Success)
			throw RuntimeError("Failed to create texture image view!");
	}

	void TextureImage::CreateSampler()
	{
		Sampler::CreateInfo samplerInfo{};

		samplerInfo.MagnificationFilter = EFilter::Linear;
		samplerInfo.MinimumFilter       = EFilter::Linear;

		samplerInfo.AddressModeU = ESamplerAddressMode::Repeat;
		samplerInfo.AddressModeV = ESamplerAddressMode::Repeat;
		samplerInfo.AddressModeW = ESamplerAddressMode::Repeat;

		samplerInfo.AnisotropyEnable = EBool::True;
		samplerInfo.MaxAnisotropy    = 16.0f;
		samplerInfo.BorderColor      = EBorderColor::Int_OpaqueBlack;

		samplerInfo.UnnormalizedCoordinates = EBool::False;

		samplerInfo.CompareEnable    = EBool::False;
		samplerInfo.CompareOperation = ECompareOperation::Always;

		samplerInfo.MipmapMode = ESamplerMipmapMode::Linear;
		samplerInfo.MipLodBias = 0.0f;
		samplerInfo.MinimumLod = 0.0f;
		samplerInfo.MaxLod     = SCast<f32>(GetMipmapLevels());

		if (sampler.Create(GPU_Comms::GetEngagedDevice(), samplerInfo) != EResult::Success)
			throw RuntimeError("Failed to create texture sampler!");
	}

	void TextureImage::GenerateMipmaps()
	{
		// Check if image format supports linear blitting
		FormatProperties formatProperties = GPU_Comms::GetEngagedDevice().GetPhysicalDevice().GetFormatProperties(GetFormat());

		if (!(formatProperties.OptimalTilingFeatures.HasFlag(EFormatFeatureFlag::SampledImageFilterLinear)))
		{
			throw std::runtime_error("Texture image format does not support linear blitting!");
		}

		auto& commandBuffer = Deck::RecordOnTransient();

		Image::Memory_Barrier barrier{};

		barrier.Image               = image              ;
		barrier.SrcQueueFamilyIndex = QueueFamily_Ignored;
		barrier.DstQueueFamilyIndex = QueueFamily_Ignored;

		barrier.SubresourceRange.AspectMask.Set(EImageAspect::Color);

		barrier.SubresourceRange.BaseArrayLayer = 0;
		barrier.SubresourceRange.LayerCount     = 1;
		barrier.SubresourceRange.LevelCount     = 1;

		si32 mipWidth  = GetExtent().Width ;
		si32 mipHeight = GetExtent().Height;

		for (uint32_t index = 1; index < GetMipmapLevels(); index++)
		{
			barrier.SubresourceRange.BaseMipLevel = index - 1;

			barrier.OldLayout = EImageLayout::TransferDestination_Optimal;
			barrier.NewLayout = EImageLayout::TransferSource_Optimal;

			barrier.SrcAccessMask.Set(EAccessFlag::TransferWrite);
			barrier.DstAccessMask.Set(EAccessFlag::TransferRead);

			commandBuffer.SubmitPipelineBarrier
			(
				EPipelineStageFlag::Transfer, EPipelineStageFlag::Transfer, 0,
				1, &barrier
			);

			Image::Blit blit{};

			blit.SrcOffsets[0].X = 0;
			blit.SrcOffsets[0].Y = 0;
			blit.SrcOffsets[0].Z = 0;

			blit.SrcOffsets[1].X = mipWidth;
			blit.SrcOffsets[1].Y = mipHeight;
			blit.SrcOffsets[1].Z = 1;

			blit.SrcSubresource.AspectMask.Set(EImageAspect::Color);
			blit.SrcSubresource.MipLevel = index - 1;
			blit.SrcSubresource.BaseArrayLayer = 0;
			blit.SrcSubresource.LayerCount = 1;

			blit.DstOffsets[0].X = 0;
			blit.DstOffsets[0].Y = 0; 
			blit.DstOffsets[0].Z = 0;

			blit.DstOffsets[1].X = mipWidth > 1 ? mipWidth / 2 : 1;
			blit.DstOffsets[1].Y = mipHeight > 1 ? mipHeight / 2 : 1; 
			blit.DstOffsets[1].Z = 1;

			blit.DstSubresource.AspectMask.Set(EImageAspect::Color);
			blit.DstSubresource.MipLevel = index;
			blit.DstSubresource.BaseArrayLayer = 0;
			blit.DstSubresource.LayerCount = 1;

			commandBuffer.BlitImage
			(
				image, EImageLayout::TransferSource_Optimal,
				image, EImageLayout::TransferDestination_Optimal,
				1, &blit,
				EFilter::Linear
			);

			barrier.OldLayout = EImageLayout::TransferSource_Optimal;
			barrier.NewLayout = EImageLayout::Shader_ReadonlyOptimal;

			barrier.SrcAccessMask.Set(EAccessFlag::TransferRead);
			barrier.DstAccessMask.Set(EAccessFlag::ShaderRead  );

			commandBuffer.SubmitPipelineBarrier
			(
				EPipelineStageFlag::Transfer, EPipelineStageFlag::FragementShader, 0,
				1, &barrier
			);

			if (mipWidth  > 1) mipWidth  /= 2;
			if (mipHeight > 1) mipHeight /= 2;
		}

		barrier.SubresourceRange.BaseMipLevel = GetMipmapLevels() - 1;

		barrier.OldLayout = EImageLayout::TransferDestination_Optimal;
		barrier.NewLayout = EImageLayout::Shader_ReadonlyOptimal     ;

		barrier.SrcAccessMask.Set(EAccessFlag::TransferWrite);
		barrier.DstAccessMask.Set(EAccessFlag::ShaderRead   );

		commandBuffer.SubmitPipelineBarrier
		(
			EPipelineStageFlag::Transfer, EPipelineStageFlag::FragementShader, 0,
			1, &barrier
		);

		Deck::EndRecordOnTransient(commandBuffer);
	}

#pragma endregion TextureImage

#pragma region GPU_Resources

	DynamicArray< SPtr<ARenderable>> GPU_Resources_Maker<Meta::EGPU_Engage::Single>::renderables;

#pragma endregion GPU_Resources
}