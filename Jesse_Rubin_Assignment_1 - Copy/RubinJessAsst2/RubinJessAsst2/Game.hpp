#pragma once

#include "d3dApp.h"
//#include "World.h"
//#include "Player.h"
#include "State.h"
#include <Vector>
using namespace std;


enum class RenderLayer : int
{
	Opaque = 0,
	Transparent,
	AlphaTested,
	AlphaTestedTreeSprites,
	Count
};

enum class States : int
{
	TITLE = 0,
	MENU = 1,
	KEYBIND = 2,
	GAME = 3,
	PAUSE = 4
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
	float screenWidth = 80;
	float screenHeight = 60;

	// Render items divided by PSO.
	std::vector<RenderItem*> mRitemLayer[(int)RenderLayer::Count];
	
	// Commands

	void ChangeState(States s);
	//void ChangeState(int s);

	void Pause();
	void UnPause();
	void MainMenu();
	void TitleScreen();
	void StartGame();
	void Keybind();
	
	void WASD();
	void ArrowKeys();

public:
	vector<unique_ptr<RenderItem>>& getRenderItems()
	{
		return mAllRitems;
	}
	unordered_map<string, unique_ptr<Material>>& getMaterials()
	{
		return mMaterials;
	}
	unordered_map<string, unique_ptr<MeshGeometry>>& getGeometries()
	{
		return mGeometries;
	}
	int numRenderItems()
	{
		return mAllRitems.size();
	}

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


	void LoadTextures();
	void BuildRootSignature();
	void BuildDescriptorHeaps();
	void BuildShadersAndInputLayouts();
	void BuildShapeGeometry();


	void BuildBoxGeometry();
	void BuildTreeSpritesGeometry();
	void BuildPSOs();
	void BuildFrameResources();
	void BuildMaterials();
	void BuildRenderItems();
	void DrawRenderItems(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderItem*>& ritems);

	void ProcessInput();

	void BuildStates();
	void ReorderStates();
	std::array<const CD3DX12_STATIC_SAMPLER_DESC, 6> GetStaticSamplers();



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

	


	PassConstants mMainPassCB;

	//XMFLOAT3 mEyePos = { 0.0f, 0.0f, 0.0f };
	//XMFLOAT4X4 mView = MathHelper::Identity4x4();
	//XMFLOAT4X4 mProj = MathHelper::Identity4x4();

	//float mTheta = 1.5f * XM_PI;
	//float mPhi = XM_PIDIV2 - 0.1f;
	//float mRadius = 50.0f;

	POINT mLastMousePos;

	float planeMove = -20.0f;

	Camera camera;
	//World world;

	XMVECTOR camPos = XMVectorSet(0.0f, 5.0f, 0.0f, 1.0f);
	XMVECTOR target = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
	XMVECTOR targetDirection = XMVectorSet(0.0f, -1.0f, 0.0f, 1.0f);
	XMVECTOR baseForward = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
	XMVECTOR baseBack = XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f);
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	XMVECTOR north = XMVectorSet(0.0f, 0.0f, 1.0f, 1.0f);


	Player mPlayer;

	std::vector<State*> stateList;

	State* activeState;
	State* previousState;

	States active;
	States prev;

	bool initializedStates;
};