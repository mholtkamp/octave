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

---@param index integer
---@return nil
function Network.GetSession(index) end

---@return any
function Network.GetSessions() end

---@param ipAddr string
---@param port? integer
function Network.Connect(ipAddr, port) end

function Network.Disconnect() end

---@param hostId integer
function Network.Kick(hostId) end

---@return integer
function Network.GetMaxClients() end

---@return integer
function Network.GetNumClients() end

---@return any
function Network.GetClients() end

---@param id integer
---@return nil
function Network.FindNetClient(id) end

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

---@param hostId integer
---@param arg2 Node
function Network.SetPawn(hostId, arg2) end

---@param hostId integer
---@return Node
function Network.GetPawn(hostId) end

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
