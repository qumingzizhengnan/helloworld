#include "d3dUtility.h"
#include "Light.h"


//声明全局的指针
ID3D11Device* device = NULL;//D3D11设备接口
IDXGISwapChain* swapChain = NULL;//交换链接口
ID3D11DeviceContext* immediateContext = NULL;
ID3D11RenderTargetView* renderTargetView = NULL;//渲染目标视图  

//着色器
//ID3D11VertexShader* m_VertexShader;
//ID3D11PixelShader* m_PixelShader;

//Effect相关全局指针
ID3D11InputLayout* vertexLayout;
ID3DX11Effect* effect;
ID3DX11EffectTechnique* technique;

//声明三个坐标系矩阵
XMMATRIX world;         //用于世界变换的矩阵
XMMATRIX view;          //用于观察变换的矩阵
XMMATRIX projection;    //用于投影变换的矩阵

//声明材质和光照的全局对象
Material		boxMaterial;      //箱子材质
Material		floorMaterial;      //地板材质
Material		waterMaterial;      //水面材质
Light			light[3];      //光源数组
int             lightType = 0;  //光源类型

ID3D11ShaderResourceView * textureBox;//箱子纹理
ID3D11ShaderResourceView * textureFloor;//地板纹理
ID3D11ShaderResourceView * textureWater;//水面纹理

ID3D11BlendState * BlendStateAlpha;//混合状态
ID3D11RasterizerState * noCulRS;//背面消隐状态

void SetLightEffect(Light light);

//ID3D11DepthStencilView* depthStencilView;  //深度模板视图
//XMVECTOR		eyePositin;                //视点位置

//定义一个顶点结构，这个顶点包含坐标和法向量
struct Vertex
{
	XMFLOAT3 Pos;
	XMFLOAT3 Normal;
	XMFLOAT2 Tex;
};

void SetLightEffect(Light light);

