#include "d3dUtility.h"

//D3D��ʼ��
//�ú��������������֣�1.����һ�����ڣ�2.��ʼ��D3D
//��������������
//1.HINSTANCE ��ǰӦ�ó���ʵ���ľ��
//2.int width, int height ���ڵĿ��
//3.ID3D10RenderTargetView ** renderTargetView Ŀ����Ⱦ��ͼָ��
//ID3D11DeviceContext ** immediateContext ����ִ��������ָ��
//IDXGISwapChain ** swapChain ������ָ��
//ID3D11Device ** device �豸��ָ�룬ÿ��D3D����������һ���豸

bool d3d::InitD3D(
	HINSTANCE hInstance,
	int width, int height,
	ID3D11RenderTargetView ** renderTargetView,
	ID3D11DeviceContext ** immediateContext,
	IDXGISwapChain ** swapChain,
	ID3D11Device ** device){
	/****************��һ���֣�����һ������ ��ʼ*********************/

	//�ο��ڶ���
	//�������ڵĲ��裺1.���һ�������ࣻ2.ע�ᴰ���ࣻ3.�������ڣ�4.������ʾ�͸���
	//1.���һ��������
	WNDCLASS wc;
	wc.style = CS_HREDRAW | CS_VREDRAW;//�����һ��������WNDCLASS�Ķ����崰�ڵ���ʽ
	//��������Ǳ�����ǰ���ڵ�ˮƽ��ֱ�ߴ緢���仯ʱ
	//���ڽ����ػ�
	wc.lpfnWndProc = (WNDPROC)d3d::WndProc;//����ָ���ص�������ָ��
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;//��ǰӦ�ó���ʵ���ľ������Win Main����
	wc.hIcon = ::LoadIcon(0, IDI_APPLICATION);//ָ��ͼ��
	wc.hCursor = ::LoadCursor(0, IDC_ARROW);//ָ�����
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.lpszMenuName = 0;//ָ�����޲˵���0Ϊ�޲˵�
	wc.lpszClassName = L"Direct3D11App";//ָ�򴰿�����ָ��
	//2.ע�ᴰ����
	if (!RegisterClass(&wc)){
		::MessageBox(0, L"RegisterClass-Failed", 0, 0);
		return false;
	}
	//3.��������
	HWND hwnd = ::CreateWindow(
		L"Direct3D11App",//��һ��Hello�����wc.lpszClassName��ͬ
		L"D3D11",//�ڶ���Helloʱ���ڵ���ʾ����
		WS_OVERLAPPEDWINDOW,//ָ���������ʱ�ص�ʽ����
		CW_USEDEFAULT,//��ʾ���ڵĺ�����ΪĬ��ֵ
		CW_USEDEFAULT,//��ʾ���ڵ�������ΪĬ��ֵ
		width,
		height,
		0,
		0,
		hInstance,
		0);
	//������ڴ���ʧ��MainWindowHandle����Ϊ0���򱨴�����
	if (!hwnd){
		::MessageBox(0, L"CreateWindow-Failed", 0, 0);
		return false;
	}
	//4.������ʾ�͸���
	::ShowWindow(hwnd, SW_SHOW);
	::UpdateWindow(hwnd);
	/****************��һ���֣�����һ������ ����*********************/

	/****************�ڶ����֣���ʼ��D3D ��ʼ*********************/
	//��ʼ��D3D�豸���裺
	//1.�����������������DXGI_SWAP_CHAIN_DESC�ṹ
	//2.ʹ��D3D11CreateDeviceAndSwapChain����D3D�豸��ID3D11Device��
	//  ִ�������Ľӿڣ�D3D11Devicecontext�����������ӿڣ�IDXGISwapChain��
	//3.����Ŀ����Ⱦ��ͼ��ID3D11ReaderTargetView��
	//4.�����ӿڣ�View Port��

	//��һ���������������������DXGI_SWAP_CHAIN_DESC�ṹ
	DXGI_SWAP_CHAIN_DESC sd;//��������һ��DXGI_SWAP_CHAIN_DESC����sd
	ZeroMemory(&sd, sizeof(sd));//��ZeroMemory��sd���г�ʼ��
	sd.BufferCount = 1;//�������к�̨����������ͨ��Ϊ1
	sd.BufferDesc.Width = width;//�������еĴ��ڿ�
	sd.BufferDesc.Height = height;//�������еĴ��ڸ�
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;//��ʾ������Alpha��8λ
	sd.BufferDesc.RefreshRate.Numerator = 60;//ˢ��Ƶ�ʵķ���Ϊ60
	sd.BufferDesc.RefreshRate.Denominator = 1;//ˢ��Ƶ�ʵķ�ĸΪ1    ˢ��Ƶ��Ϊÿ��6��
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;//����������̨�����÷�  ����CPU�Ժ�̨����ķ���
	sd.OutputWindow = hwnd;//ָ����ȾĿ�괰�ڵľ��
	sd.SampleDesc.Count = 1;//���ز����������в�����
	sd.SampleDesc.Quality = 0;//����count=1��quality=0
	sd.Windowed = TRUE;//TRUEΪ����ģʽ��FALSEΪȫ��ģʽ

	//�ڶ���������D3D�豸���������ӿڣ�ִ�������Ľӿ�
	//����һ������ȷ�����Դ���Featurelevel��˳��
	D3D_FEATURE_LEVEL featureLevels[] = {
		D3D_FEATURE_LEVEL_11_0,//D3D11��֧�ֵ�����������shader model5
		D3D_FEATURE_LEVEL_10_1,//D3D11��֧�ֵ�����������shader model4
		D3D_FEATURE_LEVEL_10_0
	};

	//��ȡD3D_FEATURE_LEVEL�����Ԫ�ظ���
	UINT numFeatureLevels = ARRAYSIZE(featureLevels);

	//����D3D11CreateDeviceAndSwapChain�������������豸��ִ��������
	//�ֱ����swapChain��device��immediateContext
	if (FAILED(D3D11CreateDeviceAndSwapChain(
		NULL,                    //ȷ����ʾ��������NULL��ʾĬ����ʾ������
		D3D_DRIVER_TYPE_HARDWARE,//�������ͣ������ʾʹ����άӲ������
		NULL,//ֻ����һ����������ΪD3D_DRIVER_TYPE_SOFTWAREʱ��ʹ�øò���
		0,   //Ҳ������ΪD3D11_CREATE_DEVICE_DEBUG��������ģʽ
		featureLevels,//ǰ�涨���D3D_FEATURE_LEVEL����
		numFeatureLevels,//D3D_FEATURE_LEVEL�����Ԫ�ظ���
		D3D11_SDK_VERSION,//SDK�İ汾������ΪD3D11
		&sd,              //ǰ�涨���DXGI_SWAP_CHAIN_DESC����sd
		swapChain,//���ش����õĽ�����ָ�룬InitiD3D�������ݵ�ʵ��
		device,//���ش����õ��豸��ָ�룬InitiD3D�������ݵ�ʵ��
		NULL,//���ص�ǰ�豸֧�ֵ�featureLevels�����еĵ�һ������һ������ΪNULL
		immediateContext)))//���ش����ŵ�ִ��������ָ�룬InitiD3D�������ݵ�ʵ��
	{
		::MessageBox(0, L"CreatDevice-Failed", 0, 0);
		return false;
	}

	//������������������Ŀ����Ⱦ��ͼ
	HRESULT hr = 0;//COMҪ�����еķ������᷵��һ��HRESULT���͵Ĵ����
	ID3D11Texture2D * pBackBuffer = NULL;//ID3D11Texture2D���͵ĺ�̨����ָ��
	//����GetBuffer()�����õ���̨������󣬲�����&pBackBuffe��
	hr = (*swapChain)->GetBuffer(0,//����������һ������Ϊ0
		__uuidof(ID3D11Texture2D),//��������
		(LPVOID*)& pBackBuffer);//����ָ��
	//�ж�GetBuffer()�Ƿ���óɹ�
	if (FAILED(hr)){
		::MessageBox(0, L"GetBuffer-Failed", 0, 0);
		return false;
	}

	//����CreateRenderTargetView��������ȾĿ����ͼ
	//�����ú����renderTargetView��
	hr = (*device)->CreateRenderTargetView(
		pBackBuffer,//���洴���õĺ�̨����
		NULL,//����ΪNULL�õ�Ĭ�ϵ���ȾĿ����ͼ
		renderTargetView);//���ش����õ���ȾĿ����ͼ
	pBackBuffer->Release();//�ͷź�̨����
	//�ж�CreateRenderTargetView�Ƿ���óɹ�
	if (FAILED(hr)){
		::MessageBox(0, L"CreateRender-Failed", 0, 0);
		return false;
	}

	//����ȾĿ����ͼ�󶨵���Ⱦ����
	(*immediateContext)->OMSetRenderTargets(1,//�󶨵�Ŀ����ͼ�ĸ���
		renderTargetView,                     //��ȾĿ����ͼ
		NULL);                                //�������ģ��

	//���Ĳ��������ӿڴ�С��D3D11Ĭ�ϲ��������ӿڣ������ֶ�����
	D3D11_VIEWPORT vp;//����һ���ӿڵĶ���
	vp.Width = width;
	vp.Height = height;
	vp.MinDepth = 0.0f;//���ֵ������
	vp.MaxDepth = 1.0f;//���ֵ������  ���ֵΪ[0,1]
	vp.TopLeftX = 0;//�ӿ����Ͻǵĺ�����
	vp.TopLeftY = 0;//�ӿ����Ͻǵ�������
	//�����ӿ�
	(*immediateContext)->RSSetViewports(1, &vp);//����Ϊ �ӿڵĸ����Լ��������ӿڶ���
	return true;
	/****************�ڶ����֣���ʼ��D3D ����*********************/

}

//��Ϣѭ����������֮ǰ��Hello World�� ������Run()��ͬ���Ĺ���
//bool(*ptr_display)(float timeDelta)��ʾ����һ������ָ����Ϊ����
//���������һ��float���͵Ĳ�������һ��bool���͵ķ���ֵ
int d3d::EnterMsgLoop(bool(*ptr_display)(float timeDelta)){
	MSG msg;
	::ZeroMemory(&msg, sizeof(MSG));//��ʼ���ڴ�
	static float lastTime = (float)timeGetTime();//��һ�λ�ȡ��ǰʱ��
	while (msg.message != WM_QUIT)
	{
		if (::PeekMessage(&msg, 0, 0, 0, PM_REMOVE)){
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
		}
		else{
			//�ڶ��λ�ȡ��ǰʱ��
			float currTime = (float)timeGetTime();
			//��ȡ����ʱ��֮���ʱ���
			float timeDelta = (currTime - lastTime)*0.001f;
			//������ʾ�������ں���ʵ��ͼ�εı仯(����ת)ʱ���õ�
			ptr_display(timeDelta);
			lastTime = currTime;
		}
	}
	return msg.wParam;
}