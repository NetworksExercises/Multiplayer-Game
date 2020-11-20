#pragma once

// TODO(you): World state replication lab session

enum class ReplicationAction
{
	None,
	Create,
	Update,
	Destroy
};

struct ReplicationCommand
{
	ReplicationCommand() {};
	ReplicationCommand(ReplicationAction _action, uint32 _networkId) : action(_action), networkId(_networkId) {}

	ReplicationAction action;
	uint32 networkId;
};