#pragma once
#include "Component.h"
#include "TagList.h"
#include "Vector2.h"

#include <string>
#include <vector>
#include <memory>


namespace FlatEngine
{
	class Button;
	class Canvas;
	class Script;
	class Animation;
	class Sprite;
	class Camera;
	class Audio;
	class Transform;
	class Text;	
	class CharacterController;
	class RigidBody;
	class BoxCollider;
	class CircleCollider;
	class CompositeCollider;
	class TileMap;

	class GameObject
	{
	public:
		
		GameObject(long parentID = -1, long myID = -1);
		~GameObject();		

		void SetIsPrefab(bool b_isPrefab);
		bool IsPrefab();
		void SetPrefabName(std::string newPrefabName);
		std::string GetPrefabName();
		void SetPrefabSpawnLocation(Vector2 newSpawnLocation);
		Vector2 GetPrefabSpawnLocation();
		void SetID(long ID);
		long GetID();
		void SetName(std::string name);
		std::string GetName();
		TagList &GetTagList();
		void SetTagList(TagList tagList);
		bool HasTag(std::string tagName);
		void SetTag(std::string tagName, bool b_hasTag);
		void SetIgnore(std::string tagName, bool b_hasTag);
		bool HasScript(std::string scriptName);
		Script* GetScript(std::string scriptName);

		// Components
		void RemoveComponent(Component* component);
		void DeleteComponents();
		Transform* AddTransform(long ID = -1, bool b_active = true, bool b_collapsed = false);
		Sprite* AddSprite(long ID = -1, bool b_active = true, bool b_collapsed = false);
		Camera* AddCamera(long ID = -1, bool b_active = true, bool b_collapsed = false);
		Script* AddScript(long ID = -1, bool b_active = true, bool b_collapsed = false);
		Button* AddButton(long ID = -1, bool b_active = true, bool b_collapsed = false);
		Canvas* AddCanvas(long ID = -1, bool b_active = true, bool b_collapsed = false);
		Animation* AddAnimation(long ID = -1, bool b_active = true, bool b_collapsed = false);
		Audio* AddAudio(long ID = -1, bool b_active = true, bool b_collapsed = false);
		Text* AddText(long ID = -1, bool b_active = true, bool b_collapsed = false);
		BoxCollider* AddBoxCollider(long ID = -1, bool _active = true, bool b_collapsed = false);
		CircleCollider* AddCircleCollider(long ID = -1, bool b_active = true, bool b_collapsed = false);
		CompositeCollider* AddCompositeCollider(long ID = -1, bool b_active = true, bool b_collapsed = false);
		RigidBody* AddRigidBody(long ID = -1, bool b_active = true, bool b_collapsed = false);
		CharacterController* AddCharacterController(long ID = -1, bool b_active = true, bool b_collapsed = false);
		TileMap* AddTileMap(long ID = -1, bool b_active = true, bool b_collapsed = false);

		Sprite* AddSpriteLua() { return AddSprite(-1, true, false); };
		Camera* AddCameraLua() { return AddCamera(-1, true, false); };
		Script* AddScriptLua() { return AddScript(-1, true, false); };
		Button* AddButtonLua() { return AddButton(-1, true, false); };
		Canvas* AddCanvasLua() { return AddCanvas(-1, true, false); };
		Animation* AddAnimationLua() { return AddAnimation(-1, true, false); };
		Audio* AddAudioLua() { return AddAudio(-1, true, false); };
		Text* AddTextLua() { return AddText(-1, true, false); };
		BoxCollider* AddBoxColliderLua() { return AddBoxCollider(-1, true, false); };
		CircleCollider* AddCircleColliderLua() { return AddCircleCollider(-1, true, false); };
		CompositeCollider* AddCompositeColliderLua() { return AddCompositeCollider(-1, true, false); };
		RigidBody* AddRigidBodyLua() { return AddRigidBody(-1, true, false); };
		CharacterController* AddCharacterControllerLua() { return AddCharacterController(-1, true, false); };
		TileMap* AddTileMapLua() { return AddTileMap(-1, true, false); };

		Component* GetComponent(ComponentTypes type);
		bool HasComponent(ComponentTypes type);
		bool HasComponent(std::string type);
		Transform* GetTransform();
		Sprite* GetSprite();
		Camera* GetCamera();
		Animation* GetAnimation();
		Audio* GetAudio();
		Button* GetButton();
		Canvas* GetCanvas();
		std::vector<Script*> GetScripts();
		Text* GetText();
		CharacterController* GetCharacterController();
		RigidBody* GetRigidBody();
		std::vector<BoxCollider*> GetBoxColliders();	
		BoxCollider* GetBoxCollider();
		std::vector<CircleCollider*> GetCircleColliders();
		CompositeCollider* GetCompositeCollider();	
		TileMap* GetTileMap();
		std::vector<Component*> GetComponents();		

		void SetParentID(long parentID);
		long GetParentID();
		GameObject *GetParent();
		void AddChild(long childID);
		void RemoveChild(long childID);
		GameObject *GetFirstChild();
		GameObject *FindChildByName(std::string name);
		std::vector<long> GetChildren();
		bool HasChildren();
		void SetActive(bool b_active);
		bool IsActive();
		void SetPersistant(bool b_persistant);
		bool IsPersistant();

	private:
		std::string m_name;
		bool m_b_isPrefab;
		std::string m_prefabName;
		Vector2 m_prefabSpawnLocation;
		TagList m_tagList;
		long m_ID;
		long m_parentID;
		bool m_b_isActive;
		std::vector<Component*> m_components;
		std::vector<long> m_childrenIDs;
		bool m_b_persistant;
	};
}