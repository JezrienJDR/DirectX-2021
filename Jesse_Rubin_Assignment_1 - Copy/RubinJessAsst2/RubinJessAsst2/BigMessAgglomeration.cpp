//***************************************************************************************
// TreeBillboardsApp.cpp 
//***************************************************************************************

#include "d3dApp.h"
#include "MathHelper.h"
#include "UploadBuffer.h"
#include "GeometryGenerator.h"
#include "FrameResource.h"
#include "Waves.h"

using Microsoft::WRL::ComPtr;
using namespace DirectX;
using namespace DirectX::PackedVector;

#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "D3D12.lib")

const int gNumFrameResources = 3;

XMVECTOR camPos = XMVectorSet(0.0f, 50.0f, 0.0f, 1.0f);
XMVECTOR target = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
XMVECTOR targetDirection = XMVectorSet(0.0f, -1.0f, 0.0f, 1.0f);
XMVECTOR baseForward = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
XMVECTOR baseBack = XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f);
XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
XMVECTOR north = XMVectorSet(0.0f, 0.0f, 1.0f, 1.0f);

float screenWidth = 80;
float screenHeight = 60;
//float orientation = 0.0f;

//XMVECTOR leftQuat = XMVectorSet(0.0f, 1.0f, 0.0f, 0.01f);

//XMVECTOR rightQuat = XMVectorSet(0.0f, 1.0f, 0.0f, -0.01f);

// Lightweight structure stores parameters to draw a shape.  This will
// vary from app-to-app.

class sphereCollider
{
public:
	sphereCollider(float _radius = 5.5f, float _xPos = 0.0f, float _yPos= 0.0f)
		: radius(_radius), xPos(_xPos), yPos(_yPos)
	{

	}
	
	float radius = 1.0f;
	float xPos = 0.0f;
	float yPos = 0.0f;

	bool willCollide(float x, float y)
	{
		float distance = sqrt((xPos - x) * (xPos - x) + (yPos - y) * (yPos - y));

		if (distance < radius)
		{
			return true;
		}
		return false;
	}

};

class RenderItem
{
public:
	RenderItem() = default;

	string name = "missingNo";

    // World matrix of the shape that describes the object's local space
    // relative to the world space, which defines the position, orientation,
    // and scale of the object in the world.
    XMFLOAT4X4 World = MathHelper::Identity4x4();

	XMFLOAT4X4 TexTransform = MathHelper::Identity4x4();

	// Dirty flag indicating the object data has changed and we need to update the constant buffer.
	// Because we have an object cbuffer for each FrameResource, we have to apply the
	// update to each FrameResource.  Thus, when we modify obect data we should set 
	// NumFramesDirty = gNumFrameResources so that each frame resource gets the update.
	int NumFramesDirty = gNumFrameResources;

	// Index into GPU constant buffer corresponding to the ObjectCB for this render item.
	UINT ObjCBIndex = -1;

	Material* Mat = nullptr;
	MeshGeometry* Geo = nullptr;

	RenderItem* parent = nullptr;

	vector<RenderItem*> children;

    // Primitive topology.
    D3D12_PRIMITIVE_TOPOLOGY PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    // DrawIndexedInstanced parameters.
    UINT IndexCount = 0;
    UINT StartIndexLocation = 0;
    int BaseVertexLocation = 0;

	vec3 position = vec3(0, 0, 0);
	vec3 localPos = vec3(0, 0, 0);
	vec3 scale = vec3(1, 1, 1);

	vec3 velocity = vec3(0, 0, 0);
	vec3 acceleration = vec3(0, 0, 0);

	float width = 1;
	float height = 1;
};



class Scene : public RenderItem
{

};

enum class RenderLayer : int
{
	Opaque = 0,
	Transparent,
	AlphaTested,
	AlphaTestedTreeSprites,
	Count
};

class Game : public D3DApp
{
public:
    Game(HINSTANCE hInstance);
    Game(const Game& rhs) = delete;
    Game& operator=(const Game& rhs) = delete;
    ~Game();

    virtual bool Initialize()override;

	int nextObjCBIndex = 0;

private:
    virtual void OnResize()override;
    virtual void Update(const GameTimer& gt)override;
    virtual void Draw(const GameTimer& gt)override;

    virtual void OnMouseDown(WPARAM btnState, int x, int y)override;
    virtual void OnMouseUp(WPARAM btnState, int x, int y)override;
    virtual void OnMouseMove(WPARAM btnState, int x, int y)override;

    void OnKeyboardInput(const GameTimer& gt);
	void UpdateCamera(const GameTimer& gt);
	void AnimateMaterials(const GameTimer& gt);
	void UpdateObjectCBs(const GameTimer& gt);
	void UpdateMaterialCBs(const GameTimer& gt);
	void UpdateMainPassCB(const GameTimer& gt);
	void UpdateWaves(const GameTimer& gt); 

	void LoadTextures();
    void BuildRootSignature();
	void BuildDescriptorHeaps();
    void BuildShadersAndInputLayouts();
	void BuildShapeGeometry();
    void BuildLandGeometry();
    void BuildWavesGeometry();
	void BuildBoxGeometry();
	void BuildTreeSpritesGeometry();
    void BuildPSOs();
    void BuildFrameResources();
    void BuildMaterials();
    void BuildRenderItems();
    void DrawRenderItems(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderItem*>& ritems);
	void AddRenderItem(string name, vec3 position, float width, float height, string material, bool AlphaTest);
	void AddPlayer(string name, vec3 position, float width, float height, string material, bool AlphaTest);
	void AddScene(string name, vec3 position, float width, float height, string material, bool AlphaTest);

	std::array<const CD3DX12_STATIC_SAMPLER_DESC, 6> GetStaticSamplers();

    float GetHillsHeight(float x, float z)const;
    XMFLOAT3 GetHillsNormal(float x, float z)const;

private:

    std::vector<std::unique_ptr<FrameResource>> mFrameResources;
    FrameResource* mCurrFrameResource = nullptr;
    int mCurrFrameResourceIndex = 0;

    UINT mCbvSrvDescriptorSize = 0;

    ComPtr<ID3D12RootSignature> mRootSignature = nullptr;

	ComPtr<ID3D12DescriptorHeap> mSrvDescriptorHeap = nullptr;

	std::unordered_map<std::string, std::unique_ptr<MeshGeometry>> mGeometries;
	std::unordered_map<std::string, std::unique_ptr<Material>> mMaterials;
	std::unordered_map<std::string, std::unique_ptr<Texture>> mTextures;
	std::unordered_map<std::string, ComPtr<ID3DBlob>> mShaders;
	std::unordered_map<std::string, ComPtr<ID3D12PipelineState>> mPSOs;

    std::vector<D3D12_INPUT_ELEMENT_DESC> mStdInputLayout;
	std::vector<D3D12_INPUT_ELEMENT_DESC> mTreeSpriteInputLayout;

    RenderItem* mWavesRitem = nullptr;

	// List of all the render items.
	std::vector<std::unique_ptr<RenderItem>> mAllRitems;

	// Render items divided by PSO.
	std::vector<RenderItem*> mRitemLayer[(int)RenderLayer::Count];

	std::unique_ptr<Waves> mWaves;

    PassConstants mMainPassCB;

	XMFLOAT3 mEyePos = { 0.0f, 0.0f, 0.0f };
	XMFLOAT4X4 mView = MathHelper::Identity4x4();
	XMFLOAT4X4 mProj = MathHelper::Identity4x4();

    float mTheta = 1.5f*XM_PI;
    float mPhi = XM_PIDIV2 - 0.1f;
    float mRadius = 50.0f;

    POINT mLastMousePos;

	float planeMove = -20.0f;
	
	Scene* scene = nullptr;
	Player* player = nullptr;
	RenderItem* sun = nullptr;
	RenderItem* planet = nullptr;
	RenderItem* planet2 = nullptr;

	//sphereCollider colliders[8];
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
    PSTR cmdLine, int showCmd)
{
    // Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

    try
    {
        Game theApp(hInstance);
        if(!theApp.Initialize())
            return 0;

        return theApp.Run();
    }
    catch(DxException& e)
    {
        MessageBox(nullptr, e.ToString().c_str(), L"HR Failed", MB_OK);
        return 0;
    }
}

Game::Game(HINSTANCE hInstance)
    : D3DApp(hInstance)
{
}

Game::~Game()
{
    if(md3dDevice != nullptr)
        FlushCommandQueue();
}

bool Game::Initialize()
{
    if(!D3DApp::Initialize())
        return false;

    // Reset the command list to prep for initialization commands.
    ThrowIfFailed(mCommandList->Reset(mDirectCmdListAlloc.Get(), nullptr));

    // Get the increment size of a descriptor in this heap type.  This is hardware specific, 
	// so we have to query this information.
    mCbvSrvDescriptorSize = md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    mWaves = std::make_unique<Waves>(128, 128, 1.0f, 0.03f, 4.0f, 0.2f);
 
	LoadTextures();
    BuildRootSignature();
	BuildDescriptorHeaps();
    BuildShadersAndInputLayouts();
	BuildShapeGeometry();
    BuildLandGeometry();
    BuildWavesGeometry();
	BuildBoxGeometry();
	BuildTreeSpritesGeometry();
	BuildMaterials();
    BuildRenderItems();
    BuildFrameResources();
    BuildPSOs();

    // Execute the initialization commands.
    ThrowIfFailed(mCommandList->Close());
    ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
    mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

    // Wait until initialization is complete.
    FlushCommandQueue();

	// Set up colliders HERE
	/*colliders[0] = sphereCollider(5.0f, 3.0f, -9.0f);
	colliders[1] = sphereCollider(5.0f, 0.0f, 4.0f);
	colliders[2] = sphereCollider(2.0f, -12.0f, 6.0f);
	colliders[3] = sphereCollider(2.0f, 12.0f, 6.0f);
	colliders[4] = sphereCollider(5.0f, 7.0f, 0.0f);
	colliders[5] = sphereCollider(5.0f, -7.0f, 0.0f);
	colliders[6] = sphereCollider(5.0f, -16.0f, 10.0f);
	colliders[7] = sphereCollider(5.0f, 16.0f, 10.0f);*/

	XMMATRIX O = XMMatrixOrthographicLH(screenWidth, screenHeight, 0.5f, 100.0f);
	XMStoreFloat4x4(&mProj, O);


    return true;
}
 
void Game::OnResize()
{
    D3DApp::OnResize();

	

    // The sun resized, so update the aspect ratio and recompute the projection matrix.
    XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f*MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
	
	XMMATRIX O = XMMatrixOrthographicLH(screenWidth, screenHeight, 0.5f, 100.0f);
    XMStoreFloat4x4(&mProj, O);
}

void Game::Update(const GameTimer& gt)
{
    OnKeyboardInput(gt);
	UpdateCamera(gt);

    // Cycle through the circular frame resource array.
    mCurrFrameResourceIndex = (mCurrFrameResourceIndex + 1) % gNumFrameResources;
    mCurrFrameResource = mFrameResources[mCurrFrameResourceIndex].get();

    // Has the GPU finished processing the commands of the current frame resource?
    // If not, wait until the GPU has completed commands up to this fence point.
    if(mCurrFrameResource->Fence != 0 && mFence->GetCompletedValue() < mCurrFrameResource->Fence)
    {
        HANDLE eventHandle = CreateEventEx(nullptr, nullptr, false, EVENT_ALL_ACCESS);
        ThrowIfFailed(mFence->SetEventOnCompletion(mCurrFrameResource->Fence, eventHandle));
        WaitForSingleObject(eventHandle, INFINITE);
        CloseHandle(eventHandle);
    }

	AnimateMaterials(gt);
	UpdateObjectCBs(gt);
	UpdateMaterialCBs(gt);
	UpdateMainPassCB(gt);
    //UpdateWaves(gt);
}

