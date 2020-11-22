#include "Networks.h"
#include "ReplicationManagerServer.h"

// TODO(you): World state replication lab session

void ReplicationManagerServer::create(uint32 networkId)
{
	//ReplicationCommand cmd(ReplicationAction::Create, networkId);

	// --- Fill packet ---
	//write(packet, cmd);
	replicationCommands[networkId].networkId = networkId;
	replicationCommands[networkId].action = ReplicationAction::Create;
}

void ReplicationManagerServer::update(uint32 networkId)
{
	//ReplicationCommand cmd(ReplicationAction::Update, networkId);

	// --- Fill packet ---
	//write(packet, cmd);
	replicationCommands[networkId].networkId = networkId;
	replicationCommands[networkId].action = ReplicationAction::Update;
}

void ReplicationManagerServer::destroy(uint32 networkId)
{
	//ReplicationCommand cmd(ReplicationAction::Destroy, networkId);
	
	replicationCommands[networkId].networkId = networkId;
	replicationCommands[networkId].action = ReplicationAction::Destroy;
	// --- Fill packet ---
	//write(packet, cmd);
}

void ReplicationManagerServer::write(OutputMemoryStream& packet)
{	
	for (std::unordered_map<uint32, ReplicationCommand>::const_reference& replicationCommand : replicationCommands)
	{
		if (replicationCommand.second.action != ReplicationAction::Destroy)
		{
			packet.Write(replicationCommand.second.networkId);
			packet.Write(replicationCommand.second.action);

			GameObject* go = App->modLinkingContext->getNetworkGameObject(replicationCommand.second.networkId);

			// Serialize go fields
			packet.Write(go->id);
			packet.Write(go->position.x);
			packet.Write(go->position.y);
			packet.Write(go->size.x);
			packet.Write(go->size.y);
			packet.Write(go->angle);

		}
	}

	replicationCommands.clear();
}
