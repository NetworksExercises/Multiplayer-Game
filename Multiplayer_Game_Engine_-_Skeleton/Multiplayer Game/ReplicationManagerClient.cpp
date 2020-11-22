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
			packet.Read(go->id);
			packet.Read(go->position.x);
			packet.Read(go->position.y);
			packet.Read(go->size.x);
			packet.Read(go->size.y);
			packet.Read(go->angle);

			}
			break;
		case ReplicationAction::Update:
			{
			GameObject* go = App->modLinkingContext->getNetworkGameObject(network_Id);

			// Deserialize go fields
			packet.Read(go->id);
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