void Game::Draw(const GameTimer& gt)
{
    auto cmdListAlloc = mCurrFrameResource->CmdListAlloc;

    // Reuse the memory associated with command recording.
    // We can only reset when the associated command lists have finished execution on the GPU.
    ThrowIfFailed(cmdListAlloc->Reset());

    // A command list can be reset after it has been added to the command queue via ExecuteCommandList.
    // Reusing the command list reuses memory.
    ThrowIfFailed(mCommandList->Reset(cmdListAlloc.Get(), mPSOs["opaque"].Get()));

    mCommandList->RSSetViewports(1, &mScreenViewport);
    mCommandList->RSSetScissorRects(1, &mScissorRect);

    // Indicate a state transition on the resource usage.
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

    // Clear the back buffer and depth buffer.
    mCommandList->ClearRenderTargetView(CurrentBackBufferView(), (float*)&mMainPassCB.FogColor, 0, nullptr);
    mCommandList->ClearDepthStencilView(DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

    // Specify the buffers we are going to render to.
    mCommandList->OMSetRenderTargets(1, &CurrentBackBufferView(), true, &DepthStencilView());

	ID3D12DescriptorHeap* descriptorHeaps[] = { mSrvDescriptorHeap.Get() };
	mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

	mCommandList->SetGraphicsRootSignature(mRootSignature.Get());

	auto passCB = mCurrFrameResource->PassCB->Resource();
	mCommandList->SetGraphicsRootConstantBufferView(2, passCB->GetGPUVirtualAddress());

    DrawRenderItems(mCommandList.Get(), mRitemLayer[(int)RenderLayer::Opaque]);

	mCommandList->SetPipelineState(mPSOs["alphaTested"].Get());
	DrawRenderItems(mCommandList.Get(), mRitemLayer[(int)RenderLayer::AlphaTested]);

	mCommandList->SetPipelineState(mPSOs["treeSprites"].Get());
	DrawRenderItems(mCommandList.Get(), mRitemLayer[(int)RenderLayer::AlphaTestedTreeSprites]);

	mCommandList->SetPipelineState(mPSOs["transparent"].Get());
	DrawRenderItems(mCommandList.Get(), mRitemLayer[(int)RenderLayer::Transparent]);

    // Indicate a state transition on the resource usage.
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

    // Done recording commands.
    ThrowIfFailed(mCommandList->Close());

    // Add the command list to the queue for execution.
    ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
    mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

    // Swap the back and front buffers
    ThrowIfFailed(mSwapChain->Present(0, 0));
	mCurrBackBuffer = (mCurrBackBuffer + 1) % SwapChainBufferCount;

    // Advance the fence value to mark commands up to this fence point.
    mCurrFrameResource->Fence = ++mCurrentFence;

    // Add an instruction to the command queue to set a new fence point. 
    // Because we are on the GPU timeline, the new fence point won't be 
    // set until the GPU finishes processing all the commands prior to this Signal().
    mCommandQueue->Signal(mFence.Get(), mCurrentFence);
}

void Game::OnMouseDown(WPARAM btnState, int x, int y)
{
    mLastMousePos.x = x;
    mLastMousePos.y = y;

    SetCapture(mhMainWnd);
}

void Game::OnMouseUp(WPARAM btnState, int x, int y)
{
    ReleaseCapture();
}

void Game::OnMouseMove(WPARAM btnState, int x, int y)
{
    if((btnState & MK_LBUTTON) != 0)
    {
        // Make each pixel correspond to a quarter of a degree.
        float dx = XMConvertToRadians(0.25f*static_cast<float>(x - mLastMousePos.x));
        float dy = XMConvertToRadians(0.25f*static_cast<float>(y - mLastMousePos.y));

        // Update angles based on input to orbit camera around box.
        mTheta += dx;
        mPhi += dy;

        // Restrict the angle mPhi.
        mPhi = MathHelper::Clamp(mPhi, 0.1f, MathHelper::Pi - 0.1f);
    }
    else if((btnState & MK_RBUTTON) != 0)
    {
        // Make each pixel correspond to 0.2 unit in the scene.
        float dx = 0.2f*static_cast<float>(x - mLastMousePos.x);
        float dy = 0.2f*static_cast<float>(y - mLastMousePos.y);

        // Update the camera radius based on input.
        mRadius += dx - dy;

        // Restrict the radius.
        mRadius = MathHelper::Clamp(mRadius, 5.0f, 150.0f);
    }

    mLastMousePos.x = x;
    mLastMousePos.y = y;
}
 
void Game::OnKeyboardInput(const GameTimer& gt)
{

	
	float maxRot = 2.0f;
	float rotStep = 0.01f;

	short keyW = GetAsyncKeyState('W');
	if (keyW & 0x8000)
	{
		bool stop = false;

		//for (sphereCollider s : colliders)
		//{
		//	if (s.willCollide(XMVectorGetX(camPos), XMVectorGetZ(camPos)))
		//	{
		//		stop = true;
		//	}
		//}

		if (stop == false)
		{
			camPos += 0.1 * targetDirection;
		}
	}
	short keyS = GetAsyncKeyState('S');
	if (keyS & 0x8000)
	{
		camPos -= 0.1f * targetDirection;
	}
	short keyA = GetAsyncKeyState('A');
	if (keyA & 0x8000)
	{
		//targetDirection = XMVector3Rotate(targetDirection, leftQuat);
		orientation += rotStep;
		if (orientation > maxRot)
		{
			//orientation = -maxRot;
		}
	}
	short keyD = GetAsyncKeyState('D');
	if (keyD & 0x8000)
	{
		//targetDirection = XMVector3Rotate(targetDirection, rightQuat);
		orientation -= rotStep;
		if (orientation < -maxRot)
		{
			//orientation = maxRot;
		}
	}

	string ori = to_string(orientation);

	//OutputDebugString(&ori);

	XMVECTOR rotQuat = XMVectorSet(0.0f, 1.0f, 0.0f, orientation / 3.14159f);

	targetDirection = XMVectorSet(-sin(orientation * 3.14159f / 180.0f), 0.0f, cos(orientation * 3.14159f / 180.0f), 0.0f);
	
	//XMVECTOR upVector = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	//targetDirection = XMQuaternionRotationAxis(upVector, orientation);


	

//	targetDirection = XMVector3Rotate(baseForward, rotQuat);
	targetDirection = XMVector3Normalize(targetDirection);

	//target = camPos + targetDirection;


	//XMMATRIX view = XMMatrixLookAtLH(pos, target, up);
	//XMStoreFloat4x4(&mView, view);

}
 
void Game::UpdateCamera(const GameTimer& gt)
{
	// Convert Spherical to Cartesian coordinates.
//	mEyePos.x = mRadius*sinf(mPhi)*cosf(mTheta);
//	mEyePos.z = mRadius*sinf(mPhi)*sinf(mTheta);
//	mEyePos.y = mRadius*cosf(mPhi);

	// Build the view matrix.
//	XMVECTOR pos = XMVectorSet(mEyePos.x, mEyePos.y, mEyePos.z, 1.0f);
//	XMVECTOR target = XMVectorZero();
//	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	mEyePos = { XMVectorGetX(camPos),XMVectorGetY(camPos),XMVectorGetZ(camPos) };

	XMMATRIX view = XMMatrixLookAtLH(camPos, target, north);
	XMStoreFloat4x4(&mView, view);
}

void Game::AnimateMaterials(const GameTimer& gt)
{
	// Scroll the water material texture coordinates.
	auto waterMat = mMaterials["water"].get();

	float& tu = waterMat->MatTransform(3, 0);
	float& tv = waterMat->MatTransform(3, 1);

	tu += 0.1f * gt.DeltaTime();
	tv += 0.02f * gt.DeltaTime();

	if(tu >= 1.0f)
		tu -= 1.0f;

	if(tv >= 1.0f)
		tv -= 1.0f;

	waterMat->MatTransform(3, 0) = tu;
	waterMat->MatTransform(3, 1) = tv;

	// Material has changed, so need to update cbuffer.
	waterMat->NumFramesDirty = gNumFrameResources;
}

void Game::UpdateObjectCBs(const GameTimer& gt)
{
	auto currObjectCB = mCurrFrameResource->ObjectCB.get();
	for(auto& e : mAllRitems)
	{
		// Only update the cbuffer data if the constants have changed.  
		// This needs to be tracked per frame resource.
		if(e->NumFramesDirty > 0)
		{
			XMMATRIX world = XMLoadFloat4x4(&e->World);
			XMMATRIX texTransform = XMLoadFloat4x4(&e->TexTransform);

			ObjectConstants objConstants;
			XMStoreFloat4x4(&objConstants.World, XMMatrixTranspose(world));
			XMStoreFloat4x4(&objConstants.TexTransform, XMMatrixTranspose(texTransform));

			currObjectCB->CopyData(e->ObjCBIndex, objConstants);

			// Next FrameResource need to be updated too.
			e->NumFramesDirty--;
		}
	}
}

void Game::UpdateMaterialCBs(const GameTimer& gt)
{
	auto currMaterialCB = mCurrFrameResource->MaterialCB.get();
	for(auto& e : mMaterials)
	{
		// Only update the cbuffer data if the constants have changed.  If the cbuffer
		// data changes, it needs to be updated for each FrameResource.
		Material* mat = e.second.get();
		if(mat->NumFramesDirty > 0)
		{
			XMMATRIX matTransform = XMLoadFloat4x4(&mat->MatTransform);

			MaterialConstants matConstants;
			matConstants.DiffuseAlbedo = mat->DiffuseAlbedo;
			matConstants.FresnelR0 = mat->FresnelR0;
			matConstants.Roughness = mat->Roughness;
			XMStoreFloat4x4(&matConstants.MatTransform, XMMatrixTranspose(matTransform));

			currMaterialCB->CopyData(mat->MatCBIndex, matConstants);

			// Next FrameResource need to be updated too.
			mat->NumFramesDirty--;
		}
	}
}

void Game::UpdateMainPassCB(const GameTimer& gt)
{

	

	XMMATRIX view = XMLoadFloat4x4(&mView);
	XMMATRIX proj = XMLoadFloat4x4(&mProj);

	XMMATRIX viewProj = XMMatrixMultiply(view, proj);
	XMMATRIX invView = XMMatrixInverse(&XMMatrixDeterminant(view), view);
	XMMATRIX invProj = XMMatrixInverse(&XMMatrixDeterminant(proj), proj);
	XMMATRIX invViewProj = XMMatrixInverse(&XMMatrixDeterminant(viewProj), viewProj);

	XMStoreFloat4x4(&mMainPassCB.View, XMMatrixTranspose(view));
	XMStoreFloat4x4(&mMainPassCB.InvView, XMMatrixTranspose(invView));
	XMStoreFloat4x4(&mMainPassCB.Proj, XMMatrixTranspose(proj));
	XMStoreFloat4x4(&mMainPassCB.InvProj, XMMatrixTranspose(invProj));
	XMStoreFloat4x4(&mMainPassCB.ViewProj, XMMatrixTranspose(viewProj));
	XMStoreFloat4x4(&mMainPassCB.InvViewProj, XMMatrixTranspose(invViewProj));
	mMainPassCB.EyePosW = mEyePos;
	//mMainPassCB.EyePosW = pos;


	mMainPassCB.RenderTargetSize = XMFLOAT2((float)mClientWidth, (float)mClientHeight);
	mMainPassCB.InvRenderTargetSize = XMFLOAT2(1.0f / mClientWidth, 1.0f / mClientHeight);
	mMainPassCB.NearZ = 1.0f;
	mMainPassCB.FarZ = 1000.0f;
	mMainPassCB.TotalTime = gt.TotalTime();
	mMainPassCB.DeltaTime = gt.DeltaTime();
	mMainPassCB.AmbientLight = { 1.0f, 1.0f,1.0f, 1.0f };
	mMainPassCB.Lights[0].Direction = { 0.0f, 1.0f, 0.0f };
	mMainPassCB.Lights[0].Strength = { 0.6f, 0.6f, 0.6f };
//	mMainPassCB.Lights[1].Direction = { -0.57735f, -0.57735f, 0.57735f };
//	mMainPassCB.Lights[1].Strength = { 0.3f, 0.3f, 0.3f };
//	mMainPassCB.Lights[2].Direction = { 0.0f, -0.707f, -0.707f };
//	mMainPassCB.Lights[2].Strength = { 0.15f, 0.15f, 0.15f };


	mMainPassCB.Lights[1].Position = { 12.0f, 19.0f, 6.0f };
	mMainPassCB.Lights[1].Strength = { 0.95f, 0.0f, 0.0f };

	mMainPassCB.Lights[2].Position = { 0.0f, 9.0f, 4.0f };
	mMainPassCB.Lights[2].Strength = { 0.0f, 0.0f, 1.95f };


	mMainPassCB.Lights[3].Position = { -12.0f, 19.0f, 6.0f };
	mMainPassCB.Lights[3].Strength = { 0.95f, 0.0f, 0.0f };


	auto currPassCB = mCurrFrameResource->PassCB.get();
	currPassCB->CopyData(0, mMainPassCB);
}

void Game::UpdateWaves(const GameTimer& gt)
{
	// Every quarter second, generate a random wave.
	static float t_base = 0.0f;
	if((mTimer.TotalTime() - t_base) >= 0.25f)
	{
		t_base += 0.25f;

		int i = MathHelper::Rand(4, mWaves->RowCount() - 5);
		int j = MathHelper::Rand(4, mWaves->ColumnCount() - 5);

		float r = MathHelper::RandF(0.2f, 0.5f);

		mWaves->Disturb(i, j, r);
	}

	// Update the wave simulation.
	mWaves->Update(gt.DeltaTime());

	// Update the wave vertex buffer with the new solution.
	auto currWavesVB = mCurrFrameResource->WavesVB.get();
	for(int i = 0; i < mWaves->VertexCount(); ++i)
	{
		Vertex v;

		v.Pos = mWaves->Position(i);
		v.Normal = mWaves->Normal(i);
		
		// Derive tex-coords from position by 
		// mapping [-w/2,w/2] --> [0,1]
		v.TexC.x = 0.5f + v.Pos.x / mWaves->Width();
		v.TexC.y = 0.5f - v.Pos.z / mWaves->Depth();

		currWavesVB->CopyData(i, v);
	}

	// Set the dynamic VB of the wave renderitem to the current frame VB.
	//mWavesRitem->Geo->VertexBufferGPU = currWavesVB->Resource();
}
void Game::BuildRootSignature()
{
	CD3DX12_DESCRIPTOR_RANGE texTable;
	texTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

    // Root parameter can be a table, root descriptor or root constants.
    CD3DX12_ROOT_PARAMETER slotRootParameter[4];

	// Perfomance TIP: Order from most frequent to least frequent.
	slotRootParameter[0].InitAsDescriptorTable(1, &texTable, D3D12_SHADER_VISIBILITY_PIXEL);
    slotRootParameter[1].InitAsConstantBufferView(0);
    slotRootParameter[2].InitAsConstantBufferView(1);
    slotRootParameter[3].InitAsConstantBufferView(2);

	auto staticSamplers = GetStaticSamplers();

    // A root signature is an array of root parameters.
	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(4, slotRootParameter,
		(UINT)staticSamplers.size(), staticSamplers.data(),
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    // create a root signature with a single slot which points to a descriptor range consisting of a single constant buffer
    ComPtr<ID3DBlob> serializedRootSig = nullptr;
    ComPtr<ID3DBlob> errorBlob = nullptr;
    HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
        serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

    if(errorBlob != nullptr)
    {
        ::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
    }
    ThrowIfFailed(hr);

    ThrowIfFailed(md3dDevice->CreateRootSignature(
		0,
        serializedRootSig->GetBufferPointer(),
        serializedRootSig->GetBufferSize(),
        IID_PPV_ARGS(mRootSignature.GetAddressOf())));
}

void Game::BuildDescriptorHeaps()
{
	//
	// Create the SRV heap.
	//
	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
	srvHeapDesc.NumDescriptors = 8;
	srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	ThrowIfFailed(md3dDevice->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&mSrvDescriptorHeap)));

	//
	// Fill out the heap with actual descriptors.
	//
	CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor(mSrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

	auto planetTex = mTextures["planetTex"]->Resource;
	auto waterTex = mTextures["waterTex"]->Resource;
	auto fenceTex = mTextures["fenceTex"]->Resource;
	auto frostTex = mTextures["frostTex"]->Resource;
	auto sunTex = mTextures["sunTex"]->Resource;
	auto DefiantTex = mTextures["DefiantTex"]->Resource;
	auto spaceTex = mTextures["spaceTex"]->Resource;
	auto treeArrayTex = mTextures["treeArrayTex"]->Resource;


	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = planetTex->GetDesc().Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = -1;
	md3dDevice->CreateShaderResourceView(planetTex.Get(), &srvDesc, hDescriptor);

	// next descriptor
	hDescriptor.Offset(1, mCbvSrvDescriptorSize);

	srvDesc.Format = waterTex->GetDesc().Format;
	md3dDevice->CreateShaderResourceView(waterTex.Get(), &srvDesc, hDescriptor);

	// next descriptor
	hDescriptor.Offset(1, mCbvSrvDescriptorSize);

	srvDesc.Format = fenceTex->GetDesc().Format;
	md3dDevice->CreateShaderResourceView(fenceTex.Get(), &srvDesc, hDescriptor);

	// next descriptor
	hDescriptor.Offset(1, mCbvSrvDescriptorSize);

	srvDesc.Format = frostTex->GetDesc().Format;
	md3dDevice->CreateShaderResourceView(frostTex.Get(), &srvDesc, hDescriptor);

	// next descriptor
	hDescriptor.Offset(1, mCbvSrvDescriptorSize);

	srvDesc.Format = sunTex->GetDesc().Format;
	md3dDevice->CreateShaderResourceView(sunTex.Get(), &srvDesc, hDescriptor);

	// next descriptor
	hDescriptor.Offset(1, mCbvSrvDescriptorSize);

	srvDesc.Format = DefiantTex->GetDesc().Format;
	md3dDevice->CreateShaderResourceView(DefiantTex.Get(), &srvDesc, hDescriptor);


	// next descriptor
	hDescriptor.Offset(1, mCbvSrvDescriptorSize);

	srvDesc.Format = spaceTex->GetDesc().Format;
	md3dDevice->CreateShaderResourceView(spaceTex.Get(), &srvDesc, hDescriptor);

	// next descriptor
	hDescriptor.Offset(1, mCbvSrvDescriptorSize);

	auto desc = treeArrayTex->GetDesc();
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
	srvDesc.Format = treeArrayTex->GetDesc().Format;
	srvDesc.Texture2DArray.MostDetailedMip = 0;
	srvDesc.Texture2DArray.MipLevels = -1;
	srvDesc.Texture2DArray.FirstArraySlice = 0;
	srvDesc.Texture2DArray.ArraySize = treeArrayTex->GetDesc().DepthOrArraySize;
	md3dDevice->CreateShaderResourceView(treeArrayTex.Get(), &srvDesc, hDescriptor);



}

void Game::BuildShadersAndInputLayouts()
{
	const D3D_SHADER_MACRO defines[] =
	{
		"FOG", "1",
		NULL, NULL
	};

	const D3D_SHADER_MACRO alphaTestDefines[] =
	{
		"FOG", "1",
		"ALPHA_TEST", "1",
		NULL, NULL
	};

	mShaders["standardVS"] = d3dUtil::CompileShader(L"Shaders\\Default.hlsl", nullptr, "VS", "vs_5_1");
	mShaders["opaquePS"] = d3dUtil::CompileShader(L"Shaders\\Default.hlsl", defines, "PS", "ps_5_1");
	mShaders["alphaTestedPS"] = d3dUtil::CompileShader(L"Shaders\\Default.hlsl", alphaTestDefines, "PS", "ps_5_1");
	
	mShaders["treeSpriteVS"] = d3dUtil::CompileShader(L"Shaders\\TreeSprite.hlsl", nullptr, "VS", "vs_5_1");
	mShaders["treeSpriteGS"] = d3dUtil::CompileShader(L"Shaders\\TreeSprite.hlsl", nullptr, "GS", "gs_5_1");
	mShaders["treeSpritePS"] = d3dUtil::CompileShader(L"Shaders\\TreeSprite.hlsl", alphaTestDefines, "PS", "ps_5_1");

    mStdInputLayout =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    };

	mTreeSpriteInputLayout =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "SIZE", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	};
}

