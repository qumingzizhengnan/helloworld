#include "d3dUtility.h"

//D3D初始化
//该函数包含两个部分：1.创建一个窗口；2.初始化D3D
//函数参数包括：
//1.HINSTANCE 当前应用程序实例的句柄
//2.int width, int height 窗口的宽高
//3.ID3D10RenderTargetView ** renderTargetView 目标渲染视图指针
//ID3D11DeviceContext ** immediateContext 立即执行上下文指针
//IDXGISwapChain ** swapChain 交换链指针
//ID3D11Device ** device 设备用指针，每个D3D程序至少有一个设备

bool d3d::InitD3D(
	HINSTANCE hInstance,
	int width, int height,
	ID3D11RenderTargetView ** renderTargetView,
	ID3D11DeviceContext ** immediateContext,
	IDXGISwapChain ** swapChain,
	ID3D11Device ** device){
	/****************第一部分：创建一个窗口 开始*********************/

	//参考第二章
	//创建窗口的步骤：1.设计一个窗口类；2.注册窗口类；3.创建窗口；4.窗口显示和更新
	//1.设计一个窗口类
	WNDCLASS wc;
	wc.style = CS_HREDRAW | CS_VREDRAW;//这就是一个窗口类WNDCLASS的对象定义窗口的样式
	//这两个标记表明当前窗口的水平或垂直尺寸发生变化时
	//窗口将被重绘
	wc.lpfnWndProc = (WNDPROC)d3d::WndProc;//这里指定回调函数的指针
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;//当前应用程序实例的句柄，由Win Main传入
	wc.hIcon = ::LoadIcon(0, IDI_APPLICATION);//指定图标
	wc.hCursor = ::LoadCursor(0, IDC_ARROW);//指定光标
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.lpszMenuName = 0;//指定有无菜单，0为无菜单
	wc.lpszClassName = L"Direct3D11App";//指向窗口名的指针
	//2.注册窗口类
	if (!RegisterClass(&wc)){
		::MessageBox(0, L"RegisterClass-Failed", 0, 0);
		return false;
	}
	//3.创建窗口
	HWND hwnd = ::CreateWindow(
		L"Direct3D11App",//第一个Hello必须和wc.lpszClassName相同
		L"D3D11",//第二个Hello时窗口的显示名称
		WS_OVERLAPPEDWINDOW,//指定这个窗口时重叠式窗口
		CW_USEDEFAULT,//表示窗口的横坐标为默认值
		CW_USEDEFAULT,//表示窗口的纵坐标为默认值
		width,
		height,
		0,
		0,
		hInstance,
		0);
	//如果窗口创建失败MainWindowHandle将会为0，则报错并返回
	if (!hwnd){
		::MessageBox(0, L"CreateWindow-Failed", 0, 0);
		return false;
	}
	//4.窗口显示和更新
	::ShowWindow(hwnd, SW_SHOW);
	::UpdateWindow(hwnd);
	/****************第一部分：创建一个窗口 结束*********************/

	/****************第二部分：初始化D3D 开始*********************/
	//初始化D3D设备步骤：
	//1.描述交换链，即填充DXGI_SWAP_CHAIN_DESC结构
	//2.使用D3D11CreateDeviceAndSwapChain创建D3D设备（ID3D11Device）
	//  执行上下文接口（D3D11Devicecontext），交换链接口（IDXGISwapChain）
	//3.创建目标渲染视图（ID3D11ReaderTargetView）
	//4.设置视口（View Port）

	//第一步，描述交换链，即填充DXGI_SWAP_CHAIN_DESC结构
	DXGI_SWAP_CHAIN_DESC sd;//首先声明一个DXGI_SWAP_CHAIN_DESC对象sd
	ZeroMemory(&sd, sizeof(sd));//用ZeroMemory对sd进行初始化
	sd.BufferCount = 1;//交换链中后台缓存数量，通常为1
	sd.BufferDesc.Width = width;//缓存区中的窗口宽
	sd.BufferDesc.Height = height;//缓存区中的窗口高
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;//表示红绿蓝Alpha各8位
	sd.BufferDesc.RefreshRate.Numerator = 60;//刷新频率的分子为60
	sd.BufferDesc.RefreshRate.Denominator = 1;//刷新频率的分母为1    刷新频率为每秒6次
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;//用来描述后台缓存用法  控制CPU对后台缓存的访问
	sd.OutputWindow = hwnd;//指向渲染目标窗口的句柄
	sd.SampleDesc.Count = 1;//多重采样，本例中不采用
	sd.SampleDesc.Quality = 0;//所以count=1，quality=0
	sd.Windowed = TRUE;//TRUE为窗口模式，FALSE为全屏模式

	//第二步，创建D3D设备、交换链接口，执行上下文接口
	//创建一个数组确定尝试创建Featurelevel的顺序
	D3D_FEATURE_LEVEL featureLevels[] = {
		D3D_FEATURE_LEVEL_11_0,//D3D11所支持的特征。包括shader model5
		D3D_FEATURE_LEVEL_10_1,//D3D11所支持的特征。包括shader model4
		D3D_FEATURE_LEVEL_10_0
	};

	//获取D3D_FEATURE_LEVEL数组的元素个数
	UINT numFeatureLevels = ARRAYSIZE(featureLevels);

	//调用D3D11CreateDeviceAndSwapChain创建交换链、设备和执行上下文
	//分别存入swapChain，device，immediateContext
	if (FAILED(D3D11CreateDeviceAndSwapChain(
		NULL,                    //确定显示适配器，NULL表示默认显示适配器
		D3D_DRIVER_TYPE_HARDWARE,//驱动类型，这里表示使用三维硬件加速
		NULL,//只有上一个参数设置为D3D_DRIVER_TYPE_SOFTWARE时才使用该参数
		0,   //也可设置为D3D11_CREATE_DEVICE_DEBUG开启调试模式
		featureLevels,//前面定义的D3D_FEATURE_LEVEL数组
		numFeatureLevels,//D3D_FEATURE_LEVEL数组的元素个数
		D3D11_SDK_VERSION,//SDK的版本，这里为D3D11
		&sd,              //前面定义的DXGI_SWAP_CHAIN_DESC对象sd
		swapChain,//返回创建好的交换链指针，InitiD3D函数传递的实参
		device,//返回创建好的设备用指针，InitiD3D函数传递的实参
		NULL,//返回当前设备支持的featureLevels数组中的第一个对象，一般设置为NULL
		immediateContext)))//返回创建号的执行上下文指针，InitiD3D函数传递的实参
	{
		::MessageBox(0, L"CreatDevice-Failed", 0, 0);
		return false;
	}

	//第三步，创建并设置目标渲染视图
	HRESULT hr = 0;//COM要求所有的方法都会返回一个HRESULT类型的错误号
	ID3D11Texture2D * pBackBuffer = NULL;//ID3D11Texture2D类型的后台缓存指针
	//调用GetBuffer()函数得到后台缓存对象，并存入&pBackBuffe中
	hr = (*swapChain)->GetBuffer(0,//缓存索引，一般设置为0
		__uuidof(ID3D11Texture2D),//缓存类型
		(LPVOID*)& pBackBuffer);//缓存指针
	//判断GetBuffer()是否调用成功
	if (FAILED(hr)){
		::MessageBox(0, L"GetBuffer-Failed", 0, 0);
		return false;
	}

	//调用CreateRenderTargetView创建好渲染目标视图
	//创建好后存入renderTargetView中
	hr = (*device)->CreateRenderTargetView(
		pBackBuffer,//上面创建好的后台缓存
		NULL,//设置为NULL得到默认的渲染目标视图
		renderTargetView);//返回创建好的渲染目标视图
	pBackBuffer->Release();//释放后台缓存
	//判断CreateRenderTargetView是否调用成功
	if (FAILED(hr)){
		::MessageBox(0, L"CreateRender-Failed", 0, 0);
		return false;
	}

	//将渲染目标视图绑定到渲染管线
	(*immediateContext)->OMSetRenderTargets(1,//绑定的目标视图的个数
		renderTargetView,                     //渲染目标视图
		NULL);                                //不绑定深度模板

	//第四部，设置视口大小，D3D11默认不会设置视口，必须手动设置
	D3D11_VIEWPORT vp;//创建一个视口的对象
	vp.Width = width;
	vp.Height = height;
	vp.MinDepth = 0.0f;//深度值的下限
	vp.MaxDepth = 1.0f;//深度值的上限  深度值为[0,1]
	vp.TopLeftX = 0;//视口左上角的横坐标
	vp.TopLeftY = 0;//视口左上角的纵坐标
	//设置视口
	(*immediateContext)->RSSetViewports(1, &vp);//参数为 视口的个数以及创建的视口对象
	return true;
	/****************第二部分：初始化D3D 结束*********************/

}

//消息循环函数，和之前“Hello World” 程序中Run()起到同样的功能
//bool(*ptr_display)(float timeDelta)表示传递一个函数指针作为参数
//这个函数有一个float类型的参数，有一个bool类型的返回值
int d3d::EnterMsgLoop(bool(*ptr_display)(float timeDelta)){
	MSG msg;
	::ZeroMemory(&msg, sizeof(MSG));//初始化内存
	static float lastTime = (float)timeGetTime();//第一次获取当前时间
	while (msg.message != WM_QUIT)
	{
		if (::PeekMessage(&msg, 0, 0, 0, PM_REMOVE)){
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
		}
		else{
			//第二次获取当前时间
			float currTime = (float)timeGetTime();
			//获取两次时间之间的时间差
			float timeDelta = (currTime - lastTime)*0.001f;
			//调用显示函数，在后面实现图形的变化(如旋转)时会用到
			ptr_display(timeDelta);
			lastTime = currTime;
		}
	}
	return msg.wParam;
}