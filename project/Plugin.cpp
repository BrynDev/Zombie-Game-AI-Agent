#include "stdafx.h"
#include "Plugin.h"
#include "IExamInterface.h"
#include "SteeringBehaviors.h"
#include "EFiniteStateMachine.h"
#include "EBlackboard.h"
#include "StatesAndTransitions.h"
#include "SteeringController.h"

//Called only once, during initialization
void Plugin::Initialize(IBaseInterface* pInterface, PluginInfo& info)
{
	//Retrieving the interface
	//This interface gives you access to certain actions the AI_Framework can perform for you
	m_pInterface = static_cast<IExamInterface*>(pInterface);
	m_pBlackboard->AddData("Interface", m_pInterface);

	//Bit information about the plugin
	//Please fill this in!!
	info.BotName = "Drip";
	info.Student_FirstName = "Bryn";
	info.Student_LastName = "Couvreur";
	info.Student_Class = "2DAE01";
}

//Called only once
void Plugin::DllInit()
{
	//Called when the plugin is loaded
	m_pSteeringController = new SteeringController();

	//setup blackboard
	m_pBlackboard = new Elite::Blackboard();
	m_pBlackboard->AddData("SteeringController", m_pSteeringController);

	std::vector<HouseInfo> houseInfoRef{};
	m_pBlackboard->AddData("HousesInFOV", houseInfoRef);

	std::vector<EntityInfo> entityInfoRef{};
	m_pBlackboard->AddData("EntitiesInFOV", entityInfoRef);

	TargetData targetData{};
	m_pBlackboard->AddData("Target", targetData);

	HouseInfo targetHouseInfo{};
	m_pBlackboard->AddData("TargetHouse", targetHouseInfo);

	EntityInfo targetItem{};
	m_pBlackboard->AddData("TargetItem", targetItem);

	EnemyInfo targetEnemy{};
	m_pBlackboard->AddData("TargetEnemy", targetEnemy);

	m_pBlackboard->AddData("HouseEntryPoint", Elite::Vector2{});
	m_pBlackboard->AddData("WeaponInventoryIndex", int{-1});
	m_pBlackboard->AddData("AutoOrient", bool{ true });

	//state machine setup

	//STATES
	m_pWanderState = new WanderState();	
	m_pFleeState = new FleeState();
	m_pEnterHouseState = new EnterHouseState();
	m_pSearchCurrentHouseState = new SearchCurrentHouseState();
	m_pExitCurrentHouseState = new ExitCurrentHouseState();
	m_pGrabItemState = new GrabItemState();
	m_pKillZombieState = new KillZombieState();
	//TRANSITIONS
	m_pSeesZombieTransition = new SeesZombieTransition();
	m_pSeesHouseTransition = new SeesHouseTransition();
	m_pSeesItemTransition = new SeesItemTransition();
	m_pFinishedFleeingTransition = new FinishedFleeingTransition();
	m_pIsInsideHouseTransition = new IsInsideHouseTransition();
	m_pIsNotInsideHouseTransition = new IsNotInsideHouseTransition();
	m_pFinishedSearchingHouseTransition = new FinishedSearchingHouseTransition();
	m_pHasGrabbedItemTransition = new HasGrabbedItemTransition();
	m_pCanKillZombieTransition = new CanKillZombieTransition();

	//STATE MACHINE
	m_pFiniteStateMachine = new FiniteStateMachine( m_pWanderState, m_pBlackboard);
	//zombie related
	m_pFiniteStateMachine->AddTransition(m_pWanderState, m_pKillZombieState, m_pCanKillZombieTransition);
	m_pFiniteStateMachine->AddTransition(m_pWanderState, m_pFleeState, m_pSeesZombieTransition);
	m_pFiniteStateMachine->AddTransition(m_pFleeState, m_pWanderState, m_pFinishedFleeingTransition);
	m_pFiniteStateMachine->AddTransition(m_pExitCurrentHouseState, m_pKillZombieState, m_pCanKillZombieTransition);
	m_pFiniteStateMachine->AddTransition(m_pSearchCurrentHouseState, m_pKillZombieState, m_pCanKillZombieTransition);
	//m_pFiniteStateMachine->AddTransition(m_pKillZombieState, m_pWanderState, m_pCanKillZombieTransition);
	//house related
	m_pFiniteStateMachine->AddTransition(m_pWanderState, m_pEnterHouseState, m_pSeesHouseTransition);
	m_pFiniteStateMachine->AddTransition(m_pEnterHouseState, m_pSearchCurrentHouseState, m_pIsInsideHouseTransition);
	m_pFiniteStateMachine->AddTransition(m_pSearchCurrentHouseState, m_pGrabItemState, m_pSeesItemTransition);
	m_pFiniteStateMachine->AddTransition(m_pGrabItemState, m_pSearchCurrentHouseState, m_pHasGrabbedItemTransition);
	m_pFiniteStateMachine->AddTransition(m_pSearchCurrentHouseState, m_pExitCurrentHouseState, m_pFinishedSearchingHouseTransition);
	m_pFiniteStateMachine->AddTransition(m_pExitCurrentHouseState, m_pFleeState, m_pIsNotInsideHouseTransition);
	m_pFiniteStateMachine->AddTransition(m_pExitCurrentHouseState, m_pGrabItemState, m_pHasGrabbedItemTransition);
}