void Game::BuildShapeGeometry()
{
	GeometryGenerator geoGen;
	GeometryGenerator::MeshData box = geoGen.CreateBox(1.0f, 1.0f, 1.0f, 0);
	GeometryGenerator::MeshData grid = geoGen.CreateGrid(80.0f, 80.0f, 60, 40);
	GeometryGenerator::MeshData sphere = geoGen.CreateSphere(0.5f, 20, 20);
	GeometryGenerator::MeshData cylinder = geoGen.CreateCylinder(0.5f, 0.3f, 3.0f, 20, 20);
	GeometryGenerator::MeshData torus = geoGen.CreateTorus(2.5f, 5.0f, 32, 64);
	GeometryGenerator::MeshData frosting = geoGen.CreateTorus(1.8f, 5.0f, 16, 32);
	//GeometryGenerator::MeshData frosting = geoGen.CreateCorkScrew(1.0f, 2.0f, 30.0f, 32, 16);
	//GeometryGenerator::MeshData frosting = geoGen.CreateGear(2.0f, 0.5f);

	//GeometryGenerator::MeshData frosting = geoGen.CreateDiamond(2.0f, 4.5f, 8);
	//GeometryGenerator::MeshData frosting = geoGen.CreateD20(9.0f);
	GeometryGenerator::MeshData cone = geoGen.CreateCone(2.0f, 1.5f, 128);
	GeometryGenerator::MeshData crystal = geoGen.CreateDiamond(1.0f, 6.0f, 8);
	GeometryGenerator::MeshData wobble = geoGen.MakeCyl(2.0f, 20.0f, 24, 128);
	GeometryGenerator::MeshData screw = geoGen.CreateCorkScrew(0.7f, 2.0f, 32.0f, 32, 16);
	GeometryGenerator::MeshData gear = geoGen.CreateGear(2.0f, 0.5f);
	GeometryGenerator::MeshData d20 = geoGen.CreateD20(1.0f);
	//GeometryGenerator::MeshData screw = geoGen.CreateCorkScrew(1.0f, 2.0f, 30.0f, 16, 16);
	//GeometryGenerator::MeshData torus = geoGen.Createtorus();
	//
	GeometryGenerator::MeshData plane = geoGen.CreatePlane(10.0f, 10.0f);



	// We are concatenating all the geometry into one big vertex/index buffer.  So
	// define the regions in the buffer each submesh covers.
	//
	// Cache the vertex offsets to each object in the concatenated vertex buffer.
	UINT boxVertexOffset = 0;
	UINT gridVertexOffset = (UINT)box.Vertices.size();
	UINT sphereVertexOffset = gridVertexOffset + (UINT)grid.Vertices.size();
	UINT cylinderVertexOffset = sphereVertexOffset + (UINT)sphere.Vertices.size();
	UINT torusVertexOffset = cylinderVertexOffset + (UINT)cylinder.Vertices.size();
	UINT frostingVertexOffset = torusVertexOffset + (UINT)torus.Vertices.size();
	UINT coneVertexOffset = frostingVertexOffset + (UINT)frosting.Vertices.size();
	UINT crystalVertexOffset = coneVertexOffset + (UINT)cone.Vertices.size();
	UINT wobbleVertexOffset = crystalVertexOffset + (UINT)crystal.Vertices.size();
	UINT screwVertexOffset = wobbleVertexOffset + (UINT)wobble.Vertices.size();
	UINT gearVertexOffset = screwVertexOffset + (UINT)screw.Vertices.size();
	UINT d20VertexOffset = gearVertexOffset + (UINT)gear.Vertices.size();
	UINT planeVertexOffset = d20VertexOffset + (UINT)d20.Vertices.size();
	// Cache the starting index for each object in the concatenated index buffer.
	UINT boxIndexOffset = 0;
	UINT gridIndexOffset = (UINT)box.Indices32.size();
	UINT sphereIndexOffset = gridIndexOffset + (UINT)grid.Indices32.size();
	UINT cylinderIndexOffset = sphereIndexOffset + (UINT)sphere.Indices32.size();
	UINT torusIndexOffset = cylinderIndexOffset + (UINT)cylinder.Indices32.size();
	UINT frostingIndexOffset = torusIndexOffset + (UINT)torus.Indices32.size();
	UINT coneIndexOffset = frostingIndexOffset + (UINT)frosting.Indices32.size();
	UINT crystalIndexOffset = coneIndexOffset + (UINT)cone.Indices32.size();
	UINT wobbleIndexOffset = crystalIndexOffset + (UINT)crystal.Indices32.size();
	UINT screwIndexOffset = wobbleIndexOffset + (UINT)wobble.Indices32.size();
	UINT gearIndexOffset = screwIndexOffset + (UINT)screw.Indices32.size();
	UINT d20IndexOffset = gearIndexOffset + (UINT)gear.Indices32.size();
	UINT planeIndexOffset = d20IndexOffset + (UINT)d20.Indices32.size();
	// Define the SubmeshGeometry that cover different
	// regions of the vertex/index buffers.
	SubmeshGeometry boxSubmesh;
	boxSubmesh.IndexCount = (UINT)box.Indices32.size();
	boxSubmesh.StartIndexLocation = boxIndexOffset;
	boxSubmesh.BaseVertexLocation = boxVertexOffset;
	SubmeshGeometry gridSubmesh;
	gridSubmesh.IndexCount = (UINT)grid.Indices32.size();
	gridSubmesh.StartIndexLocation = gridIndexOffset;
	gridSubmesh.BaseVertexLocation = gridVertexOffset;
	SubmeshGeometry sphereSubmesh;
	sphereSubmesh.IndexCount = (UINT)sphere.Indices32.size();
	sphereSubmesh.StartIndexLocation = sphereIndexOffset;
	sphereSubmesh.BaseVertexLocation = sphereVertexOffset;
	SubmeshGeometry cylinderSubmesh;
	cylinderSubmesh.IndexCount = (UINT)cylinder.Indices32.size();
	cylinderSubmesh.StartIndexLocation = cylinderIndexOffset;
	cylinderSubmesh.BaseVertexLocation = cylinderVertexOffset;
	SubmeshGeometry torusSubmesh;
	torusSubmesh.IndexCount = (UINT)torus.Indices32.size();
	torusSubmesh.StartIndexLocation = torusIndexOffset;
	torusSubmesh.BaseVertexLocation = torusVertexOffset;
	SubmeshGeometry frostingSubmesh;
	frostingSubmesh.IndexCount = (UINT)frosting.Indices32.size();
	frostingSubmesh.StartIndexLocation = frostingIndexOffset;
	frostingSubmesh.BaseVertexLocation = frostingVertexOffset;
	SubmeshGeometry coneSubmesh;
	coneSubmesh.IndexCount = (UINT)cone.Indices32.size();
	coneSubmesh.StartIndexLocation = coneIndexOffset;
	coneSubmesh.BaseVertexLocation = coneVertexOffset;
	SubmeshGeometry crystalSubmesh;
	crystalSubmesh.IndexCount = (UINT)crystal.Indices32.size();
	crystalSubmesh.StartIndexLocation = crystalIndexOffset;
	crystalSubmesh.BaseVertexLocation = crystalVertexOffset;
	SubmeshGeometry wobbleSubmesh;
	wobbleSubmesh.IndexCount = (UINT)wobble.Indices32.size();
	wobbleSubmesh.StartIndexLocation = wobbleIndexOffset;
	wobbleSubmesh.BaseVertexLocation = wobbleVertexOffset;
	SubmeshGeometry screwSubmesh;
	screwSubmesh.IndexCount = (UINT)screw.Indices32.size();
	screwSubmesh.StartIndexLocation = screwIndexOffset;
	screwSubmesh.BaseVertexLocation = screwVertexOffset;
	SubmeshGeometry gearSubmesh;
	gearSubmesh.IndexCount = (UINT)gear.Indices32.size();
	gearSubmesh.StartIndexLocation = gearIndexOffset;
	gearSubmesh.BaseVertexLocation = gearVertexOffset;
	SubmeshGeometry d20Submesh;
	d20Submesh.IndexCount = (UINT)d20.Indices32.size();
	d20Submesh.StartIndexLocation = d20IndexOffset;
	d20Submesh.BaseVertexLocation = d20VertexOffset;
	SubmeshGeometry planeSubmesh;
	planeSubmesh.IndexCount = (UINT)plane.Indices32.size();
	planeSubmesh.StartIndexLocation = planeIndexOffset;
	planeSubmesh.BaseVertexLocation = planeVertexOffset;

	//
	// Extract the vertex elements we are interested in and pack the
	// vertices of all the meshes into one vertex buffer.
	//
	auto totalVertexCount =
		(box.Vertices.size() +
			grid.Vertices.size() +
			sphere.Vertices.size() +
			cylinder.Vertices.size() +

			torus.Vertices.size() +

			frosting.Vertices.size() +
			cone.Vertices.size() +
			crystal.Vertices.size() +
			wobble.Vertices.size() +
			screw.Vertices.size() +
			gear.Vertices.size() +
			d20.Vertices.size() +
			plane.Vertices.size()
			);
	std::vector<Vertex> vertices(totalVertexCount);
	UINT k = 0;
	for (size_t i = 0; i < box.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = box.Vertices[i].Position;
		vertices[k].Normal = box.Vertices[i].Normal;
		vertices[k].TexC = box.Vertices[i].TexC;
	}
	for (size_t i = 0; i < grid.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = grid.Vertices[i].Position;
		vertices[k].Normal = grid.Vertices[i].Normal;
		vertices[k].TexC = grid.Vertices[i].TexC;
	}
	for (size_t i = 0; i < sphere.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = sphere.Vertices[i].Position;
		vertices[k].Normal = sphere.Vertices[i].Normal;
		vertices[k].TexC = sphere.Vertices[i].TexC;
	}
	for (size_t i = 0; i < cylinder.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = cylinder.Vertices[i].Position;
		vertices[k].Normal = cylinder.Vertices[i].Normal;
		vertices[k].TexC = cylinder.Vertices[i].TexC;
	}
	for (size_t i = 0; i < torus.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = torus.Vertices[i].Position;
		vertices[k].Normal = torus.Vertices[i].Normal;
		vertices[k].TexC = torus.Vertices[i].TexC;
	}
	for (size_t i = 0; i < frosting.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = frosting.Vertices[i].Position;
		vertices[k].Normal = frosting.Vertices[i].Normal;
		vertices[k].TexC = frosting.Vertices[i].TexC;
	}
	for (size_t i = 0; i < cone.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = cone.Vertices[i].Position;
		vertices[k].Normal = cone.Vertices[i].Normal;
		vertices[k].TexC = cone.Vertices[i].TexC;
	}
	for (size_t i = 0; i < crystal.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = crystal.Vertices[i].Position;
		vertices[k].Normal = crystal.Vertices[i].Normal;
		vertices[k].TexC = crystal.Vertices[i].TexC;
	}
	for (size_t i = 0; i < wobble.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = wobble.Vertices[i].Position;
		vertices[k].Normal = wobble.Vertices[i].Normal;
		vertices[k].TexC = wobble.Vertices[i].TexC;
	}
	for (size_t i = 0; i < screw.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = screw.Vertices[i].Position;
		vertices[k].Normal = screw.Vertices[i].Normal;
		vertices[k].TexC = screw.Vertices[i].TexC;
	}
	for (size_t i = 0; i < gear.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = gear.Vertices[i].Position;
		vertices[k].Normal = gear.Vertices[i].Normal;
		vertices[k].TexC = gear.Vertices[i].TexC;
	}
	for (size_t i = 0; i < d20.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = d20.Vertices[i].Position;
		vertices[k].Normal = d20.Vertices[i].Normal;
		vertices[k].TexC = d20.Vertices[i].TexC;
	}
	for (size_t i = 0; i < plane.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = plane.Vertices[i].Position;
		vertices[k].Normal = plane.Vertices[i].Normal;
		vertices[k].TexC = plane.Vertices[i].TexC;
	}
	std::vector<std::uint16_t> indices;
	indices.insert(indices.end(), std::begin(box.GetIndices16()), std::end(box.GetIndices16()));
	indices.insert(indices.end(), std::begin(grid.GetIndices16()), std::end(grid.GetIndices16()));
	indices.insert(indices.end(), std::begin(sphere.GetIndices16()), std::end(sphere.GetIndices16()));
	indices.insert(indices.end(), std::begin(cylinder.GetIndices16()), std::end(cylinder.GetIndices16()));
	indices.insert(indices.end(), std::begin(torus.GetIndices16()), std::end(torus.GetIndices16()));
	indices.insert(indices.end(), std::begin(frosting.GetIndices16()), std::end(frosting.GetIndices16()));
	indices.insert(indices.end(), std::begin(cone.GetIndices16()), std::end(cone.GetIndices16()));
	indices.insert(indices.end(), std::begin(crystal.GetIndices16()), std::end(crystal.GetIndices16()));
	indices.insert(indices.end(), std::begin(wobble.GetIndices16()), std::end(wobble.GetIndices16()));
	indices.insert(indices.end(), std::begin(screw.GetIndices16()), std::end(screw.GetIndices16()));
	indices.insert(indices.end(), std::begin(gear.GetIndices16()), std::end(gear.GetIndices16()));
	indices.insert(indices.end(), std::begin(d20.GetIndices16()), std::end(d20.GetIndices16()));
	indices.insert(indices.end(), std::begin(plane.GetIndices16()), std::end(plane.GetIndices16()));
	const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);
	auto geo = std::make_unique<MeshGeometry>();
	geo->Name = "shapeGeo";
	ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
	CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);
	ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
	CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);
	geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);
	geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);
	geo->VertexByteStride = sizeof(Vertex);
	geo->VertexBufferByteSize = vbByteSize;
	geo->IndexFormat = DXGI_FORMAT_R16_UINT;
	geo->IndexBufferByteSize = ibByteSize;
	geo->DrawArgs["box"] = boxSubmesh;
	geo->DrawArgs["grid"] = gridSubmesh;
	geo->DrawArgs["sphere"] = sphereSubmesh;
	geo->DrawArgs["cylinder"] = cylinderSubmesh;
	geo->DrawArgs["torus"] = torusSubmesh;
	geo->DrawArgs["frosting"] = frostingSubmesh;
	geo->DrawArgs["cone"] = coneSubmesh;
	geo->DrawArgs["crystal"] = crystalSubmesh;
	geo->DrawArgs["wobble"] = wobbleSubmesh;
	geo->DrawArgs["screw"] = screwSubmesh;
	geo->DrawArgs["gear"] = gearSubmesh;
	geo->DrawArgs["d20"] = d20Submesh;
	geo->DrawArgs["plane"] = planeSubmesh;
	mGeometries[geo->Name] = std::move(geo);
}

