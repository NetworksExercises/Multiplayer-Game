#pragma once

// TODO(you): Reliability on top of UDP lab session

class DeliveryManager;

class DeliveryDelegate 
{
public:
	virtual void onDeliverySuccess(DeliveryManager* deliveryManager) = 0;
	virtual void onDeliveryFailure(DeliveryManager* deliveryManager) = 0;
};

struct Delivery
{
	~Delivery();
	uint32 sequenceNumber = 0;
	double dispatchTime = 0.0;
	DeliveryDelegate* delegate = nullptr;
};

class DeliveryManager
{
public:

	~DeliveryManager();

	// For senders to write a new sequence number into a packet
	Delivery* writeSequenceNumber(OutputMemoryStream& packet);

	// For receivers to process the sequence number from an incoming packet
	bool processSequenceNumber(const InputMemoryStream& packet);

	// For receivers to write ack'ed seq. numbers into a packet
	bool hasSequenceNumbersPendingAck() const;
	void writeSequenceNumbersPendingAck(OutputMemoryStream& packet);

	// For senders to process ack'ed seq. numbers from a packet
	void processAckdSequenceNumbers(const InputMemoryStream& packet);
	void processTimeOutPackets();

	void clear();

private:

	// Private members (sender side)

	// - The next outgoing sequence number
	uint32 outgoing_seqNumber = 0;
	// - A list of pending deliveries
	std::vector<Delivery*> pending_Deliveries;

	// Private members (receiver side)

	// - The next expected sequence number
	uint32 expected_seqNumber = 0;
	// - A list of sequence numbers pending ack
	std::vector<uint32> pending_ACK;

};