#include "d3dUtility.h"
#include "Light.h"


//����ȫ�ֵ�ָ��
ID3D11Device* device = NULL;//D3D11�豸�ӿ�
IDXGISwapChain* swapChain = NULL;//�������ӿ�
ID3D11DeviceContext* immediateContext = NULL;
ID3D11RenderTargetView* renderTargetView = NULL;//��ȾĿ����ͼ  

//��ɫ��
//ID3D11VertexShader* m_VertexShader;
//ID3D11PixelShader* m_PixelShader;

//Effect���ȫ��ָ��
ID3D11InputLayout* vertexLayout;
ID3DX11Effect* effect;
ID3DX11EffectTechnique* technique;

//������������ϵ����
XMMATRIX world;         //��������任�ľ���
XMMATRIX view;          //���ڹ۲�任�ľ���
XMMATRIX projection;    //����ͶӰ�任�ľ���

//�������ʺ͹��յ�ȫ�ֶ���
Material		boxMaterial;      //���Ӳ���
Material		floorMaterial;      //�ذ����
Material		waterMaterial;      //ˮ�����
Light			light[3];      //��Դ����
int             lightType = 0;  //��Դ����

ID3D11ShaderResourceView * textureBox;//��������
ID3D11ShaderResourceView * textureFloor;//�ذ�����
ID3D11ShaderResourceView * textureWater;//ˮ������

ID3D11BlendState * BlendStateAlpha;//���״̬
ID3D11RasterizerState * noCulRS;//��������״̬

void SetLightEffect(Light light);

//ID3D11DepthStencilView* depthStencilView;  //���ģ����ͼ
//XMVECTOR		eyePositin;                //�ӵ�λ��

//����һ������ṹ����������������ͷ�����
struct Vertex
{
	XMFLOAT3 Pos;
	XMFLOAT3 Normal;
	XMFLOAT2 Tex;
};

void SetLightEffect(Light light);

