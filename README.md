# Welcome to LibAse!

LibAse is a library written in C++17 to facilitate parsing of Aseprite files so that they can be passed to a renderer and rendered in your application. This library is currently only been tested on Windows, but you're welcome to test it on Linux and report any issue that you may find!


# How to use
There is a Visual Studio 2022 solution file provided so that you can compile it into a shared or static library if you choose. However, you _do not need to_ and you should simply be able to include `Parser.h` and so long as you maintain the directory hierarchy you can use the library that way!

This library posses logging capabilities, ***HOWEVER***, in order to utilize this you would need to call `ASE::Log::Init()` at some point in the initialization process of your application. Be warned this logging would carry over to any build that you would make, so if you'd like to strip this then you need to wrap it in some header guards. For example,

```cpp
...
#ifdef MY_APP_DEBUG_BUILD
	ASE::Log::Init();
#endif
...
```
# Example Usage

Below is an example of how your application, ideally, would implement this library. All the user has to do is create the image with their own implementation based on the example function`CreateImage()`  below and from there they can create a 2D Texture using their own graphics pipeline  and present it as they would like.

```cpp
std::shared_ptr<Texture2D> CreateImage(const std::string& Filename, bool ShouldCreateTexture, bool ShouldFlipOnLoad = false)
{
	std::vector<uint8_t> ImgData;

	uint32_t Width = AsepriteData[Filename].Header.Width;
	uint32_t Height = AsepriteData[Filename].Header.Height;
	int Channels = AsepriteData[Filename].Header.Depth == 32 ? 4 : 3;
	uint8_t RGBType = AsepriteData[Filename].Header.Depth == 32 ? 2 : 1;


	ReadLayerChunk(AsepriteData[Filename]);
	ReadCelChunk(AsepriteData[Filename]);
	ReorderLayers(Filename);

	for (auto& F : AsepriteData[Filename].Frames)
	{
		ReadOldPaletteChunk(AsepriteData[Filename]);
		ReadNewPaletteChunk(AsepriteData[Filename]);
	}

	ImageSpecification Spec(Width, Height, Channels, RGBType, AsepriteData[Filename]);
	Ref<Image> Img = CreateRef<Image>(Spec, ShouldFlipOnLoad);

	ImagePairs[Filename] = Img;

	if (ShouldCreateTexture)
	{
		return CreateTexture(Filename);
	}

	return nullptr;
}
```
```cpp
std::shared_ptr<Texture2D> CreateTexture(std::string ImageName)
{
	auto& Img = ImagePairs[ImageName];
	return Texture2D::Create(Img.get(), Img->GetImageSpec().GetWidth(), Img->GetImageSpec().GetHeight(), Img->GetImageSpec().GetChannels(), Img->GetImageByteSize());
}
```

# Important Notes

- It's important to know that this currently only works on sprites that have a single layer, in the future I hope to have support for multi-layered sprites.
- Support for animations is currently untested so I have no idea if an animated single layered sprite would work but there are plans to add support for this in the future.