//Called only once
void Plugin::DllShutdown()
{
	//Called when the plugin gets unloaded

	//delete m_pBlackboard;
	delete m_pFiniteStateMachine;
	//STATES
	delete m_pWanderState;
	delete m_pEnterHouseState;
	delete m_pSearchCurrentHouseState;
	delete m_pExitCurrentHouseState;
	delete m_pGrabItemState;
	delete m_pKillZombieState;
	//TRANSITIONS
	delete m_pSeesZombieTransition;
	delete m_pSeesHouseTransition;
	delete m_pFinishedFleeingTransition;
	delete m_pSeesItemTransition;
	delete m_pIsInsideHouseTransition;
	delete m_pIsNotInsideHouseTransition;
	delete m_pHasGrabbedItemTransition;
	delete m_pCanKillZombieTransition;

	delete m_pSteeringController;
}

//Called only once, during initialization
void Plugin::InitGameDebugParams(GameDebugParams& params)
{
	params.AutoFollowCam = true; //Automatically follow the AI? (Default = true)
	params.RenderUI = true; //Render the IMGUI Panel? (Default = true)
	params.SpawnEnemies = true; //Do you want to spawn enemies? (Default = true)
	params.EnemyCount = 20; //How many enemies? (Default = 20)
	params.GodMode = false; //GodMode > You can't die, can be usefull to inspect certain behaviours (Default = false)
	params.AutoGrabClosestItem = true; //A call to Item_Grab(...) returns the closest item that can be grabbed. (EntityInfo argument is ignored)
}

//Only Active in DEBUG Mode
//(=Use only for Debug Purposes)
void Plugin::Update(float dt)
{
	//Demo Event Code
	//In the end your AI should be able to walk around without external input
	if (m_pInterface->Input_IsMouseButtonUp(Elite::InputMouseButton::eLeft))
	{
		//Update target based on input
		Elite::MouseData mouseData = m_pInterface->Input_GetMouseData(Elite::InputType::eMouseButton, Elite::InputMouseButton::eLeft);
		const Elite::Vector2 pos = Elite::Vector2(static_cast<float>(mouseData.X), static_cast<float>(mouseData.Y));
		m_Target = m_pInterface->Debug_ConvertScreenToWorld(pos);
	}
	else if (m_pInterface->Input_IsKeyboardKeyDown(Elite::eScancode_Space))
	{
		m_CanRun = true;
	}
	else if (m_pInterface->Input_IsKeyboardKeyDown(Elite::eScancode_Left))
	{
		m_AngSpeed -= Elite::ToRadians(10);
	}
	else if (m_pInterface->Input_IsKeyboardKeyDown(Elite::eScancode_Right))
	{
		m_AngSpeed += Elite::ToRadians(10);
	}
	else if (m_pInterface->Input_IsKeyboardKeyDown(Elite::eScancode_G))
	{
		m_GrabItem = true;
	}
	else if (m_pInterface->Input_IsKeyboardKeyDown(Elite::eScancode_U))
	{
		m_UseItem = true;
	}
	else if (m_pInterface->Input_IsKeyboardKeyDown(Elite::eScancode_R))
	{
		m_RemoveItem = true;
	}
	else if (m_pInterface->Input_IsKeyboardKeyUp(Elite::eScancode_Space))
	{
		m_CanRun = false;
	}
}

