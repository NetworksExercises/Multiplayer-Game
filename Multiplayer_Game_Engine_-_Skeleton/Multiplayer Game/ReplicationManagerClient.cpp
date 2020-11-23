#include "Networks.h"
#include "ReplicationManagerClient.h"

// TODO(you): World state replication lab session

void ReplicationManagerClient::read(const InputMemoryStream& packet)
{
	if (packet.GetSize() > 0)
	{
		uint32 network_Id;
		ReplicationAction repAction;

		packet.Read(network_Id);
		packet.Read(repAction);

		switch (repAction)
		{
		case ReplicationAction::None:
			break;
		case ReplicationAction::Create:
			{
			GameObject* go = App->modGameObject->Instantiate();
			App->modLinkingContext->registerNetworkGameObjectWithNetworkId(go, network_Id);

			// Deserialize go fields
			//packet.Read(go->id);
			packet.Read(go->position.x);
			packet.Read(go->position.y);
			packet.Read(go->size.x);
			packet.Read(go->size.y);
			packet.Read(go->angle);

			std::string texture;
			packet.Read(texture);

			// Sprite
			if (go->sprite == nullptr) 
			{
				go->sprite = App->modRender->addSprite(go);

				if (go->sprite)
				{

					if (texture == "space_background.jpg")
						go->sprite->texture = App->modResources->space;

					else if (texture == "asteroid1.png")
						go->sprite->texture = App->modResources->asteroid1;

					else if (texture == "asteroid2.png")
						go->sprite->texture = App->modResources->asteroid2;

					else if (texture == "spacecraft1.png")
						go->sprite->texture = App->modResources->spacecraft1;

					else if (texture == "spacecraft2.png")
						go->sprite->texture = App->modResources->spacecraft2;

					else if (texture == "spacecraft3.png")
						go->sprite->texture = App->modResources->spacecraft3;

					else if (texture == "laser.png")
						go->sprite->texture = App->modResources->laser;

				}
			}

			packet.Read(go->sprite->order);

			// Collider

			ColliderType type = ColliderType::None;
			packet.Read(type);
			if (go->collider == nullptr) 
				go->collider = App->modCollision->addCollider(type, go);
			if (go->collider != nullptr)
				packet.Read(go->collider->isTrigger);

			// Behaviour

			if (go->behaviour == nullptr) 
			{
				switch (type) 
				{
					case ColliderType::Player: 
					{
						go->behaviour = new Spaceship;		
						break;
					}
					case ColliderType::Laser: 
					{
						go->behaviour = new Laser;
						break;
					}
					default: 
					{
						break;
					}
				}

				if (go->behaviour != nullptr)
				{
					go->behaviour->gameObject = go;
				}
			}

			packet.Read(go->tag);
			}
			break;
		case ReplicationAction::Update:
			{
			GameObject* go = App->modLinkingContext->getNetworkGameObject(network_Id);

			// Deserialize go fields
			//packet.Read(go->id);
			packet.Read(go->position.x);
			packet.Read(go->position.y);
			packet.Read(go->size.x);
			packet.Read(go->size.y);
			packet.Read(go->angle);

			}
			break;
		case ReplicationAction::Destroy:
			{
			GameObject* go = App->modLinkingContext->getNetworkGameObject(network_Id);
			App->modLinkingContext->unregisterNetworkGameObject(go);
			App->modGameObject->Destroy(go);
			}
			break;
		default:
			break;
		}
	}
}
