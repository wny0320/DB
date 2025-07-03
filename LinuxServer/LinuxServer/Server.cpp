#include <algorithm>
#include <unistd.h> // For close
#include <errno.h>  // For errno
#include <cstdlib>  // For getenv

#include "Server.h"
#include "Packet.h"

// MySQL Connector/C++
#include <mysql_connection.h>
#include <mysql_driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>

Server::Server()
{
	Driver = get_driver_instance();
	Connection = nullptr;
	InitializeDatabase();
}

Server::~Server()
{
	for (auto const& pair : RecvPacketMap)
	{
		delete pair.second;
	}
	RecvPacketMap.clear();

	for (int s : SocketVector)
	{
		close(s);
	}
	SocketVector.clear();

	if (Connection)
	{
		delete Connection;
	}
}

bool Server::InitializeDatabase()
{
	const char* dbHost = getenv("DB_HOST");
	const char* dbPort = getenv("DB_PORT");
	const char* dbUser = getenv("DB_USER");
	const char* dbPass = getenv("DB_PASS");
	const char* dbName = getenv("DB_NAME"); // Assuming DB_NAME is also set

	if (!dbHost || !dbPort || !dbUser || !dbPass || !dbName)
	{
		std::cerr << "Error: Database environment variables not set!" << std::endl;
		return false;
	}

	std::string dbUrl = std::string("tcp://") + dbHost + ":" + dbPort;

	try
	{
		Connection = Driver->connect(dbUrl, dbUser, dbPass);
		Connection->setSchema(dbName);
		std::cout << "Successfully connected to RDS database!" << std::endl;
		return true;
	}
	catch (sql::SQLException &e)
	{
		std::cerr << "Error connecting to database: " << e.what() << std::endl;
		return false;
	}
}

bool Server::GetPlayerDataFromDB(int InPlayerId, PlayerData& OutPlayerData)
{
	if (!Connection || Connection->isClosed())
	{
		std::cerr << "Database connection is not open!" << std::endl;
		return false;
	}

	try
	{
		sql::PreparedStatement* pstmt;
		sql::ResultSet* res;

		pstmt = Connection->prepareStatement("SELECT PlayerID, Username, PlayerHp, PlayerStamina FROM Players WHERE PlayerID = ?");
		pstmt->setInt(1, InPlayerId);
		res = pstmt->executeQuery();

		if (res->next())
		{
			OutPlayerData.PlayerId = res->getInt("PlayerID");
			OutPlayerData.Username = res->getString("Username");
			OutPlayerData.PlayerHp = res->getUInt("PlayerHp");
			OutPlayerData.PlayerStamina = res->getUInt("PlayerStamina");
			delete res;
			delete pstmt;
			return true;
		}
		delete res;
		delete pstmt;
		return false; // Player not found
	}
	catch (sql::SQLException &e)
	{
		std::cerr << "Error querying player data: " << e.what() << std::endl;
		return false;
	}
}

void Server::SendPacket(int InSocket, Packet* InPacket)
{
	if (InPacket->Header->IsSerialized() == false)
	{
		InPacket->Header->Serialize();
	}
	if (InPacket->Body->IsSerialized() == false)
	{
		InPacket->Body->Serialize();
	}
	//Send Header
	if (send(InSocket, (char*)InPacket->Header, sizeof(PacketHeader), 0) < 0)
	{
		RemoveSocket(InSocket);
		return;
	}
	//Send Body
	if (send(InSocket, (char*)InPacket->Body, InPacket->Body->GetPacketBodySize(), 0) < 0)
	{
		RemoveSocket(InSocket);
		return;
	}
}

bool Server::RecvPacket(int InSocket)
{
	PacketHeader* NewHeader = new PacketHeader();
	int RecvBytes = recv(InSocket, (char*)NewHeader, sizeof(PacketHeader), 0);
	if (RecvBytes <= 0)
	{
		RemoveSocket(InSocket);
		delete NewHeader;
		return false;
	}
	NewHeader->Deserialize();
	PacketBodyBase* NewBody = nullptr;
	switch ((EEventCode)(NewHeader->PacketCode))
	{
	case EEventCode::CodeError:
		break;
	case EEventCode::GetSessionData:
		break;
	case EEventCode::GetPlayerData:
		NewBody = new PlayerData();
		// Attempt to get data from DB
		{
			PlayerData TempPlayerData;
			// Assuming the incoming packet body for GetPlayerData contains the PlayerId
			// For now, we'll just use a dummy ID or assume it's in the header for testing.
			// A proper implementation would deserialize the incoming packet body to get the ID.
			// For demonstration, let's assume PlayerId is 1 for now.
			int RequestedPlayerId = 1; // Placeholder, should come from incoming packet

			if (GetPlayerDataFromDB(RequestedPlayerId, TempPlayerData))
			{
				// If data found, populate NewBody with TempPlayerData
				// This part needs careful handling as NewBody is PacketBodyBase*
				// and TempPlayerData is PlayerData.
				// In a real scenario, you'd create a new PlayerData packet with the retrieved data.
				PlayerData* RetrievedData = new PlayerData();
				RetrievedData->PlayerId = TempPlayerData.PlayerId;
				RetrievedData->Username = TempPlayerData.Username;
				RetrievedData->PlayerHp = TempPlayerData.PlayerHp;
				RetrievedData->PlayerStamina = TempPlayerData.PlayerStamina;
				NewBody = RetrievedData;
			}
			else
			{
				// Player not found or DB error, handle accordingly
				delete NewBody; // Delete the initially created PlayerData()
				NewBody = nullptr;
			}
		}
		break;
	case EEventCode::Max:
		break;
	default:
		break;
	}
	
	//If Body Null, Recv Packet And Remove
	if (NewBody == nullptr)
	{
		if (NewHeader->PacketBodyLength > 0)
		{
			char* TempBuffer = new char[NewHeader->PacketBodyLength];
			int BytesRead = recv(InSocket, TempBuffer, NewHeader->PacketBodyLength, 0);
			if (BytesRead <= 0) {
				RemoveSocket(InSocket);
				delete NewHeader;
				delete[] TempBuffer;
				return false;
			}
			delete[] TempBuffer;
		}
		delete NewHeader;
		return true; // Still a valid connection, just an unhandled packet
	}

	int RecvBytesBody = recv(InSocket, (char*)NewBody, NewHeader->PacketBodyLength, 0);
	//Recv Failed
	if (RecvBytesBody <= 0)
	{
		RemoveSocket(InSocket);
		delete NewHeader;
		delete NewBody;
		return false;
	}

	NewBody->Deserialize();
	Packet* NewPacket = new Packet(NewHeader, NewBody);
	//Emplace Avoid Copy
	RecvPacketMap.emplace(InSocket, NewPacket);
	return true;
}

void Server::AddSocket(int InSocket)
{
	SocketVector.push_back(InSocket);
}

void Server::RemoveSocket(int InSocket)
{
	auto It = std::find(SocketVector.begin(), SocketVector.end(), InSocket);
	if (It != SocketVector.end())
	{
		SocketVector.erase(It);
	}

	auto MapIt = RecvPacketMap.find(InSocket);
	if (MapIt != RecvPacketMap.end())
	{
		delete MapIt->second;
		RecvPacketMap.erase(MapIt);
	}

	close(InSocket);
}