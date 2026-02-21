--- @meta

---@class NetworkModule
Network = {}

---@param arg1 table
function Network.OpenSession(arg1) end

function Network.CloseSession() end

---@param arg1 table
function Network.JoinSession(arg1) end

function Network.BeginSessionSearch() end

function Network.EndSessionSearch() end

---@return boolean
function Network.IsSearching() end

---@return integer
function Network.GetNumSessions() end

---@param arg1 integer
---@return nil
function Network.GetSession(arg1) end

---@return any
function Network.GetSessions() end

---@param ipAddr string
---@param arg2? integer
function Network.Connect(ipAddr, arg2) end

function Network.Disconnect() end

---@param hostId integer
function Network.Kick(hostId) end

---@return integer
function Network.GetMaxClients() end

---@return integer
function Network.GetNumClients() end

---@return any
function Network.GetClients() end

---@param arg1 integer
---@return nil
function Network.FindNetClient(arg1) end

---@return string
function Network.GetNetStatus() end

---@param value boolean
function Network.EnableIncrementalReplication(value) end

---@param value boolean
function Network.EnableReliableReplication(value) end

---@return integer
function Network.GetBytesSent() end

---@return integer
function Network.GetBytesReceived() end

---@return number
function Network.GetUploadRate() end

---@return number
function Network.GetDownloadRate() end

---@return boolean
function Network.IsServer() end

---@return boolean
function Network.IsClient() end

---@return boolean
function Network.IsLocal() end

---@return boolean
function Network.IsAuthority() end

---@return integer
function Network.GetHostId() end

---@param dist number
function Network.SetRelevancyDistance(dist) end

---@param arg1 integer
---@param arg2 Node
function Network.SetPawn(arg1, arg2) end

---@param arg1 integer
---@return Node
function Network.GetPawn(arg1) end

---@param value boolean
function Network.EnableNetRelevancy(value) end

---@param arg1 function
function Network.SetConnectCallback(arg1) end

---@param arg1 function
function Network.SetAcceptCallback(arg1) end

---@param arg1 function
function Network.SetRejectCallback(arg1) end

---@param arg1 function
function Network.SetDisconnectCallback(arg1) end

---@param arg1 function
function Network.SetKickCallback(arg1) end
