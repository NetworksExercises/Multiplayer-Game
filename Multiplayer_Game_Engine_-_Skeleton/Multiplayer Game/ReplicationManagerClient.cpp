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
			GameObject* go = App->modLinkingContext->getNetworkGameObject(network_Id);

			if (go == nullptr)
			{
				go = App->modGameObject->Instantiate();
				//GameObject* go = App->modGameObject->Instantiate();
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

						else if (texture == "explosion1.png")
						{
							go->sprite->texture = App->modResources->explosion1;
							go->animation = App->modRender->addAnimation(go);
							go->animation->clip = App->modResources->explosionClip;
							App->modSound->playAudioClip(App->modResources->audioClipExplosion);
						}

					}
				}

				packet.Read(go->sprite->order);

				// Collider

				ColliderType type = ColliderType::None;
				packet.Read(type);
				if (go->collider == nullptr && type != ColliderType::None)
					go->collider = App->modCollision->addCollider(type, go);
				if (go->collider != nullptr)
					packet.Read(go->collider->isTrigger);

				if (type == ColliderType::None)
				{
					ASSERT(go->sprite->texture != nullptr);

					if (go->sprite->texture->filename == "laser.png")
						type = ColliderType::Laser;
					else if (go->sprite->texture->filename == "spacecraft1.png"
						|| go->sprite->texture->filename == "spacecraft2.png"
						|| go->sprite->texture->filename == "spacecraft3.png")
						type = ColliderType::Player;
				}

				// Behaviour

				if (go->behaviour == nullptr)
				{
					switch (type)
					{
					case ColliderType::Player:
					{
						go->behaviour = App->modBehaviour->addSpaceship(go);;
						break;
					}
					case ColliderType::Laser:
					{
						go->behaviour = App->modBehaviour->addLaser(go);
						break;
					}
					default:
					{
						break;
					}
					}
				}

				packet.Read(go->tag);
			}

			}
			break;
		case ReplicationAction::Update:
			{
			GameObject* go = App->modLinkingContext->getNetworkGameObject(network_Id);

			// Deserialize go fields
			packet.Read(go->position.x);
			packet.Read(go->position.y);
			packet.Read(go->size.x);
			packet.Read(go->size.y);
			packet.Read(go->angle);	

		
			go->behaviour->read(packet);

			}
			break;
		case ReplicationAction::Destroy:
			{
			GameObject* go = App->modLinkingContext->getNetworkGameObject(network_Id);

			ASSERT(go != nullptr);
			App->modLinkingContext->unregisterNetworkGameObject(go);
			App->modGameObject->Destroy(go);
			
			//if (go->sprite)
			//	App->modRender->removeSprite(go);

			//if (go->animation)
			//	App->modRender->removeAnimation(go);		

			//if (go->collider)
			//	App->modCollision->removeCollider(go->collider);
					
			}
			break;
		default:
			break;
		}
	}
}