void Game::BuildLandGeometry()
{
    GeometryGenerator geoGen;
    GeometryGenerator::MeshData grid = geoGen.CreateGrid(160.0f, 160.0f, 50, 50);

    //
    // Extract the vertex elements we are interested and apply the height function to
    // each vertex.  In addition, color the vertices based on their height so we have
    // sandy looking beaches, planety low hills, and snow mountain peaks.
    //

    std::vector<Vertex> vertices(grid.Vertices.size());
    for(size_t i = 0; i < grid.Vertices.size(); ++i)
    {
        auto& p = grid.Vertices[i].Position;
        vertices[i].Pos = p;
        vertices[i].Pos.y = GetHillsHeight(p.x, p.z);
        vertices[i].Normal = GetHillsNormal(p.x, p.z);
		vertices[i].TexC = grid.Vertices[i].TexC;
    }

    const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);

    std::vector<std::uint16_t> indices = grid.GetIndices16();
    const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	auto geo = std::make_unique<MeshGeometry>();
	geo->Name = "landGeo";

	ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
	CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
	CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);

	geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

	geo->VertexByteStride = sizeof(Vertex);
	geo->VertexBufferByteSize = vbByteSize;
	geo->IndexFormat = DXGI_FORMAT_R16_UINT;
	geo->IndexBufferByteSize = ibByteSize;

	SubmeshGeometry submesh;
	submesh.IndexCount = (UINT)indices.size();
	submesh.StartIndexLocation = 0;
	submesh.BaseVertexLocation = 0;

	geo->DrawArgs["grid"] = submesh;

	mGeometries["landGeo"] = std::move(geo);
}

