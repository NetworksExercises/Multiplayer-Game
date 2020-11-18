#include "Networks.h"
#include "ReplicationManagerServer.h"

// TODO(you): World state replication lab session

void ReplicationManagerServer::create(uint32 networkId)
{
	netId = networkId;
}

void ReplicationManagerServer::update(uint32 networkId)
{
}

void ReplicationManagerServer::destroy(uint32 networkId)
{
}

void ReplicationManagerServer::write(OutputMemoryStream& packet, ReplicationCommand command)
{
	packet.Write(command.networkId);
	packet.Write(command.action);

	switch (command.action)
	{
	case ReplicationAction::None:
		break;
	case ReplicationAction::Create:
		{
		GameObject* go = App->modLinkingContext->getNetworkGameObject(netId);

		// Serialize go fields
		}
		break;
	case ReplicationAction::Update:
		{
		GameObject* go = App->modLinkingContext->getNetworkGameObject(netId);

		// Serialize go fields
		}
		break;
	case ReplicationAction::Destroy:
		break;
	default:
		break;
	}
}
