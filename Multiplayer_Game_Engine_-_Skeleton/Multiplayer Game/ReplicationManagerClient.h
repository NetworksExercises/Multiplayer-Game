#pragma once

// TODO(you): World state replication lab session

class ReplicationManagerClient
{
public:
	void read(const InputMemoryStream& packet);

	void ReadCreateAndUpdateObject(const InputMemoryStream& packet, GameObject* go);
};