void Game::BuildWavesGeometry()
{
    std::vector<std::uint16_t> indices(3 * mWaves->TriangleCount()); // 3 indices per face
	assert(mWaves->VertexCount() < 0x0000ffff);

    // Iterate over each quad.
    int m = mWaves->RowCount();
    int n = mWaves->ColumnCount();
    int k = 0;
    for(int i = 0; i < m - 1; ++i)
    {
        for(int j = 0; j < n - 1; ++j)
        {
            indices[k] = i*n + j;
            indices[k + 1] = i*n + j + 1;
            indices[k + 2] = (i + 1)*n + j;

            indices[k + 3] = (i + 1)*n + j;
            indices[k + 4] = i*n + j + 1;
            indices[k + 5] = (i + 1)*n + j + 1;

            k += 6; // next quad
        }
    }

	UINT vbByteSize = mWaves->VertexCount()*sizeof(Vertex);
	UINT ibByteSize = (UINT)indices.size()*sizeof(std::uint16_t);

	auto geo = std::make_unique<MeshGeometry>();
	geo->Name = "waterGeo";

	// Set dynamically.
	geo->VertexBufferCPU = nullptr;
	geo->VertexBufferGPU = nullptr;

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
	CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

	geo->VertexByteStride = sizeof(Vertex);
	geo->VertexBufferByteSize = vbByteSize;
	geo->IndexFormat = DXGI_FORMAT_R16_UINT;
	geo->IndexBufferByteSize = ibByteSize;

	SubmeshGeometry submesh;
	submesh.IndexCount = (UINT)indices.size();
	submesh.StartIndexLocation = 0;
	submesh.BaseVertexLocation = 0;

	geo->DrawArgs["grid"] = submesh;

	mGeometries["waterGeo"] = std::move(geo);
}

void Game::BuildBoxGeometry()
{
	GeometryGenerator geoGen;
	GeometryGenerator::MeshData box = geoGen.CreateBox(8.0f, 8.0f, 8.0f, 3);

	std::vector<Vertex> vertices(box.Vertices.size());
	for (size_t i = 0; i < box.Vertices.size(); ++i)
	{
		auto& p = box.Vertices[i].Position;
		vertices[i].Pos = p;
		vertices[i].Normal = box.Vertices[i].Normal;
		vertices[i].TexC = box.Vertices[i].TexC;
	}

	const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);

	std::vector<std::uint16_t> indices = box.GetIndices16();
	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	auto geo = std::make_unique<MeshGeometry>();
	geo->Name = "boxGeo";

	ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
	CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
	CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);

	geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

	geo->VertexByteStride = sizeof(Vertex);
	geo->VertexBufferByteSize = vbByteSize;
	geo->IndexFormat = DXGI_FORMAT_R16_UINT;
	geo->IndexBufferByteSize = ibByteSize;

	SubmeshGeometry submesh;
	submesh.IndexCount = (UINT)indices.size();
	submesh.StartIndexLocation = 0;
	submesh.BaseVertexLocation = 0;

	geo->DrawArgs["box"] = submesh;

	mGeometries["boxGeo"] = std::move(geo);
}

void Game::BuildTreeSpritesGeometry()
{
	//step5
	struct TreeSpriteVertex
	{
		XMFLOAT3 Pos;
		XMFLOAT2 Size;
	};

	static const int treeCount = 32;
	std::array<TreeSpriteVertex, 32> vertices;
	for(UINT i = 0; i < treeCount; ++i)
	{
		float x = 0;
		if (i < 16)
		{
			x = MathHelper::RandF(30.0f, 38.0f);
		}
		else
		{
			x = MathHelper::RandF(-30.0f, -38.0f);
		}
		float z = MathHelper::RandF(-30.0f, 35.0f);
		//float y = GetHillsHeight(x, z);
		float y = 0.0f;
		// Move tree slightly above land height.
		y += 8.0f;

		vertices[i].Pos = XMFLOAT3(x, y, z);
		vertices[i].Size = XMFLOAT2(20.0f, 20.0f);
	}



	std::array<std::uint16_t, 32> indices =
	{
		0, 1, 2, 3, 4, 5, 6, 7,
		8, 9, 10, 11, 12, 13, 14, 15,
		16, 17, 18, 19, 20, 21, 22, 23, 
		24, 25, 26, 27, 28, 29, 30, 31
	};

	const UINT vbByteSize = (UINT)vertices.size() * sizeof(TreeSpriteVertex);
	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	auto geo = std::make_unique<MeshGeometry>();
	geo->Name = "treeSpritesGeo";

	ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
	CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
	CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);

	geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

	geo->VertexByteStride = sizeof(TreeSpriteVertex);
	geo->VertexBufferByteSize = vbByteSize;
	geo->IndexFormat = DXGI_FORMAT_R16_UINT;
	geo->IndexBufferByteSize = ibByteSize;

	SubmeshGeometry submesh;
	submesh.IndexCount = (UINT)indices.size();
	submesh.StartIndexLocation = 0;
	submesh.BaseVertexLocation = 0;

	geo->DrawArgs["points"] = submesh;

	mGeometries["treeSpritesGeo"] = std::move(geo);
}

void Game::BuildPSOs()
{
    D3D12_GRAPHICS_PIPELINE_STATE_DESC opaquePsoDesc;

	//
	// PSO for opaque objects.
	//
    ZeroMemory(&opaquePsoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	opaquePsoDesc.InputLayout = { mStdInputLayout.data(), (UINT)mStdInputLayout.size() };
	opaquePsoDesc.pRootSignature = mRootSignature.Get();
	opaquePsoDesc.VS = 
	{ 
		reinterpret_cast<BYTE*>(mShaders["standardVS"]->GetBufferPointer()), 
		mShaders["standardVS"]->GetBufferSize()
	};
	opaquePsoDesc.PS = 
	{ 
		reinterpret_cast<BYTE*>(mShaders["opaquePS"]->GetBufferPointer()),
		mShaders["opaquePS"]->GetBufferSize()
	};
	opaquePsoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	opaquePsoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	opaquePsoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	opaquePsoDesc.SampleMask = UINT_MAX;
	opaquePsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	opaquePsoDesc.NumRenderTargets = 1;
	opaquePsoDesc.RTVFormats[0] = mBackBufferFormat;

	//there is abug with F2 key that is supposed to turn on the multisampling!
//Set4xMsaaState(true);
	//m4xMsaaState = true;

	opaquePsoDesc.SampleDesc.Count = m4xMsaaState ? 4 : 1;
	opaquePsoDesc.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0;
	opaquePsoDesc.DSVFormat = mDepthStencilFormat;
    ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&opaquePsoDesc, IID_PPV_ARGS(&mPSOs["opaque"])));

	//
	// PSO for transparent objects
	//

	D3D12_GRAPHICS_PIPELINE_STATE_DESC transparentPsoDesc = opaquePsoDesc;

	D3D12_RENDER_TARGET_BLEND_DESC transparencyBlendDesc;
	transparencyBlendDesc.BlendEnable = true;
	transparencyBlendDesc.LogicOpEnable = false;
	transparencyBlendDesc.SrcBlend = D3D12_BLEND_SRC_ALPHA;
	transparencyBlendDesc.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	transparencyBlendDesc.BlendOp = D3D12_BLEND_OP_ADD;
	transparencyBlendDesc.SrcBlendAlpha = D3D12_BLEND_ONE;
	transparencyBlendDesc.DestBlendAlpha = D3D12_BLEND_ZERO;
	transparencyBlendDesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
	transparencyBlendDesc.LogicOp = D3D12_LOGIC_OP_NOOP;
	transparencyBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	//transparentPsoDesc.BlendState.AlphaToCoverageEnable = true;

	transparentPsoDesc.BlendState.RenderTarget[0] = transparencyBlendDesc;
	ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&transparentPsoDesc, IID_PPV_ARGS(&mPSOs["transparent"])));

	//
	// PSO for alpha tested objects
	//

	D3D12_GRAPHICS_PIPELINE_STATE_DESC alphaTestedPsoDesc = opaquePsoDesc;
	alphaTestedPsoDesc.PS = 
	{ 
		reinterpret_cast<BYTE*>(mShaders["alphaTestedPS"]->GetBufferPointer()),
		mShaders["alphaTestedPS"]->GetBufferSize()
	};
	alphaTestedPsoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&alphaTestedPsoDesc, IID_PPV_ARGS(&mPSOs["alphaTested"])));

	//
	// PSO for tree sprites
	//
	D3D12_GRAPHICS_PIPELINE_STATE_DESC treeSpritePsoDesc = opaquePsoDesc;
	treeSpritePsoDesc.VS =
	{
		reinterpret_cast<BYTE*>(mShaders["treeSpriteVS"]->GetBufferPointer()),
		mShaders["treeSpriteVS"]->GetBufferSize()
	};
	treeSpritePsoDesc.GS =
	{
		reinterpret_cast<BYTE*>(mShaders["treeSpriteGS"]->GetBufferPointer()),
		mShaders["treeSpriteGS"]->GetBufferSize()
	};
	treeSpritePsoDesc.PS =
	{
		reinterpret_cast<BYTE*>(mShaders["treeSpritePS"]->GetBufferPointer()),
		mShaders["treeSpritePS"]->GetBufferSize()
	};
	//step1
	treeSpritePsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
	treeSpritePsoDesc.InputLayout = { mTreeSpriteInputLayout.data(), (UINT)mTreeSpriteInputLayout.size() };
	treeSpritePsoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;

	ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&treeSpritePsoDesc, IID_PPV_ARGS(&mPSOs["treeSprites"])));
}

void Game::BuildFrameResources()
{
    for(int i = 0; i < gNumFrameResources; ++i)
    {
        mFrameResources.push_back(std::make_unique<FrameResource>(md3dDevice.Get(),
            1, (UINT)mAllRitems.size(), (UINT)mMaterials.size(), mWaves->VertexCount()));
    }
}

