#include "Networks.h"
#include "ReplicationManagerServer.h"

// TODO(you): World state replication lab session

void ReplicationManagerServer::create(uint32 networkId)
{
	OutputMemoryStream packet;
	ReplicationCommand cmd(ReplicationAction::Create, networkId);

	// --- Fill packet ---
	write(packet, cmd);

	// --- Send packet ---


}

void ReplicationManagerServer::update(uint32 networkId)
{
	OutputMemoryStream packet;
	ReplicationCommand cmd(ReplicationAction::Update, networkId);

	// --- Fill packet ---
	write(packet, cmd);

	// --- Send packet ---


}

void ReplicationManagerServer::destroy(uint32 networkId)
{
	OutputMemoryStream packet;
	ReplicationCommand cmd(ReplicationAction::Destroy, networkId);
	
	// --- Fill packet ---
	write(packet, cmd);

	// --- Send packet ---


}

void ReplicationManagerServer::write(OutputMemoryStream& packet, ReplicationCommand command)
{	
	if (command.action != ReplicationAction::Destroy)
	{
		packet.Write(command.networkId);
		packet.Write(command.action);

		GameObject* go = App->modLinkingContext->getNetworkGameObject(command.networkId);

		// Serialize go fields

	}
}
