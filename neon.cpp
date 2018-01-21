//reference: https://github.com/psycholns/TinyDX11
//Crinkler command: /CRINKLER/ PRIORITY:NORMAL/ UNSAFEIMPORT/ COMPMODE : SLOW / TINYHEADER/ ORDERTRIES : 1000/ OVERRIDEALIGNMENTS : 8

#define WIDTH 1920
#define HEIGHT 1080                   

#define D3D11CreateDeviceAndSwapChain X
#include <d3d11.h>
#undef D3D11CreateDeviceAndSwapChain
#undef WINAPI
#define WINAPI __declspec(dllimport)  __stdcall 
#include <d3dx11.h>
#include <stdio.h>

extern "C" 
{
	HRESULT WINAPI D3D11CreateDeviceAndSwapChain(__in_opt IDXGIAdapter* pAdapter,D3D_DRIVER_TYPE DriverType,HMODULE Software,
		UINT Flags,__in_ecount_opt(FeatureLevels) CONST D3D_FEATURE_LEVEL* pFeatureLevels,UINT FeatureLevels,UINT SDKVersion,
		__in_opt CONST DXGI_SWAP_CHAIN_DESC* pSwapChainDesc,__out_opt IDXGISwapChain** ppSwapChain,__out_opt ID3D11Device** ppDevice,
		__out_opt D3D_FEATURE_LEVEL* pFeatureLevel,__out_opt ID3D11DeviceContext** ppImmediateContext);
	HRESULT __stdcall  _imp__D3DX11CompileFromMemory(LPCSTR pSrcData, SIZE_T SrcDataLen, LPCSTR pFileName, 
		CONST D3D10_SHADER_MACRO* pDefines, LPD3D10INCLUDE pInclude,LPCSTR pFunctionName, LPCSTR pProfile, 
		UINT Flags1, UINT Flags2, ID3DX11ThreadPump* pPump, ID3D10Blob** ppShader, ID3D10Blob** ppErrorMsgs, HRESULT* pHResult);
};

static char shader[] =
"int t;"
"RWTexture2D<float4> k;"
"[numthreads(16,16,1)]"
"void cs_5_0(uint3 i:sv_dispatchthreadid)"
"{"
	"float2 u = (2.*i.xy - float2(1920, 1080)) / 1080,"
	"o = float2(-.5,.5);"
	"u = float2(cos(t*.001)*u.x + sin(t*.001)*u.y, -sin(t*.001)*u.x + cos(t*.001)*u.y);"
	"float a = .05 / length(u + o),"
	"b = .05 / length(u + o.yy),"
	"c = .05 / length(u + o.xx),"
	"d = .05 / length(u + o.yx);"
	"k[i.xy] = float4(b+c+d,c+d,a+d,1);"
"}";

static int image[] = {WIDTH, HEIGHT, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 0, 1, 0, DXGI_USAGE_UNORDERED_ACCESS, 1, 0, 0, 0, 0 }; 
static D3D11_BUFFER_DESC buffer_descriptor = { 16, D3D11_USAGE_DEFAULT, D3D11_BIND_CONSTANT_BUFFER, 0, 0, 0 };

void WinMainCRTStartup()
{
	HWND  hwnd = CreateWindowExA(0,(LPCSTR)0xC018, 0, WS_POPUP | WS_VISIBLE | WS_MAXIMIZE, 0, 0, 0, 0, 0, 0, 0, 0); 
	ShowCursor(0);
	image[11] = (int)hwnd;
	ID3D11Device* device;
	ID3D11DeviceContext* context;
	IDXGISwapChain* surface;
	D3D11CreateDeviceAndSwapChain(NULL,D3D_DRIVER_TYPE_HARDWARE,NULL,0,0,0,D3D11_SDK_VERSION,(DXGI_SWAP_CHAIN_DESC*)&image[0],&surface,&device,NULL,&context);
	IID* iid = (IID*)(*(char**)_imp__D3DX11CompileFromMemory - 36181);  
	surface->GetBuffer(0, *iid, (LPVOID*)&image[0]);	
	device->CreateUnorderedAccessView((ID3D11Texture2D*)image[0], NULL, (ID3D11UnorderedAccessView**)&image[0]);
	context->CSSetUnorderedAccessViews(0, 1, (ID3D11UnorderedAccessView**)&image[0], 0);
	ID3D11Buffer *buffer;
	device->CreateBuffer(&buffer_descriptor, NULL, &buffer);
	context->CSSetConstantBuffers(0, 1, &buffer);
	ID3D11ComputeShader *compute_shader;
	ID3D10Blob* blob;
	D3DX11CompileFromMemory(shader, 2048, 0, 0, 0, "cs_5_0", "cs_5_0", 0, 0, 0, &blob, 0, 0);
	device->CreateComputeShader((void*)(((int*)blob)[3]), ((int*)blob)[2], NULL, &compute_shader);
	context->CSSetShader(compute_shader, NULL, 0);
	do
	{
		image[0] = GetTickCount();
		context->UpdateSubresource(buffer, 0, 0, &image[0], 4, 4);
		context->Dispatch(WIDTH / 16, HEIGHT / 16, 1);
		surface->Present(0, 0);
	} while (!GetAsyncKeyState(VK_ESCAPE));
	ExitProcess(0);
}