void Game::BuildMaterials()
{
	auto planet = std::make_unique<Material>();
	planet->Name = "planet";
	planet->MatCBIndex = 0;
	planet->DiffuseSrvHeapIndex = 0;
	planet->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	planet->FresnelR0 = XMFLOAT3(0.01f, 0.01f, 0.01f);
	planet->Roughness = 0.125f;

	// This is not a good water material definition, but we do not have all the rendering
	// tools we need (transparency, environment reflection), so we fake it for now.
	auto water = std::make_unique<Material>();
	water->Name = "water";
	water->MatCBIndex = 1;
	water->DiffuseSrvHeapIndex = 1;
	water->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 0.5f);
	water->FresnelR0 = XMFLOAT3(0.1f, 0.1f, 0.1f);
	water->Roughness = 0.0f;

	auto wirefence = std::make_unique<Material>();
	wirefence->Name = "wirefence";
	wirefence->MatCBIndex = 2;
	wirefence->DiffuseSrvHeapIndex = 2;
	wirefence->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	wirefence->FresnelR0 = XMFLOAT3(0.02f, 0.02f, 0.02f);
	wirefence->Roughness = 0.25f;

	auto frost = std::make_unique<Material>();
	frost->Name = "frost";
	frost->MatCBIndex = 3;
	frost->DiffuseSrvHeapIndex = 3;
	frost->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	frost->FresnelR0 = XMFLOAT3(0.02f, 0.02f, 0.02f);
	frost->Roughness = 0.0f;

	auto sun = std::make_unique<Material>();
	sun->Name = "sun";
	sun->MatCBIndex = 4;
	sun->DiffuseSrvHeapIndex = 4;
	sun->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	sun->FresnelR0 = XMFLOAT3(0.02f, 0.02f, 0.02f);
	sun->Roughness = 0.25f;

	auto Defiant = std::make_unique<Material>();
	Defiant->Name = "Defiant";
	Defiant->MatCBIndex = 5;
	Defiant->DiffuseSrvHeapIndex = 5;
	Defiant->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	Defiant->FresnelR0 = XMFLOAT3(0.02f, 0.02f, 0.02f);
	Defiant->Roughness = 0.25f;

	auto space = std::make_unique<Material>();
	space->Name = "space";
	space->MatCBIndex = 6;
	space->DiffuseSrvHeapIndex = 6;
	space->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	space->FresnelR0 = XMFLOAT3(0.02f, 0.02f, 0.02f);
	space->Roughness = 0.25f;



	auto treeSprites = std::make_unique<Material>();
	treeSprites->Name = "treeSprites";
	treeSprites->MatCBIndex = 7;
	treeSprites->DiffuseSrvHeapIndex = 7;
	treeSprites->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	treeSprites->FresnelR0 = XMFLOAT3(0.01f, 0.01f, 0.01f);
	treeSprites->Roughness = 0.125f;


	mMaterials["planet"] = std::move(planet);
	mMaterials["water"] = std::move(water);
	mMaterials["wirefence"] = std::move(wirefence);
	mMaterials["frost"] = std::move(frost);
	mMaterials["sun"] = std::move(sun);
	mMaterials["Defiant"] = std::move(Defiant);
	mMaterials["space"] = std::move(space);
	mMaterials["treeSprites"] = std::move(treeSprites);
}

void Game::AddRenderItem(string name, vec3 position, float width, float height, string material, bool AlphaTest)
{
	auto item = std::make_unique<RenderItem>();
	item->name = name;
	item->width = width;
	item->height = height;
	item->position = position;
	item->localPos = position;
	item->parent = scene;

	//item->parent = parent;

	XMStoreFloat4x4(&item->World, XMMatrixScaling(width, 1.0f, height) * XMMatrixTranslation(position.x, position.y, position.z));
	XMStoreFloat4x4(&item->TexTransform, XMMatrixScaling(1.0f, 1.0f, 1.0f));
	item->ObjCBIndex = nextObjCBIndex;
	nextObjCBIndex += 1;
	
	item->Mat = mMaterials[material].get();
	item->Geo = mGeometries["shapeGeo"].get();
	item->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	item->IndexCount = item->Geo->DrawArgs["plane"].IndexCount;
	item->StartIndexLocation = item->Geo->DrawArgs["plane"].StartIndexLocation;
	item->BaseVertexLocation = item->Geo->DrawArgs["plane"].BaseVertexLocation;

	if (AlphaTest) 
	{
		mRitemLayer[(int)RenderLayer::AlphaTested].push_back(item.get());
	}
	else
	{
		mRitemLayer[(int)RenderLayer::Opaque].push_back(item.get());
	}

	mAllRitems.push_back(std::move(item));


}

void Game::AddPlayer(string name, vec3 position, float width, float height, string material, bool AlphaTest)
{
	auto item = std::make_unique<Player>();
	item->name = name;
	item->width = width;
	item->height = height;
	item->position = position;
	item->localPos = position;

	item->parent = scene;

	//item->parent = parent;

	XMStoreFloat4x4(&item->World, XMMatrixScaling(width, 1.0f, height) * XMMatrixTranslation(position.x, position.y, position.z));
	XMStoreFloat4x4(&item->TexTransform, XMMatrixScaling(1.0f, 1.0f, 1.0f));
	item->ObjCBIndex = nextObjCBIndex;
	nextObjCBIndex += 1;

	item->Mat = mMaterials[material].get();
	item->Geo = mGeometries["shapeGeo"].get();
	item->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	item->IndexCount = item->Geo->DrawArgs["plane"].IndexCount;
	item->StartIndexLocation = item->Geo->DrawArgs["plane"].StartIndexLocation;
	item->BaseVertexLocation = item->Geo->DrawArgs["plane"].BaseVertexLocation;

	if (AlphaTest)
	{
		mRitemLayer[(int)RenderLayer::AlphaTested].push_back(item.get());
	}
	else
	{
		mRitemLayer[(int)RenderLayer::Opaque].push_back(item.get());
	}

	mAllRitems.push_back(std::move(item));


}

void Game::AddScene(string name, vec3 position, float width, float height, string material, bool AlphaTest)
{
	auto item = std::make_unique<Scene>();
	item->name = name;
	item->width = width;
	item->height = height;
	item->position = position;
	item->localPos = position;

	item->parent = nullptr;

	//item->parent = parent;

	XMStoreFloat4x4(&item->World, XMMatrixScaling(width, 1.0f, height) * XMMatrixTranslation(position.x, position.y, position.z));
	XMStoreFloat4x4(&item->TexTransform, XMMatrixScaling(1.0f, 1.0f, 1.0f));
	item->ObjCBIndex = nextObjCBIndex;
	nextObjCBIndex += 1;

	item->Mat = mMaterials[material].get();
	item->Geo = mGeometries["shapeGeo"].get();
	item->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	item->IndexCount = item->Geo->DrawArgs["plane"].IndexCount;
	item->StartIndexLocation = item->Geo->DrawArgs["plane"].StartIndexLocation;
	item->BaseVertexLocation = item->Geo->DrawArgs["plane"].BaseVertexLocation;

	if (AlphaTest)
	{
		mRitemLayer[(int)RenderLayer::AlphaTested].push_back(item.get());
	}
	else
	{
		mRitemLayer[(int)RenderLayer::Opaque].push_back(item.get());
	}

	mAllRitems.push_back(std::move(item));


}




