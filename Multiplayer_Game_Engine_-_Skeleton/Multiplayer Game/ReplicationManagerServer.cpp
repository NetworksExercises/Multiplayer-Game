#include "Networks.h"
#include "ReplicationManagerServer.h"

// TODO(you): World state replication lab session

void ReplicationManagerServer::create(uint32 networkId)
{
	replicationCommands[networkId].action = ReplicationAction::Create;
	replicationCommands[networkId].networkId = networkId;
}

void ReplicationManagerServer::update(uint32 networkId)
{
	if (replicationCommands[networkId].action != ReplicationAction::Create
		&& replicationCommands[networkId].action != ReplicationAction::Destroy)
	{
		replicationCommands[networkId].action = ReplicationAction::Update;
		replicationCommands[networkId].networkId = networkId;
	}
}

void ReplicationManagerServer::destroy(uint32 networkId)
{
	replicationCommands[networkId].action = ReplicationAction::Destroy;
	replicationCommands[networkId].networkId = networkId;
}

void ReplicationManagerServer::write(OutputMemoryStream& packet, ReplicationManagerDeliveryDelegate* deliveryDelegate)
{	
	for (std::unordered_map<uint32, ReplicationCommand>::reference& replicationCommand : replicationCommands)
	{
		packet.Write(replicationCommand.first); // net id
		packet.Write(replicationCommand.second.action);

		if (replicationCommand.second.action == ReplicationAction::Create
			|| replicationCommand.second.action == ReplicationAction::Update)
		{

			GameObject* go = App->modLinkingContext->getNetworkGameObject(replicationCommand.first);

			// Serialize go fields
			packet.Write(go->position.x);
			packet.Write(go->position.y);
			packet.Write(go->size.x);
			packet.Write(go->size.y);
			packet.Write(go->angle);

			std::string texture = "";

			// sprite 
			if (go->sprite)
			{
				texture = go->sprite->texture != nullptr ? go->sprite->texture->filename : "";
				packet.Write(texture);
				packet.Write(go->sprite->order);
			}
			else
			{
				packet.Write(texture);
				packet.Write(0);
			}

			// Collider 
			ColliderType colliderType = go->collider != nullptr ? go->collider->type : ColliderType::None;
			packet.Write(colliderType);

			if (go->collider)
				packet.Write(go->collider->isTrigger);

			if (go->behaviour)
				go->behaviour->write(packet);
			
			packet.Write(go->tag);
			packet.Write(go->kills);
		}

		deliveryDelegate->AddCommand(replicationCommand.second);
	}

	replicationCommands.clear();

}

ReplicationManagerDeliveryDelegate::ReplicationManagerDeliveryDelegate(ReplicationManagerServer* repManager_s)
{
	this->RepManager_s = repManager_s;
}

ReplicationManagerDeliveryDelegate::~ReplicationManagerDeliveryDelegate()
{
}

void ReplicationManagerDeliveryDelegate::onDeliverySuccess()
{
}

void ReplicationManagerDeliveryDelegate::onDeliveryFailure()
{
	for (const ReplicationCommand& replicationCommand : replicationCommands) 
	{
		switch (replicationCommand.action)
		{
		case ReplicationAction::Create:
		{
			if (App->modLinkingContext->getNetworkGameObject(replicationCommand.networkId) != nullptr)
			{
				RepManager_s->create(replicationCommand.networkId);
			}
			break;
		}

		case ReplicationAction::Update:
		{
			if (App->modLinkingContext->getNetworkGameObject(replicationCommand.networkId) != nullptr)
			{
				RepManager_s->update(replicationCommand.networkId);
			}
			break;
		}

		case ReplicationAction::Destroy:
		{
			if (App->modLinkingContext->getNetworkGameObject(replicationCommand.networkId) == nullptr)
			{
				RepManager_s->destroy(replicationCommand.networkId);
			}
			break;
		}

		default:
		{
			break;
		}
		}
	}
}

void ReplicationManagerDeliveryDelegate::AddCommand(const ReplicationCommand& replicationCommand)
{
	replicationCommands.push_back(replicationCommand);
}
