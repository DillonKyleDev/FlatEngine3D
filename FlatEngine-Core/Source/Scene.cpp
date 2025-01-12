#include "Scene.h"
#include "FlatEngine.h"
#include "Application.h"
#include "GameObject.h"
#include "Transform.h"
#include "Sprite.h"
#include "Camera.h"
#include "Script.h"
#include "Button.h"
#include "Canvas.h"
#include "Animation.h"
#include "Audio.h"
#include "Text.h"
#include "Collider.h"
#include "CompositeCollider.h"
#include "BoxCollider.h"
#include "CircleCollider.h"
#include "RigidBody.h"
#include "GameLoop.h"
#include "ECSManager.h"
#include "TileMap.h"
#include "FlatEngine.h"

namespace FL = FlatEngine;


namespace FlatEngine
{
	Scene::Scene()
	{
		m_name = "New Scene";
		m_path = "";
		m_sceneObjects = std::map<long, GameObject>();		
		m_animatorPreviewObjects = std::vector<GameObject*>();
		m_ECSManager = ECSManager();
		m_primaryCamera = nullptr;
		m_nextGameObjectID = 0;
		m_nextComponentID = 0;
		m_freedComponentIDs = std::vector<long>();
		m_freedGameObjectIDs = std::vector<long>();
		m_b_persistantScene = false;
	}

	Scene::~Scene()
	{
	}

    void Scene::SetName(std::string newName)
    {
		m_name = newName;
    }

	std::string Scene::GetName()
	{
		return m_name;
	}

	void Scene::SetPath(std::string scenePath)
	{
		m_path = scenePath;
	}

	std::string Scene::GetPath()
	{
		return m_path;
	}

	void Scene::UnloadECSManager()
	{
		m_ECSManager.Cleanup();
	}

	void Scene::UnloadSceneObjects()
	{
		m_sceneObjects.clear();
	}

	GameObject* Scene::AddSceneObject(GameObject sceneObject)
	{
		long id = sceneObject.GetID();
		m_sceneObjects.emplace(id, sceneObject);
		KeepNextGameObjectIDUpToDate(id);

		if (sceneObject.HasComponent("BoxCollider") || (sceneObject.HasComponent("TileMap") && sceneObject.GetTileMap()->GetCollisionAreas().size() > 0))
		{
			UpdateColliderPairs();
		}

		return &m_sceneObjects.at(id);
	}

	void Scene::KeepNextGameObjectIDUpToDate(long ID)
	{
		if (ID >= m_nextGameObjectID)
		{
			m_nextGameObjectID = ID + 1;
		}
	}

	std::map<long, GameObject> &Scene::GetSceneObjects()
	{
		return m_sceneObjects;
	}

	void Scene::SetAnimatorPreviewObjects(std::vector<GameObject*> previewObjects)
	{
		m_animatorPreviewObjects = previewObjects;
	}

	std::vector<GameObject*> Scene::GetAnimatorPreviewObjects()
	{
		return m_animatorPreviewObjects;
	}

	GameObject* Scene::GetObjectByID(long ID)
	{
		if (m_sceneObjects.count(ID) > 0)
		{
			return &m_sceneObjects.at(ID);
		}
		else 
		{
			return nullptr;
		}
	}

	GameObject* Scene::GetObjectByName(std::string name)
	{
		for (std::map<long, GameObject>::iterator iter = m_sceneObjects.begin(); iter != m_sceneObjects.end(); iter++)
		{
			if (name == iter->second.GetName())
			{
				return &iter->second;
			}
		
		}
		for (GameObject *animPreviewObject : m_animatorPreviewObjects)
		{
			if (name == animPreviewObject->GetName())
			{
				return animPreviewObject;
			}
		}
		return nullptr;
	}

	GameObject* Scene::GetObjectByTag(std::string tag)
	{
		for (std::map<long, GameObject>::iterator iter = m_sceneObjects.begin(); iter != m_sceneObjects.end(); iter++)
		{
			if (iter->second.GetTagList().HasTag(tag))
			{
				return &iter->second;
			}
		}
		return nullptr;
	}

	GameObject* Scene::CreateGameObject(long parentID, long myID)
	{
		GameObject newObject = GameObject(parentID, myID);
		newObject.SetPersistant(m_b_persistantScene);
		newObject.AddTransform();

		if (parentID != -1 && m_sceneObjects.count(parentID))
		{
			m_sceneObjects.at(parentID).AddChild(newObject.GetID());
		}

		return AddSceneObject(newObject);
	}