//**************����Ϊ��ܺ���******************
bool Setup()
{
	//������Ҫ����3����Ҫ����
	//��һ����.fx�ļ�����ID3DEffect����
	//�ڶ����������㻺��
	//���������ñ任����ϵ
	//���Ĳ����ò��ʺ͹���
	//*************��һ����.�����ⲿ�ļ�������fx�ļ���ͼ���ļ���****************************
	HRESULT hr = S_OK;              //����HRESULT�Ķ������ڼ�¼���������Ƿ�ɹ�
	ID3DBlob* pTechBlob = NULL;     //����ID3DBlob�Ķ������ڴ�Ŵ��ļ���ȡ����Ϣ
	//������֮ǰ������.fx�ļ���ȡ��ɫ�������Ϣ
	hr = D3DX11CompileFromFile(L"Shader.fx", NULL, NULL, NULL, "fx_5_0",
		D3DCOMPILE_ENABLE_STRICTNESS, 0, NULL, &pTechBlob, NULL, NULL);
	if (FAILED(hr))
	{
		::MessageBox(NULL, L"fx�ļ�����ʧ��", L"Error", MB_OK); //�����ȡʧ�ܣ�����������Ϣ
		return hr;
	}
	//����D3DX11CreateEffectFromMemory����ID3DEffect����
	hr = D3DX11CreateEffectFromMemory(pTechBlob->GetBufferPointer(),
		pTechBlob->GetBufferSize(), 0, device, &effect);

	if (FAILED(hr))
	{
		::MessageBox(NULL, L"����Effectʧ��", L"Error", MB_OK);  //����ʧ�ܣ�����������Ϣ
		return hr;
	}
	//��������
	D3DX11CreateShaderResourceViewFromFile(device, L"BOX.BMP", NULL, NULL, &textureBox, NULL);
    //�ذ弰ǽ������
	D3DX11CreateShaderResourceViewFromFile(device, L"checkboard.dds", NULL, NULL, &textureFloor, NULL);
	//ˮ������
	D3DX11CreateShaderResourceViewFromFile(device, L"water.dds", NULL, NULL, &textureWater, NULL);

	//*************��һ����.�����ⲿ�ļ�������fx�ļ���ͼ���ļ���****************************

	//*************�ڶ�������������Ⱦ״̬***************************
	//�ȴ���һ�����״̬������
	D3D11_BLEND_DESC blendDesc;
	ZeroMemory(&blendDesc, sizeof(blendDesc));
	blendDesc.AlphaToCoverageEnable = false;//�ر�AlphaToCoverage���ز�������
	//�رն��RenderTargetʹ�ò�ͬ�Ļ��״̬
	blendDesc.IndependentBlendEnable = false;
	//ֻ���RenderTarget[0]���û��ƻ��״̬������1~7
	blendDesc.RenderTarget[0].BlendEnable = true;//�������
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;//����Դ����
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;//Ŀ������
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;//��ϲ���
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;//Դ��ϰٷֱ�
	//Ŀ���ϰٷֱ�����
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	//��ϰٷֱȲ���
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;//д����
	//����ID3D11BlendState�ӿ�
	device->CreateBlendState(&blendDesc, &BlendStateAlpha);
	//�رձ�������
	D3D11_RASTERIZER_DESC ncDesc;
	ZeroMemory(&ncDesc, sizeof(ncDesc));
	ncDesc.CullMode = D3D11_CULL_NONE;//�޳��ض������Σ����ﲻ�޳��κ�������
	ncDesc.FillMode = D3D11_FILL_SOLID;//���ģʽ�������������������
	ncDesc.FrontCounterClockwise = false;//�Ƿ�������ʱ�������������Ϊ����
	ncDesc.DepthClipEnable = true;//������Ȳü�
	//����һ���رձ���������״̬������Ҫ��ʱ������ø�ִ��������
	if (FAILED(device->CreateRasterizerState(&ncDesc,&noCulRS)))
	{
		::MessageBox(NULL, L"CreateRasterizerStateʧ��", L"Error", MB_OK);  //����ʧ�ܣ�����������Ϣ
		return false;
	}
	
	//*************�ڶ�������������Ⱦ״̬****************************

	//*************�������������벼��***********************
	//��GetTechniqueByName��ȡID3D11EffectTechnique�Ķ���
	//�������޹��յĵ�Effect
	technique = effect->GetTechniqueByName("TexTech");

	//D3DX11_PASS_DESC�ṹ��������һ��Effect Pass
	D3DX11_PASS_DESC PassDesc;
	//����GetPassByIndex��ȡEffect Pass
	//������GetDesc��ȡEffect Pass��������������PassDesc������
	technique->GetPassByIndex(0)->GetDesc(&PassDesc);

	//�������������벼��
	//�������Ƕ���һ��D3D11_INPUT_ELEMENT_DESC���飬
	//�������Ƕ���Ķ���ṹ����λ������ͷ��������������������������Ԫ��
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	//layoutԪ�ظ���
	UINT numElements = ARRAYSIZE(layout);
	//����CreateInputLayout�������벼��
	hr = device->CreateInputLayout(layout, numElements, PassDesc.pIAInputSignature,
		PassDesc.IAInputSignatureSize, &vertexLayout);
	//�������ɵ����벼�ֵ��豸��������
	immediateContext->IASetInputLayout(vertexLayout);
	if (FAILED(hr))
	{
		::MessageBox(NULL, L"����Input Layoutʧ��", L"Error", MB_OK);
		return hr;
	}

	//*************�������������벼��***********************

	//*************���Ĳ��������㻺��***********************
	Vertex vertices[] =
	{
		//���ӵĶ���----------------------------------------------------------------------
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
		//���ӵĶ���----------------------------------------------------------------------
	    //�ذ�
		{ XMFLOAT3(-10.0f, -1.0f, 10.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(10.0f, -1.0f, 10.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(10.0f, 0.0f) },
		{ XMFLOAT3(-10.0f, -1.0f, -10.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 10.0f) },

		{ XMFLOAT3(-10.0f, -1.0f, -10.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 10.0f) },
		{ XMFLOAT3(10.0f, -1.0f, 10.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(10.0f, 0.0f) },
		{ XMFLOAT3(10.0f, -1.0f, -10.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(10.0f, 10.0f) },

		//ǰ��ǽ
		{ XMFLOAT3(-10.0f, 1.0f, -10.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-10.0f, -1.0f, -10.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(2.0f, 0.0f) },
		{ XMFLOAT3(10.0f, 1.0f, -10.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 10.0f) },

		{ XMFLOAT3(-10.0f, -1.0f, -10.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(2.0f, 0.0f) },
		{ XMFLOAT3(10.0f, -1.0f, -10.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(2.0f, 10.0f) },
		{ XMFLOAT3(10.0f, 1.0f, -10.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 10.0f) },

		//����ǽ
		{ XMFLOAT3(-10.0f, 1.0f, 10.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(10.0f, 1.0f, 10.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(0.0f, 10.0f) },
		{ XMFLOAT3(-10.0f, -1.0f, 10.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(2.0f, 0.0f) },

		{ XMFLOAT3(-10.0f, -1.0f, 10.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(2.0f, 0.0f) },
		{ XMFLOAT3(10.0f, 1.0f, 10.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(0.0f, 10.0f) },
		{ XMFLOAT3(10.0f, -1.0f, 10.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(2.0f, 10.0f) },

		//���ǽ
		{ XMFLOAT3(-10.0f, 1.0f, -10.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-10.0f, -1.0f, 10.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(2.0f, 10.0f) },
		{ XMFLOAT3(-10.0f, -1.0f, -10.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(2.0f, 0.0f) },

		{ XMFLOAT3(-10.0f, 1.0f, -10.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-10.0f, 1.0f, 10.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 10.0f) },
		{ XMFLOAT3(-10.0f, -1.0f, 10.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(2.0f, 10.0f) },

		//�Ҳ�ǽ
		{ XMFLOAT3(10.0f, 1.0f, -10.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(10.0f, -1.0f, -10.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(2.0f, 0.0f) },
		{ XMFLOAT3(10.0f, -1.0f, 10.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(2.0f, 10.0f) },

		{ XMFLOAT3(10.0f, 1.0f, -10.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(10.0f, -1.0f, 10.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(2.0f, 10.0f) },
		{ XMFLOAT3(10.0f, 1.0f, 10.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 10.0f) },

		//ˮ��Ķ���----------------------------------------------------------------------
		{ XMFLOAT3(-10.0f, 1.0f, 10.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(10.0f, 1.0f, 10.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(10.0f, 0.0f) },
		{ XMFLOAT3(-10.0f, 1.0f, -10.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 10.0f) },

		{ XMFLOAT3(-10.0f, 1.0f, -10.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 10.0f) },
		{ XMFLOAT3(10.0f, 1.0f, 10.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(10.0f, 0.0f) },
		{ XMFLOAT3(10.0f, 1.0f, -10.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(10.0f, 10.0f) },


	};	UINT vertexCount = ARRAYSIZE(vertices);
	//��������һ��D3D11_BUFFER_DESC�Ķ���bd
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(Vertex) * vertexCount;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;  //ע�⣺�����ʾ�������Ƕ��㻺��
	bd.CPUAccessFlags = 0;

	//����һ��D3D11_SUBRESOURCE_DATA�������ڳ�ʼ������Դ
	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = vertices;         //������Ҫ��ʼ�������ݣ���������ݾ��Ƕ�������
	InitData.SysMemPitch = 0;
	InitData.SysMemSlicePitch = 0;

	//����һ��ID3D11Buffer������Ϊ���㻺��
	ID3D11Buffer* vertexBuffer;
	//����CreateBuffer�������㻺��
	hr = device->CreateBuffer(&bd, &InitData, &vertexBuffer);
	if (FAILED(hr))
	{
		::MessageBox(NULL, L"����VertexBufferʧ��", L"Error", MB_OK);
		return hr;
	}

	UINT stride = sizeof(Vertex);                 //��ȡVertex�Ĵ�С��Ϊ���
	UINT offset = 0;                              //����ƫ����Ϊ0
	//���ö��㻺�棬�����Ľ��ͼ�ʵ��4
	immediateContext->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
	//ָ��ͼԪ���ͣ�D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST��ʾͼԪΪ������
	immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//*************���Ĳ��������㻺��***********************

	//*************���岽���ò��ʺ͹���***********************
	// ���ò��ʣ�3�й��յķ������Լ�����ⷴ��ϵ��
	//��������ǰ��λ��ʾ��������ķ����ʣ�1��ʾ��ȫ���䣬0��ʾ��ȫ����
	//���ӵذ弰ǽ�Ĳ���
	floorMaterial.ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f); //ǰ��λ�ֱ��ʾ��������ķ�����
	floorMaterial.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f); //ͬ��
	floorMaterial.specular = XMFLOAT4(0.3f, 0.3f, 0.3f, 16.0f);//ͬ��
	floorMaterial.power = 5.0f;

	//���Ӳ���
	boxMaterial.ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f); //ǰ��λ�ֱ��ʾ��������ķ�����
	boxMaterial.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f); //ͬ��
	boxMaterial.specular = XMFLOAT4(0.3f, 0.3f, 0.3f, 16.0f);//ͬ��
	boxMaterial.power = 5.0f;

	//ˮ�����
	waterMaterial.ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f); //ǰ��λ�ֱ��ʾ��������ķ�����
	//waterMaterial.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 0.8f); //͸����20%
	//waterMaterial.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 0.2f); //͸����80%
	waterMaterial.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 0.5f); //͸����
	waterMaterial.specular = XMFLOAT4(0.8f, 0.8f, 0.8f, 32.0f);//ͬ��
	waterMaterial.power = 5.0f;

	// ���ù�Դ
	Light dirLight, pointLight, spotLight;
	// �����ֻ��Ҫ���ã�����3�ֹ���ǿ��
	dirLight.type = 0;
	dirLight.direction = XMFLOAT4(0.0f, -1.0f, 0.0f, 1.0f); //���շ���
	dirLight.ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);   //ǰ��λ�ֱ��ʾ���������ǿ��
	dirLight.diffuse = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);   //ͬ��
	dirLight.specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);  //ͬ��


	// ���Դ��Ҫ���ã�λ�á�3�й���ǿ�ȡ�3��˥������
	pointLight.type = 1;
	pointLight.position = XMFLOAT4(0.0f, 5.0f, 0.0f, 1.0f); //��Դλ��
	pointLight.ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);   //ǰ��λ�ֱ��ʾ���������ǿ��
	pointLight.diffuse = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);   //ͬ��
	pointLight.specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);  //ͬ��
	pointLight.attenuation0 = 0;      //����˥������
	pointLight.attenuation1 = 0.1f;   //һ��˥������
	pointLight.attenuation2 = 0;      //����˥������

	// �۹����Ҫ����Light�ṹ�����еĳ�Ա
	spotLight.type = 2;
	spotLight.position = XMFLOAT4(0.0f, 10.0f, 0.0f, 1.0f); //��Դλ��
	spotLight.direction = XMFLOAT4(0.0f, -1.0f, 0.0f, 1.0f); //���շ���
	spotLight.ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);   //ǰ��λ�ֱ��ʾ���������ǿ��
	spotLight.diffuse = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);   //ͬ��
	spotLight.specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);  //ͬ��

	spotLight.attenuation0 = 0;    //����˥������
	spotLight.attenuation1 = 0.1f; //һ��˥������
	spotLight.attenuation2 = 0;    //����˥������
	spotLight.alpha = XM_PI / 6;   //��׶�Ƕ�
	spotLight.beta = XM_PI / 3;    //��׶�Ƕ�
	spotLight.fallOff = 1.0f;      //˥��ϵ����һ��Ϊ1.0

	light[0] = dirLight;
	light[1] = pointLight;
	light[2] = spotLight;
	//*************���岽���ò��ʺ͹���***********************
	return true;
}

void Cleanup()
{
	//�ͷ�ȫ��ָ��
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
		//����һ����������ɫ��Ϣ��4��Ԫ�طֱ��ʾ�죬�̣����Լ�alpha
		float ClearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f };
		immediateContext->ClearRenderTargetView(renderTargetView, ClearColor);
		//ָ��������ӣ�һ�㲻����������������������ָ��ʹ��Ϊblend factor
		float BlendFactor[] = { 0, 0, 0, 0 };

		//*****************��һ���� ����3������ϵ�����յ��ⲿ����
		// ͨ�����̵��������Ҽ����ı���������ͷ����.
		static float angle = XM_PI;   //����һ����̬�������ڼ�¼�Ƕ�
		static float height = 2.0f;

		//���ְ�����Ӧ������������֪ʶ��
		if (::GetAsyncKeyState(VK_LEFT) & 0x8000f) //��Ӧ���������
			angle -= 2.0f * timeDelta;
		if (::GetAsyncKeyState(VK_RIGHT) & 0x8000f) //��Ӧ�����ҷ����
			angle += 2.0f * timeDelta;
		if (::GetAsyncKeyState(VK_UP) & 0x8000f)    //��Ӧ�����Ϸ����
			height += 5.0f * timeDelta;
		if (::GetAsyncKeyState(VK_DOWN) & 0x8000f)  //��Ӧ�����·����
			height -= 5.0f * timeDelta;

		if (height < -5.0f) height = -5.0f;//���ƾ�ͷ��Զ����
		if (height > 5.0f) height = 5.0f;//���ƾ�ͷ�������

		//��ʼ���������
		world = XMMatrixIdentity();
		XMVECTOR Eye = XMVectorSet(cosf(angle)*height, 3.0f, sinf(angle)*height, 0.0f);//���λ��
		XMVECTOR At = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);  //Ŀ��λ��
		XMVECTOR Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);  //up
		view = XMMatrixLookAtLH(Eye, At, Up);   //���ù۲�����ϵ
		//����ͶӰ����
		projection = XMMatrixPerspectiveFovLH(XM_PIDIV2, 800.0f / 600.0f, 0.01f, 100.0f);

		//������任����ĳ��������еľ�����������õ�Effect�����---------------------
		//ע�⣺�����"World"��"View"��"Projection"��"EyePosition"����.fx�ļ��ж����
		effect->GetVariableByName("World")->AsMatrix()->SetMatrix((float*)&world);  //������������ϵ
		effect->GetVariableByName("View")->AsMatrix()->SetMatrix((float*)&view);    //���ù۲�����ϵ
		effect->GetVariableByName("Projection")->AsMatrix()->SetMatrix((float*)&projection); //����ͶӰ����ϵ
		effect->GetVariableByName("EyePosition")->AsMatrix()->SetMatrix((float*)&Eye); //�����ӵ�
		//��Դ�ĳ��������еĹ�Դ��Ϣ���õ�Effect�����
		SetLightEffect(light[lightType]);
		//************************��һ���ֽ���*************************

		//************************�ڶ����� ���Ƹ�������*************************
		//���ƶ������ʱ�����Ȼ��Ʋ�͸�����壬�ٻ���͸�����壬��Ϊ����Ƶ�����ᵲס�Ȼ��Ƶ�����
		D3DX11_TECHNIQUE_DESC techDesc;
		technique->GetDesc(&techDesc);

		//���Ƴ���
		//���ó��ӵĲ�����Ϣ
		//������Ϣ�ĳ��������еĲ�����Ϣ���õ�Effect�����-----------------------------
		effect->GetVariableByName("MatAmbient")->AsVector()->SetFloatVector((float*)&(floorMaterial.ambient));
		effect->GetVariableByName("MatDiffuse")->AsVector()->SetFloatVector((float*)&(floorMaterial.diffuse));
		effect->GetVariableByName("MatSpecular")->AsVector()->SetFloatVector((float*)&(floorMaterial.specular));
		effect->GetVariableByName("MatPower")->AsScalar()->SetFloat(floorMaterial.power);
		//���ó�������
		effect->GetVariableByName("Texture")->AsShaderResource()->SetResource(textureFloor);
		technique->GetPassByIndex(0)->Apply(0, immediateContext);
		//�ڶ���������ʾ�Ӷ��������36������0��ʼ���㣩���㿪ʼ����
		immediateContext->Draw(30, 36);

		//��������
		//�������ӵĲ�����Ϣ
		//������Ϣ�ĳ��������еĲ�����Ϣ���õ�Effect�����-----------------------------
		effect->GetVariableByName("MatAmbient")->AsVector()->SetFloatVector((float*)&(boxMaterial.ambient));
		effect->GetVariableByName("MatDiffuse")->AsVector()->SetFloatVector((float*)&(boxMaterial.diffuse));
		effect->GetVariableByName("MatSpecular")->AsVector()->SetFloatVector((float*)&(boxMaterial.specular));
		effect->GetVariableByName("MatPower")->AsScalar()->SetFloat(boxMaterial.power);
		//������������
		effect->GetVariableByName("Texture")->AsShaderResource()->SetResource(textureBox);
		technique->GetPassByIndex(0)->Apply(0, immediateContext);
		immediateContext->Draw(36,0);

		//����ˮ��
		immediateContext->OMSetBlendState(BlendStateAlpha, BlendFactor, 0xffffffff);//�������
		immediateContext->RSSetState(noCulRS);//�رձ�������

		if (::GetAsyncKeyState(0x41) & 0x8000f)
			waterMaterial.diffuse.w -= 0.1f*timeDelta;
		if (::GetAsyncKeyState(0x44) & 0x8000f)
			waterMaterial.diffuse.w += 0.1f*timeDelta;

		//����ˮ��Ĳ�����Ϣ
		//������Ϣ�ĳ��������еĲ�����Ϣ���õ�Effect�����-----------------------------
		effect->GetVariableByName("MatAmbient")->AsVector()->SetFloatVector((float*)&(waterMaterial.ambient));
		effect->GetVariableByName("MatDiffuse")->AsVector()->SetFloatVector((float*)&(waterMaterial.diffuse));
		effect->GetVariableByName("MatSpecular")->AsVector()->SetFloatVector((float*)&(waterMaterial.specular));
		effect->GetVariableByName("MatPower")->AsScalar()->SetFloat(waterMaterial.power);
		//����ˮ������
		effect->GetVariableByName("Texture")->AsShaderResource()->SetResource(textureWater);
		technique->GetPassByIndex(0)->Apply(0, immediateContext);
		immediateContext->Draw(6, 66);
		//�رջ��
		immediateContext->OMSetBlendState(0, 0, 0xffffffff);
		immediateContext->RSSetState(0);//�ָ���������
		//************************�ڶ����� ���Ƹ�������*************************

		swapChain->Present(0, 0);
	}
	return true;
}
//**************��ܺ���******************


//
// �ص�����
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

		if (wParam == VK_F1)  //��F1������Դ���͸�Ϊ�����
			lightType = 0;
		if (wParam == VK_F2)  //��F2������Դ���͸�Ϊ���Դ
			lightType = 1;
		if (wParam == VK_F3)  //��F3������Դ���͸�Ϊ�۹�ƹ�Դ
			lightType = 2;

		break;
	}
	return ::DefWindowProc(hwnd, msg, wParam, lParam);
}

//
// ������WinMain
//
int WINAPI WinMain(HINSTANCE hinstance,
	HINSTANCE prevInstance,
	PSTR cmdLine,
	int showCmd)
{

	//��ʼ��
	//**ע��**:������������IDirect3DDevice9ָ�룬��������Ϊ��������InitD3D����
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

//��Դ�ĳ����������õ�Effect�����
//���ڹ������ñȽϸ��ӣ�������һ����������������
void SetLightEffect(Light light)
{
	//���Ƚ��������ͣ�������ǿ�ȣ������ǿ�ȣ������ǿ�����õ�Effect��
	effect->GetVariableByName("type")->AsScalar()->SetInt(light.type);
	effect->GetVariableByName("LightAmbient")->AsVector()->SetFloatVector((float*)&(light.ambient));
	effect->GetVariableByName("LightDiffuse")->AsVector()->SetFloatVector((float*)&(light.diffuse));
	effect->GetVariableByName("LightSpecular")->AsVector()->SetFloatVector((float*)&(light.specular));

	//������ݹ������͵Ĳ�ͬ���ò�ͬ������
	if (light.type == 0)  //�����
	{
		//�����ֻ��Ҫ������������Լ���
		effect->GetVariableByName("LightDirection")->AsVector()->SetFloatVector((float*)&(light.direction));
		//��������Tectnique���õ�Effect
		technique = effect->GetTechniqueByName("T_DirLight");
	}
	else if (light.type == 1)  //���Դ
	{
		//���Դ��Ҫ��λ�á���������˥�����ӡ�����һ��˥�����ӡ���������˥�����ӡ�
		effect->GetVariableByName("LightPosition")->AsVector()->SetFloatVector((float*)&(light.position));
		effect->GetVariableByName("LightAtt0")->AsScalar()->SetFloat(light.attenuation0);
		effect->GetVariableByName("LightAtt1")->AsScalar()->SetFloat(light.attenuation1);
		effect->GetVariableByName("LightAtt2")->AsScalar()->SetFloat(light.attenuation2);

		//�����Դ��Tectnique���õ�Effect
		technique = effect->GetTechniqueByName("T_PointLight");
	}
	else if (light.type == 2) //�۹�ƹ�Դ
	{
		//���Դ��Ҫ�����򡱣������򡱣�������˥�����ӡ�����һ��˥�����ӡ���������˥�����ӡ�
		//����׶�Ƕȡ�������׶�Ƕȡ������۹��˥��ϵ����
		effect->GetVariableByName("LightPosition")->AsVector()->SetFloatVector((float*)&(light.position));
		effect->GetVariableByName("LightDirection")->AsVector()->SetFloatVector((float*)&(light.direction));

		effect->GetVariableByName("LightAtt0")->AsScalar()->SetFloat(light.attenuation0);
		effect->GetVariableByName("LightAtt1")->AsScalar()->SetFloat(light.attenuation1);
		effect->GetVariableByName("LightAtt2")->AsScalar()->SetFloat(light.attenuation2);

		effect->GetVariableByName("LightAlpha")->AsScalar()->SetFloat(light.alpha);
		effect->GetVariableByName("LightBeta")->AsScalar()->SetFloat(light.beta);
		effect->GetVariableByName("LightFallOff")->AsScalar()->SetFloat(light.fallOff);

		//���۹�ƹ�Դ��Tectnique���õ�Effect
		technique = effect->GetTechniqueByName("T_SpotLight");
	}
}