#pragma once

// TODO(you): World state replication lab session

class ReplicationManagerDeliveryDelegate;

class ReplicationManagerServer
{
public:

	void create(uint32 networkId);
	void update(uint32 networkId);
	void destroy(uint32 networkId);

	void write(OutputMemoryStream &packet, ReplicationManagerDeliveryDelegate* deliveryDelegate);


	std::unordered_map<uint32, ReplicationCommand> replicationCommands;
};

class ReplicationManagerDeliveryDelegate : public DeliveryDelegate
{
public: 
	ReplicationManagerDeliveryDelegate(ReplicationManagerServer* repManager_s);
	~ReplicationManagerDeliveryDelegate();

	void onDeliverySuccess() override;
	void onDeliveryFailure() override;

	void AddCommand(const ReplicationCommand& replicationCommand);

private:
	ReplicationManagerServer* RepManager_s;
	std::vector<ReplicationCommand> replicationCommands;
};