	void Scene::DeleteGameObject(long sceneObjectID)
	{
		GameObject *objectToDelete = GetObjectByID(sceneObjectID);	

		if (objectToDelete != nullptr)
		{
			Scene::DeleteChildrenAndSelf(objectToDelete);
		}
	}

	void Scene::DeleteGameObject(GameObject *objectToDelete)
	{
		if (objectToDelete != nullptr)
		{
			Scene::DeleteChildrenAndSelf(objectToDelete);
		}
	}

	// Recursive
	void Scene::DeleteChildrenAndSelf(GameObject *objectToDelete)
	{
		long ID = objectToDelete->GetID();

		if (m_primaryCamera != nullptr && m_primaryCamera->GetParentID() == objectToDelete->GetID())
		{
			m_primaryCamera->SetPrimaryCamera(false);
			m_primaryCamera = nullptr;
		}
		
		if (objectToDelete->GetAnimation() != nullptr)
		{
			objectToDelete->GetAnimation()->StopAll();
		}

		for (Component* component : objectToDelete->GetComponents())
		{
			RemoveComponent(component);
		}

		long parentID = objectToDelete->GetParentID();
		if (parentID != -1)
		{
			GetObjectByID(parentID)->RemoveChild(ID);
		}

		if (objectToDelete->HasChildren())
		{
			std::vector<long> childrenIDs = objectToDelete->GetChildren();

			for (int i = 0; i < objectToDelete->GetChildren().size(); i++)
			{				
				GameObject *child = GetObjectByID(childrenIDs[i]);
				if (child != nullptr)
				{
					Scene::DeleteChildrenAndSelf(child);
				}
			}
		}

		m_sceneObjects.erase(ID);
		m_freedGameObjectIDs.push_back(ID);
	}

	void Scene::IncrementGameObjectID()
	{
		m_nextGameObjectID += 1;
	}

	void Scene::SetNextGameObjectID(long nextID)
	{
		m_nextGameObjectID = nextID;
	}

	long Scene::GetNextGameObjectID()
	{
		long ID;

		if (m_freedGameObjectIDs.size() > 0)
		{
			ID = m_freedGameObjectIDs.back();
			m_freedGameObjectIDs.pop_back();
		}
		else
		{
			ID = m_nextGameObjectID;
			IncrementGameObjectID();
		}

		return ID;
	}

	void  Scene::IncrementComponentID()
	{
		m_nextComponentID += 1;
	}

	void Scene::SetNextComponentID(long nextID)
	{
		m_nextComponentID = nextID;
	}

	long  Scene::GetNextComponentID()
	{
		long ID;

		if (m_freedComponentIDs.size() > 0)
		{
			ID = m_freedComponentIDs.back();
			m_freedComponentIDs.pop_back();
		}
		else
		{
			ID = m_nextComponentID;
			IncrementComponentID();
		}

		return ID;
	}

	void Scene::SetPrimaryCamera(Camera* camera)
	{
		if (camera != nullptr)
		{
			// Remove the old m_primaryCamera
			if (m_primaryCamera != nullptr)
			{
				m_primaryCamera->SetPrimaryCamera(false);
			}

			m_primaryCamera = camera;
			m_primaryCamera->SetPrimaryCamera(true);
		}
		else
		{
			LogError("Scene::SetPrimaryCamera() - The Camera pointer you tried to set as the primary Camera was a nullptr.");
		}
	}

	void Scene::RemovePrimaryCamera()
	{
		m_primaryCamera = nullptr;
	}

	Camera *Scene::GetPrimaryCamera()
	{
		return m_primaryCamera;
	}

	std::vector<std::pair<Collider*, Collider*>> Scene::GetColliderPairs()
	{
		return m_ECSManager.GetColliderPairs();
	}

	void Scene::SetPersistantScene(bool b_persistant)
	{
		m_b_persistantScene = b_persistant;
	}

	bool Scene::IsPersistantScene()
	{
		return m_b_persistantScene;
	}

	void Scene::OnPrefabInstantiated()
	{
		m_ECSManager.UpdateColliderPairs();
		FL::UpdateColliderPairs();
	}

	void Scene::UpdateColliderPairs()
	{
		m_ECSManager.UpdateColliderPairs();
		FL::UpdateColliderPairs();
	}

