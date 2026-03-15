--- @meta

---@class Node
Node = {}

---@return Node
function Node:Construct() end

function Node:Destruct() end

function Node:DestroyDeferred() end

---@return boolean
function Node:IsValid() end

---@return string
function Node:GetName() end

---@param name string
function Node:SetName(name) end

---@param active boolean
function Node:SetActive(active) end

---@param recurse? boolean
---@return boolean
function Node:IsActive(recurse) end

---@param visible boolean
function Node:SetVisible(visible) end

---@param recurse? boolean
---@return boolean
function Node:IsVisible(recurse) end

---@param value boolean
function Node:SetPersistent(value) end

---@return boolean
function Node:IsPersistent() end

---@return World
function Node:GetWorld() end

---@return Node
function Node:GetParent() end

---@param newParent? Node
---@param keepWorldTransform? boolean
---@param childIndex? integer
function Node:Attach(newParent, keepWorldTransform, childIndex) end

---@param keepWorldTransform? boolean
function Node:Detach(keepWorldTransform) end

---@param index integer
---@return Node
function Node:GetChild(index) end

---@param typeName string
---@return Node
function Node:GetChildByType(typeName) end

---@return integer
function Node:GetNumChildren() end

---@param newChild Node
---@param index? integer
function Node:AddChild(newChild, index) end

---@param childName string
function Node:RemoveChild(childName) end

---@param name string
---@param recurse? boolean
---@return Node
function Node:FindChild(name, recurse) end

---@param tag string
---@param recurse? boolean
---@return Node
function Node:FindChildWithTag(tag, recurse) end

---@param typeName string
---@param recurse? boolean
---@return Node
function Node:FindChildOfType(typeName, recurse) end

---@param name string
---@return Node
function Node:FindDescendant(name) end

---@param name string
---@return Node
function Node:FindAncestor(name) end

---@param otherNode Node
---@return boolean
function Node:HasAncestor(otherNode) end

---@return Node
function Node:GetSubRoot() end

---@return Node
function Node:GetRoot() end

---@return boolean
function Node:IsWorldRoot() end

---@param arg1 function
---@param inverted? boolean
function Node:Traverse(arg1, inverted) end

---@param arg1 function
---@param inverted? boolean
function Node:ForEach(arg1, inverted) end

---@param nodeClass string
---@return Node
function Node:CreateChild(nodeClass) end

---@param srcNode Node
---@param recurse boolean
---@return Node
function Node:CreateChildClone(srcNode, recurse) end

---@param recurse boolean
---@return Node
function Node:Clone(recurse) end

---@param childName string
function Node:DestroyChild(childName) end

function Node:DestroyAllChildren() end

function Node:Start() end

---@return boolean
function Node:HasStarted() end

---@return integer
function Node:GetNodeId() end

---@param signalName string
function Node:EmitSignal(signalName) end

---@param signalName string
---@param listener Node
---@param arg3 function
function Node:ConnectSignal(signalName, listener, arg3) end

---@param signalName string
---@param listener Node
function Node:DisconnectSignal(signalName, listener) end

---@return boolean
function Node:IsDestroyed() end

---@return boolean
function Node:IsPendingDestroy() end

---@param enable boolean
function Node:EnableTick(enable) end

---@return boolean
function Node:IsTickEnabled() end

---@return Asset
function Node:GetScene() end

---@param value string
function Node:SetScript(value) end

---@return integer
function Node:GetNetId() end

---@return integer
function Node:GetOwningHost() end

---@param netHostId integer
---@param setPawn? boolean
function Node:SetOwningHost(netHostId, setPawn) end

---@param replicate boolean
function Node:SetReplicate(replicate) end

---@return boolean
function Node:IsReplicated() end

---@param value boolean
function Node:SetReplicateTransform(value) end

---@return boolean
function Node:IsTransformReplicated() end

function Node:ForceReplication() end

---@param tag string
---@return boolean
function Node:HasTag(tag) end

---@param tag string
function Node:AddTag(tag) end

---@param tag string
function Node:RemoveTag(tag) end

---@return boolean
function Node:HasAuthority() end

---@return boolean
function Node:IsOwned() end

---@return boolean
function Node:IsLateTickEnabled() end

---@param value boolean
function Node:EnableLateTick(value) end

---@return boolean
function Node:IsAlwaysRelevant() end

---@param value boolean
function Node:SetAlwaysRelevant(value) end

---@param funcName string
function Node:InvokeNetFunc(funcName) end

---@param typeName string
function Node:CheckType(typeName) end

---@return Node
function Node:New() end

function Node:Doom() end

function Node:SetPendingDestroy() end

---@return boolean
function Node:IsDoomed() end

---@param value string
function Node:SetScriptFile(value) end

---@return boolean
function Node:IsLocallyControlled() end

---@param typeName string
function Node:Is(typeName) end

---@param typeName string
function Node:IsA(typeName) end