//**************以下为框架函数******************
bool Setup()
{
	//这里主要包含3个主要步骤
	//第一步从.fx文件创建ID3DEffect对象
	//第二步创建顶点缓存
	//第三步设置变换坐标系
	//第四步设置材质和光照
	//*************第一步从.载入外部文件（包括fx文件及图像文件）****************************
	HRESULT hr = S_OK;              //声明HRESULT的对象用于记录函数调用是否成功
	ID3DBlob* pTechBlob = NULL;     //声明ID3DBlob的对象用于存放从文件读取的信息
	//从我们之前建立的.fx文件读取着色器相关信息
	hr = D3DX11CompileFromFile(L"Shader.fx", NULL, NULL, NULL, "fx_5_0",
		D3DCOMPILE_ENABLE_STRICTNESS, 0, NULL, &pTechBlob, NULL, NULL);
	if (FAILED(hr))
	{
		::MessageBox(NULL, L"fx文件载入失败", L"Error", MB_OK); //如果读取失败，弹出错误信息
		return hr;
	}
	//调用D3DX11CreateEffectFromMemory创建ID3DEffect对象
	hr = D3DX11CreateEffectFromMemory(pTechBlob->GetBufferPointer(),
		pTechBlob->GetBufferSize(), 0, device, &effect);

	if (FAILED(hr))
	{
		::MessageBox(NULL, L"创建Effect失败", L"Error", MB_OK);  //创建失败，弹出错误信息
		return hr;
	}
	//箱子纹理
	D3DX11CreateShaderResourceViewFromFile(device, L"BOX.BMP", NULL, NULL, &textureBox, NULL);
    //地板及墙的纹理
	D3DX11CreateShaderResourceViewFromFile(device, L"checkboard.dds", NULL, NULL, &textureFloor, NULL);
	//水面纹理
	D3DX11CreateShaderResourceViewFromFile(device, L"water.dds", NULL, NULL, &textureWater, NULL);

	//*************第一步从.载入外部文件（包括fx文件及图像文件）****************************

	//*************第二步创建各种渲染状态***************************
	//先创建一个混合状态的描述
	D3D11_BLEND_DESC blendDesc;
	ZeroMemory(&blendDesc, sizeof(blendDesc));
	blendDesc.AlphaToCoverageEnable = false;//关闭AlphaToCoverage多重采样技术
	//关闭多个RenderTarget使用不同的混合状态
	blendDesc.IndependentBlendEnable = false;
	//只针对RenderTarget[0]设置绘制混合状态，忽略1~7
	blendDesc.RenderTarget[0].BlendEnable = true;//开启混合
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;//设置源因子
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;//目标因子
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;//混合操作
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;//源混合百分比
	//目标混合百分比因子
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	//混合百分比操作
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;//写掩码
	//创建ID3D11BlendState接口
	device->CreateBlendState(&blendDesc, &BlendStateAlpha);
	//关闭背面消隐
	D3D11_RASTERIZER_DESC ncDesc;
	ZeroMemory(&ncDesc, sizeof(ncDesc));
	ncDesc.CullMode = D3D11_CULL_NONE;//剔除特定三角形，这里不剔除任何三角形
	ncDesc.FillMode = D3D11_FILL_SOLID;//填充模式，这里利用三角形填充
	ncDesc.FrontCounterClockwise = false;//是否设置逆时针绕序的三角形为正面
	ncDesc.DepthClipEnable = true;//开启深度裁剪
	//创建一个关闭背面消隐的状态，在需要的时候才设置给执行上下文
	if (FAILED(device->CreateRasterizerState(&ncDesc,&noCulRS)))
	{
		::MessageBox(NULL, L"CreateRasterizerState失败", L"Error", MB_OK);  //创建失败，弹出错误信息
		return false;
	}
	
	//*************第二步创建各种渲染状态****************************

	//*************第三步创建输入布局***********************
	//用GetTechniqueByName获取ID3D11EffectTechnique的对象
	//先设置无光照的到Effect
	technique = effect->GetTechniqueByName("TexTech");

	//D3DX11_PASS_DESC结构用于描述一个Effect Pass
	D3DX11_PASS_DESC PassDesc;
	//利用GetPassByIndex获取Effect Pass
	//再利用GetDesc获取Effect Pass的描述，并存入PassDesc对象中
	technique->GetPassByIndex(0)->GetDesc(&PassDesc);

	//创建并设置输入布局
	//这里我们定义一个D3D11_INPUT_ELEMENT_DESC数组，
	//由于我们定义的顶点结构包括位置坐标和法向量，所以这个数组里有两个元素
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	//layout元素个数
	UINT numElements = ARRAYSIZE(layout);
	//调用CreateInputLayout创建输入布局
	hr = device->CreateInputLayout(layout, numElements, PassDesc.pIAInputSignature,
		PassDesc.IAInputSignatureSize, &vertexLayout);
	//设置生成的输入布局到设备上下文中
	immediateContext->IASetInputLayout(vertexLayout);
	if (FAILED(hr))
	{
		::MessageBox(NULL, L"创建Input Layout失败", L"Error", MB_OK);
		return hr;
	}

	//*************第三步创建输入布局***********************

	//*************第四步创建顶点缓存***********************
	Vertex vertices[] =
	{
		//箱子的顶点----------------------------------------------------------------------
		{ XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(1.0f, 1.0f) },

		{ XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(1.0f, 1.0f) },

		{ XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 1.0f) },

		{ XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(1.0f, 1.0f) },

		{ XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(1.0f, 1.0f) },

		{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT2(1.0f, 1.0f) },
		//箱子的顶点----------------------------------------------------------------------
	    //地板
		{ XMFLOAT3(-10.0f, -1.0f, 10.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(10.0f, -1.0f, 10.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(10.0f, 0.0f) },
		{ XMFLOAT3(-10.0f, -1.0f, -10.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 10.0f) },

		{ XMFLOAT3(-10.0f, -1.0f, -10.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 10.0f) },
		{ XMFLOAT3(10.0f, -1.0f, 10.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(10.0f, 0.0f) },
		{ XMFLOAT3(10.0f, -1.0f, -10.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(10.0f, 10.0f) },

		//前面墙
		{ XMFLOAT3(-10.0f, 1.0f, -10.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-10.0f, -1.0f, -10.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(2.0f, 0.0f) },
		{ XMFLOAT3(10.0f, 1.0f, -10.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 10.0f) },

		{ XMFLOAT3(-10.0f, -1.0f, -10.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(2.0f, 0.0f) },
		{ XMFLOAT3(10.0f, -1.0f, -10.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(2.0f, 10.0f) },
		{ XMFLOAT3(10.0f, 1.0f, -10.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 10.0f) },

		//后面墙
		{ XMFLOAT3(-10.0f, 1.0f, 10.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(10.0f, 1.0f, 10.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(0.0f, 10.0f) },
		{ XMFLOAT3(-10.0f, -1.0f, 10.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(2.0f, 0.0f) },

		{ XMFLOAT3(-10.0f, -1.0f, 10.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(2.0f, 0.0f) },
		{ XMFLOAT3(10.0f, 1.0f, 10.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(0.0f, 10.0f) },
		{ XMFLOAT3(10.0f, -1.0f, 10.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(2.0f, 10.0f) },

		//左侧墙
		{ XMFLOAT3(-10.0f, 1.0f, -10.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-10.0f, -1.0f, 10.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(2.0f, 10.0f) },
		{ XMFLOAT3(-10.0f, -1.0f, -10.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(2.0f, 0.0f) },

		{ XMFLOAT3(-10.0f, 1.0f, -10.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-10.0f, 1.0f, 10.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 10.0f) },
		{ XMFLOAT3(-10.0f, -1.0f, 10.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(2.0f, 10.0f) },

		//右侧墙
		{ XMFLOAT3(10.0f, 1.0f, -10.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(10.0f, -1.0f, -10.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(2.0f, 0.0f) },
		{ XMFLOAT3(10.0f, -1.0f, 10.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(2.0f, 10.0f) },

		{ XMFLOAT3(10.0f, 1.0f, -10.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(10.0f, -1.0f, 10.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(2.0f, 10.0f) },
		{ XMFLOAT3(10.0f, 1.0f, 10.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 10.0f) },

		//水面的顶点----------------------------------------------------------------------
		{ XMFLOAT3(-10.0f, 1.0f, 10.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(10.0f, 1.0f, 10.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(10.0f, 0.0f) },
		{ XMFLOAT3(-10.0f, 1.0f, -10.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 10.0f) },

		{ XMFLOAT3(-10.0f, 1.0f, -10.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 10.0f) },
		{ XMFLOAT3(10.0f, 1.0f, 10.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(10.0f, 0.0f) },
		{ XMFLOAT3(10.0f, 1.0f, -10.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(10.0f, 10.0f) },


	};	UINT vertexCount = ARRAYSIZE(vertices);
	//首先声明一个D3D11_BUFFER_DESC的对象bd
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(Vertex) * vertexCount;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;  //注意：这里表示创建的是顶点缓存
	bd.CPUAccessFlags = 0;

	//声明一个D3D11_SUBRESOURCE_DATA数据用于初始化子资源
	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = vertices;         //设置需要初始化的数据，这里的数据就是顶点数组
	InitData.SysMemPitch = 0;
	InitData.SysMemSlicePitch = 0;

	//声明一个ID3D11Buffer对象作为顶点缓存
	ID3D11Buffer* vertexBuffer;
	//调用CreateBuffer创建顶点缓存
	hr = device->CreateBuffer(&bd, &InitData, &vertexBuffer);
	if (FAILED(hr))
	{
		::MessageBox(NULL, L"创建VertexBuffer失败", L"Error", MB_OK);
		return hr;
	}

	UINT stride = sizeof(Vertex);                 //获取Vertex的大小作为跨度
	UINT offset = 0;                              //设置偏移量为0
	//设置顶点缓存，参数的解释见实验4
	immediateContext->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
	//指定图元类型，D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST表示图元为三角形
	immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//*************第四步创建顶点缓存***********************

	//*************第五步设置材质和光照***********************
	// 设置材质：3中光照的反射率以及镜面光反射系数
	//反射率中前三位表示红绿蓝光的反射率，1表示完全反射，0表示完全吸收
	//池子地板及墙的材质
	floorMaterial.ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f); //前三位分别表示红绿蓝光的反射率
	floorMaterial.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f); //同上
	floorMaterial.specular = XMFLOAT4(0.3f, 0.3f, 0.3f, 16.0f);//同上
	floorMaterial.power = 5.0f;

	//箱子材质
	boxMaterial.ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f); //前三位分别表示红绿蓝光的反射率
	boxMaterial.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f); //同上
	boxMaterial.specular = XMFLOAT4(0.3f, 0.3f, 0.3f, 16.0f);//同上
	boxMaterial.power = 5.0f;

	//水面材质
	waterMaterial.ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f); //前三位分别表示红绿蓝光的反射率
	//waterMaterial.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 0.8f); //透明度20%
	//waterMaterial.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 0.2f); //透明度80%
	waterMaterial.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 0.5f); //透明度
	waterMaterial.specular = XMFLOAT4(0.8f, 0.8f, 0.8f, 32.0f);//同上
	waterMaterial.power = 5.0f;

	// 设置光源
	Light dirLight, pointLight, spotLight;
	// 方向光只需要设置：方向、3种光照强度
	dirLight.type = 0;
	dirLight.direction = XMFLOAT4(0.0f, -1.0f, 0.0f, 1.0f); //光照方向
	dirLight.ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);   //前三位分别表示红绿蓝光的强度
	dirLight.diffuse = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);   //同上
	dirLight.specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);  //同上


	// 点光源需要设置：位置、3中光照强度、3个衰减因子
	pointLight.type = 1;
	pointLight.position = XMFLOAT4(0.0f, 5.0f, 0.0f, 1.0f); //光源位置
	pointLight.ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);   //前三位分别表示红绿蓝光的强度
	pointLight.diffuse = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);   //同上
	pointLight.specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);  //同上
	pointLight.attenuation0 = 0;      //常量衰减因子
	pointLight.attenuation1 = 0.1f;   //一次衰减因子
	pointLight.attenuation2 = 0;      //二次衰减因子

	// 聚光灯需要设置Light结构中所有的成员
	spotLight.type = 2;
	spotLight.position = XMFLOAT4(0.0f, 10.0f, 0.0f, 1.0f); //光源位置
	spotLight.direction = XMFLOAT4(0.0f, -1.0f, 0.0f, 1.0f); //光照方向
	spotLight.ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);   //前三位分别表示红绿蓝光的强度
	spotLight.diffuse = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);   //同上
	spotLight.specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);  //同上

	spotLight.attenuation0 = 0;    //常量衰减因子
	spotLight.attenuation1 = 0.1f; //一次衰减因子
	spotLight.attenuation2 = 0;    //二次衰减因子
	spotLight.alpha = XM_PI / 6;   //内锥角度
	spotLight.beta = XM_PI / 3;    //外锥角度
	spotLight.fallOff = 1.0f;      //衰减系数，一般为1.0

	light[0] = dirLight;
	light[1] = pointLight;
	light[2] = spotLight;
	//*************第五步设置材质和光照***********************
	return true;
}

void Cleanup()
{
	//释放全局指针
	if (renderTargetView) renderTargetView->Release();
	if (immediateContext) immediateContext->Release();
	if (swapChain) swapChain->Release();
	if (device) device->Release();

	if (vertexLayout) vertexLayout->Release();
	if (effect) effect->Release();

	if (textureBox) textureBox->Release();
	if (textureFloor) textureFloor->Release();
	if (textureWater) textureWater->Release();

	if (BlendStateAlpha) BlendStateAlpha->Release();
	if (noCulRS) noCulRS->Release();
}

bool Display(float timeDelta)
{
	if (device)
	{
		//声明一个数组存放颜色信息，4个元素分别表示红，绿，蓝以及alpha
		float ClearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f };
		immediateContext->ClearRenderTargetView(renderTargetView, ClearColor);
		//指定混合因子，一般不用它，除非在上面混合因子指定使用为blend factor
		float BlendFactor[] = { 0, 0, 0, 0 };

		//*****************第一部分 设置3个坐标系及光照的外部变量
		// 通过键盘的上下左右键来改变虚拟摄像头方向.
		static float angle = XM_PI;   //声明一个静态变量用于记录角度
		static float height = 2.0f;

		//这种案件相应方法见“补充知识”
		if (::GetAsyncKeyState(VK_LEFT) & 0x8000f) //响应键盘左方向键
			angle -= 2.0f * timeDelta;
		if (::GetAsyncKeyState(VK_RIGHT) & 0x8000f) //响应键盘右方向键
			angle += 2.0f * timeDelta;
		if (::GetAsyncKeyState(VK_UP) & 0x8000f)    //响应键盘上方向键
			height += 5.0f * timeDelta;
		if (::GetAsyncKeyState(VK_DOWN) & 0x8000f)  //响应键盘下方向键
			height -= 5.0f * timeDelta;

		if (height < -5.0f) height = -5.0f;//限制镜头最远距离
		if (height > 5.0f) height = 5.0f;//限制镜头最近距离

		//初始化世界矩阵
		world = XMMatrixIdentity();
		XMVECTOR Eye = XMVectorSet(cosf(angle)*height, 3.0f, sinf(angle)*height, 0.0f);//相机位置
		XMVECTOR At = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);  //目标位置
		XMVECTOR Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);  //up
		view = XMMatrixLookAtLH(Eye, At, Up);   //设置观察坐标系
		//设置投影矩阵
		projection = XMMatrixPerspectiveFovLH(XM_PIDIV2, 800.0f / 600.0f, 0.01f, 100.0f);

		//将坐标变换矩阵的常量缓存中的矩阵和坐标设置到Effect框架中---------------------
		//注意：这里的"World"，"View"，"Projection"，"EyePosition"是在.fx文件中定义的
		effect->GetVariableByName("World")->AsMatrix()->SetMatrix((float*)&world);  //设置世界坐标系
		effect->GetVariableByName("View")->AsMatrix()->SetMatrix((float*)&view);    //设置观察坐标系
		effect->GetVariableByName("Projection")->AsMatrix()->SetMatrix((float*)&projection); //设置投影坐标系
		effect->GetVariableByName("EyePosition")->AsMatrix()->SetMatrix((float*)&Eye); //设置视点
		//光源的常量缓存中的光源信息设置到Effect框架中
		SetLightEffect(light[lightType]);
		//************************第一部分结束*************************

		//************************第二部分 绘制各个物体*************************
		//绘制多个物体时必须先绘制不透明物体，再绘制透明物体，因为后绘制的物体会挡住先绘制的物体
		D3DX11_TECHNIQUE_DESC techDesc;
		technique->GetDesc(&techDesc);

		//绘制池子
		//设置池子的材质信息
		//材质信息的常量缓存中的材质信息设置到Effect框架中-----------------------------
		effect->GetVariableByName("MatAmbient")->AsVector()->SetFloatVector((float*)&(floorMaterial.ambient));
		effect->GetVariableByName("MatDiffuse")->AsVector()->SetFloatVector((float*)&(floorMaterial.diffuse));
		effect->GetVariableByName("MatSpecular")->AsVector()->SetFloatVector((float*)&(floorMaterial.specular));
		effect->GetVariableByName("MatPower")->AsScalar()->SetFloat(floorMaterial.power);
		//设置池子纹理
		effect->GetVariableByName("Texture")->AsShaderResource()->SetResource(textureFloor);
		technique->GetPassByIndex(0)->Apply(0, immediateContext);
		//第二个参数表示从顶点数组第36个（从0开始计算）顶点开始绘制
		immediateContext->Draw(30, 36);

		//绘制箱子
		//设置箱子的材质信息
		//材质信息的常量缓存中的材质信息设置到Effect框架中-----------------------------
		effect->GetVariableByName("MatAmbient")->AsVector()->SetFloatVector((float*)&(boxMaterial.ambient));
		effect->GetVariableByName("MatDiffuse")->AsVector()->SetFloatVector((float*)&(boxMaterial.diffuse));
		effect->GetVariableByName("MatSpecular")->AsVector()->SetFloatVector((float*)&(boxMaterial.specular));
		effect->GetVariableByName("MatPower")->AsScalar()->SetFloat(boxMaterial.power);
		//设置箱子纹理
		effect->GetVariableByName("Texture")->AsShaderResource()->SetResource(textureBox);
		technique->GetPassByIndex(0)->Apply(0, immediateContext);
		immediateContext->Draw(36,0);

		//绘制水面
		immediateContext->OMSetBlendState(BlendStateAlpha, BlendFactor, 0xffffffff);//开启混合
		immediateContext->RSSetState(noCulRS);//关闭背面消隐

		if (::GetAsyncKeyState(0x41) & 0x8000f)
			waterMaterial.diffuse.w -= 0.1f*timeDelta;
		if (::GetAsyncKeyState(0x44) & 0x8000f)
			waterMaterial.diffuse.w += 0.1f*timeDelta;

		//设置水面的材质信息
		//材质信息的常量缓存中的材质信息设置到Effect框架中-----------------------------
		effect->GetVariableByName("MatAmbient")->AsVector()->SetFloatVector((float*)&(waterMaterial.ambient));
		effect->GetVariableByName("MatDiffuse")->AsVector()->SetFloatVector((float*)&(waterMaterial.diffuse));
		effect->GetVariableByName("MatSpecular")->AsVector()->SetFloatVector((float*)&(waterMaterial.specular));
		effect->GetVariableByName("MatPower")->AsScalar()->SetFloat(waterMaterial.power);
		//设置水面纹理
		effect->GetVariableByName("Texture")->AsShaderResource()->SetResource(textureWater);
		technique->GetPassByIndex(0)->Apply(0, immediateContext);
		immediateContext->Draw(6, 66);
		//关闭混合
		immediateContext->OMSetBlendState(0, 0, 0xffffffff);
		immediateContext->RSSetState(0);//恢复背面消隐
		//************************第二部分 绘制各个物体*************************

		swapChain->Present(0, 0);
	}
	return true;
}
//**************框架函数******************


//
// 回调函数
//
LRESULT CALLBACK d3d::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_DESTROY:
		::PostQuitMessage(0);
		break;

	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE)
			::DestroyWindow(hwnd);

		if (wParam == VK_F1)  //按F1键将光源类型改为方向光
			lightType = 0;
		if (wParam == VK_F2)  //按F2键将光源类型改为点光源
			lightType = 1;
		if (wParam == VK_F3)  //按F3键将光源类型改为聚光灯光源
			lightType = 2;

		break;
	}
	return ::DefWindowProc(hwnd, msg, wParam, lParam);
}

//
// 主函数WinMain
//
int WINAPI WinMain(HINSTANCE hinstance,
	HINSTANCE prevInstance,
	PSTR cmdLine,
	int showCmd)
{

	//初始化
	//**注意**:最上面声明的IDirect3DDevice9指针，在这里作为参数传给InitD3D函数
	if (!d3d::InitD3D(hinstance,
		800,
		600,
		&renderTargetView,
		&immediateContext,
		&swapChain,
		&device))// [out]The created device.
	{
		::MessageBox(0, L"InitD3D() - FAILED", 0, 0);
		return 0;
	}

	if (!Setup())
	{
		::MessageBox(0, L"Setup() - FAILED", 0, 0);
		return 0;
	}

	d3d::EnterMsgLoop(Display);

	Cleanup();

	return 0;
}

//光源的常量缓存设置到Effect框架中
//由于光照设置比较复杂，所以以一个函数来进行设置
void SetLightEffect(Light light)
{
	//首先将光照类型，环境光强度，漫射光强度，镜面光强度设置到Effect中
	effect->GetVariableByName("type")->AsScalar()->SetInt(light.type);
	effect->GetVariableByName("LightAmbient")->AsVector()->SetFloatVector((float*)&(light.ambient));
	effect->GetVariableByName("LightDiffuse")->AsVector()->SetFloatVector((float*)&(light.diffuse));
	effect->GetVariableByName("LightSpecular")->AsVector()->SetFloatVector((float*)&(light.specular));

	//下面根据光照类型的不同设置不同的属性
	if (light.type == 0)  //方向光
	{
		//方向光只需要“方向”这个属性即可
		effect->GetVariableByName("LightDirection")->AsVector()->SetFloatVector((float*)&(light.direction));
		//将方向光的Tectnique设置到Effect
		technique = effect->GetTechniqueByName("T_DirLight");
	}
	else if (light.type == 1)  //点光源
	{
		//点光源需要“位置”，“常量衰变因子”，“一次衰变因子”，“二次衰变因子”
		effect->GetVariableByName("LightPosition")->AsVector()->SetFloatVector((float*)&(light.position));
		effect->GetVariableByName("LightAtt0")->AsScalar()->SetFloat(light.attenuation0);
		effect->GetVariableByName("LightAtt1")->AsScalar()->SetFloat(light.attenuation1);
		effect->GetVariableByName("LightAtt2")->AsScalar()->SetFloat(light.attenuation2);

		//将点光源的Tectnique设置到Effect
		technique = effect->GetTechniqueByName("T_PointLight");
	}
	else if (light.type == 2) //聚光灯光源
	{
		//点光源需要“方向”，“方向”，“常量衰变因子”，“一次衰变因子”，“二次衰变因子”
		//“内锥角度”，“外锥角度”，“聚光灯衰减系数”
		effect->GetVariableByName("LightPosition")->AsVector()->SetFloatVector((float*)&(light.position));
		effect->GetVariableByName("LightDirection")->AsVector()->SetFloatVector((float*)&(light.direction));

		effect->GetVariableByName("LightAtt0")->AsScalar()->SetFloat(light.attenuation0);
		effect->GetVariableByName("LightAtt1")->AsScalar()->SetFloat(light.attenuation1);
		effect->GetVariableByName("LightAtt2")->AsScalar()->SetFloat(light.attenuation2);

		effect->GetVariableByName("LightAlpha")->AsScalar()->SetFloat(light.alpha);
		effect->GetVariableByName("LightBeta")->AsScalar()->SetFloat(light.beta);
		effect->GetVariableByName("LightFallOff")->AsScalar()->SetFloat(light.fallOff);

		//将聚光灯光源的Tectnique设置到Effect
		technique = effect->GetTechniqueByName("T_SpotLight");
	}
}