void Game::BuildRenderItems()
{
	{
		//   auto wavesRitem = std::make_unique<RenderItem>();
		//   wavesRitem->World = MathHelper::Identity4x4();
		   //XMStoreFloat4x4(&wavesRitem->World, XMMatrixScaling(1.0f, 1.0f, 1.0f) * XMMatrixRotationX(0.0f) * XMMatrixTranslation(0.0f, -1.0f, 0.0f));
		   //XMStoreFloat4x4(&wavesRitem->TexTransform, XMMatrixScaling(5.0f, 5.0f, 1.0f));
		   //wavesRitem->ObjCBIndex = 0;
		   //wavesRitem->Mat = mMaterials["water"].get();
		   //wavesRitem->Geo = mGeometries["waterGeo"].get();
		   //wavesRitem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		   //wavesRitem->IndexCount = wavesRitem->Geo->DrawArgs["grid"].IndexCount;
		   //wavesRitem->StartIndexLocation = wavesRitem->Geo->DrawArgs["grid"].StartIndexLocation;
		   //wavesRitem->BaseVertexLocation = wavesRitem->Geo->DrawArgs["grid"].BaseVertexLocation;

		//   mWavesRitem = wavesRitem.get();

		   //mRitemLayer[(int)RenderLayer::Transparent].push_back(wavesRitem.get());
		   /*
		   auto gridRitem = std::make_unique<RenderItem>();
		   gridRitem->World = MathHelper::Identity4x4();
		   XMStoreFloat4x4(&gridRitem->TexTransform, XMMatrixScaling(5.0f, 5.0f, 1.0f));
		   gridRitem->ObjCBIndex = 1;
		   gridRitem->Mat = mMaterials["planet"].get();
		   gridRitem->Geo = mGeometries["landGeo"].get();
		   gridRitem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		   gridRitem->IndexCount = gridRitem->Geo->DrawArgs["grid"].IndexCount;
		   gridRitem->StartIndexLocation = gridRitem->Geo->DrawArgs["grid"].StartIndexLocation;
		   gridRitem->BaseVertexLocation = gridRitem->Geo->DrawArgs["grid"].BaseVertexLocation;

		   mRitemLayer[(int)RenderLayer::Opaque].push_back(gridRitem.get());
		   */

		   //auto boxRitem = std::make_unique<RenderItem>();
		   //XMStoreFloat4x4(&boxRitem->World, XMMatrixTranslation(3.0f, 2.0f, -9.0f));
		   //boxRitem->ObjCBIndex = 1;
		   //boxRitem->Mat = mMaterials["sun"].get();
		   //boxRitem->Geo = mGeometries["boxGeo"].get();
		   //boxRitem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		   //boxRitem->IndexCount = boxRitem->Geo->DrawArgs["box"].IndexCount;
		   //boxRitem->StartIndexLocation = boxRitem->Geo->DrawArgs["box"].StartIndexLocation;
		   //boxRitem->BaseVertexLocation = boxRitem->Geo->DrawArgs["box"].BaseVertexLocation;

		   //mRitemLayer[(int)RenderLayer::AlphaTested].push_back(boxRitem.get());


		//   mAllRitems.push_back(std::move(wavesRitem));
		//  // mAllRitems.push_back(std::move(gridRitem));
		   //mAllRitems.push_back(std::move(boxRitem));
	}

	int ObjCBIndex = 0;

	{

		//auto gridRitem = std::make_unique<RenderItem>();
		//gridRitem->World = MathHelper::Identity4x4();
		//XMStoreFloat4x4(&gridRitem->World, XMMatrixScaling(1.0f, 1.0f, 1.0f) * XMMatrixRotationX(0.0f) * XMMatrixTranslation(0.0f, 0.0f, 5.0f));
		//XMStoreFloat4x4(&gridRitem->TexTransform, XMMatrixScaling(1.0f, 1.0f, 0.5f));
		//gridRitem->ObjCBIndex = ObjCBIndex++;
		//gridRitem->Mat = mMaterials["planet"].get();
		//gridRitem->Geo = mGeometries["shapeGeo"].get();
		//gridRitem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		//gridRitem->IndexCount = gridRitem->Geo->DrawArgs["grid"].IndexCount;
		//gridRitem->StartIndexLocation = gridRitem->Geo->DrawArgs["grid"].StartIndexLocation;
		//gridRitem->BaseVertexLocation = gridRitem->Geo->DrawArgs["grid"].BaseVertexLocation;

		//mRitemLayer[(int)RenderLayer::Opaque].push_back(gridRitem.get());
		//mAllRitems.push_back(std::move(gridRitem));



		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//auto torusRitem = std::make_unique<RenderItem>();
		//XMStoreFloat4x4(&torusRitem->World, XMMatrixScaling(1.0f, 1.0f, 1.0f) * XMMatrixRotationX(-0.35f) * XMMatrixTranslation(0.0f, 14.5f, 19.0f));
		//XMStoreFloat4x4(&torusRitem->TexTransform, XMMatrixScaling(1.0f, 1.0f, 1.0f));
		//torusRitem->ObjCBIndex = ObjCBIndex++;
		//torusRitem->Mat = mMaterials["sun"].get();
		//torusRitem->Geo = mGeometries["shapeGeo"].get();
		//torusRitem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		//torusRitem->IndexCount = torusRitem->Geo->DrawArgs["torus"].IndexCount;
		//torusRitem->StartIndexLocation = torusRitem->Geo->DrawArgs["torus"].StartIndexLocation;
		//torusRitem->BaseVertexLocation = torusRitem->Geo->DrawArgs["torus"].BaseVertexLocation;


		//mRitemLayer[(int)RenderLayer::Opaque].push_back(torusRitem.get());
		//mAllRitems.push_back(std::move(torusRitem));
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//auto frostingRitem = std::make_unique<RenderItem>();
		////gearAngle += 5.0f;
		//XMStoreFloat4x4(&frostingRitem->World, XMMatrixScaling(1.0f, 1.0f, 1.0f) * XMMatrixRotationX(-0.35f) * XMMatrixTranslation(0.0f, 15.5f, 18.5f));
		//XMStoreFloat4x4(&frostingRitem->TexTransform, XMMatrixScaling(1.0f, 1.0f, 1.0f));
		//frostingRitem->ObjCBIndex = ObjCBIndex++;
		////gearIndex = frostingRitem->ObjCBIndex;
		//frostingRitem->Mat = mMaterials["frost"].get();
		//frostingRitem->Geo = mGeometries["shapeGeo"].get();
		//frostingRitem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		//frostingRitem->IndexCount = frostingRitem->Geo->DrawArgs["frosting"].IndexCount;
		//frostingRitem->StartIndexLocation = frostingRitem->Geo->DrawArgs["frosting"].StartIndexLocation;
		//frostingRitem->BaseVertexLocation = frostingRitem->Geo->DrawArgs["frosting"].BaseVertexLocation;

		//mRitemLayer[(int)RenderLayer::Opaque].push_back(frostingRitem.get());
		//mAllRitems.push_back(std::move(frostingRitem));
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//auto coneRitem = std::make_unique<RenderItem>();
		////gearAngle += 5.0f;
		//XMStoreFloat4x4(&coneRitem->World, XMMatrixScaling(2.0f, 3.0f, 2.0f) * XMMatrixTranslation(0.0f, 0.0f, 4.0f));
		//XMStoreFloat4x4(&coneRitem->TexTransform, XMMatrixScaling(1.0f, 1.0f, 1.0f));
		//coneRitem->ObjCBIndex = ObjCBIndex++;
		////gearIndex = coneRitem->ObjCBIndex;
		//coneRitem->Mat = mMaterials["sun"].get();
		//coneRitem->Geo = mGeometries["shapeGeo"].get();
		//coneRitem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		//coneRitem->IndexCount = coneRitem->Geo->DrawArgs["cone"].IndexCount;
		//coneRitem->StartIndexLocation = coneRitem->Geo->DrawArgs["cone"].StartIndexLocation;
		//coneRitem->BaseVertexLocation = coneRitem->Geo->DrawArgs["cone"].BaseVertexLocation;
		//mRitemLayer[(int)RenderLayer::Opaque].push_back(coneRitem.get());
		//mAllRitems.push_back(std::move(coneRitem));

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//auto crystalRitem = std::make_unique<RenderItem>();
		//XMStoreFloat4x4(&crystalRitem->World, XMMatrixScaling(1.0f, 1.0f, 1.0f) * XMMatrixTranslation(0.0f, 9.0f, 4.0f) );
		//XMStoreFloat4x4(&crystalRitem->TexTransform, XMMatrixScaling(1.0f, 1.0f, 1.0f));
		//crystalRitem->ObjCBIndex = ObjCBIndex++;
		////gearIndex = crystalRitem->ObjCBIndex;
		//crystalRitem->Mat = mMaterials["sun"].get();
		//crystalRitem->Geo = mGeometries["shapeGeo"].get();
		//crystalRitem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		//crystalRitem->IndexCount = crystalRitem->Geo->DrawArgs["crystal"].IndexCount;
		//crystalRitem->StartIndexLocation = crystalRitem->Geo->DrawArgs["crystal"].StartIndexLocation;
		//crystalRitem->BaseVertexLocation = crystalRitem->Geo->DrawArgs["crystal"].BaseVertexLocation;

		//mRitemLayer[(int)RenderLayer::Opaque].push_back(crystalRitem.get());
		//mAllRitems.push_back(std::move(crystalRitem));
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//auto LwobbleRitem = std::make_unique<RenderItem>();
		//XMStoreFloat4x4(&LwobbleRitem->World, XMMatrixScaling(1.0f, 1.0f, 1.0f) * XMMatrixTranslation(-12.0f, 0.0f, 6.0f));
		//XMStoreFloat4x4(&LwobbleRitem->TexTransform, XMMatrixScaling(1.0f, 1.0f, 1.0f));
		//LwobbleRitem->ObjCBIndex = ObjCBIndex++;
		////gearIndex = wobbleRitem->ObjCBIndex;
		//LwobbleRitem->Mat = mMaterials["sun"].get();
		//LwobbleRitem->Geo = mGeometries["shapeGeo"].get();
		//LwobbleRitem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		//LwobbleRitem->IndexCount = LwobbleRitem->Geo->DrawArgs["wobble"].IndexCount;
		//LwobbleRitem->StartIndexLocation = LwobbleRitem->Geo->DrawArgs["wobble"].StartIndexLocation;
		//LwobbleRitem->BaseVertexLocation = LwobbleRitem->Geo->DrawArgs["wobble"].BaseVertexLocation;

		//mRitemLayer[(int)RenderLayer::Opaque].push_back(LwobbleRitem.get());
		//mAllRitems.push_back(std::move(LwobbleRitem));
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//auto RwobbleRitem = std::make_unique<RenderItem>();
		//XMStoreFloat4x4(&RwobbleRitem->World, XMMatrixScaling(1.0f, 1.0f, 1.0f) * XMMatrixTranslation(12.0f, 0.0f, 6.0f));
		//XMStoreFloat4x4(&RwobbleRitem->TexTransform, XMMatrixScaling(1.0f, 1.0f, 1.0f));
		//RwobbleRitem->ObjCBIndex = ObjCBIndex++;
		////gearIndex = wobbleRitem->ObjCBIndex;
		//RwobbleRitem->Mat = mMaterials["sun"].get();
		//RwobbleRitem->Geo = mGeometries["shapeGeo"].get();
		//RwobbleRitem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		//RwobbleRitem->IndexCount = RwobbleRitem->Geo->DrawArgs["wobble"].IndexCount;
		//RwobbleRitem->StartIndexLocation = RwobbleRitem->Geo->DrawArgs["wobble"].StartIndexLocation;
		//RwobbleRitem->BaseVertexLocation = RwobbleRitem->Geo->DrawArgs["wobble"].BaseVertexLocation;

		//mRitemLayer[(int)RenderLayer::Opaque].push_back(RwobbleRitem.get());
		//mAllRitems.push_back(std::move(RwobbleRitem));
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//auto LscrewRitem = std::make_unique<RenderItem>();
		//XMStoreFloat4x4(&LscrewRitem->World, XMMatrixScaling(1.0f, 1.0f, 1.0f) * XMMatrixRotationY(2.0f) * XMMatrixTranslation(-7.0f, -0.5f, 0.0f));
		//XMStoreFloat4x4(&LscrewRitem->TexTransform, XMMatrixScaling(1.0f, 1.0f, 1.0f));
		//LscrewRitem->ObjCBIndex = ObjCBIndex++;
		////gearIndex = LscrewRitem->ObjCBIndex;
		//LscrewRitem->Mat = mMaterials["sun"].get();
		//LscrewRitem->Geo = mGeometries["shapeGeo"].get();
		//LscrewRitem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		//LscrewRitem->IndexCount = LscrewRitem->Geo->DrawArgs["screw"].IndexCount;
		//LscrewRitem->StartIndexLocation = LscrewRitem->Geo->DrawArgs["screw"].StartIndexLocation;
		//LscrewRitem->BaseVertexLocation = LscrewRitem->Geo->DrawArgs["screw"].BaseVertexLocation;

		//mRitemLayer[(int)RenderLayer::Opaque].push_back(LscrewRitem.get());
		//mAllRitems.push_back(std::move(LscrewRitem));
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//auto RscrewRitem = std::make_unique<RenderItem>();
		//XMStoreFloat4x4(&RscrewRitem->World, XMMatrixScaling(1.0f, 1.0f, 1.0f) * XMMatrixTranslation(7.0f, -0.5f, 0.0f));
		//XMStoreFloat4x4(&RscrewRitem->TexTransform, XMMatrixScaling(1.0f, 1.0f, 1.0f));
		//RscrewRitem->ObjCBIndex = ObjCBIndex++;
		//RscrewRitem->Mat = mMaterials["sun"].get();
		//RscrewRitem->Geo = mGeometries["shapeGeo"].get();
		//RscrewRitem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		//RscrewRitem->IndexCount = RscrewRitem->Geo->DrawArgs["screw"].IndexCount;
		//RscrewRitem->StartIndexLocation = RscrewRitem->Geo->DrawArgs["screw"].StartIndexLocation;
		//RscrewRitem->BaseVertexLocation = RscrewRitem->Geo->DrawArgs["screw"].BaseVertexLocation;

		//mRitemLayer[(int)RenderLayer::Opaque].push_back(RscrewRitem.get());
		//mAllRitems.push_back(std::move(RscrewRitem));
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//auto LgearRitem = std::make_unique<RenderItem>();
		//XMStoreFloat4x4(&LgearRitem->World, XMMatrixScaling(4.0f, 4.0f, 4.0f) * XMMatrixRotationZ(-0.8f) * XMMatrixRotationY(0.5f) * XMMatrixTranslation(-16.0f, 4.0f, 10.0f));
		//XMStoreFloat4x4(&LgearRitem->TexTransform, XMMatrixScaling(1.0f, 1.0f, 1.0f));
		//LgearRitem->ObjCBIndex = ObjCBIndex++;
		////gearIndex = gearRitem->ObjCBIndex;
		//LgearRitem->Mat = mMaterials["sun"].get();
		//LgearRitem->Geo = mGeometries["shapeGeo"].get();
		//LgearRitem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		//LgearRitem->IndexCount = LgearRitem->Geo->DrawArgs["gear"].IndexCount;
		//LgearRitem->StartIndexLocation = LgearRitem->Geo->DrawArgs["gear"].StartIndexLocation;
		//LgearRitem->BaseVertexLocation = LgearRitem->Geo->DrawArgs["gear"].BaseVertexLocation;

		//mRitemLayer[(int)RenderLayer::Opaque].push_back(LgearRitem.get());
		//mAllRitems.push_back(std::move(LgearRitem));
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//auto RgearRitem = std::make_unique<RenderItem>();
		//XMStoreFloat4x4(&RgearRitem->World, XMMatrixScaling(4.0f, 4.0f, 4.0f) * XMMatrixRotationZ(0.8f) * XMMatrixRotationY(-0.5f) * XMMatrixTranslation(16.0f, 4.0f, 10.0f));
		//XMStoreFloat4x4(&RgearRitem->TexTransform, XMMatrixScaling(1.0f, 1.0f, 1.0f));
		//RgearRitem->ObjCBIndex = ObjCBIndex++;
		////gearIndex = gearRitem->ObjCBIndex;
		//RgearRitem->Mat = mMaterials["sun"].get();
		//RgearRitem->Geo = mGeometries["shapeGeo"].get();
		//RgearRitem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		//RgearRitem->IndexCount = RgearRitem->Geo->DrawArgs["gear"].IndexCount;
		//RgearRitem->StartIndexLocation = RgearRitem->Geo->DrawArgs["gear"].StartIndexLocation;
		//RgearRitem->BaseVertexLocation = RgearRitem->Geo->DrawArgs["gear"].BaseVertexLocation;

		//mRitemLayer[(int)RenderLayer::Opaque].push_back(RgearRitem.get());
		//mAllRitems.push_back(std::move(RgearRitem));
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//auto Ld20Ritem = std::make_unique<RenderItem>();
		//XMStoreFloat4x4(&Ld20Ritem->World, XMMatrixScaling(1.5f, 1.5f, 1.5f) * XMMatrixRotationX(0.6f) * XMMatrixTranslation(-12.0f, 19.0f, 6.0f));
		//XMStoreFloat4x4(&Ld20Ritem->TexTransform, XMMatrixScaling(1.0f, 1.0f, 1.0f));
		//Ld20Ritem->ObjCBIndex = ObjCBIndex++;
		////gearIndex = d20Ritem->ObjCBIndex;
		//Ld20Ritem->Mat = mMaterials["sun"].get();
		//Ld20Ritem->Geo = mGeometries["shapeGeo"].get();
		//Ld20Ritem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		//Ld20Ritem->IndexCount = Ld20Ritem->Geo->DrawArgs["d20"].IndexCount;
		//Ld20Ritem->StartIndexLocation = Ld20Ritem->Geo->DrawArgs["d20"].StartIndexLocation;
		//Ld20Ritem->BaseVertexLocation = Ld20Ritem->Geo->DrawArgs["d20"].BaseVertexLocation;

		//mRitemLayer[(int)RenderLayer::Opaque].push_back(Ld20Ritem.get());
		//mAllRitems.push_back(std::move(Ld20Ritem));
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//
		//
		//auto Rd20Ritem = std::make_unique<RenderItem>();
		//XMStoreFloat4x4(&Rd20Ritem->World, XMMatrixScaling(1.5f, 1.5f, 1.5f) * XMMatrixRotationX(0.6f) * XMMatrixTranslation(12.0f, 19.0f, 6.0f));
		//XMStoreFloat4x4(&Rd20Ritem->TexTransform, XMMatrixScaling(1.0f, 1.0f, 1.0f));
		//Rd20Ritem->ObjCBIndex = ObjCBIndex++;
		////gearIndex = d20Ritem->ObjCBIndex;
		//Rd20Ritem->Mat = mMaterials["sun"].get();
		//Rd20Ritem->Geo = mGeometries["shapeGeo"].get();
		//Rd20Ritem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		//Rd20Ritem->IndexCount = Rd20Ritem->Geo->DrawArgs["d20"].IndexCount;
		//Rd20Ritem->StartIndexLocation = Rd20Ritem->Geo->DrawArgs["d20"].StartIndexLocation;
		//Rd20Ritem->BaseVertexLocation = Rd20Ritem->Geo->DrawArgs["d20"].BaseVertexLocation;

		//mRitemLayer[(int)RenderLayer::Opaque].push_back(Rd20Ritem.get());
		//mAllRitems.push_back(std::move(Rd20Ritem));
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	}

	nextObjCBIndex = 0;


	AddScene("space", vec3(0.0f, 0.0f, 0.0f), 12.0f, 6.0f, "space", true);
	AddPlayer("player", vec3(0.0f, 2.0f, -20.0f), 0.82f, 1.0f, "Defiant", true);
	AddRenderItem("sun", vec3(-30.0f, 1.0f, 0.0f), 2.0f, 2.0f, "sun", true);
	AddRenderItem("planet", vec3(-14.0f, 0.0f, 0.0f), 0.4f, 0.4f, "planet", true);
	AddRenderItem("planet2", vec3(-10.0f, 0.0f, 0.0f), 0.2f, 0.2f, "planet", true);
	
	scene = (Scene*)mAllRitems[0].get();

	player = (Player*)mAllRitems[1].get();
	sun = mAllRitems[2].get();
	planet = mAllRitems[3].get();
	planet2 = mAllRitems[4].get();

	player->parent = scene;
	sun->parent = scene;

	planet->parent = sun;
	planet2->parent = sun;

	{
		//auto ployer = std::make_unique<RenderItem>();
		//player->name = "player";
		//XMStoreFloat4x4(&player->World, XMMatrixScaling(1.0f, 1.0f, 1.0f)* XMMatrixTranslation(0.0f, 1.0f, -100.0f));
		//XMStoreFloat4x4(&player->TexTransform, XMMatrixScaling(1.0f, 1.0f, 1.0f));
		//player->ObjCBIndex = nextObjCBIndex;
		//nextObjCBIndex++;
		////gearIndex = wobbleRitem->ObjCBIndex;
		//player->Mat = mMaterials["Defiant"].get();
		//player->Geo = mGeometries["shapeGeo"].get();
		//player->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		//player->IndexCount = player->Geo->DrawArgs["plane"].IndexCount;
		//player->StartIndexLocation = player->Geo->DrawArgs["plane"].StartIndexLocation;
		//player->BaseVertexLocation = player->Geo->DrawArgs["plane"].BaseVertexLocation;

		//
		//
		////mRitemLayer[(int)RenderLayer::Opaque].push_back(planeRitem.get());
		//mRitemLayer[(int)RenderLayer::AlphaTested].push_back(player.get());
		//mAllRitems.push_back(std::move(player));
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


		//auto space = std::make_unique<RenderItem>();
		//space->name = "space";
		//XMStoreFloat4x4(&space->World, XMMatrixScaling(12.0f, 1.0f, 6.0f)* XMMatrixTranslation(0.0f, -1.0f, 0.0f));
		//XMStoreFloat4x4(&space->TexTransform, XMMatrixScaling(1.0f, 1.0f, 1.0f));
		//space->ObjCBIndex = nextObjCBIndex;
		//nextObjCBIndex++;

		//space->Mat = mMaterials["space"].get();
		//space->Geo = mGeometries["shapeGeo"].get();
		//space->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		//space->IndexCount = space->Geo->DrawArgs["plane"].IndexCount;

		//space->StartIndexLocation = space->Geo->DrawArgs["plane"].StartIndexLocation;
		//space->BaseVertexLocation = space->Geo->DrawArgs["plane"].BaseVertexLocation;
		////mRitemLayer[(int)RenderLayer::AlphaTestedTreeSprites].push_back(space.get());

		//mRitemLayer[(int)RenderLayer::Opaque].push_back(space.get());
		////mRitemLayer[(int)RenderLayer::AlphaTested].push_back(space.get());
		//mAllRitems.push_back(std::move(space));
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


		//auto treeSpritesRitem = std::make_unique<RenderItem>();
		//treeSpritesRitem->World = MathHelper::Identity4x4();
		//treeSpritesRitem->ObjCBIndex = ObjCBIndex++;
		//treeSpritesRitem->Mat = mMaterials["treeSprites"].get();
		//treeSpritesRitem->Geo = mGeometries["treeSpritesGeo"].get();
		////step2
		//treeSpritesRitem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
		//treeSpritesRitem->IndexCount = treeSpritesRitem->Geo->DrawArgs["points"].IndexCount;
		//treeSpritesRitem->StartIndexLocation = treeSpritesRitem->Geo->DrawArgs["points"].StartIndexLocation;
		//treeSpritesRitem->BaseVertexLocation = treeSpritesRitem->Geo->DrawArgs["points"].BaseVertexLocation;

		//mRitemLayer[(int)RenderLayer::AlphaTestedTreeSprites].push_back(treeSpritesRitem.get());
		//mAllRitems.push_back(std::move(treeSpritesRitem));
	}
}