//Update
//This function calculates the new SteeringOutput, called once per frame
SteeringPlugin_Output Plugin::UpdateSteering(float dt)
{


	std::vector<HouseInfo> vHousesInFOV = GetHousesInFOV();//uses m_pInterface->Fov_GetHouseByIndex(...)
	m_pBlackboard->ChangeData("HousesInFOV", vHousesInFOV);
	std::vector<EntityInfo> vEntitiesInFOV = GetEntitiesInFOV(); //uses m_pInterface->Fov_GetEntityByIndex(...)
	m_pBlackboard->ChangeData("EntitiesInFOV", vEntitiesInFOV);
	
	//UseConsumables(agentInfo, vEntitiesInFOV);
	m_pFiniteStateMachine->Update(dt);
	auto agentInfo = m_pInterface->Agent_GetInfo();

	//return steering;
	SteeringPlugin_Output steering{ m_pSteeringController->CalculateSteering(dt, agentInfo) };

	if (agentInfo.WasBitten)
	{
		m_CanRun = true;
	}
	if (agentInfo.RunMode && agentInfo.Stamina <= 0.1)
	{
		m_CanRun = false;
	}
	steering.RunMode = m_CanRun;

	m_pBlackboard->GetData("AutoOrient", steering.AutoOrient);
	return steering;
}

//This function should only be used for rendering debug elements
void Plugin::Render(float dt) const
{
	//This Render function should only contain calls to Interface->Draw_... functions
	m_pInterface->Draw_SolidCircle(m_Target, .7f, { 0,0 }, { 1, 0, 0 });
}

vector<HouseInfo> Plugin::GetHousesInFOV() const
{
	vector<HouseInfo> vHousesInFOV = {};

	HouseInfo hi = {};
	for (int i = 0;; ++i)
	{
		if (m_pInterface->Fov_GetHouseByIndex(i, hi))
		{
			vHousesInFOV.push_back(hi);
			continue;
		}

		break;
	}

	return vHousesInFOV;
}

vector<EntityInfo> Plugin::GetEntitiesInFOV() const
{
	vector<EntityInfo> vEntitiesInFOV = {};

	EntityInfo ei = {};
	for (int i = 0;; ++i)
	{
		if (m_pInterface->Fov_GetEntityByIndex(i, ei))
		{
			vEntitiesInFOV.push_back(ei);
			continue;
		}

		break;
	}

	return vEntitiesInFOV;
}

void Plugin::UseConsumables(const AgentInfo& agentInfo, const std::vector<EntityInfo>& entitiesInFOV)
{
	const unsigned int invCapacity{ m_pInterface->Inventory_GetCapacity() };

	//check all inventory items
	for (unsigned int i{ 0 }; i < invCapacity; ++i)
	{
		ItemInfo invItem{};

		if (!m_pInterface->Inventory_GetItem(i, invItem))
		{
			//if this slot is empty, continue
			continue;
		}
		
		int itemValue{};
		switch (invItem.Type)
		{
		case eItemType::FOOD:
			itemValue = m_pInterface->Food_GetEnergy(invItem);
			if (agentInfo.Energy + itemValue < 10.f)
			{
				m_pInterface->Inventory_UseItem(i);
				m_pInterface->Inventory_RemoveItem(i);
			}		
			break;
		case eItemType::MEDKIT:
			itemValue = m_pInterface->Medkit_GetHealth(invItem);
			if (agentInfo.Health + itemValue < 10.f)
			{
				m_pInterface->Inventory_UseItem(i);
				m_pInterface->Inventory_RemoveItem(i);
			}
			break;
		case eItemType::PISTOL:
			/*for(const EntityInfo& info : entitiesInFOV)
			{
				if (info.Type == eEntityType::ENEMY)
				{
					m_pInterface->Inventory_UseItem(i);
				}
			}*/
			break;
		default:
			break;
		}
	}
}