	void Scene::KeepNextComponentIDUpToDate(long ID)
	{
		if (ID >= m_nextComponentID)
		{
			m_nextComponentID = ID + 1;
		}
	}

	Transform* Scene::AddTransform(Transform transform, long ownerID)
	{
		KeepNextComponentIDUpToDate(transform.GetID());
		return m_ECSManager.AddTransform(transform, ownerID);
	}

	Sprite* Scene::AddSprite(Sprite sprite, long ownerID)
	{
		KeepNextComponentIDUpToDate(sprite.GetID());
		return m_ECSManager.AddSprite(sprite, ownerID);
	}

	Camera* Scene::AddCamera(Camera camera, long ownerID)
	{
		KeepNextComponentIDUpToDate(camera.GetID());
		return m_ECSManager.AddCamera(camera, ownerID);
	}

	Script* Scene::AddScript(Script script, long ownerID)
	{
		KeepNextComponentIDUpToDate(script.GetID());
		return m_ECSManager.AddScript(script, ownerID);
	}

	Canvas* Scene::AddCanvas(Canvas canvas, long ownerID)
	{
		KeepNextComponentIDUpToDate(canvas.GetID());
		return m_ECSManager.AddCanvas(canvas, ownerID);
	}

	Audio* Scene::AddAudio(Audio audio, long ownerID)
	{
		KeepNextComponentIDUpToDate(audio.GetID());
		return m_ECSManager.AddAudio(audio, ownerID);
	}

	Text* Scene::AddText(Text text, long ownerID)
	{
		KeepNextComponentIDUpToDate(text.GetID());
		return m_ECSManager.AddText(text, ownerID);
	}

	CompositeCollider* Scene::AddCompositeCollider(CompositeCollider collider, long ownerID)
	{
		KeepNextComponentIDUpToDate(collider.GetID());
		return m_ECSManager.AddCompositeCollider(collider, ownerID);
	}

	BoxCollider* Scene::AddBoxCollider(BoxCollider collider, long ownerID)
	{
		KeepNextComponentIDUpToDate(collider.GetID());
		return m_ECSManager.AddBoxCollider(collider, ownerID);
	}

	CircleCollider* Scene::AddCircleCollider(CircleCollider collider, long ownerID)
	{
		KeepNextComponentIDUpToDate(collider.GetID());
		return m_ECSManager.AddCircleCollider(collider, ownerID);
	}

	Animation* Scene::AddAnimation(Animation animation, long ownerID)
	{
		KeepNextComponentIDUpToDate(animation.GetID());
		return m_ECSManager.AddAnimation(animation, ownerID);
	}

	Button* Scene::AddButton(Button button, long ownerID)
	{
		KeepNextComponentIDUpToDate(button.GetID());
		return m_ECSManager.AddButton(button, ownerID);
	}

	RigidBody* Scene::AddRigidBody(RigidBody rigidBody, long ownerID)
	{
		KeepNextComponentIDUpToDate(rigidBody.GetID());
		return m_ECSManager.AddRigidBody(rigidBody, ownerID);
	}

	CharacterController* Scene::AddCharacterController(CharacterController characterController, long ownerID)
	{
		KeepNextComponentIDUpToDate(characterController.GetID());
		return m_ECSManager.AddCharacterController(characterController, ownerID);
	}

	TileMap* Scene::AddTileMap(TileMap tileMap, long ownerID)
	{
		KeepNextComponentIDUpToDate(tileMap.GetID());
		return m_ECSManager.AddTileMap(tileMap, ownerID);
	}

	void Scene::RemoveComponent(Component* component)
	{
		long ID = component->GetID();
		if (m_ECSManager.RemoveComponent(component))
		{
			m_freedComponentIDs.push_back(ID);
		}
	}

	Transform* Scene::GetTransformByOwner(long ownerID)
	{
		return m_ECSManager.GetTransformByOwner(ownerID);
	}

	Sprite* Scene::GetSpriteByOwner(long ownerID)
	{
		return m_ECSManager.GetSpriteByOwner(ownerID);
	}

	Camera* Scene::GetCameraByOwner(long ownerID)
	{
		return m_ECSManager.GetCameraByOwner(ownerID);
	}

	std::vector<Script*> Scene::GetScriptsByOwner(long ownerID)
	{
		return m_ECSManager.GetScriptsByOwner(ownerID);
	}

