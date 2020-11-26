#include "Networks.h"
#include "DeliveryManager.h"

// TODO(you): Reliability on top of UDP lab session

DeliveryManager::~DeliveryManager()
{
	clear();
}

Delivery* DeliveryManager::writeSequenceNumber(OutputMemoryStream& packet)
{
	Delivery* delivery = new Delivery;
	delivery->sequenceNumber = outgoing_seqNumber++;
	delivery->dispatchTime = Time.time;
	pending_Deliveries.push_back(delivery);

	packet.Write(delivery->sequenceNumber);

	return delivery;
}

bool DeliveryManager::processSequenceNumber(const InputMemoryStream& packet)
{
	uint32 received_seqNumber; 
	packet.Read(received_seqNumber);

	if (received_seqNumber >= expected_seqNumber)
	{
		pending_ACK.push_back(received_seqNumber);
		expected_seqNumber = received_seqNumber + 1;
		return true;
	}

	return false;
}

bool DeliveryManager::hasSequenceNumbersPendingAck() const
{
	return !pending_ACK.empty();
}

void DeliveryManager::writeSequenceNumbersPendingAck(OutputMemoryStream& packet)
{
	if (hasSequenceNumbersPendingAck())
	{
		packet.Write(pending_ACK.size());
		packet.Write(pending_ACK.front());
		pending_ACK.clear();
	}
}

void DeliveryManager::processAckdSequenceNumbers(const InputMemoryStream& packet)
{
	uint32 seqNumber;
	uint32 size;
	packet.Read(size);
	packet.Read(seqNumber);

	while (seqNumber < seqNumber + size 
		&& !pending_Deliveries.empty())
	{
		Delivery* delivery = pending_Deliveries.front();

		if (delivery->sequenceNumber == seqNumber)
		{
			delivery->delegate->onDeliverySuccess(this);
			pending_Deliveries.erase(pending_Deliveries.begin());
			seqNumber++;
			delete delivery;
		}
		else if (delivery->sequenceNumber < seqNumber)
		{
			pending_Deliveries.erase(pending_Deliveries.begin());
			delivery->delegate->onDeliveryFailure(this);
			delete delivery;
		}
		else
		{
			seqNumber++;
		}
	}
	
}

void DeliveryManager::processTimeOutPackets()
{
	while (!pending_Deliveries.empty())
	{
		Delivery* delivery = pending_Deliveries.front();

		if (Time.time - delivery->dispatchTime >= ACK_INTERVAL_SECONDS)
		{
			delivery->delegate->onDeliveryFailure(this);		
			pending_Deliveries.erase(pending_Deliveries.begin());
			delete delivery;
		}
		else
		{
			break;
		}
	}
}

void DeliveryManager::clear()
{
	while (!pending_Deliveries.empty())
	{
		Delivery* delivery = pending_Deliveries.front();
		pending_Deliveries.erase(pending_Deliveries.begin());
		delete delivery;
	}

	pending_Deliveries.clear();
	pending_ACK.clear();

	outgoing_seqNumber = 0;
	expected_seqNumber = 0;
}

Delivery::~Delivery()
{
	delete delegate;
}
