# Network

System for handling multiplayer networking and platform services.

---
### OpenSession
Open a multiplayer session.

Sig: `Network.OpenSession(options)`
 - Arg: `table options`
   - `string name` Name to identify the session/lobby
   - `boolean lan` Whether session should only be accessible on local network
   - `boolean private` If private, only friends can join
   - `integer port` Port to use (may not be respected if making online session)
   - `integer maxPlayers` Max players for session (including this server host!)
---
### CloseSession
Close a multiplayer session if hosting, or disconnect from a remote game session.

Sig: `Network.CloseSession()`

---
### JoinSession
Join a multiplayer session.

Sig: `Network.JoinSession(data)`
 - Arg: `table data`
   - `string ipAddress` Server IP address (use for LAN)
   - `integer port` Server port (use for LAN)
   - `string lobbyId` Online platform Lobby ID (use for Steam)
---
### BeginSessionSearch
Begin searching for network sessions. Use Network.GetNumSessions(), Network.GetSession(), and/or Network.GetSessions() to check for discovered sessions.

Sig: `Network.BeginSessionSearch()`

---
### EndSessionSearch
Stop searching for network sessions.

Sig: `Network.EndSessionSearch()`

---
### IsSearching
Check if the engine is searching for network sessions.

Sig: `searching = Network.IsSearching()`
 - Ret: `boolean searching` Searching for sessions
---
### EnableSessionBroadcast
Set whether to broadcast this active session on the network. This is only used by LAN sessions. Steam sessions do not need this enabled to be discovered.

Sig: `Network.EnableSessionBroadcast(enable)`
 - Arg: `boolean enable` Enable broadcast
---
### IsSessionBroadcastEnabled
Check if the active session info is being broadcast over the network. Only used when a LAN session is active.

Sig: `enabled = Network.IsSessionBroadcastEnabled()`
 - Ret: `boolean enabled` Broadcast enabled
---
### GetNumSessions
Get the number of found network sessions. Network sessions will only be discovered once Network.BeginSessionSearch() has been invoked.

Sig: `num = Network.GetNumSessions()`
 - Ret: `integer num` Number of network sessions found
---
### GetSession
Get info on a discovered network session. Network sessions will only be discovered once Network.BeginSessionSearch() has been invoked.

Sig: `sessionData = Network.GetSession(index)`
 - Arg: `integer index` Index of the session
 - Ret: `table sessionData`
   - `string ipAddress`
   - `integer port`
   - `string lobbyId`
   - `string name`
   - `integer maxPlayers`
   - `integer numPlayers`
---
### GetSessions
Get an array of discovered network sessions. Network sessions will only be discovered once Network.BeginSessionSearch() has been invoked.

Sig: `sessionArray = Network.GetSessions()`
 - Ret: `table sessionArray` Array
   - `table sessionData` Element of array
     - `string ipAddress`
     - `integer port`
     - `string lobbyId`
     - `string name`
     - `integer maxPlayers`
     - `integer numPlayers`
---
### Connect
Connect to a game session by IP and port.
Note: You should probably use Network.JoinSession() instead.

Sig: `Network.Connect(ipAddress, port=DefaultPort)`
 - Arg: `string ipAddress` IP Address
 - Arg: `integer port` Port (if not provided, the default port will be used)
---
### Disconnect
Disconnect from a network session.
Note: This is the same as calling Network.CloseSession().

Sig: `Network.Disconnect()`

---
### Kick
Kick a player from the network session. Only callable by the server host.

Sig: `Network.Kick(hostId)`
 - Arg: `integer hostId` Host to kick
---
### GetMaxClients
Get the maximum number of clients. Only call as the server host. Otherwise, it will not provide an accurate number.

Sig: `max = Network.GetMaxClients()`
 - Ret: `integer max` Max number of clients
---
### GetNumClients
Get the number of connected clients. Only call as the server host.

Sig: `num = Network.GetNumClients()`
 - Ret: `integer num` Number of connected clients
---
### GetClients
Get the list of connected clients. Only call as the server host.

Sig: `clients = Network.GetClients()`
 - Ret: `table clients` Client array
   - `table hostProfile` Element of array
     - `string ipAddress`
     - `integer port`
     - `integer hostId`
     - `string onlineId` (e.g. SteamID)
     - `number ping`
     - `boolean ready`
---
### FindNetClient
Find client data for a given host ID.

Sig: `client = Network.FindNetClient(hostId)`
 - Arg: `integer hostId` Host ID
 - Ret: `table client`
   - `string ipAddress`
   - `integer port`
   - `integer hostId`
   - `string onlineId` (e.g. SteamID)
   - `number ping`
   - `boolean ready`
---
### GetNetStatus
Get current network status.
Possible values are:
 - "Client"
 - "Server"
 - "Connecting"
 - "Local"

Sig: `status = Network.GetNetStatus()`
 - Ret: `string status` Network status
---
### EnableIncrementalReplication
Set whether incremental replication should be used.

Sig: `Network.EnableIncrementalReplication(enable)`
 - Arg: `boolean enable` Enable incremental replication
---
### IsIncrementalReplicationEnabled
Check whether incremental replication is enabled.

Sig: `enabled = Network.IsIncrementalReplicationEnabled()`
 - Ret: `boolean enabled` Incremental replication enabled
---
### GetBytesSent
Get the number of bytes sent over the network this frame.

Sig: `num = Network.GetBytesSent()`
 - Ret: `integer num` Number of bytes sent
---
### GetBytesReceived
Get the number of bytes received over the network this frame.

Sig: `num = Network.GetBytesReceived()`
 - Ret: `integer num` Number of bytes received
---
### GetUploadRate
Get the average network upload rate.

Sig: `rate = Network.GetUploadRate()`
 - Ret: `number rate` Upload rate
---
### GetDownloadRate
Get the average network download rate.

Sig: `rate = Network.GetDownloadRate()`
 - Ret: `number rate` Download rate
---
### IsServer
Check if this host is the server.

Sig: `server = Network.IsServer()`
 - Ret: `boolean server` Is server
---
### IsClient
Check if this host is a client.

Sig: `client = Network.IsClient()`
 - Ret: `boolean client` Is client
---
### IsLocal
Check if this host is playing locally (not in a network session).

Sig: `local = Network.IsLocal()`
 - Ret: `boolean local` Is local
---
### IsAuthority
Check if this host has gameplay authority. This will be true if this host is a Server or is playing locally.

Sig: `authority = Network.IsAuthority()`
 - Ret: `boolean authority` Is the gameplay authority
---
### GetHostId
Get this host's HostID.

Sig: `hostId = Network.GetHostId()`
 - Ret: `integer hostId` Host ID
---
### SetConnectCallback
Set a callback function that will be called when a Connect message is received.

Sig: `Network.SetConnectCallback(func)`
 - Arg: `function func` Callback function
---
### SetAcceptCallback
Set a callback function that will be called when an Accept message is received.

Sig: `Network.SetAcceptCallback(func)`
 - Arg: `function func` Callback function
---
### SetRejectCallback
Set a callback function that will be called when a Reject message is received.

Sig: `Network.SetRejectCallback(func)`
 - Arg: `function func` Callback function
---
### SetDisconnectCallback
Set a callback function that will be called when a Disconnect message is received.

Sig: `Network.SetDisconnectCallback(func)`
 - Arg: `function func` Callback function
---
### SetKickCallback
Set a callback function that will be called when a Kick message is received.

Sig: `Network.SetKickCallback(func)`
 - Arg: `function func` Callback function
---