void Game::DrawRenderItems(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderItem*>& ritems)
{
    UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));
    UINT matCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(MaterialConstants));

	auto objectCB = mCurrFrameResource->ObjectCB->Resource();
	auto matCB = mCurrFrameResource->MaterialCB->Resource();

	player->localPos.z += 0.0005f;
	player->NumFramesDirty = gNumFrameResources;

	vec3 pPos = planet->localPos;
	pPos.Yrotate(0.01f);
	planet->localPos = pPos.Yrotate(0.01f);
	planet->NumFramesDirty = gNumFrameResources;

	planet2->localPos = planet2->localPos.Yrotate(0.05f);
	planet2->NumFramesDirty = gNumFrameResources;

	sun->localPos.x += 0.0005;
	sun->NumFramesDirty = gNumFrameResources;

	//mAllRitems[3].get()->position = mAllRitems[2].get()->position.add(mAllRitems[3].get()->localPos);

    // For each render item...
    for(size_t i = 0; i < ritems.size(); ++i)
    {
        auto ri = ritems[i];

		//if (ri->name == "player")
		//{
		//	ri->NumFramesDirty = gNumFrameResources;

		//	if(planeMove < 20)
		//	XMStoreFloat4x4(&ri->World, XMMatrixScaling(ri->width, 1.0f, ri->height) * XMMatrixTranslation(0.0f, 0.0f, planeMove));
		//}

		if (ri->NumFramesDirty > 0)
		{
			if (ri->parent != nullptr)
			{
				ri->position = ri->parent->position.add(ri->localPos);
			}
			
			XMStoreFloat4x4(&ri->World, XMMatrixScaling(ri->width, 1.0f, ri->height) * XMMatrixTranslation(ri->position.x, ri->position.y, ri->position.z));

			
		}

        cmdList->IASetVertexBuffers(0, 1, &ri->Geo->VertexBufferView());
        cmdList->IASetIndexBuffer(&ri->Geo->IndexBufferView());
		//step3
        cmdList->IASetPrimitiveTopology(ri->PrimitiveType);

		CD3DX12_GPU_DESCRIPTOR_HANDLE tex(mSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
		tex.Offset(ri->Mat->DiffuseSrvHeapIndex, mCbvSrvDescriptorSize);

        D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = objectCB->GetGPUVirtualAddress() + ri->ObjCBIndex*objCBByteSize;
		D3D12_GPU_VIRTUAL_ADDRESS matCBAddress = matCB->GetGPUVirtualAddress() + ri->Mat->MatCBIndex*matCBByteSize;

		cmdList->SetGraphicsRootDescriptorTable(0, tex);
        cmdList->SetGraphicsRootConstantBufferView(1, objCBAddress);
        cmdList->SetGraphicsRootConstantBufferView(3, matCBAddress);

        cmdList->DrawIndexedInstanced(ri->IndexCount, 1, ri->StartIndexLocation, ri->BaseVertexLocation, 0);
    }
}

std::array<const CD3DX12_STATIC_SAMPLER_DESC, 6> Game::GetStaticSamplers()
{
	// Applications usually only need a handful of samplers.  So just define them all up front
	// and keep them available as part of the root signature.  

	const CD3DX12_STATIC_SAMPLER_DESC pointWrap(
		0, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC pointClamp(
		1, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC linearWrap(
		2, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC linearClamp(
		3, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC anisotropicWrap(
		4, // shaderRegister
		D3D12_FILTER_ANISOTROPIC, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressW
		0.0f,                             // mipLODBias
		8);                               // maxAnisotropy

	const CD3DX12_STATIC_SAMPLER_DESC anisotropicClamp(
		5, // shaderRegister
		D3D12_FILTER_ANISOTROPIC, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressW
		0.0f,                              // mipLODBias
		8);                                // maxAnisotropy

	return { 
		pointWrap, pointClamp,
		linearWrap, linearClamp, 
		anisotropicWrap, anisotropicClamp };
}

float Game::GetHillsHeight(float x, float z)const
{
    return 0.3f*(z*sinf(0.1f*x) + x*cosf(0.1f*z));
}

XMFLOAT3 Game::GetHillsNormal(float x, float z)const
{
    // n = (-df/dx, 1, -df/dz)
    XMFLOAT3 n(
        -0.03f*z*cosf(0.1f*x) - 0.3f*cosf(0.1f*z),
        1.0f,
        -0.3f*sinf(0.1f*x) + 0.03f*x*sinf(0.1f*z));

    XMVECTOR unitNormal = XMVector3Normalize(XMLoadFloat3(&n));
    XMStoreFloat3(&n, unitNormal);

    return n;
}
