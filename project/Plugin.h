#pragma once
#include "IExamPlugin.h"
#include "Exam_HelperStructs.h"

class IBaseInterface;
class IExamInterface;

class SteeringController;
namespace Elite
{
	class FiniteStateMachine;
	class Blackboard;
	class FSMState;
	class FSMTransition;
}

class Plugin :public IExamPlugin
{
public:
	Plugin() {};
	virtual ~Plugin() {};

	void Initialize(IBaseInterface* pInterface, PluginInfo& info) override;
	void DllInit() override;
	void DllShutdown() override;

	void InitGameDebugParams(GameDebugParams& params) override;
	void Update(float dt) override;

	SteeringPlugin_Output UpdateSteering(float dt) override;
	void Render(float dt) const override;

private:
	//Interface, used to request data from/perform actions with the AI Framework
	IExamInterface* m_pInterface = nullptr;
	vector<HouseInfo> GetHousesInFOV() const;
	vector<EntityInfo> GetEntitiesInFOV() const;
	void UseConsumables(const AgentInfo& agentInfo);

	Elite::Vector2 m_Target = {};
	bool m_CanRun = false; //Demo purpose
	bool m_GrabItem = false; //Demo purpose
	bool m_UseItem = false; //Demo purpose
	bool m_RemoveItem = false; //Demo purpose
	float m_AngSpeed = 0.f; //Demo purpose

	//=========
	Elite::FiniteStateMachine* m_pFiniteStateMachine = nullptr;
	Elite::Blackboard* m_pBlackboard = nullptr;
	Elite::FSMState* m_pWanderState = nullptr;
	Elite::FSMState* m_pFleeState = nullptr;
	Elite::FSMState* m_pEnterHouseState = nullptr;
	Elite::FSMState* m_pSearchCurrentHouseState = nullptr;
	Elite::FSMState* m_pExitCurrentHouseState = nullptr;
	Elite::FSMState* m_pGrabItemState = nullptr;
	Elite::FSMState* m_pKillZombieState = nullptr;
	Elite::FSMState* m_pGoToWorldCenterState = nullptr;
	Elite::FSMState* m_pFleePurgeZoneState = nullptr;

	Elite::FSMTransition* m_pSeesZombieTransition = nullptr;
	Elite::FSMTransition* m_pSeesHouseTransition = nullptr;
	Elite::FSMTransition* m_pSeesItemTransition = nullptr;
	Elite::FSMTransition* m_pFinishedFleeingTransition = nullptr;
	Elite::FSMTransition* m_pIsInsideHouseTransition = nullptr;
	Elite::FSMTransition* m_pIsNotInsideHouseTransition = nullptr;
	Elite::FSMTransition* m_pFinishedSearchingHouseTransition = nullptr;
	Elite::FSMTransition* m_pHasGrabbedItemTransition = nullptr;
	Elite::FSMTransition* m_pCanKillZombieTransition = nullptr;
	Elite::FSMTransition* m_pHasKilledZombieTransition = nullptr;
	Elite::FSMTransition* m_pHasLeftWorldTransition = nullptr;
	Elite::FSMTransition* m_pIsAtWorldCenterTransition = nullptr;
	Elite::FSMTransition* m_pSeesPurgeZoneTransition = nullptr;
	Elite::FSMTransition* m_pHasLeftPurgeZoneTransition = nullptr;

	SteeringController* m_pSteeringController = nullptr;
	//=========
};

//ENTRY
//This is the first function that is called by the host program
//The plugin returned by this function is also the plugin used by the host program
extern "C"
{
	__declspec (dllexport) IPluginBase* Register()
	{
		return new Plugin();
	}
}