	Canvas* Scene::GetCanvasByOwner(long ownerID)
	{
		return m_ECSManager.GetCanvasByOwner(ownerID);
	}

	Audio* Scene::GetAudioByOwner(long ownerID)
	{
		return m_ECSManager.GetAudioByOwner(ownerID);
	}

	Text* Scene::GetTextByOwner(long ownerID)
	{
		return m_ECSManager.GetTextByOwner(ownerID);
	}

	CompositeCollider* Scene::GetCompositeColliderByOwner(long ownerID)
	{
		return m_ECSManager.GetCompositeColliderByOwner(ownerID);
	}

	std::vector<BoxCollider*> Scene::GetBoxCollidersByOwner(long ownerID)
	{
		return m_ECSManager.GetBoxCollidersByOwner(ownerID);
	}

	std::vector<CircleCollider*> Scene::GetCircleCollidersByOwner(long ownerID)
	{
		return m_ECSManager.GetCircleCollidersByOwner(ownerID);
	}

	Animation* Scene::GetAnimationByOwner(long ownerID)
	{
		return m_ECSManager.GetAnimationByOwner(ownerID);
	}

	Button* Scene::GetButtonByOwner(long ownerID)
	{
		return m_ECSManager.GetButtonByOwner(ownerID);
	}

	RigidBody* Scene::GetRigidBodyByOwner(long ownerID)
	{
		return m_ECSManager.GetRigidBodyByOwner(ownerID);
	}

	CharacterController* Scene::GetCharacterControllerByOwner(long ownerID)
	{
		return m_ECSManager.GetCharacterControllerByOwner(ownerID);
	}

	TileMap* Scene::GetTileMapByOwner(long ownerID)
	{
		return m_ECSManager.GetTileMapByOwner(ownerID);
	}

	std::map<long, Transform> &Scene::GetTransforms()
	{
		return m_ECSManager.GetTransforms();
	}
	std::map<long, Sprite>& Scene::GetSprites()
	{
		return m_ECSManager.GetSprites();
	}
	std::map<long, Camera>& Scene::GetCameras()
	{
		return m_ECSManager.GetCameras();
	}
	std::map<long, std::map<long, Script>>& Scene::GetScripts()
	{
		return m_ECSManager.GetScripts();
	}
	std::map<long, std::vector<std::string>> &Scene::GetLuaScriptsByOwner()
	{
		return m_ECSManager.GetLuaScriptsByOwner();
	}
	std::map<long, Button>& Scene::GetButtons()
	{
		return m_ECSManager.GetButtons();
	}
	std::map<long, Canvas>& Scene::GetCanvases()
	{
		return m_ECSManager.GetCanvases();
	}
	std::map<long, Animation>& Scene::GetAnimations()
	{
		return m_ECSManager.GetAnimations();
	}
	std::map<long, Audio>& Scene::GetAudios()
	{
		return m_ECSManager.GetAudios();
	}
	std::map<long, Text>& Scene::GetTexts()
	{
		return m_ECSManager.GetTexts();
	}
	std::vector<Collider*> Scene::GetColliders()
	{
		return m_ECSManager.GetColliders();
	}
	std::map<long, CompositeCollider>& Scene::GetCompositeColliders()
	{
		return m_ECSManager.GetCompositeColliders();
	}
	std::map<long, std::map<long, BoxCollider>>& Scene::GetBoxColliders()
	{
		return m_ECSManager.GetBoxColliders();
	}
	std::map<long, std::map<long, CircleCollider>>& Scene::GetCircleColliders()
	{
		return m_ECSManager.GetCircleColliders();
	}
	std::map<long, RigidBody>& Scene::GetRigidBodies()
	{
		return m_ECSManager.GetRigidBodies();
	}
	std::map<long, CharacterController>& Scene::GetCharacterControllers()
	{
		return m_ECSManager.GetCharacterControllers();
	}
	std::map<long, TileMap>& Scene::GetTileMaps()
	{
		return m_ECSManager.GetTileMaps();
	}
	std::map<long, Mesh>& Scene::GetMeshes()
	{
		return m_ECSManager.GetMeshes();
	}
	std::map<std::string, std::vector<Mesh>>& Scene::GetMeshesByMaterial()
	{
		return m_ECSManager.GetMeshesByMaterial();
	}
}