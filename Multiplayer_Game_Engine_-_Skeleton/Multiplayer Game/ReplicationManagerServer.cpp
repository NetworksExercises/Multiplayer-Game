#include "Networks.h"
#include "ReplicationManagerServer.h"

// TODO(you): World state replication lab session

void ReplicationManagerServer::create(uint32 networkId)
{
	//ReplicationCommand cmd(ReplicationAction::Create, networkId);

	// --- Fill packet ---
	//write(packet, cmd);
	replicationCommands[networkId].action = ReplicationAction::Create;
}

void ReplicationManagerServer::update(uint32 networkId)
{
	//ReplicationCommand cmd(ReplicationAction::Update, networkId);

	// --- Fill packet ---
	//write(packet, cmd);
	if (replicationCommands[networkId].action != ReplicationAction::Create
		&& replicationCommands[networkId].action != ReplicationAction::Destroy)
	{
		replicationCommands[networkId].action = ReplicationAction::Update;
	}
}

void ReplicationManagerServer::destroy(uint32 networkId)
{
	//ReplicationCommand cmd(ReplicationAction::Destroy, networkId);
	
	replicationCommands[networkId].action = ReplicationAction::Destroy;
	// --- Fill packet ---
	//write(packet, cmd);
}

void ReplicationManagerServer::write(OutputMemoryStream& packet)
{	
	for (std::unordered_map<uint32, ReplicationCommand>::const_reference& replicationCommand : replicationCommands)
	{
		packet.Write(replicationCommand.first); // net id
		packet.Write(replicationCommand.second.action);

		if (replicationCommand.second.action == ReplicationAction::Create)
		{

			GameObject* go = App->modLinkingContext->getNetworkGameObject(replicationCommand.first);

			// Serialize go fields
			//packet.Write(go->id);
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

			
			packet.Write(go->tag);
			//replicationCommands.erase(replicationCommand.first);

			//break;
		}
		else if (replicationCommand.second.action == ReplicationAction::Update)
		{
			GameObject* go = App->modLinkingContext->getNetworkGameObject(replicationCommand.first);

			// Serialize go fields
			packet.Write(go->position.x);
			packet.Write(go->position.y);
			packet.Write(go->size.x);
			packet.Write(go->size.y);
			packet.Write(go->angle);

			if(go->behaviour)
				go->behaviour->write(packet);

		}

		replicationCommands.erase(replicationCommand.first);
		break;
		// TODO: do we need the for?
	}

}
